// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4LevelModel.h"

#include "GameFramework/Actor.h"
#include "Misc/MessageDialog.h"
#include "HAL/FileManager.h"
#include "Modules/ModuleManager.h"
#include "Misc/PackageName.h"
#include "Engine/Brush.h"
#include "GameFramework/WorldSettings.h"
#include "Engine/Selection.h"
#include "Editor.h"
#include "ScopedTransaction.h"
#include "LevelUtils.h"
#include "EditorLevelUtils.h"
#include "ActorEditorUtils.h"

#include "Engine/LevelStreaming.h"

#include "Engine/LevelScriptBlueprint.h"
#include "Toolkits/AssetEditorManager.h"
#include "T4LevelCollectionModel.h"
#include "AssetRegistryModule.h"

#define LOCTEXT_NAMESPACE "WorldMap"

FT4LevelModel::FT4LevelModel(FT4LevelCollectionModel& InLevelCollectionModel)
	: LevelCollectionModel(InLevelCollectionModel)
	, bSelected(false)
	, bExpanded(false)
	, bLoadingLevel(false)
	, bFilteredOut(false)
	, LevelTranslationDelta(0,0)
	, LevelActorsCount(0)
	, bEditorSelected(false) // #90
{
	SimulationStatus = FSimulationLevelStatus();

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetRegistryModule.Get().OnAssetRenamed().AddRaw(this, &FT4LevelModel::OnAssetRenamed);
}

FT4LevelModel::~FT4LevelModel()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetRegistryModule.Get().OnAssetRenamed().RemoveAll(this);
}

void FT4LevelModel::OnAssetRenamed(const FAssetData& AssetData, const FString& OldObjectPath)
{
	const FString CurrentPackage = GetLongPackageName().ToString();

	if (FPackageName::ObjectPathToPackageName(OldObjectPath) == CurrentPackage)
	{
		UpdateAsset(AssetData);
		UpdateDisplayName();
	}
}

void FT4LevelModel::SetLevelSelectionFlag(bool bSelectedFlag)
{
	bSelected = bSelectedFlag;
}

bool FT4LevelModel::GetLevelSelectionFlag() const
{
	return bSelected;
}

void FT4LevelModel::SetLevelExpansionFlag(bool bExpandedFlag)
{
	bExpanded = bExpandedFlag;
}

bool FT4LevelModel::GetLevelExpansionFlag() const
{
	return bExpanded;
}

void FT4LevelModel::SetLevelFilteredOutFlag(bool bFiltredOutFlag)
{
	bFilteredOut = bFiltredOutFlag;
}
	
bool FT4LevelModel::GetLevelFilteredOutFlag() const
{
	return bFilteredOut;
}

FString FT4LevelModel::GetDisplayName() const
{
	return DisplayName;
}

FString FT4LevelModel::GetPackageFileName() const
{
	const FName LocalPackageName = GetLongPackageName();
	if (LocalPackageName != NAME_None)
	{
		return FPackageName::LongPackageNameToFilename(LocalPackageName.ToString(), FPackageName::GetMapPackageExtension());
	}
	else
	{
		return FString();
	}
}

void FT4LevelModel::Accept(FT4LevelModelVisitor& Vistor)
{
	Vistor.Visit(*this);
	
	for (auto It = AllChildren.CreateIterator(); It; ++It)
	{
		(*It)->Accept(Vistor);
	}
}

void FT4LevelModel::Update()
{
	UpdateLevelActorsCount();
	BroadcastChangedEvent();
}
	
void FT4LevelModel::UpdateVisuals()
{
	BroadcastChangedEvent();
}

bool FT4LevelModel::IsSimulating() const
{
	return LevelCollectionModel.IsSimulating();
}

bool FT4LevelModel::IsCurrent() const
{
	if (GetLevelObject())
	{
		return GetLevelObject()->IsCurrentLevel();
	}

	return false;
}

bool FT4LevelModel::IsPersistent() const
{
	if (nullptr == LevelCollectionModel.GetWorld()->WorldComposition)
	{
		return false; // #91 : World Single
	}
	return LevelCollectionModel.GetWorld()->PersistentLevel == GetLevelObject();
}

bool FT4LevelModel::IsEditable() const
{
	return (IsLoaded() == true && IsLocked() == false);
}

bool FT4LevelModel::IsDirty() const
{
	if (GetLevelObject())
	{
		return GetLevelObject()->GetOutermost()->IsDirty();
	}
	
	return false;
}

bool FT4LevelModel::IsLightingScenario() const
{
	if (GetLevelObject())
	{
		return GetLevelObject()->bIsLightingScenario;
	}
	
	return false;
}

void FT4LevelModel::SetIsLightingScenario(bool bNew)
{
	if (GetLevelObject())
	{
		GetLevelObject()->SetLightingScenario(bNew);
	}
}

bool FT4LevelModel::IsLoaded() const
{
	return (LevelCollectionModel.IsSimulating() ? SimulationStatus.bLoaded : (GetLevelObject() != NULL));
}

bool FT4LevelModel::IsLoading() const
{
	return (LevelCollectionModel.IsSimulating() ? SimulationStatus.bLoading : bLoadingLevel);
}

bool FT4LevelModel::IsVisible() const
{
	if (LevelCollectionModel.IsSimulating())
	{
		return SimulationStatus.bVisible;
	}
	else
	{
		ULevel* Level = GetLevelObject();
		if (Level)
		{
			if (ULevelStreaming* StreamingLevel = FLevelUtils::FindStreamingLevel(Level))
			{
				return StreamingLevel->ShouldBeVisible();
			}
			else
			{
				return FLevelUtils::IsLevelVisible(Level);
			}
		}
		return false;
	}
}

bool FT4LevelModel::IsLocked() const
{
	ULevel* Level = GetLevelObject();
	if (Level)
	{
		return FLevelUtils::IsLevelLocked(Level);
	}

	return false;	
}

bool FT4LevelModel::IsFileReadOnly() const
{
	if (HasValidPackage())
	{
		FName PackageName = GetLongPackageName();
		
		FString PackageFileName;
		if (FPackageName::DoesPackageExist(PackageName.ToString(), NULL, &PackageFileName))
		{
			return IFileManager::Get().IsReadOnly(*PackageFileName);
		}
	}

	return false;
}

void FT4LevelModel::LoadLevel()
{

}

void FT4LevelModel::SetVisible(bool bVisible)
{
	//don't create unnecessary transactions
	if (IsVisible() == bVisible)
	{
		return;
	}

	const bool oldIsDirty = IsDirty();

	const FScopedTransaction Transaction(LOCTEXT("ToggleVisibility", "Toggle Level Visibility"));

	//this call hides all owned actors, etc
	EditorLevelUtils::SetLevelVisibility( GetLevelObject(), bVisible, false );

	if (!oldIsDirty)
	{
		// don't set the dirty flag if we're just changing the visibility of the level within the editor
		ULevel* Level = GetLevelObject();
		if (Level)
		{
			Level->GetOutermost()->SetDirtyFlag(false);
		}
	}
}

void FT4LevelModel::SetLocked(bool bLocked)
{
	if (LevelCollectionModel.IsReadOnly())
	{
		return;
	}
		
	ULevel* Level = GetLevelObject();

	if (Level == NULL)
	{
		return;
	}

	// Do nothing if attempting to set the level to the same locked state
	if (bLocked == IsLocked())
	{
		return;
	}

	// If locking the level, deselect all of its actors and BSP surfaces
	if (bLocked)
	{
		DeselectAllActors();
		DeselectAllSurfaces();

		// Tell the editor selection status was changed.
		GEditor->NoteSelectionChange();

		// If locking the current level, reset the p-level as the current level
		//@todo: fix this!
	}

	// Change the level's locked status
	FLevelUtils::ToggleLevelLock(Level);
}

void FT4LevelModel::MakeLevelCurrent()
{
	if (LevelCollectionModel.IsReadOnly())
	{
		return;
	}

	if (!IsLoaded())
	{
		// Load level from disk
		FT4LevelModelList LevelsList; LevelsList.Add(this->AsShared());
		LevelCollectionModel.LoadLevels(LevelsList);
	}

	ULevel* Level = GetLevelObject();
	if (Level == NULL)
	{
		return;
	}
		
	// Locked levels can't be made current.
	if (!FLevelUtils::IsLevelLocked(Level))
	{ 
		// Make current.
		if (LevelCollectionModel.GetWorld()->SetCurrentLevel(Level))
		{
			FEditorDelegates::NewCurrentLevel.Broadcast();
				
			// Deselect all selected builder brushes.
			bool bDeselectedSomething = false;
			for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
			{
				AActor* Actor = static_cast<AActor*>(*It);
				checkSlow(Actor->IsA(AActor::StaticClass()));

				ABrush* Brush = Cast< ABrush >( Actor );
				if (Brush && FActorEditorUtils::IsABuilderBrush(Brush))
				{
					GEditor->SelectActor(Actor, /*bInSelected=*/ false, /*bNotify=*/ false);
					bDeselectedSomething = true;
				}
			}

			// Send a selection change callback if necessary.
			if (bDeselectedSomething)
			{
				GEditor->NoteSelectionChange();
			}
		}
							
		// Force the current level to be visible.
		SetVisible(true);
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("UnrealEd", "Error_OperationDisallowedOnLockedLevelMakeLevelCurrent", "MakeLevelCurrent: The requested operation could not be completed because the level is locked."));
	}

	Update();
}

bool FT4LevelModel::HitTest2D(const FVector2D& Point) const
{
	return false;
}
	
FVector2D FT4LevelModel::GetLevelPosition2D() const
{
	return FVector2D::ZeroVector;
}

FVector2D FT4LevelModel::GetLevelSize2D() const
{
	return FVector2D::ZeroVector;
}

FBox FT4LevelModel::GetLevelBounds() const
{
	return FBox(ForceInit);
}

FVector2D FT4LevelModel::GetLevelTranslationDelta() const
{
	return LevelTranslationDelta;
}

void FT4LevelModel::SetLevelTranslationDelta(FVector2D InAbsoluteDelta)
{
	LevelTranslationDelta = InAbsoluteDelta;
	
	for (auto It = AllChildren.CreateIterator(); It; ++It)
	{
		(*It)->SetLevelTranslationDelta(InAbsoluteDelta);
	}
}

FLinearColor FT4LevelModel::GetLevelColor() const
{
	// Returns Constant color, base classes will override this
	// Currently not all base classes have the requisite support, so I've not made it pure virtual.
	return FLinearColor::White;
}

void FT4LevelModel::SetLevelColor(FLinearColor InColor)
{
	// Does nothing, base classes will override this
	// Currently not all base classes have the requisite support, so I've not made it pure virtual.
}

bool FT4LevelModel::IsVisibleInCompositionView() const
{
	return false;
}

bool FT4LevelModel::HasKismet() const
{
	return (GetLevelObject() != NULL);
}

void FT4LevelModel::OpenKismet()
{
	if (LevelCollectionModel.IsReadOnly())
	{
		return;
	}
	
	ULevel* Level = GetLevelObject();

	if (Level == NULL)
	{
		return;
	}
	
	ULevelScriptBlueprint* LevelScriptBlueprint = Level->GetLevelScriptBlueprint();
	if (LevelScriptBlueprint)
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(LevelScriptBlueprint);
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, NSLOCTEXT("UnrealEd", "UnableToCreateLevelScript", "Unable to find or create a level blueprint for this level.") );
	}
}

bool FT4LevelModel::AttachTo(TSharedPtr<FT4LevelModel> InParent)
{
	if (LevelCollectionModel.IsReadOnly() || 
		!IsLoaded() || 
		IsPersistent() ||
		InParent.IsValid() == false ||
		InParent.Get() == this ||
		HasDescendant(InParent))
	{
		return false;
	}

	TSharedPtr<FT4LevelModel> CurrentParent = GetParent();
	if (CurrentParent.IsValid())
	{
		CurrentParent->RemoveChild(this->AsShared());
	}

	Parent = InParent;

	CurrentParent = GetParent();
	if (CurrentParent.IsValid())
	{
		CurrentParent->AddChild(this->AsShared());
	}

	OnParentChanged();
	return true;
}

void FT4LevelModel::OnFilterChanged()
{
	FilteredChildren.Empty();
	
	for (const auto& LevelModel : AllChildren)
	{
		LevelModel->OnFilterChanged();

		// Item will pass filtering regardless of filter settings if it has children that passes filtering
		if (LevelModel->GetChildren().Num() > 0 || LevelCollectionModel.PassesAllFilters(*LevelModel))
		{
			FilteredChildren.Add(LevelModel);
		}
	}
}

const FT4LevelModelList& FT4LevelModel::GetChildren() const
{
	return FilteredChildren;
}
	
TSharedPtr<FT4LevelModel> FT4LevelModel::GetParent() const
{
	return Parent.Pin();
}

void FT4LevelModel::SetParent(TSharedPtr<FT4LevelModel> InParent)
{
	Parent = InParent;
}

void FT4LevelModel::RemoveAllChildren()
{
	FilteredChildren.Empty();
	AllChildren.Empty();
}

void FT4LevelModel::RemoveChild(TSharedPtr<FT4LevelModel> InChild)
{
	FilteredChildren.Remove(InChild);
	AllChildren.Remove(InChild);
}
	
void FT4LevelModel::AddChild(TSharedPtr<FT4LevelModel> InChild)
{
	AllChildren.AddUnique(InChild);

	if (LevelCollectionModel.PassesAllFilters(*InChild))
	{
		FilteredChildren.Add(InChild);
	}
}

bool FT4LevelModel::HasAncestor(const TSharedPtr<FT4LevelModel>& InLevel) const
{
	TSharedPtr<FT4LevelModel> ParentModel = GetParent();
	while (ParentModel.IsValid())
	{
		if (ParentModel == InLevel)
		{
			return true;
		}
		
		ParentModel = ParentModel->GetParent();
	}
	
	return false;
}

bool FT4LevelModel::HasDescendant(const TSharedPtr<FT4LevelModel>& InLevel) const
{
	if (AllChildren.Find(InLevel) != INDEX_NONE)
	{
		return true;
	}
	
	for (auto It = AllChildren.CreateConstIterator(); It; ++It)
	{
		if ((*It)->HasDescendant(InLevel))
		{
			return true;
		}
	}
	
	return false;
}

void FT4LevelModel::OnDrop(const TSharedPtr<FLevelDragDropOp>& Op)
{
}
	
bool FT4LevelModel::IsGoodToDrop(const TSharedPtr<FLevelDragDropOp>& Op) const
{
	return false;
}

void FT4LevelModel::OnLevelAddedToWorld(ULevel* InLevel)
{
	UpdateLevelActorsCount();
}

void FT4LevelModel::OnLevelRemovedFromWorld()
{
	UpdateLevelActorsCount();
}

void FT4LevelModel::BroadcastChangedEvent()
{
	ChangedEvent.Broadcast();
}

void FT4LevelModel::UpdateSimulationStatus(ULevelStreaming* StreamingLevel)
{
	SimulationStatus = FSimulationLevelStatus();
	
	// Persistent level always loaded and visible in PIE
	if (IsPersistent())
	{
		SimulationStatus.bLoaded = true;
		SimulationStatus.bVisible = true;
		return;
	}

	if (StreamingLevel == nullptr)
	{
		return;
	}
		
	if (StreamingLevel->GetLoadedLevel())
	{
		SimulationStatus.bLoaded = true;
				
		if (StreamingLevel->GetLoadedLevel()->bIsVisible)
		{
			SimulationStatus.bVisible = true;
		}
	}
	else if (StreamingLevel->HasLoadRequestPending())
	{
		SimulationStatus.bLoading = true;
	}
}

void FT4LevelModel::DeselectAllSurfaces()
{
	ULevel* Level = GetLevelObject();

	if (Level == NULL)
	{
		return;
	}

	UModel* Model = Level->Model;
	for (int32 SurfaceIndex = 0; SurfaceIndex < Model->Surfs.Num(); ++SurfaceIndex)
	{
		FBspSurf& Surf = Model->Surfs[SurfaceIndex];
		if (Surf.PolyFlags & PF_Selected)
		{
			Model->ModifySurf(SurfaceIndex, false);
			Surf.PolyFlags&= ~PF_Selected;
		}
	}
}

void FT4LevelModel::DeselectAllActors()
{
	ULevel* Level = GetLevelObject();

	if (Level == NULL)
	{
		return;
	}

	USelection* SelectedActors = GEditor->GetSelectedActors();
	SelectedActors->Modify();

	// Deselect all level actors 
	for (auto It = Level->Actors.CreateIterator(); It; ++It)
	{
		AActor* CurActor = (*It);
		if (CurActor)
		{
			SelectedActors->Deselect(CurActor);
		}
	}
}

void FT4LevelModel::SelectActors(bool bSelect, bool bNotify, bool bSelectEvenIfHidden,
							   const TSharedPtr<ActorFilter>& Filter)
{
	if (LevelCollectionModel.IsReadOnly())
	{
		return;
	}
	
	ULevel* Level = GetLevelObject();

	if (Level == NULL || IsLocked())
	{
		return;
	}

	GEditor->GetSelectedActors()->BeginBatchSelectOperation();
	bool bChangesOccurred = false;

	// Iterate over all actors, looking for actors in this level.
	for (auto It = Level->Actors.CreateIterator(); It; ++It)
	{
		AActor* Actor = (*It);
		if (Actor)
		{
			if (Filter.IsValid() && !Filter->PassesFilter(Actor))
			{
				continue;
			}
			
			//exclude the world settings and builder brush from actors selected
			const bool bIsWorldSettings = Actor->IsA(AWorldSettings::StaticClass());
			const bool bIsBuilderBrush = (Actor->IsA(ABrush::StaticClass()) && FActorEditorUtils::IsABuilderBrush(Actor));
			if (bIsWorldSettings || bIsBuilderBrush)
			{
				continue;
			}

			bool bNotifyForActor = false;
			GEditor->GetSelectedActors()->Modify();
			GEditor->SelectActor(Actor, bSelect, bNotifyForActor, bSelectEvenIfHidden);
			bChangesOccurred = true;
		}
	}

	GEditor->GetSelectedActors()->EndBatchSelectOperation();

	if (bNotify)
	{
		GEditor->NoteSelectionChange();
	}
}

void FT4LevelModel::UpdateLevelActorsCount()
{
	LevelActorsCount = 0;
	ULevel* Level = GetLevelObject();
	
	if (Level)
	{
		// Count the actors contained in these levels
		// NOTE: We subtract two here to omit "default actors" in the count (default brush, and WorldSettings)
		LevelActorsCount = Level->Actors.Num()-2;

		// Count deleted actors
		int32 NumDeletedActors = 0;
		for (int32 ActorIdx = 0; ActorIdx < Level->Actors.Num(); ++ActorIdx)
		{
			if (!Level->Actors[ActorIdx])
			{
				++NumDeletedActors;
			}
		}
		
		// Subtract deleted actors from the actor count
		LevelActorsCount -= NumDeletedActors;
	}

	UpdateDisplayName();
}

void FT4LevelModel::UpdateDisplayName()
{
	if (IsPersistent())
	{
		DisplayName = LOCTEXT("PersistentTag", "Persistent Level").ToString();
	}
	else
	{
		DisplayName = GetLongPackageName().ToString();
		if (!LevelCollectionModel.GetDisplayPathsState())
		{
			DisplayName = FPackageName::GetShortName(DisplayName);
		}
	}

	if (HasValidPackage())
	{
		// Append actors count
		if (LevelCollectionModel.GetDisplayActorsCountState() && IsLoaded())
		{
			DisplayName += TEXT(" (");
			DisplayName.AppendInt(LevelActorsCount);
			DisplayName += TEXT(")");
		}
	}
	else
	{
		DisplayName+= LOCTEXT("MissingLevelErrorText", " [Missing Level] ").ToString();
	}
}

FString FT4LevelModel::GetLightmassSizeString() const
{
	FString MemorySizeString;
	ULevel* Level = GetLevelObject();

	//if (Level && GetDefault<ULevelBrowserSettings>()->bDisplayLightmassSize)
	//{
	//	// Update metrics
	//	static const float ByteConversion = 1.0f / 1024.0f;
	//	float LightmapSize = Level->LightmapTotalSize * ByteConversion;
	//	
	//	MemorySizeString += FString::Printf(TEXT( "%.2f" ), LightmapSize);
	//}

	return MemorySizeString;
}

FString FT4LevelModel::GetFileSizeString() const
{
	FString MemorySizeString;
	ULevel* Level = GetLevelObject();

	//if (Level && GetDefault<ULevelBrowserSettings>()->bDisplayFileSize)
	//{
	//	// Update metrics
	//	static const float ByteConversion = 1.0f / 1024.0f;
	//	float FileSize = Level->GetOutermost()->GetFileSize() * ByteConversion * ByteConversion;
	//	
	//	MemorySizeString += FString::Printf(TEXT( "%.2f" ), FileSize);
	//}

	return MemorySizeString;
}

UClass* FT4LevelModel::GetStreamingClass() const
{
	return nullptr;
}

bool FT4LevelModel::IsWorldCompositionEnabled() const // #91
{
	return (nullptr != LevelCollectionModel.GetWorld()->WorldComposition) ? true : false;
}

UWorld* FT4LevelModel::GetEditorWorld() const // #91
{
	return LevelCollectionModel.GetWorld();
}

UWorld* FT4LevelModel::GetSimulationWorld() const // #93
{
	return LevelCollectionModel.GetSimulationWorld();
}

ULevel* FT4LevelModel::GetThumbnailLoadedLevel() const // #84
{
	const FName CurrentPackageName = GetLongPackageName();
	UWorld* InPreviewWorld = LevelCollectionModel.GetSimulationWorld();
	check(nullptr != InPreviewWorld);
	bool bWorldCompositionEnabled = (nullptr != InPreviewWorld->WorldComposition) ? true : false;
	if (!bWorldCompositionEnabled)
	{
		return InPreviewWorld->PersistentLevel; // #91 : World Single
	}
	for (ULevelStreaming* StreamingLevel : InPreviewWorld->GetStreamingLevels())
	{
		if (!StreamingLevel->HasLoadedLevel())
		{
			continue;
		}
		if (CurrentPackageName == StreamingLevel->PackageNameToLoad)
		{
			return StreamingLevel->GetLoadedLevel();
		}
	}
	return nullptr;
}

UT4MapEntityAsset* FT4LevelModel::GetThumbnailTargetAsset() const // #84
{
	return LevelCollectionModel.GetThumbnailTargetAsset();
}

bool FT4LevelModel::GetPreviewLevelSelectionFlag() const // #104
{
	const FName CurrentPackageName = GetLongPackageName();
	if (LevelCollectionModel.GetPreviewLoadedLevelLists().Contains(CurrentPackageName))
	{
		return true;
	}
	return false;
}

#undef LOCTEXT_NAMESPACE
