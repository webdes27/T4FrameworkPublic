// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4LevelCollectionModel.h"

#include "Misc/PackageName.h"
#include "AssetData.h"
#include "Misc/MessageDialog.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FeedbackContext.h"
#include "Modules/ModuleManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ISourceControlOperation.h"
#include "SourceControlOperations.h"
#include "ISourceControlModule.h"
#include "SourceControlHelpers.h"
#include "Settings/EditorLoadingSavingSettings.h"
#include "Engine/Selection.h"
#include "EditorModeManager.h"
#include "EditorModes.h"
#include "FileHelpers.h"
#include "EditorModeInterpolation.h"
#include "ScopedTransaction.h"
#include "EditorLevelUtils.h"
#include "T4LevelCollectionCommands.h"
#include "SourceControlWindows.h"
#include "IAssetTools.h"
#include "IAssetTypeActions.h"
#include "AssetToolsModule.h"
#include "EditorSupportDelegates.h"
#include "Matinee/MatineeActor.h"
#include "GameFramework/WorldSettings.h"

#include "WorldBrowser/Private/LevelModel.h" // #90
#include "WorldBrowser/Private/LevelCollectionModel.h" // #90
#include "WorldBrowserModule.h" // #90
#include "Modules/ModuleManager.h" // #90

#include "ShaderCompiler.h"
#include "FoliageEditModule.h"
#include "InstancedFoliageActor.h"
#include "FoliageEditUtility.h"
#include "LevelUtils.h"

#define LOCTEXT_NAMESPACE "WorldMap"

FT4LevelCollectionModel::FT4LevelCollectionModel()
	: bRequestedUpdateAllLevels(false)
	, bRequestedRedrawAllLevels(false)
	, bRequestedUpdateActorsCount(false)
	, CommandList(MakeShareable(new FUICommandList))
	, Filters(MakeShareable(new T4LevelFilterCollection))
	, WorldSize(FIntPoint::ZeroValue)
	, bDisplayPaths(false)
	, bCanExecuteSCCCheckOut(false)
	, bCanExecuteSCCOpenForAdd(false)
	, bCanExecuteSCCCheckIn(false)
	, bCanExecuteSCC(false)
	, bSelectionHasChanged(true)
	, bUpdatingLevelsSelection(false)
{
}

FT4LevelCollectionModel::~FT4LevelCollectionModel()
{
	SaveSettings();
	
	Filters->OnChanged().RemoveAll(this);
	FWorldDelegates::LevelAddedToWorld.RemoveAll(this);
	FWorldDelegates::LevelRemovedFromWorld.RemoveAll(this);
	FEditorSupportDelegates::RedrawAllViewports.RemoveAll(this);
	GEditor->OnLevelActorAdded().RemoveAll(this);
	GEditor->OnLevelActorDeleted().RemoveAll(this);
	if (CurrentWorld.IsValid())
	{
		CurrentWorld->OnSelectedLevelsChanged().RemoveAll(this);
	}
}

void FT4LevelCollectionModel::Initialize(UWorld* InWorld)
{
	LoadSettings();
	
	CurrentWorld = InWorld;

	Filters->OnChanged().AddSP(this, &FT4LevelCollectionModel::OnFilterChanged);
	FWorldDelegates::LevelAddedToWorld.AddSP(this, &FT4LevelCollectionModel::OnLevelAddedToWorld);
	FWorldDelegates::LevelRemovedFromWorld.AddSP(this, &FT4LevelCollectionModel::OnLevelRemovedFromWorld);
	FEditorSupportDelegates::RedrawAllViewports.AddSP(this, &FT4LevelCollectionModel::OnRedrawAllViewports);
	GEditor->OnLevelActorAdded().AddSP( this, &FT4LevelCollectionModel::OnLevelActorAdded);
	GEditor->OnLevelActorDeleted().AddSP( this, &FT4LevelCollectionModel::OnLevelActorDeleted);
	USelection::SelectionChangedEvent.AddSP(this, &FT4LevelCollectionModel::OnActorSelectionChanged);
	SelectionChanged.AddSP(this, &FT4LevelCollectionModel::OnActorOrLevelSelectionChanged);
	CurrentWorld->OnSelectedLevelsChanged().AddSP(this, &FT4LevelCollectionModel::OnLevelsSelectionChangedOutside);

	PopulateLevelsList();
}

void FT4LevelCollectionModel::BindCommands()
{
	const FT4LevelCollectionCommands& Commands = FT4LevelCollectionCommands::Get();
	FUICommandList& ActionList = *CommandList;

	ActionList.MapAction(Commands.RefreshBrowser,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::RefreshBrowser_Executed));

	ActionList.MapAction(Commands.ExpandSelectedItems,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::ExpandSelectedItems_Executed),
		FCanExecuteAction::CreateSP(this, &FT4LevelCollectionModel::AreAnyLevelsSelected));

	ActionList.MapAction(Commands.World_MakeLevelCurrent,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::MakeLevelCurrent_Executed),
		FCanExecuteAction::CreateSP(this, &FT4LevelCollectionModel::IsOneLevelSelected));
	
	ActionList.MapAction(Commands.World_FindInContentBrowser,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::FindInContentBrowser_Executed),
		FCanExecuteAction::CreateSP(this, &FT4LevelCollectionModel::IsValidFindInContentBrowser));
	
	ActionList.MapAction(Commands.MoveActorsToSelected,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::MoveActorsToSelected_Executed),
		FCanExecuteAction::CreateSP(this, &FT4LevelCollectionModel::IsValidMoveActorsToLevel));

	ActionList.MapAction(Commands.MoveFoliageToSelected,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::MoveFoliageToSelected_Executed),
		FCanExecuteAction::CreateSP(this, &FT4LevelCollectionModel::IsValidMoveFoliageToLevel));
		
	ActionList.MapAction(Commands.World_SaveSelectedLevels,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::SaveSelectedLevels_Executed),
		FCanExecuteAction::CreateSP(this, &FT4LevelCollectionModel::AreAnySelectedLevelsDirty));

	ActionList.MapAction(Commands.World_SaveSelectedLevelAs,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::SaveSelectedLevelAs_Executed),
		FCanExecuteAction::CreateSP(this, &FT4LevelCollectionModel::IsSelectedLevelEditable));
		
	ActionList.MapAction(Commands.World_LoadLevel,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::LoadSelectedLevels_Executed),
		FCanExecuteAction::CreateSP(this, &FT4LevelCollectionModel::AreAnySelectedLevelsUnloaded));

	ActionList.MapAction(Commands.World_UnloadLevel,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::UnloadSelectedLevels_Executed),
		FCanExecuteAction::CreateSP(this, &FT4LevelCollectionModel::AreAnySelectedLevelsLoaded));

	ActionList.MapAction( Commands.World_MigrateSelectedLevels,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::MigrateSelectedLevels_Executed),
		FCanExecuteAction::CreateSP( this, &FT4LevelCollectionModel::AreAllSelectedLevelsEditable));

	ActionList.MapAction(Commands.PreviewWorld_LoadLevel,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::LoadSelectedPreviewLevels_Executed),
		FCanExecuteAction::CreateSP(this, &FT4LevelCollectionModel::AreAnySelectedPreviewLevelsUnloaded)); // #104

	ActionList.MapAction(Commands.PreviewWorld_UnloadLevel,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::UnloadSelectedPreviewLevels_Executed),
		FCanExecuteAction::CreateSP(this, &FT4LevelCollectionModel::AreAnySelectedPreviewLevelsLoaded)); // #104

	//actors
	ActionList.MapAction(Commands.AddsActors,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::SelectActors_Executed),
		FCanExecuteAction::CreateSP(this, &FT4LevelCollectionModel::AreAnySelectedLevelsEditable));
	
	ActionList.MapAction(Commands.RemovesActors,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::DeselectActors_Executed),
		FCanExecuteAction::CreateSP(this, &FT4LevelCollectionModel::AreAnySelectedLevelsEditable));

	//visibility
	ActionList.MapAction( Commands.World_ShowSelectedLevels,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::ShowSelectedLevels_Executed  ),
		FCanExecuteAction::CreateSP( this, &FT4LevelCollectionModel::AreAnySelectedLevelsLoaded ) );
	
	ActionList.MapAction( Commands.World_HideSelectedLevels,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::HideSelectedLevels_Executed  ),
		FCanExecuteAction::CreateSP( this, &FT4LevelCollectionModel::AreAnySelectedLevelsLoaded ) );
	
	ActionList.MapAction( Commands.World_ShowOnlySelectedLevels,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::ShowOnlySelectedLevels_Executed  ),
		FCanExecuteAction::CreateSP( this, &FT4LevelCollectionModel::AreAnySelectedLevelsLoaded ) );

	ActionList.MapAction(Commands.World_ShowAllButSelectedLevels,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::ShowAllButSelectedLevels_Executed),
		FCanExecuteAction::CreateSP(this, &FT4LevelCollectionModel::AreAnySelectedLevelsLoaded));

	ActionList.MapAction(Commands.World_ShowAllLevels,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::ShowAllLevels_Executed));
	
	ActionList.MapAction(Commands.World_HideAllLevels,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::HideAllLevels_Executed));
		
	//lock
	ActionList.MapAction( Commands.World_LockSelectedLevels,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::LockSelectedLevels_Executed  ) );
	
	ActionList.MapAction( Commands.World_UnockSelectedLevels,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::UnlockSelectedLevels_Executed  ) );

	ActionList.MapAction(Commands.World_LockOnlySelectedLevels,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::LockOnlySelectedLevels_Executed));

	ActionList.MapAction(Commands.World_LockAllButSelectedLevels,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::LockAllButSelectedLevels_Executed));
	
	ActionList.MapAction( Commands.World_LockAllLevels,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::LockAllLevels_Executed  ) );
	
	ActionList.MapAction( Commands.World_UnockAllLevels,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::UnockAllLevels_Executed  ) );

	ActionList.MapAction( Commands.World_LockReadOnlyLevels,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::ToggleReadOnlyLevels_Executed  ) );

	ActionList.MapAction( Commands.World_UnlockReadOnlyLevels,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::ToggleReadOnlyLevels_Executed  ) );
	
	// #84
	ActionList.MapAction(Commands.World_ThumbnailSelectedLevels,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::ThumbnailSelectedLevels_Executed));

	// #90
	ActionList.MapAction(Commands.PreviewWorld_CameraLookAtLocation,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::PreviewCameraLookAtLocation_Executed));
	ActionList.MapAction(Commands.World_CameraLookAtLocation,
		FExecuteAction::CreateSP(this, &FT4LevelCollectionModel::EditorCameraLookAtLocation_Executed));
	
	//level selection
	ActionList.MapAction( Commands.SelectAllLevels,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::SelectAllLevels_Executed  ) );
	
	ActionList.MapAction( Commands.DeselectAllLevels,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::DeselectAllLevels_Executed  ) );

	ActionList.MapAction( Commands.InvertLevelSelection,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::InvertSelection_Executed  ) );
	
	//source control
	ActionList.MapAction( Commands.SCCCheckOut,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::OnSCCCheckOut  ) );

	ActionList.MapAction( Commands.SCCCheckIn,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::OnSCCCheckIn  ) );

	ActionList.MapAction( Commands.SCCOpenForAdd,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::OnSCCOpenForAdd  ) );

	ActionList.MapAction( Commands.SCCHistory,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::OnSCCHistory  ) );

	ActionList.MapAction( Commands.SCCRefresh,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::OnSCCRefresh  ) );

	ActionList.MapAction( Commands.SCCDiffAgainstDepot,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::OnSCCDiffAgainstDepot  ) );

	ActionList.MapAction( Commands.SCCConnect,
		FExecuteAction::CreateSP( this, &FT4LevelCollectionModel::OnSCCConnect  ) );
	
}

void FT4LevelCollectionModel::PopulateLevelsList()
{
	RootLevelsList.Empty();
	AllLevelsList.Empty();
	FilteredLevelsList.Empty();
	SelectedLevelsList.Empty();
	AllLevelsMap.Empty();

	OnLevelsCollectionChanged();
}

void FT4LevelCollectionModel::PopulateFilteredLevelsList()
{
	FilteredLevelsList.Empty();

	// Filter out our flat list
	for (auto& LevelModel : AllLevelsList)
	{
		LevelModel->SetLevelFilteredOutFlag(true);
		if (LevelModel->IsPersistent() || PassesAllFilters(*LevelModel))
		{
			FilteredLevelsList.Add(LevelModel);
			LevelModel->SetLevelFilteredOutFlag(false);
		}
	}

	// Walk through hierarchy and filter it out
	for (auto It = RootLevelsList.CreateIterator(); It; ++It)
	{
		(*It)->OnFilterChanged();
	}
}

void FT4LevelCollectionModel::Tick( float DeltaTime )
{
	if (GEditor == nullptr)
	{
		// Could it be during hot-reloading?
		return;
	}
	
	if (bRequestedUpdateAllLevels)
	{
		UpdateAllLevels();
	}

	if (bRequestedRedrawAllLevels)
	{		
		RedrawAllLevels();
	}

	if (bRequestedUpdateActorsCount)
	{
		UpdateLevelActorsCount();
	}
	
	if (IsSimulating())
	{
		UWorld* SimulationWorld = GetSimulationWorld();
		if (nullptr == SimulationWorld)
		{
			return;
		}

		// Reset simulation status for all levels
		for (TSharedPtr<FT4LevelModel>& LevelModel : AllLevelsList)
		{
			LevelModel->UpdateSimulationStatus(nullptr);
		}

		// Traverse streaming levels and update simulation status for corresponding level models
		for (ULevelStreaming* StreamingLevel : GetSimulationWorld()->GetStreamingLevels())
		{
			// Rebuild the original NonPrefixedPackageName so we can find it
			const FString PrefixedPackageName = StreamingLevel->GetWorldAssetPackageName();
			const FString NonPrefixedPackageName = FPackageName::GetLongPackagePath(PrefixedPackageName) + "/" 
					+ FPackageName::GetLongPackageAssetName(PrefixedPackageName).RightChop(GetSimulationWorld()->StreamingLevelsPrefix.Len());
								
			TSharedPtr<FT4LevelModel> LevelModel = FindLevelModel(FName(*NonPrefixedPackageName));
			if (LevelModel.IsValid())
			{
				LevelModel->UpdateSimulationStatus(StreamingLevel);
			}
		}
	}
}

TStatId FT4LevelCollectionModel::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FT4LevelCollectionModel, STATGROUP_Tickables);
}

bool FT4LevelCollectionModel::IsReadOnly() const
{
	// read only in PIE/SIE
	return IsSimulating();
}

bool FT4LevelCollectionModel::IsSimulating() const
{
	return (GEditor->bIsSimulatingInEditor || GEditor->PlayWorld != NULL);
}

UWorld* FT4LevelCollectionModel::GetSimulationWorld() const 
{ 
	return GEditor->PlayWorld; 
}

bool FT4LevelCollectionModel::IsOriginRebasingEnabled() const
{
	UWorld* ThisWorld = GetWorld();
	return ThisWorld && ThisWorld->GetWorldSettings()->bEnableWorldOriginRebasing;
}

FT4LevelModelList& FT4LevelCollectionModel::GetRootLevelList()
{ 
	return RootLevelsList;
}

const FT4LevelModelList& FT4LevelCollectionModel::GetAllLevels() const
{
	return AllLevelsList;
}

const FT4LevelModelList& FT4LevelCollectionModel::GetFilteredLevels() const
{
	return FilteredLevelsList;
}

const FT4LevelModelList& FT4LevelCollectionModel::GetSelectedLevels() const
{
	return SelectedLevelsList;
}

void FT4LevelCollectionModel::AddFilter(const TSharedRef<T4LevelFilter>& InFilter)
{
	Filters->Add(InFilter);
	OnFilterChanged();
}

void FT4LevelCollectionModel::RemoveFilter(const TSharedRef<T4LevelFilter>& InFilter)
{
	Filters->Remove(InFilter);
	OnFilterChanged();
}

bool FT4LevelCollectionModel::IsFilterActive() const
{
	return (AllLevelsList.Num() != FilteredLevelsList.Num());
}

void FT4LevelCollectionModel::SetSelectedLevels(const FT4LevelModelList& InList)
{
	// Clear selection flag from currently selected levels
	for (auto LevelModel : SelectedLevelsList)
	{
		LevelModel->SetLevelSelectionFlag(false);
	}
	
	SelectedLevelsList.Empty(); 
	
	// Set selection flag to selected levels	
	for (auto& LevelModel : InList)
	{
		if (LevelModel.IsValid() && PassesAllFilters(*LevelModel))
		{
			LevelModel->SetLevelSelectionFlag(true);
			SelectedLevelsList.Add(LevelModel);
		}
	}

	OnLevelsSelectionChanged();
}

void FT4LevelCollectionModel::SetSelectedLevelsFromWorld()
{
#if 1
	if (!CurrentWorld.IsValid())
	{
		return;
	}

	TGuardValue<bool> UpdateGuard(bUpdatingLevelsSelection, true); // #91 : FLevelCollectionModel 와 재규 호출 방지!

	// #90 : WorldBrowser 의 LevelHierarchy 에서 선택될 경우 선택된 Level 을 Preview 에 표시해준다. (특히, 로딩전 레벨 표시가 유효한 기능)
	FWorldBrowserModule& WorldBrowserModule = FModuleManager::LoadModuleChecked<FWorldBrowserModule>("WorldBrowser");
	TSharedPtr<FLevelCollectionModel> WorldModelCopy = WorldBrowserModule.SharedWorldModel(CurrentWorld.Get());
	if (!WorldModelCopy.IsValid())
	{
		return;
	}

	for (auto LevelModel : SelectedEditorLevelsList)
	{
		LevelModel->SetEditorLevelSelectionFlag(false);
	}
	SelectedEditorLevelsList.Empty();

	const FLevelModelList& SelectedLevels = WorldModelCopy->GetSelectedLevels();
	for (auto It = SelectedLevels.CreateConstIterator(); It; ++It)
	{
		FLevelModel* EditorLevelModel = It->Get();
		check(nullptr != EditorLevelModel);
		TSharedPtr<FT4LevelModel> LevelModel = FindLevelModel(EditorLevelModel->GetLongPackageName());
		if (LevelModel.IsValid() && PassesAllFilters(*LevelModel))
		{
			LevelModel->SetEditorLevelSelectionFlag(true);
			SelectedEditorLevelsList.Add(LevelModel);
		}
	}

#else
	TArray<ULevel*>& SelectedLevelObjects = CurrentWorld->GetSelectedLevels();
	FT4LevelModelList LevelsToSelect;
	for (ULevel* LevelObject : SelectedLevelObjects)
	{
		TSharedPtr<FT4LevelModel> LevelModel = FindLevelModel(LevelObject);
		if (LevelModel.IsValid())
		{
			LevelsToSelect.Add(LevelModel);
		}
	}

	SetSelectedLevels(LevelsToSelect);
#endif
}

TSharedPtr<FT4LevelModel> FT4LevelCollectionModel::FindLevelModel(ULevel* InLevel) const
{
	if (InLevel)
	{
		for (auto It = AllLevelsList.CreateConstIterator(); It; ++It)
		{
			if ((*It)->GetLevelObject() == InLevel)
			{
				return (*It);
			}
		}
	}

	// not found
	return TSharedPtr<FT4LevelModel>();
}

TSharedPtr<FT4LevelModel> FT4LevelCollectionModel::FindLevelModel(const FName& PackageName) const
{
	const TSharedPtr<FT4LevelModel>* LevelModel = AllLevelsMap.Find(PackageName);
	if (LevelModel != NULL)
	{
		return *LevelModel;
	}
	
	// not found
	return TSharedPtr<FT4LevelModel>();
}

void FT4LevelCollectionModel::IterateHierarchy(FT4LevelModelVisitor& Visitor)
{
	for (auto It = RootLevelsList.CreateIterator(); It; ++It)
	{
		(*It)->Accept(Visitor);
	}
}

void FT4LevelCollectionModel::HideLevels(const FT4LevelModelList& InLevelList)
{
	if (IsReadOnly())
	{
		return;
	}
	
	for (auto It = InLevelList.CreateConstIterator(); It; ++It)
	{
		(*It)->SetVisible(false);
	}

	RequestUpdateAllLevels();
}
	
void FT4LevelCollectionModel::ShowLevels(const FT4LevelModelList& InLevelList)
{
	if (IsReadOnly())
	{
		return;
	}
	
	OnPreShowLevels(InLevelList);

	for (auto It = InLevelList.CreateConstIterator(); It; ++It)
	{
		(*It)->SetVisible(true);
	}

	RequestUpdateAllLevels();
}

void FT4LevelCollectionModel::ShowOnlySelectedLevels()
{
	ShowOnlySelectedLevels_Executed();
}

void FT4LevelCollectionModel::ShowAllButSelectedLevels()
{
	ShowAllButSelectedLevels_Executed();
}

void FT4LevelCollectionModel::UnlockLevels(const FT4LevelModelList& InLevelList)
{
	if (!IsReadOnly())
	{
		const FText UndoTransactionText = (InLevelList.Num() == 1) ?
			LOCTEXT("UnlockLevel", "Unlock Level") :
			LOCTEXT("UnlockMultipleLevels", "Unlock Multiple Levels");

		const FScopedTransaction Transaction(UndoTransactionText);

		for (auto It = InLevelList.CreateConstIterator(); It; ++It)
		{
			(*It)->SetLocked(false);
		}
	}
}
	
void FT4LevelCollectionModel::LockLevels(const FT4LevelModelList& InLevelList)
{
	if (!IsReadOnly())
	{
		const FText UndoTransactionText = (InLevelList.Num() == 1) ?
			LOCTEXT("LockLevel", "Lock Level") :
			LOCTEXT("LockMultipleLevels", "Lock Multiple Levels");

		const FScopedTransaction Transaction(UndoTransactionText);

		for (auto It = InLevelList.CreateConstIterator(); It; ++It)
		{
			(*It)->SetLocked(true);
		}
	}
}

void FT4LevelCollectionModel::LockOnlySelectedLevels()
{
	LockOnlySelectedLevels_Executed();
}

void FT4LevelCollectionModel::LockAllButSelectedLevels()
{
	LockAllButSelectedLevels_Executed();
}

void FT4LevelCollectionModel::SaveLevels(const FT4LevelModelList& InLevelList, bool bSelection)
{
	if (IsReadOnly())
	{
		return;
	}

		
	FT4LevelModelList		LevelModelsToSave;
	TArray<ULevel*>		LevelsToSave;
	for (auto It = InLevelList.CreateConstIterator(); It; ++It)
	{
		if ((*It)->GetLevelObject())
		{
			if (!(*It)->IsVisible())
			{
				FMessageDialog::Open( EAppMsgType::Ok, NSLOCTEXT("UnrealEd", "UnableToSaveInvisibleLevels", "Save aborted.  Levels must be made visible before they can be saved.") );
				return;
			}
			else if ((*It)->IsLocked())
			{
				FMessageDialog::Open( EAppMsgType::Ok, NSLOCTEXT("UnrealEd", "UnableToSaveLockedLevels", "Save aborted.  Level must be unlocked before it can be saved.") );
				return;
			}
						
			LevelModelsToSave.Add(*It);
			LevelsToSave.Add((*It)->GetLevelObject());
		}
	}

	TArray< UPackage* > PackagesNotNeedingCheckout;
	// Prompt the user to check out the levels from source control before saving
	if (FEditorFileUtils::PromptToCheckoutLevels(false, LevelsToSave, &PackagesNotNeedingCheckout))
	{
		for (auto It = LevelsToSave.CreateIterator(); It; ++It)
		{
			FEditorFileUtils::SaveLevel(*It);
		}
	}
	else if (PackagesNotNeedingCheckout.Num() > 0)
	{
		// The user canceled the checkout dialog but some packages didn't need to be checked out in order to save
		// For each selected level if the package its in didn't need to be saved, save the level!
		for (int32 LevelIdx = 0; LevelIdx < LevelsToSave.Num(); ++LevelIdx)
		{
			ULevel* Level = LevelsToSave[LevelIdx];
			if (PackagesNotNeedingCheckout.Contains(Level->GetOutermost()))
			{
				FEditorFileUtils::SaveLevel(Level);
			}
			else
			{
				//remove it from the list, so that only successfully saved levels are highlighted when saving is complete
				LevelModelsToSave.RemoveAt(LevelIdx);
				LevelsToSave.RemoveAt(LevelIdx);
			}
		}
	}

	if (bSelection) // #86
	{
		// Select tiles that were saved successfully
		SetSelectedLevels(LevelModelsToSave);
	}
}

void FT4LevelCollectionModel::LoadLevels(const FT4LevelModelList& InLevelList)
{
	if (IsReadOnly())
	{
		return;
	}

	GWarn->BeginSlowTask(LOCTEXT("LoadWorldTiles", "Loading levels"), true);
	
	OnPreLoadLevels(InLevelList);
	
	int32 LevelIdx = 0;
	for (TSharedPtr<FT4LevelModel> LevelModel : InLevelList)
	{
		GWarn->StatusUpdate(LevelIdx++, InLevelList.Num(), 
			FText::Format(LOCTEXT("LoadingWorldTiles", "Loading: {0}..." ), FText::FromString(LevelModel->GetLongPackageName().ToString()))
			);

		LevelModel->LoadLevel();
	}

	if (InLevelList.Num() > 0)
	{
		GEditor->ResetTransaction(LOCTEXT("LoadingWorldTilesTransReset", "Loading Levels"));
	}

	GWarn->EndSlowTask();	
}

void FT4LevelCollectionModel::UnloadLevels(const FT4LevelModelList& InLevelList)
{
	if (InLevelList.Num() == 0)
	{
		return;
	}

	UWorld* ThisWorld = GetWorld();
	check(ThisWorld != nullptr);

	// If matinee is opened, and if it belongs to the level being removed, close it
	if (GLevelEditorModeTools().IsModeActive(FBuiltinEditorModes::EM_InterpEdit))
	{
		TArray<ULevel*> LevelsToRemove = GetLevelObjectList(InLevelList);
		
		const FEdModeInterpEdit* InterpEditMode = (const FEdModeInterpEdit*)GLevelEditorModeTools().GetActiveMode(FBuiltinEditorModes::EM_InterpEdit);

		if (InterpEditMode && InterpEditMode->MatineeActor && LevelsToRemove.Contains(InterpEditMode->MatineeActor->GetLevel()))
		{
			GLevelEditorModeTools().ActivateDefaultMode();
		}
	}
	else if(GLevelEditorModeTools().IsModeActive(FBuiltinEditorModes::EM_Landscape))
	{
		GLevelEditorModeTools().ActivateDefaultMode();
	}

	BroadcastPreLevelsUnloaded();

	// Take a copy of the list rather than using a reference to the selected levels list, as this will be modified in the loop below
	const FT4LevelModelList LevelListCopy = InLevelList;
	for (auto It = LevelListCopy.CreateConstIterator(); It; ++It)
	{
		TSharedPtr<FT4LevelModel> LevelModel = (*It);
		ULevel* Level = LevelModel->GetLevelObject();

		if (Level != nullptr && !LevelModel->IsPersistent())
		{
			// Unselect all actors before removing the level
			// This avoids crashing in areas that rely on getting a selected actors level. The level will be invalid after its removed.
			for (auto ActorIt = Level->Actors.CreateIterator(); ActorIt; ++ActorIt)
			{
				GEditor->SelectActor((*ActorIt), /*bInSelected=*/ false, /*bSelectEvenIfHidden=*/ false);
			}
			
			// In case we have created temporary streaming level object for this sub-level - remove it before unloading sub-level
			{
				FName LevelPackageName = LevelModel->GetLongPackageName();
				auto Predicate = [&](ULevelStreaming* StreamingLevel) 
				{
					return (StreamingLevel && StreamingLevel->GetWorldAssetPackageFName() == LevelPackageName && StreamingLevel->HasAnyFlags(RF_Transient));
				};
				
				if (ULevelStreaming*const* StreamingLevel = ThisWorld->GetStreamingLevels().FindByPredicate(Predicate))
				{
					(*StreamingLevel)->MarkPendingKill();
					ThisWorld->RemoveStreamingLevel(*StreamingLevel);
				}
			}
			
			// Unload sub-level
			{
				FUnmodifiableObject ImmuneWorld(CurrentWorld.Get());
				EditorLevelUtils::RemoveLevelFromWorld(Level);
			}
		}
		else if (ULevelStreaming* StreamingLevel = Cast<ULevelStreaming>(LevelModel->GetNodeObject()))
		{
			StreamingLevel->MarkPendingKill();
			ThisWorld->RemoveStreamingLevel(StreamingLevel);
		}
	}

	BroadcastPostLevelsUnloaded();

	GEditor->ResetTransaction( LOCTEXT("RemoveLevelTransReset", "Removing Levels from World") );

	// Collect garbage to clear out the destroyed level
	CollectGarbage( GARBAGE_COLLECTION_KEEPFLAGS );

	PopulateLevelsList();
}

void FT4LevelCollectionModel::TranslateLevels(const FT4LevelModelList& InLevels, FVector2D InDelta, bool bSnapDelta)
{
}

FVector2D FT4LevelCollectionModel::SnapTranslationDelta(const FT4LevelModelList& InLevelList, FVector2D InTranslationDelta, bool bBoundsSnapping, float InSnappingValue)
{
	return InTranslationDelta;
}

void FT4LevelCollectionModel::UpdateTranslationDelta(const FT4LevelModelList& InLevelList, FVector2D InTranslationDelta, bool bBoundsSnapping, float InSnappingValue)
{
	FT4LevelModelList EditableLevels;
	// Only editable levels could be moved
	for (auto It = InLevelList.CreateConstIterator(); It; ++It)
	{
		if ((*It)->IsEditable())
		{
			EditableLevels.Add(*It);
		}
	}
	
	// Snap	translation delta
	if (InTranslationDelta != FVector2D::ZeroVector)
	{
		InTranslationDelta = SnapTranslationDelta(EditableLevels, InTranslationDelta, bBoundsSnapping, InSnappingValue);
	}
		
	for (auto It = EditableLevels.CreateIterator(); It; ++It)
	{
		(*It)->SetLevelTranslationDelta(InTranslationDelta);
	}
}

void FT4LevelCollectionModel::AssignParent(const FT4LevelModelList& InLevels, TSharedPtr<FT4LevelModel> InParent)
{
	// Attach levels to the new parent
	for (auto It = InLevels.CreateConstIterator(); It; ++It)
	{
		(*It)->AttachTo(InParent);
	}

	OnLevelsHierarchyChanged();
}

void FT4LevelCollectionModel::AddExistingLevelsFromAssetData(const TArray<FAssetData>& WorldList)
{
	
}

bool FT4LevelCollectionModel::PassesAllFilters(const FT4LevelModel& Item) const
{
	if (Item.IsPersistent() || Filters->PassesAllFilters(&Item))
	{
		return true;
	}
	
	return false;
}

void FT4LevelCollectionModel::BuildHierarchyMenu(FMenuBuilder& InMenuBuilder) const
{
}

void FT4LevelCollectionModel::CustomizeFileMainMenu(FMenuBuilder& InMenuBuilder) const
{
	const FT4LevelCollectionCommands& Commands = FT4LevelCollectionCommands::Get();

	// Cache SCC state
	CacheCanExecuteSourceControlVars();

	InMenuBuilder.AddSubMenu( 
		LOCTEXT("SourceControl", "Source Control"),
		LOCTEXT("SourceControl_ToolTip", "Source Control Options"),
		FNewMenuDelegate::CreateSP(this, &FT4LevelCollectionModel::FillSourceControlSubMenu));
		
	if (AreAnyLevelsSelected())
	{
		InMenuBuilder.AddMenuEntry( Commands.World_SaveSelectedLevels );
		InMenuBuilder.AddMenuEntry( Commands.World_SaveSelectedLevelAs );
		InMenuBuilder.AddMenuEntry( Commands.World_MigrateSelectedLevels );
	}
}

bool FT4LevelCollectionModel::GetPlayerView(
	FVector& OutCameraLocation,
	FRotator& OutCameraRotation,
	FVector& OutPlayerLocation
) const
{
	return false;
}

bool FT4LevelCollectionModel::GetObserverView(FVector& Location, FRotator& Rotation) const
{
	return false;
}

bool FT4LevelCollectionModel::CompareLevelsZOrder(TSharedPtr<FT4LevelModel> InA, TSharedPtr<FT4LevelModel> InB) const
{
	return false;
}

void FT4LevelCollectionModel::RequestUpdateAllLevels()
{
	bRequestedUpdateAllLevels = true;
	BroadcastSubLevelChanged(); // #83
}

void FT4LevelCollectionModel::RequestRedrawAllLevels()
{
	bRequestedRedrawAllLevels = true;
}

void FT4LevelCollectionModel::UpdateAllLevels()
{
	bRequestedUpdateAllLevels = false;

	for (auto It = AllLevelsList.CreateConstIterator(); It; ++It)
	{
		(*It)->Update();
	}
	
	// Update world size
	FBox WorldBounds = GetLevelsBoundingBox(AllLevelsList, false);
	WorldSize.X = FMath::RoundToInt(WorldBounds.GetSize().X);
	WorldSize.Y = FMath::RoundToInt(WorldBounds.GetSize().Y);
}

void FT4LevelCollectionModel::RedrawAllLevels()
{
	bRequestedRedrawAllLevels = false;

	for (auto It = AllLevelsList.CreateConstIterator(); It; ++It)
	{
		(*It)->UpdateVisuals();
	}
}

void FT4LevelCollectionModel::UpdateLevelActorsCount()
{
	for( auto It = AllLevelsList.CreateIterator(); It; ++It )
	{
		(*It)->UpdateLevelActorsCount();
	}

	bRequestedUpdateActorsCount = false;
}

bool FT4LevelCollectionModel::IsOneLevelSelected() const
{
	return SelectedLevelsList.Num() == 1;
}

bool FT4LevelCollectionModel::AreAnyLevelsSelected() const
{
	return SelectedLevelsList.Num() > 0;
}

bool FT4LevelCollectionModel::AreAllSelectedLevelsLoaded() const
{
	for (int32 LevelIdx = 0; LevelIdx < SelectedLevelsList.Num(); LevelIdx++)
	{
		if (SelectedLevelsList[LevelIdx]->IsLoaded() == false)
		{
			return false;
		}
	}

	return AreAnyLevelsSelected();
}

bool FT4LevelCollectionModel::AreAnySelectedLevelsLoaded() const
{
	return !AreAllSelectedLevelsUnloaded();
}

bool FT4LevelCollectionModel::AreAllSelectedLevelsUnloaded() const
{
	for (int32 LevelIdx = 0; LevelIdx < SelectedLevelsList.Num(); LevelIdx++)
	{
		if (SelectedLevelsList[LevelIdx]->IsLoaded() == true)
		{
			return false;
		}
	}

	return true;
}

bool FT4LevelCollectionModel::AreAnySelectedLevelsUnloaded() const
{
	return !AreAllSelectedLevelsLoaded();
}

bool FT4LevelCollectionModel::AreAllSelectedLevelsEditable() const
{
	for (auto It = SelectedLevelsList.CreateConstIterator(); It; ++It)
	{
		if ((*It)->IsEditable() == false)
		{
			return false;
		}
	}
	
	return AreAnyLevelsSelected();
}

bool FT4LevelCollectionModel::AreAllSelectedLevelsEditableAndNotPersistent() const
{
	for (auto It = SelectedLevelsList.CreateConstIterator(); It; ++It)
	{
		if ((*It)->IsEditable() == false || (*It)->IsPersistent())
		{
			return false;
		}
	}
	
	return AreAnyLevelsSelected();
}

bool FT4LevelCollectionModel::AreAllSelectedLevelsEditableAndVisible() const
{
	for (auto It = SelectedLevelsList.CreateConstIterator(); It; ++It)
	{
		if ((*It)->IsEditable() == false ||
			(*It)->IsVisible() == false)
		{
			return false;
		}
	}
	
	return AreAnyLevelsSelected();
}

bool FT4LevelCollectionModel::AreAnySelectedLevelsEditable() const
{
	for (auto It = SelectedLevelsList.CreateConstIterator(); It; ++It)
	{
		if ((*It)->IsEditable() == true)
		{
			return true;
		}
	}
	
	return false;
}

bool FT4LevelCollectionModel::AreAnySelectedLevelsEditableAndVisible() const
{
	for (auto It = SelectedLevelsList.CreateConstIterator(); It; ++It)
	{
		if ((*It)->IsEditable() == true && 
			(*It)->IsVisible() == true)
		{
			return true;
		}
	}
	
	return false;
}

bool FT4LevelCollectionModel::IsSelectedLevelEditable() const
{
	if (SelectedLevelsList.Num() == 1)
	{
		return SelectedLevelsList[0]->IsEditable();
	}
	
	return false;
}

bool FT4LevelCollectionModel::IsNewLightingScenarioState(bool bExistingState) const
{
	if (SelectedLevelsList.Num() == 1)
	{
		return SelectedLevelsList[0]->IsLightingScenario() != bExistingState;
	}
	
	return false;
}

void FT4LevelCollectionModel::SetIsLightingScenario(bool bNewLightingScenario)
{
	if (SelectedLevelsList.Num() == 1)
	{
		SelectedLevelsList[0]->SetIsLightingScenario(bNewLightingScenario);
	}
}

bool FT4LevelCollectionModel::AreAnySelectedLevelsDirty() const
{
	for (auto It = SelectedLevelsList.CreateConstIterator(); It; ++It)
	{
		if ((*It)->IsLoaded() == true && (*It)->IsDirty() == true)
		{
			return true;
		}
	}
	
	return false;
}

bool FT4LevelCollectionModel::AreActorsSelected() const
{
	return GEditor->GetSelectedActorCount() > 0;
}

bool FT4LevelCollectionModel::GetDisplayPathsState() const
{
	return bDisplayPaths;
}

void FT4LevelCollectionModel::SetDisplayPathsState(bool InDisplayPaths)
{
	bDisplayPaths = InDisplayPaths;

	for (auto It = AllLevelsList.CreateIterator(); It; ++It)
	{
		(*It)->UpdateDisplayName();
	}
}

bool FT4LevelCollectionModel::GetDisplayActorsCountState() const
{
	return bDisplayActorsCount;
}

void FT4LevelCollectionModel::SetDisplayActorsCountState(bool InDisplayActorsCount)
{
	bDisplayActorsCount = InDisplayActorsCount;

	for (auto It = AllLevelsList.CreateIterator(); It; ++It)
	{
		(*It)->UpdateDisplayName();
	}
}

void FT4LevelCollectionModel::BroadcastSelectionChanged()
{
	SelectionChanged.Broadcast();
}

void FT4LevelCollectionModel::BroadcastCollectionChanged()
{
	CollectionChanged.Broadcast();
}
		
void FT4LevelCollectionModel::BroadcastHierarchyChanged()
{
	HierarchyChanged.Broadcast();
}

void FT4LevelCollectionModel::BroadcastPreLevelsUnloaded()
{
	PreLevelsUnloaded.Broadcast();
}

void FT4LevelCollectionModel::BroadcastPostLevelsUnloaded()
{
	PostLevelsUnloaded.Broadcast();
}

void FT4LevelCollectionModel::BroadcastSubLevelChanged() // #83
{
	OnSubLevelChanged.Broadcast();
}

float FT4LevelCollectionModel::EditableAxisLength()
{ 
	return HALF_WORLD_MAX; 
};

FBox FT4LevelCollectionModel::EditableWorldArea()
{
	float AxisLength = EditableAxisLength();
	
	return FBox(	
		FVector(-AxisLength, -AxisLength, -AxisLength), 
		FVector(+AxisLength, +AxisLength, +AxisLength)
		);
}

void FT4LevelCollectionModel::SCCCheckOut(const FT4LevelModelList& InList)
{
	TArray<FString> FilenamesToCheckOut = GetFilenamesList(InList);
	
	// Update the source control status of all potentially relevant packages
	ISourceControlModule::Get().GetProvider().Execute(
		ISourceControlOperation::Create<FUpdateStatus>(), FilenamesToCheckOut
		);

	// Now check them out
	FEditorFileUtils::CheckoutPackages(FilenamesToCheckOut);
}

void FT4LevelCollectionModel::SCCCheckIn(const FT4LevelModelList& InList)
{
	TArray<UPackage*> PackagesToCheckIn = GetPackagesList(InList);
	TArray<FString> FilenamesToCheckIn = GetFilenamesList(InList);
	
	// Prompt the user to ask if they would like to first save any dirty packages they are trying to check-in
	const auto UserResponse = FEditorFileUtils::PromptForCheckoutAndSave(PackagesToCheckIn, true, true);

	// If the user elected to save dirty packages, but one or more of the packages failed to save properly OR if the user
	// canceled out of the prompt, don't follow through on the check-in process
	const bool bShouldProceed = UserResponse == FEditorFileUtils::EPromptReturnCode::PR_Success || 
								UserResponse == FEditorFileUtils::EPromptReturnCode::PR_Declined;
	if (bShouldProceed)
	{
		const bool bUseSourceControlStateCache = false;
		FSourceControlWindows::PromptForCheckin(bUseSourceControlStateCache, FilenamesToCheckIn);
	}
	else
	{
		// If a failure occurred, alert the user that the check-in was aborted. This warning shouldn't be necessary if the user cancelled
		// from the dialog, because they obviously intended to cancel the whole operation.
		if (UserResponse == FEditorFileUtils::EPromptReturnCode::PR_Failure)
		{
			FMessageDialog::Open(EAppMsgType::Ok, 
				NSLOCTEXT("UnrealEd", "SCC_Checkin_Aborted", "Check-in aborted as a result of save failure.")
				);
		}
	}
}

void FT4LevelCollectionModel::SCCOpenForAdd(const FT4LevelModelList& InList)
{
	ISourceControlProvider& SourceControlProvider = ISourceControlModule::Get().GetProvider();
	TArray<FString>		FilenamesList = GetFilenamesList(InList);
	TArray<FString>		FilenamesToAdd;
	TArray<UPackage*>	PackagesToSave;

	for (auto It = FilenamesList.CreateConstIterator(); It; ++It)
	{
		const FSourceControlStatePtr SourceControlState = SourceControlProvider.GetState(*It, EStateCacheUsage::Use);
		if (SourceControlState.IsValid() && !SourceControlState->IsSourceControlled())
		{
			FilenamesToAdd.Add(*It);

			// Make sure the file actually exists on disk before adding it
			FString LongPackageName = FPackageName::FilenameToLongPackageName(*It);
			if (!FPackageName::DoesPackageExist(LongPackageName))
			{
				UPackage* Package = FindPackage(NULL, *LongPackageName);
				if (Package)
				{
					PackagesToSave.Add(Package);
				}
			}
		}
	}

	if (FilenamesToAdd.Num() > 0)
	{
		// If any of the packages are new, save them now
		if (PackagesToSave.Num() > 0)
		{
			const bool bCheckDirty = false;
			const bool bPromptToSave = false;
			const auto Return = FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, bCheckDirty, bPromptToSave);
		}

		SourceControlProvider.Execute(ISourceControlOperation::Create<FMarkForAdd>(), FilenamesToAdd);
	}
}

void FT4LevelCollectionModel::SCCHistory(const FT4LevelModelList& InList)
{
	// This is odd, why SCC needs package names, instead of filenames? 
	TArray<FString> PackageNames;
	for (auto It = InList.CreateConstIterator(); It; ++It)
	{
		if ((*It)->HasValidPackage())
		{
			PackageNames.Add((*It)->GetLongPackageName().ToString());
		}
	}

	FSourceControlWindows::DisplayRevisionHistory(PackageNames);
}

void FT4LevelCollectionModel::SCCRefresh(const FT4LevelModelList& InList)
{
	if(ISourceControlModule::Get().IsEnabled())
	{
		ISourceControlModule::Get().QueueStatusUpdate(GetFilenamesList(InList));
	}
}

void FT4LevelCollectionModel::SCCDiffAgainstDepot(const FT4LevelModelList& InList, UEditorEngine* InEditor)
{
	// Load the asset registry module
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

	ISourceControlProvider& SourceControlProvider = ISourceControlModule::Get().GetProvider();

	// Iterate over each selected asset
	for (auto It = InList.CreateConstIterator(); It; ++It)
	{
		ULevel* Level = (*It)->GetLevelObject();
		if (Level == NULL)
		{
			return;
		}
		
		UPackage* OriginalPackage = Level->GetOutermost();
		FString PackageName = OriginalPackage->GetName();

		// Make sure our history is up to date
		auto UpdateStatusOperation = ISourceControlOperation::Create<FUpdateStatus>();
		UpdateStatusOperation->SetUpdateHistory(true);
		SourceControlProvider.Execute(UpdateStatusOperation, OriginalPackage);

		// Get the SCC state
		FSourceControlStatePtr SourceControlState = SourceControlProvider.GetState(
			OriginalPackage, EStateCacheUsage::Use
			);

		// If the level is in SCC.
		if (SourceControlState.IsValid() && SourceControlState->IsSourceControlled())
		{
			// Get the file name of package
			FString RelativeFileName;
			if(FPackageName::DoesPackageExist(PackageName, NULL, &RelativeFileName))
			{
				if (SourceControlState->GetHistorySize() > 0)
				{
					auto Revision = SourceControlState->GetHistoryItem(0);
					check(Revision.IsValid());

					// Get the head revision of this package from source control
					FString AbsoluteFileName = FPaths::ConvertRelativePathToFull(RelativeFileName);
					FString TempFileName;
					if (Revision->Get(TempFileName))
					{
						// Try and load that package
						FText NotMapReason;
						UPackage* OldPackage = LoadPackage(NULL, *TempFileName, LOAD_DisableCompileOnLoad);
						if(OldPackage != NULL && InEditor->PackageIsAMapFile(*TempFileName, NotMapReason))
						{
							/* Set the revision information*/
							UPackage* Package = OriginalPackage;

							FRevisionInfo OldRevision;
							OldRevision.Changelist = Revision->GetCheckInIdentifier();
							OldRevision.Date = Revision->GetDate();
							OldRevision.Revision = Revision->GetRevision();

							FRevisionInfo NewRevision; 
							NewRevision.Revision = TEXT("");

							// Dump assets to temp text files
							FString OldTextFilename = AssetToolsModule.Get().DumpAssetToTempFile(OldPackage);
							FString NewTextFilename = AssetToolsModule.Get().DumpAssetToTempFile(OriginalPackage);
							FString DiffCommand = GetDefault<UEditorLoadingSavingSettings>()->TextDiffToolPath.FilePath;

							AssetToolsModule.Get().CreateDiffProcess(DiffCommand, OldTextFilename, NewTextFilename);
							AssetToolsModule.Get().DiffAssets(OldPackage, OriginalPackage, OldRevision, NewRevision);
						}
					}
				}
			} 
		}
	}
}

TArray<FName> FT4LevelCollectionModel::GetPackageNamesList(const FT4LevelModelList& InList)
{
	TArray<FName> ResultList;
	
	for (auto It = InList.CreateConstIterator(); It; ++It)
	{
		if ((*It)->HasValidPackage())
		{
			ResultList.Add((*It)->GetLongPackageName());
		}
	}

	return ResultList;
}

TArray<FString> FT4LevelCollectionModel::GetFilenamesList(const FT4LevelModelList& InList)
{
	TArray<FString> ResultList;
	
	for (auto It = InList.CreateConstIterator(); It; ++It)
	{
		if ((*It)->HasValidPackage())
		{
			ResultList.Add((*It)->GetPackageFileName());
		}
	}

	return ResultList;
}

TArray<UPackage*> FT4LevelCollectionModel::GetPackagesList(const FT4LevelModelList& InList)
{
	TArray<UPackage*> ResultList;
	
	for (auto It = InList.CreateConstIterator(); It; ++It)
	{
		ULevel* Level = (*It)->GetLevelObject();
		if (Level)
		{
			ResultList.Add(Level->GetOutermost());
		}
	}

	return ResultList;
}
	
TArray<ULevel*> FT4LevelCollectionModel::GetLevelObjectList(const FT4LevelModelList& InList)
{
	TArray<ULevel*> ResultList;
	
	for (auto It = InList.CreateConstIterator(); It; ++It)
	{
		ULevel* Level = (*It)->GetLevelObject();
		if (Level)
		{
			ResultList.Add(Level);
		}
	}

	return ResultList;
}

FT4LevelModelList FT4LevelCollectionModel::GetLoadedLevels(const FT4LevelModelList& InList)
{
	FT4LevelModelList ResultList;
	for (auto It = InList.CreateConstIterator(); It; ++It)
	{
		if ((*It)->IsLoaded())
		{
			ResultList.Add(*It);
		}
	}
	
	return ResultList;
}

FT4LevelModelList FT4LevelCollectionModel::GetLevelsHierarchy(const FT4LevelModelList& InList)
{
	struct FHierarchyCollector : public FT4LevelModelVisitor
	{
		virtual void Visit(FT4LevelModel& Item) override
		{
			ResultList.AddUnique(Item.AsShared());
		}

		FT4LevelModelList ResultList;
	};

	FHierarchyCollector HierarchyCollector;
	for (auto It = InList.CreateConstIterator(); It; ++It)
	{
		(*It)->Accept(HierarchyCollector);
	}
	
	return HierarchyCollector.ResultList;
}

FBox FT4LevelCollectionModel::GetLevelsBoundingBox(const FT4LevelModelList& InList, bool bIncludeChildren)
{
	FBox TotalBounds(ForceInit);
	for (auto It = InList.CreateConstIterator(); It; ++It)
	{
		if (bIncludeChildren)
		{
			TotalBounds+= GetVisibleLevelsBoundingBox((*It)->GetChildren(), bIncludeChildren);
		}
		
		TotalBounds+= (*It)->GetLevelBounds();
	}

	return TotalBounds;
}

FBox FT4LevelCollectionModel::GetVisibleLevelsBoundingBox(const FT4LevelModelList& InList, bool bIncludeChildren)
{
	FBox TotalBounds(ForceInit);
	for (auto It = InList.CreateConstIterator(); It; ++It)
	{
		if (bIncludeChildren)
		{
			TotalBounds+= GetVisibleLevelsBoundingBox((*It)->GetChildren(), bIncludeChildren);
		}
		
		if ((*It)->IsVisible())
		{
			TotalBounds+= (*It)->GetLevelBounds();
		}
	}

	return TotalBounds;
}

const TSharedRef<const FUICommandList> FT4LevelCollectionModel::GetCommandList() const
{
	return CommandList;
}

const FString ConfigIniSection = TEXT("WorldMap");

void FT4LevelCollectionModel::LoadSettings()
{
	// Display paths
	bool bDisplayPathsSetting = false;
	GConfig->GetBool(*ConfigIniSection, TEXT("DisplayPaths"), bDisplayPathsSetting, GEditorPerProjectIni);
	SetDisplayPathsState(bDisplayPathsSetting);

	// Display actors count
	bool bDisplayActorsCountSetting = false;
	GConfig->GetBool(*ConfigIniSection, TEXT("DisplayActorsCount"), bDisplayActorsCountSetting, GEditorPerProjectIni);
	SetDisplayActorsCountState(bDisplayActorsCountSetting);
}
	
void FT4LevelCollectionModel::SaveSettings()
{
	// Display paths
	GConfig->SetBool(*ConfigIniSection, TEXT("DisplayPaths"), GetDisplayPathsState(), GEditorPerProjectIni);

	// Display actors count
	GConfig->SetBool(*ConfigIniSection, TEXT("DisplayActorsCount"), GetDisplayActorsCountState(), GEditorPerProjectIni);
}

void FT4LevelCollectionModel::LoadEditorSubLevel(const TArray<FName>& InSubLevelNames) // #104
{
	FT4LevelModelList LoadLevelList;
	for (FName LevelName : InSubLevelNames)
	{
		TSharedPtr<FT4LevelModel> LevelModel = FindLevelModel(LevelName);
		if (LevelModel.IsValid())
		{
			LoadLevelList.Add(LevelModel);
		}
	}
	if (0 >= LoadLevelList.Num())
	{
		return;
	}
	LoadLevels(LoadLevelList);
}

void FT4LevelCollectionModel::LoadPreviewSubLevel(const TArray<FName>& InSubLevelNames) // #104
{
	bool bBoardcast = false;
	for (FName LevelName : InSubLevelNames)
	{
		if (!PreviewLoadedLevelList.Contains(LevelName))
		{
			PreviewLoadedLevelList.Add(LevelName);
			bBoardcast = true;
		}
	}
	if (bBoardcast)
	{
		BroadcastPreviewLoadedLevelChanged();
	}
}

void FT4LevelCollectionModel::UnloadPreviewSubLevel(const TArray<FName>& InSubLevelNames) // #104
{
	bool bBoardcast = false;
	for (FName LevelName : InSubLevelNames)
	{
		if (PreviewLoadedLevelList.Contains(LevelName))
		{
			PreviewLoadedLevelList.Remove(LevelName);
			bBoardcast = true;
		}
	}
	if (bBoardcast)
	{
		BroadcastPreviewLoadedLevelChanged();
	}
}

void FT4LevelCollectionModel::BroadcastPreviewLoadedLevelChanged() // #104
{
	PreviewLoadedLevelChanged.Broadcast();
}

void FT4LevelCollectionModel::BroadcastEditorLoadedLevelChanged() // #104
{
	EditorLoadedLevelChanged.Broadcast();
}

void FT4LevelCollectionModel::RefreshBrowser_Executed()
{
	PopulateLevelsList();
}

void FT4LevelCollectionModel::LoadSelectedLevels_Executed()
{
	LoadLevels(GetSelectedLevels());
}

void FT4LevelCollectionModel::UnloadSelectedLevels_Executed()
{
	UnloadLevels(GetSelectedLevels());
}

void FT4LevelCollectionModel::OnSCCCheckOut()
{
	SCCCheckOut(GetSelectedLevels());
}

void FT4LevelCollectionModel::OnSCCCheckIn()
{
	SCCCheckIn(GetSelectedLevels());
}

void FT4LevelCollectionModel::OnSCCOpenForAdd()
{
	SCCOpenForAdd(GetSelectedLevels());
}

void FT4LevelCollectionModel::OnSCCHistory()
{
	SCCHistory(GetSelectedLevels());
}

void FT4LevelCollectionModel::OnSCCRefresh()
{
	SCCRefresh(GetSelectedLevels());
}

void FT4LevelCollectionModel::OnSCCDiffAgainstDepot()
{
	SCCDiffAgainstDepot(GetSelectedLevels(), GEditor);
}

void FT4LevelCollectionModel::OnSCCConnect() const
{
	ISourceControlModule::Get().ShowLoginDialog(FSourceControlLoginClosed(), ELoginWindowMode::Modeless);
}

void FT4LevelCollectionModel::SaveSelectedLevels_Executed()
{
	SaveLevels(GetSelectedLevels(), true);
}

void FT4LevelCollectionModel::SaveSelectedLevelAs_Executed()
{
	if (SelectedLevelsList.Num() > 0)
	{
		ULevel* Level = SelectedLevelsList[0]->GetLevelObject();
		if (Level)
		{
			FEditorFileUtils::SaveLevelAs(Level);
		}
	}
}

void FT4LevelCollectionModel::MigrateSelectedLevels_Executed()
{
	// Gather the package names for the levels
	TArray<FName> PackageNames = GetPackageNamesList(GetSelectedLevels());
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
	AssetToolsModule.Get().MigratePackages(PackageNames);
}

void FT4LevelCollectionModel::SelectAllLevels_Executed()
{
	SetSelectedLevels(FilteredLevelsList);
}

void FT4LevelCollectionModel::DeselectAllLevels_Executed()
{
	FT4LevelModelList NoLevels;
	SetSelectedLevels(NoLevels);
}

void FT4LevelCollectionModel::InvertSelection_Executed()
{
	FT4LevelModelList InvertedLevels;

	for (auto It = FilteredLevelsList.CreateIterator(); It; ++It)
	{
		if (!SelectedLevelsList.Contains(*It))
		{
			InvertedLevels.Add(*It);
		}
	}

	SetSelectedLevels(InvertedLevels);
}

void FT4LevelCollectionModel::ShowSelectedLevels_Executed()
{
	ShowLevels(GetSelectedLevels());
}

void FT4LevelCollectionModel::HideSelectedLevels_Executed()
{
	HideLevels(GetSelectedLevels());
}

void FT4LevelCollectionModel::ShowOnlySelectedLevels_Executed()
{
	//stash off a copy of the original array, as setting visibility can destroy the selection
	FT4LevelModelList SelectedLevelsCopy = GetSelectedLevels();
	
	InvertSelection_Executed();
	HideSelectedLevels_Executed();
	SetSelectedLevels(SelectedLevelsCopy);
	ShowSelectedLevels_Executed();
}

void FT4LevelCollectionModel::ShowAllButSelectedLevels_Executed()
{
	//stash off a copy of the original array, as setting visibility can destroy the selection
	FT4LevelModelList SelectedLevelsCopy = GetSelectedLevels();

	InvertSelection_Executed();
	ShowSelectedLevels_Executed();
	SetSelectedLevels(SelectedLevelsCopy);
	HideSelectedLevels_Executed();
}


void FT4LevelCollectionModel::ShowAllLevels_Executed()
{
	ShowLevels(GetFilteredLevels());
}

void FT4LevelCollectionModel::HideAllLevels_Executed()
{
	HideLevels(GetFilteredLevels());
}

void FT4LevelCollectionModel::LockSelectedLevels_Executed()
{
	LockLevels(GetSelectedLevels());
}

void FT4LevelCollectionModel::UnlockSelectedLevels_Executed()
{
	UnlockLevels(GetSelectedLevels());
}

void FT4LevelCollectionModel::LockOnlySelectedLevels_Executed()
{
	//stash off a copy of the original array, as setting visibility can destroy the selection
	FT4LevelModelList SelectedLevelsCopy = GetSelectedLevels();

	InvertSelection_Executed();
	UnlockSelectedLevels_Executed();
	SetSelectedLevels(SelectedLevelsCopy);
	LockSelectedLevels_Executed();
}

void FT4LevelCollectionModel::LockAllButSelectedLevels_Executed()
{
	//stash off a copy of the original array, as setting visibility can destroy the selection
	FT4LevelModelList SelectedLevelsCopy = GetSelectedLevels();

	InvertSelection_Executed();
	LockSelectedLevels_Executed();
	SetSelectedLevels(SelectedLevelsCopy);
	UnlockSelectedLevels_Executed();
}

void FT4LevelCollectionModel::LockAllLevels_Executed()
{
	if (!IsReadOnly())
	{
		const FScopedTransaction Transaction(LOCTEXT("LockAllLevels", "Lock All Levels"));
		LockLevels(GetFilteredLevels());
	}
}

void FT4LevelCollectionModel::UnockAllLevels_Executed()
{
	if (!IsReadOnly())
	{
		const FScopedTransaction Transaction(LOCTEXT("UnlockAllLevels", "Unlock All Levels"));
		UnlockLevels(GetFilteredLevels());
	}
}

void FT4LevelCollectionModel::ToggleReadOnlyLevels_Executed()
{
	//We are about to lock some Levels, deselect all actor and surfaces from the read only levels
	if (!GEditor->bLockReadOnlyLevels)
	{
		DeselectActorsInAllReadOnlyLevel(GetAllLevels());
		DeselectSurfaceInAllReadOnlyLevel(GetAllLevels());
		// Tell the editor selection status was changed.
		GEditor->NoteSelectionChange();
	}

	GEditor->bLockReadOnlyLevels = !GEditor->bLockReadOnlyLevels;
}

void FT4LevelCollectionModel::ThumbnailSelectedLevels_Executed() // #84
{
	for (auto& LevelModel : SelectedLevelsList)
	{
		LevelModel->BroadcastChangedEvent();
	}
}

void FT4LevelCollectionModel::MakeLevelCurrent_Executed()
{
	check( SelectedLevelsList.Num() == 1 );
	SelectedLevelsList[0]->MakeLevelCurrent();
}

void FT4LevelCollectionModel::FindInContentBrowser_Executed()
{
	TArray<UObject*> Objects;
	for (auto It = SelectedLevelsList.CreateConstIterator(); It; ++It)
	{
		ULevel* Level = (*It)->GetLevelObject();
		if (Level)
		{
			UObject* LevelOuter = Level->GetOuter();
			if (LevelOuter)
			{
				// Search for the level's outer (the UWorld) as this is the actual asset shown by the content browser
				Objects.AddUnique(LevelOuter);
			}
		}
	}

	GEditor->SyncBrowserToObjects(Objects);
}

bool FT4LevelCollectionModel::IsValidFindInContentBrowser()
{
	return true;
}

void FT4LevelCollectionModel::MoveActorsToSelected_Executed()
{
	// If matinee is open, and if an actor being moved belongs to it, message the user
	if (GLevelEditorModeTools().IsModeActive(FBuiltinEditorModes::EM_InterpEdit))
	{
		const FEdModeInterpEdit* InterpEditMode = (const FEdModeInterpEdit*)GLevelEditorModeTools().GetActiveMode(FBuiltinEditorModes::EM_InterpEdit);
		if (InterpEditMode && InterpEditMode->MatineeActor)
		{
			TArray<AActor*> ControlledActors;
			InterpEditMode->MatineeActor->GetControlledActors(ControlledActors);

			// are any of the selected actors in the matinee
			USelection* SelectedActors = GEditor->GetSelectedActors();
			for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
			{
				AActor* Actor = CastChecked<AActor>(*Iter);
				if (Actor != nullptr && (Actor == InterpEditMode->MatineeActor || ControlledActors.Contains(Actor)))
				{
					const bool ExitInterp = EAppReturnType::Yes == FMessageDialog::Open(EAppMsgType::YesNo, NSLOCTEXT("UnrealEd", "MatineeUnableToMove", "You must close Matinee before moving actors.\nDo you wish to do this now and continue?"));
					if (!ExitInterp)
					{
						return;
					}
					GLevelEditorModeTools().DeactivateMode(FBuiltinEditorModes::EM_InterpEdit);
					break;
				}
			}
		}
	}

	MakeLevelCurrent_Executed();

	const FScopedTransaction Transaction(LOCTEXT("MoveSelectedActorsToSelectedLevel", "Move Selected Actors to Level"));

	// Redirect selected foliage actor to use the MoveActorFoliageInstancesToLevel functionality as we can't move the foliage actor only instances
	USelection* SelectedActors = GEditor->GetSelectedActors();
	for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
	{
		AInstancedFoliageActor* Actor = Cast<AInstancedFoliageActor>(*Iter);

		if (Actor != nullptr)
		{
			FFoliageEditUtility::MoveActorFoliageInstancesToLevel(GetWorld()->GetCurrentLevel());
		}
	}

	UEditorLevelUtils::MoveSelectedActorsToLevel(GetWorld()->GetCurrentLevel());

	RequestUpdateAllLevels();
}

void FT4LevelCollectionModel::MoveFoliageToSelected_Executed()
{
	if (GetSelectedLevels().Num() == 1)
	{
		ULevel* TargetLevel = GetSelectedLevels()[0]->GetLevelObject();

		// Need to only permit this action when the foliage mode is open as the selection is being done there
		if (GLevelEditorModeTools().IsModeActive(FBuiltinEditorModes::EM_Foliage))
		{
			IFoliageEditModule& FoliageModule = FModuleManager::GetModuleChecked<IFoliageEditModule>("FoliageEdit");
			FoliageModule.MoveSelectedFoliageToLevel(TargetLevel);
		}
	}
}

void FT4LevelCollectionModel::SelectActors_Executed()
{
	//first clear any existing actor selection
	const FScopedTransaction Transaction( LOCTEXT("SelectActors", "Select Actors in Level") );
	GEditor->GetSelectedActors()->Modify();
	GEditor->SelectNone( false, true );

	for(auto It = SelectedLevelsList.CreateConstIterator(); It; ++It)
	{
		(*It)->SelectActors(/*bSelect*/ true, /*bNotify*/ true, /*bSelectEvenIfHidden*/ true);
	}
}

void FT4LevelCollectionModel::DeselectActors_Executed()
{
	const FScopedTransaction Transaction( LOCTEXT("DeselectActors", "Deselect Actors in Level") );
	
	for(auto It = SelectedLevelsList.CreateConstIterator(); It; ++It)
	{
		(*It)->SelectActors(/*bSelect*/ false, /*bNotify*/ true, /*bSelectEvenIfHidden*/ true);
	}
}

void FT4LevelCollectionModel::ExpandSelectedItems_Executed()
{
	struct FExpandLevelVisitor : public FT4LevelModelVisitor
	{
		virtual void Visit(FT4LevelModel& Item) override { Item.SetLevelExpansionFlag(true); }
	} Expander;
	
	for (TSharedPtr<FT4LevelModel> LevelModel: SelectedLevelsList)
	{
		LevelModel->Accept(Expander);
	}

	BroadcastHierarchyChanged();
}

void FT4LevelCollectionModel::FillLockSubMenu(FMenuBuilder& InMenuBuilder)
{
	const FT4LevelCollectionCommands& Commands = FT4LevelCollectionCommands::Get();

	InMenuBuilder.AddMenuEntry( Commands.World_LockSelectedLevels );
	InMenuBuilder.AddMenuEntry( Commands.World_UnockSelectedLevels );
	InMenuBuilder.AddMenuEntry(Commands.World_LockOnlySelectedLevels);
	InMenuBuilder.AddMenuEntry(Commands.World_LockAllButSelectedLevels);
	InMenuBuilder.AddMenuEntry( Commands.World_LockAllLevels );
	InMenuBuilder.AddMenuEntry( Commands.World_UnockAllLevels );

	if (GEditor->bLockReadOnlyLevels)
	{
		InMenuBuilder.AddMenuEntry( Commands.World_UnlockReadOnlyLevels );
	}
	else
	{
		InMenuBuilder.AddMenuEntry( Commands.World_LockReadOnlyLevels );
	}
}

void FT4LevelCollectionModel::FillVisibilitySubMenu(FMenuBuilder& InMenuBuilder)
{
	const FT4LevelCollectionCommands& Commands = FT4LevelCollectionCommands::Get();

	InMenuBuilder.AddMenuEntry( Commands.World_ShowSelectedLevels );
	InMenuBuilder.AddMenuEntry( Commands.World_HideSelectedLevels );
	InMenuBuilder.AddMenuEntry( Commands.World_ShowOnlySelectedLevels );
	InMenuBuilder.AddMenuEntry(Commands.World_ShowAllButSelectedLevels);
	InMenuBuilder.AddMenuEntry( Commands.World_ShowAllLevels );
	InMenuBuilder.AddMenuEntry( Commands.World_HideAllLevels );
}

void FT4LevelCollectionModel::FillSourceControlSubMenu(FMenuBuilder& InMenuBuilder)
{
	const FT4LevelCollectionCommands& Commands = FT4LevelCollectionCommands::Get();
	
	if (CanExecuteSCC())
	{
		if (CanExecuteSCCCheckOut())
		{
			InMenuBuilder.AddMenuEntry( Commands.SCCCheckOut );
		}

		if (CanExecuteSCCOpenForAdd())
		{
			InMenuBuilder.AddMenuEntry( Commands.SCCOpenForAdd );
		}

		if (CanExecuteSCCCheckIn())
		{
			InMenuBuilder.AddMenuEntry( Commands.SCCCheckIn );
		}

		InMenuBuilder.AddMenuEntry( Commands.SCCRefresh );
		InMenuBuilder.AddMenuEntry( Commands.SCCHistory );
		InMenuBuilder.AddMenuEntry( Commands.SCCDiffAgainstDepot );
	}
	else
	{
		InMenuBuilder.AddMenuEntry( Commands.SCCConnect );
	}
}

void FT4LevelCollectionModel::LoadSelectedPreviewLevels_Executed() // #104
{
	for (TSharedPtr<FT4LevelModel> LevelModel : SelectedLevelsList)
	{
		if (LevelModel.IsValid())
		{
			PreviewLoadedLevelList.Add(LevelModel->GetLongPackageName());
		}
	}
	BroadcastPreviewLoadedLevelChanged();
}

void FT4LevelCollectionModel::UnloadSelectedPreviewLevels_Executed() // #104
{
	for (TSharedPtr<FT4LevelModel> LevelModel : SelectedLevelsList)
	{
		if (LevelModel.IsValid())
		{
			PreviewLoadedLevelList.Remove(LevelModel->GetLongPackageName());
		}
	}
	BroadcastPreviewLoadedLevelChanged();
}

bool FT4LevelCollectionModel::AreAnySelectedPreviewLevelsLoaded() const // #104
{
	for (int32 LevelIdx = 0; LevelIdx < SelectedLevelsList.Num(); LevelIdx++)
	{
		if (PreviewLoadedLevelList.Contains(SelectedLevelsList[LevelIdx]->GetLongPackageName()))
		{
			return true;
		}
	}

	return false;
}

bool FT4LevelCollectionModel::AreAnySelectedPreviewLevelsUnloaded() const // #104
{
	for (int32 LevelIdx = 0; LevelIdx < SelectedLevelsList.Num(); LevelIdx++)
	{
		if (!PreviewLoadedLevelList.Contains(SelectedLevelsList[LevelIdx]->GetLongPackageName()))
		{
			return true;
		}
	}

	return false;
}

void FT4LevelCollectionModel::DeselectActorsInAllReadOnlyLevel(const FT4LevelModelList& InLevelList)
{
	const FScopedTransaction Transaction(LOCTEXT("DeselectActorsInReadOnlyLevel", "Deselect Actors in all read only Level"));

	for (auto It = InLevelList.CreateConstIterator(); It; ++It)
	{
		if ((*It)->IsFileReadOnly())
		{
			(*It)->SelectActors(/*bSelect*/ false, /*bNotify*/ true, /*bSelectEvenIfHidden*/ true);
		}
	}
}

void FT4LevelCollectionModel::DeselectSurfaceInAllReadOnlyLevel(const FT4LevelModelList& InLevelList)
{
	const FScopedTransaction Transaction(LOCTEXT("DeselectSurfacesInReadOnlyLevel", "Deselect Surfaces in all read only Level"));

	for (auto It = InLevelList.CreateConstIterator(); It; ++It)
	{
		if ((*It)->IsFileReadOnly())
		{
			(*It)->DeselectAllSurfaces();
		}
	}
}


void FT4LevelCollectionModel::OnLevelsCollectionChanged()
{
	UpdateAllLevels();
	PopulateFilteredLevelsList();

	BroadcastCollectionChanged();
}

void FT4LevelCollectionModel::OnLevelsSelectionChanged()
{
	if (bUpdatingLevelsSelection)
	{
		return;
	}
	
	TGuardValue<bool> UpdateGuard(bUpdatingLevelsSelection, true);
	
	// Pass the list we just created to the world to set the selection
	CurrentWorld->SetSelectedLevels(
		GetLevelObjectList(SelectedLevelsList)
	);

	// Request SC status update for selected levels
	ISourceControlModule::Get().QueueStatusUpdate(
		GetFilenamesList(SelectedLevelsList)
	);

	// Expand hierarchy to selected levels 
	for (auto It = SelectedLevelsList.CreateIterator(); It; ++It)
	{
		TSharedPtr<FT4LevelModel> ParentLevelModel = (*It)->GetParent();
		while (ParentLevelModel.IsValid())
		{
			ParentLevelModel->SetLevelExpansionFlag(true);
			ParentLevelModel = ParentLevelModel->GetParent();
		}
	}
		
	BroadcastSelectionChanged();
}

void FT4LevelCollectionModel::OnLevelsSelectionChangedOutside()
{
	if (!bUpdatingLevelsSelection)
	{
		SetSelectedLevelsFromWorld();
	}
}

void FT4LevelCollectionModel::OnLevelsHierarchyChanged()
{
	BroadcastHierarchyChanged();
}

void FT4LevelCollectionModel::OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld)
{
	if (InWorld == GetWorld())
	{
		TSharedPtr<FT4LevelModel> LevelModel = FindLevelModel(InLevel->GetOutermost()->GetFName());
		if (LevelModel.IsValid())
		{
			LevelModel->OnLevelAddedToWorld(InLevel);
			BroadcastEditorLoadedLevelChanged(); // #104
		}
	}
}

void FT4LevelCollectionModel::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	if (InWorld == GetWorld())
	{
		TSharedPtr<FT4LevelModel> LevelModel = FindLevelModel(InLevel->GetOutermost()->GetFName());
		if (LevelModel.IsValid())
		{
			LevelModel->OnLevelRemovedFromWorld();
			BroadcastEditorLoadedLevelChanged(); // #104
		}
	}
}

void FT4LevelCollectionModel::OnRedrawAllViewports()
{
	if (GShaderCompilingManager && GShaderCompilingManager->IsCompiling())
	{
		// Editor seems like still compiling shaders, do not request tiles redraw until all shaders complation is finished
		// Basically redraw only on last event
		return;
	}
	
	RequestRedrawAllLevels();
}

void FT4LevelCollectionModel::OnLevelActorAdded(AActor* InActor)
{
	if (InActor && 
		InActor->GetWorld() == CurrentWorld.Get()) // we care about our world only
	{
		bRequestedUpdateActorsCount = true;
	}
}

void FT4LevelCollectionModel::OnLevelActorDeleted(AActor* InActor)
{
	bRequestedUpdateActorsCount = true;
}

void FT4LevelCollectionModel::OnFilterChanged()
{
	PopulateFilteredLevelsList();
	BroadcastCollectionChanged();
}

void FT4LevelCollectionModel::CacheCanExecuteSourceControlVars() const
{
	bCanExecuteSCCCheckOut = false;
	bCanExecuteSCCOpenForAdd = false;
	bCanExecuteSCCCheckIn = false;
	bCanExecuteSCC = false;

	ISourceControlProvider& SourceControlProvider = ISourceControlModule::Get().GetProvider();
	for (auto It = SelectedLevelsList.CreateConstIterator(); It; ++It)
	{
		if (ISourceControlModule::Get().IsEnabled() && SourceControlProvider.IsAvailable())
		{
			bCanExecuteSCC = true;
			
			ULevel* Level = (*It)->GetLevelObject();
			if (Level)
			{
				// Check the SCC state for each package in the selected paths
				FSourceControlStatePtr SourceControlState = SourceControlProvider.GetState(Level->GetOutermost(), EStateCacheUsage::Use);

				if (SourceControlState.IsValid())
				{
					if (SourceControlState->CanCheckout())
					{
						bCanExecuteSCCCheckOut = true;
					}
					else if (!SourceControlState->IsSourceControlled())
					{
						bCanExecuteSCCOpenForAdd = true;
					}
					else if (SourceControlState->IsCheckedOut() || SourceControlState->IsAdded())
					{
						bCanExecuteSCCCheckIn = true;
					}
				}
			}
		}

		if (bCanExecuteSCCCheckOut && 
			bCanExecuteSCCOpenForAdd && 
			bCanExecuteSCCCheckIn)
		{
			// All options are available, no need to keep iterating
			break;
		}
	}
}

bool FT4LevelCollectionModel::IsValidMoveActorsToLevel() const
{
	static bool bCachedIsValidActorMoveResult = false;
	if (bSelectionHasChanged)
	{
		bSelectionHasChanged = false;
		bCachedIsValidActorMoveResult = false;

		// We can only operate on a single selected level
		if ( SelectedLevelsList.Num() == 1 )
		{
			ULevel* Level = SelectedLevelsList[0]->GetLevelObject();
			if (Level)
			{
				// Allow the move if at least one actor is in another level
				USelection* SelectedActors = GEditor->GetSelectedActors();
				for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
				{
					AActor* Actor = CastChecked<AActor>(*Iter);
					if (Actor != nullptr)
					{
						if (Actor->GetLevel() != Level)
						{
							bCachedIsValidActorMoveResult = true;
							break;
						}
					}
				}
			}
		}
	}
			
	// if non of the selected actors are in the level, just check the level is unlocked
	return bCachedIsValidActorMoveResult && AreAllSelectedLevelsEditableAndVisible();
}

bool FT4LevelCollectionModel::IsValidMoveFoliageToLevel() const
{
	if (IsOneLevelSelected() && 
		AreAllSelectedLevelsEditableAndVisible() && 
		GLevelEditorModeTools().IsModeActive(FBuiltinEditorModes::EM_Foliage))
	{
		IFoliageEditModule& FoliageModule = FModuleManager::GetModuleChecked<IFoliageEditModule>("FoliageEdit");
		ULevel* TargetLevel = GetSelectedLevels()[0]->GetLevelObject();

		return FoliageModule.CanMoveSelectedFoliageToLevel(TargetLevel);
	}

	return false;
}

void FT4LevelCollectionModel::OnActorSelectionChanged(UObject* obj)
{
	OnActorOrLevelSelectionChanged();
}

void FT4LevelCollectionModel::OnActorOrLevelSelectionChanged()
{
	bSelectionHasChanged = true;
}


#undef LOCTEXT_NAMESPACE
