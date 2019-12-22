// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WorldTileModel.h"
#include "T4LevelCollectionModel.h"

#include "Products/T4RehearsalEditorUtils.h" // #91

#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "UObject/Package.h"
#include "Engine/LevelBounds.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Editor.h"
#include "ScopedTransaction.h"
#include "EditorLevelUtils.h"

#include "LevelDragDropOp.h"

#include "T4WorldTileDetails.h"
#include "T4WorldTileCollectionModel.h"
#include "Engine/WorldComposition.h"
#include "GameFramework/WorldSettings.h"
#include "LandscapeInfo.h"
#include "LandscapeEditorModule.h"
#include "LandscapeFileFormatInterface.h"
#include "LandscapeStreamingProxy.h"
#include "Landscape.h"


#define LOCTEXT_NAMESPACE "WorldMap"
DEFINE_LOG_CATEGORY_STATIC(WorldBrowser, Log, All);

FT4WorldTileModel::FT4WorldTileModel(FT4WorldTileCollectionModel& InWorldModel, int32 InTileIdx)
	: FT4LevelModel(InWorldModel) 
	, TileIdx(InTileIdx)
	, TileDetails(NULL)
	, bWasShelved(false)
{
	UWorldComposition* WorldComposition = LevelCollectionModel.GetWorld()->WorldComposition;

	// Tile display details object
	TileDetails = NewObject<UT4WorldTileDetails>(GetTransientPackage(), NAME_None, RF_Transient|RF_Transactional);
	TileDetails->AddToRoot();

	// Subscribe to tile properties changes
	// Un-subscribe in dtor if new is added!
	TileDetails->PostUndoEvent.AddRaw(this, &FT4WorldTileModel::OnPostUndoEvent);
	TileDetails->PositionChangedEvent.AddRaw(this, &FT4WorldTileModel::OnPositionPropertyChanged);
	TileDetails->ParentPackageNameChangedEvent.AddRaw(this, &FT4WorldTileModel::OnParentPackageNamePropertyChanged);
	TileDetails->LODSettingsChangedEvent.AddRaw(this, &FT4WorldTileModel::OnLODSettingsPropertyChanged);
	TileDetails->ZOrderChangedEvent.AddRaw(this, &FT4WorldTileModel::OnZOrderPropertyChanged);
	TileDetails->HideInTileViewChangedEvent.AddRaw(this, &FT4WorldTileModel::OnHideInTileViewChanged);
			
	if (nullptr == WorldComposition) // #91 : World Single
	{
		check(INDEX_NONE == InTileIdx);
		LoadedLevel = LevelCollectionModel.GetWorld()->PersistentLevel;
		TileDetails->PackageName = LevelCollectionModel.GetWorld()->GetOutermost()->GetFName();

		SetAssetName(TileDetails->PackageName);
		//FString AssetNameString = FString::Printf(TEXT("%s %s.TheWorld:PersistentLevel"), *ULevel::StaticClass()->GetName(), *Tile.PackageName.ToString());
		FString AssetNameString = FString::Printf(TEXT("%s %s"), *UPackage::StaticClass()->GetName(), *TileDetails->PackageName.ToString());
		AssetName = FName(*AssetNameString);

		TileDetails->bPersistentLevel = false;
		TileDetails->bTileEditable = true; // Enable tile properties
		{
#if 0
			EnsureLevelHasBoundsActor();
#else
			if (LoadedLevel.IsValid() && !LoadedLevel->LevelBoundsActor.IsValid())
			{
				FActorSpawnParameters SpawnParameters;
				SpawnParameters.ObjectFlags |= RF_Transient;	// World Singe 은 LevelBound 를 저장할 필요가 없다.
				SpawnParameters.OverrideLevel = LoadedLevel.Get();
				ALevelBounds* SpawnedActor = LevelCollectionModel.GetWorld()->SpawnActor<ALevelBounds>(SpawnParameters);
				check(nullptr != SpawnedActor);
			}
#endif
			LoadedLevel.Get()->LevelBoundsActorUpdated().AddRaw(this, &FT4WorldTileModel::OnLevelBoundsActorUpdated);
		}
		return;
	}

	// Initialize tile details
	if (WorldComposition->GetTilesList().IsValidIndex(TileIdx))
	{
		FWorldCompositionTile& Tile = WorldComposition->GetTilesList()[TileIdx];
		
		TileDetails->PackageName = Tile.PackageName;
		TileDetails->bPersistentLevel = false;
				
		// Asset name for storing tile thumbnail inside package
		SetAssetName(Tile.PackageName);
		//FString AssetNameString = FString::Printf(TEXT("%s %s.TheWorld:PersistentLevel"), *ULevel::StaticClass()->GetName(), *Tile.PackageName.ToString());
		FString AssetNameString = FString::Printf(TEXT("%s %s"), *UPackage::StaticClass()->GetName(), *Tile.PackageName.ToString());
		AssetName =	FName(*AssetNameString);
	
		// Assign level object in case this level already loaded
		UPackage* LevelPackage = Cast<UPackage>(StaticFindObjectFast( UPackage::StaticClass(), NULL, Tile.PackageName) );
		if (LevelPackage)
		{
			// Find the world object
			UWorld* World = UWorld::FindWorldInPackage(LevelPackage);
			if (World)
			{
				LoadedLevel = World->PersistentLevel;
				// Enable tile properties
				TileDetails->bTileEditable = true;
				if (World->PersistentLevel->bIsVisible)
				{
					LoadedLevel.Get()->LevelBoundsActorUpdated().AddRaw(this, &FT4WorldTileModel::OnLevelBoundsActorUpdated);
				}
			}
		}

		TileDetails->SetInfo(Tile.Info, LoadedLevel.Get());
	}
	else
	{
		TileDetails->PackageName = LevelCollectionModel.GetWorld()->GetOutermost()->GetFName();
		TileDetails->bPersistentLevel = true;
		LoadedLevel = LevelCollectionModel.GetWorld()->PersistentLevel;
	}
}

FT4WorldTileModel::~FT4WorldTileModel()
{
	if (TileDetails)
	{
		TileDetails->PostUndoEvent.RemoveAll(this);
		TileDetails->PositionChangedEvent.RemoveAll(this);
		TileDetails->ParentPackageNameChangedEvent.RemoveAll(this);
		TileDetails->LODSettingsChangedEvent.RemoveAll(this);
		TileDetails->ZOrderChangedEvent.RemoveAll(this);
		TileDetails->HideInTileViewChangedEvent.RemoveAll(this);
				
		TileDetails->RemoveFromRoot();
		TileDetails->MarkPendingKill();
	}

	if (LoadedLevel.IsValid())
	{
		LoadedLevel.Get()->LevelBoundsActorUpdated().RemoveAll(this);
	}
}

UObject* FT4WorldTileModel::GetNodeObject()
{
	// this pointer is used as unique key in SNodePanel
	return TileDetails;
}

ULevel* FT4WorldTileModel::GetLevelObject() const
{
	return LoadedLevel.Get();
}

bool FT4WorldTileModel::IsRootTile() const
{
	return TileDetails->bPersistentLevel;
}

void FT4WorldTileModel::SetAssetName(const FName& PackageName)
{
	FString AssetNameString = FString::Printf(TEXT("%s %s"), *UPackage::StaticClass()->GetName(), *PackageName.ToString());
	AssetName = FName(*AssetNameString);
}

FName FT4WorldTileModel::GetAssetName() const
{
	return AssetName;
}

FName FT4WorldTileModel::GetLongPackageName() const
{
	return TileDetails->PackageName;
}

void FT4WorldTileModel::UpdateAsset(const FAssetData& AssetData)
{
	check(TileDetails != nullptr);
	const FName OldPackageName = TileDetails->PackageName;

	// Patch up any parent references which have been renamed
	for (const TSharedPtr<FT4LevelModel>& LevelModel : LevelCollectionModel.GetAllLevels())
	{
		TSharedPtr<FT4WorldTileModel> WorldTileModel = StaticCastSharedPtr<FT4WorldTileModel>(LevelModel);

		check(WorldTileModel->TileDetails != nullptr);
		if (WorldTileModel->TileDetails->ParentPackageName == OldPackageName)
		{
			WorldTileModel->TileDetails->ParentPackageName = AssetData.PackageName;
		}
	}

	const FName PackageName = AssetData.PackageName;
	SetAssetName(PackageName);
	TileDetails->PackageName = PackageName;
}

FVector2D FT4WorldTileModel::GetLevelPosition2D() const
{
	if (TileDetails->Bounds.IsValid && !TileDetails->bHideInTileView)
	{
		FVector LevelPosition = GetLevelCurrentPosition();
		FVector2D LevelPosition2D = FVector2D(LevelPosition);
		return LevelPosition2D - FVector2D(TileDetails->Bounds.GetExtent()) + GetLevelTranslationDelta();
	}

	return FVector2D(0, 0);
}

FVector2D FT4WorldTileModel::GetLevelSize2D() const
{
	if (TileDetails->Bounds.IsValid && !TileDetails->bHideInTileView)
	{
		FVector LevelSize = TileDetails->Bounds.GetSize();
		return FVector2D(LevelSize.X, LevelSize.Y);
	}
	
	return FVector2D(-1, -1);
}

void FT4WorldTileModel::OnDrop(const TSharedPtr<FLevelDragDropOp>& Op)
{
	FT4LevelModelList LevelModelList;

	for (auto It = Op->LevelsToDrop.CreateConstIterator(); It; ++It)
	{
		ULevel* Level = (*It).Get();
		TSharedPtr<FT4LevelModel> LevelModel = LevelCollectionModel.FindLevelModel(Level);
		if (LevelModel.IsValid())
		{
			LevelModelList.Add(LevelModel);
		}
	}	
	
	if (LevelModelList.Num())
	{
		const FScopedTransaction AssignParentTransaction(LOCTEXT("AssignParentTransaction", "Assign Parent Level"));
		LevelCollectionModel.AssignParent(LevelModelList, this->AsShared());
	}
}

bool FT4WorldTileModel::IsGoodToDrop(const TSharedPtr<FLevelDragDropOp>& Op) const
{
	return true;
}

bool FT4WorldTileModel::ShouldBeVisible(FBox EditableArea) const
{
	if (IsRootTile())
	{
		return true;
	}

	// Visibility does not depend on level positions when world origin rebasing is disabled
	if (!LevelCollectionModel.IsOriginRebasingEnabled())
	{
		return true;
	}

	// When this hack is activated level should be visible regardless of current world origin
	if (LevelCollectionModel.GetWorld()->WorldComposition->bTemporallyDisableOriginTracking)
	{
		return true;
	}
	
	FBox LevelBBox = GetLevelBounds();

	// Visible if level has no valid bounds
	if (!LevelBBox.IsValid)
	{
		return true;
	}

	// Visible if level bounds inside editable area
	if (EditableArea.IsInsideXY(LevelBBox))
	{
		return true;
	}

	// Visible if level bounds intersects editable area
	if (LevelBBox.IntersectXY(EditableArea))
	{
		return true;
	}

	return false;
}

void FT4WorldTileModel::SetVisible(bool bVisible)
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
	
	// Don't create unnecessary transactions
	if (IsVisible() == bVisible)
	{
		return;
	}

	// Can not show level outside of editable area
	if (bVisible && !ShouldBeVisible(LevelCollectionModel.EditableWorldArea()))
	{
		return;
	}
	
	// The level is no longer shelved
	bWasShelved = false;
	
	{
		FScopedTransaction Transaction( LOCTEXT("ToggleVisibility", "Toggle Level Visibility") );
		
		// This call hides/shows all owned actors, etc
		EditorLevelUtils::SetLevelVisibility( Level, bVisible, true );	// we need to enable layers too here so the LODs export correctly
			
		// Ensure operation is completed succesfully
		check(GetLevelObject()->bIsVisible == bVisible);

		// Now there is no way to correctly undo level visibility
		// remove ability to undo this operation
		Transaction.Cancel();
	}
}

bool FT4WorldTileModel::IsShelved() const
{
	return (GetLevelObject() == NULL || bWasShelved);
}

void FT4WorldTileModel::Shelve()
{
	if (LevelCollectionModel.IsReadOnly() || IsShelved() || IsRootTile() || !LevelCollectionModel.IsOriginRebasingEnabled())
	{
		return;
	}
	
	//
	SetVisible(false);
	bWasShelved = true;
}

void FT4WorldTileModel::Unshelve()
{
	if (LevelCollectionModel.IsReadOnly() || !IsShelved())
	{
		return;
	}

	//
	SetVisible(true);
	bWasShelved = false;
}

bool FT4WorldTileModel::IsLandscapeBased() const
{
	return Landscape.IsValid();
}

bool FT4WorldTileModel::IsTiledLandscapeBased() const
{
	if (IsLandscapeBased() && !GetLandscape()->ReimportHeightmapFilePath.IsEmpty())
	{
		// Check if single landscape actor resolution matches heightmap file size
		ILandscapeEditorModule& LandscapeEditorModule = FModuleManager::GetModuleChecked<ILandscapeEditorModule>("LandscapeEditor");
		const FString TargetExtension = FPaths::GetExtension(GetLandscape()->ReimportHeightmapFilePath, true);
		const ILandscapeHeightmapFileFormat* HeightmapFormat = LandscapeEditorModule.GetHeightmapFormatByExtension(*TargetExtension);

		FLandscapeHeightmapInfo HeightmapInfo = HeightmapFormat->Validate(*GetLandscape()->ReimportHeightmapFilePath);
		if (HeightmapInfo.ResultCode != ELandscapeImportResult::Error)
		{
			FIntRect ComponentsRect = GetLandscape()->GetBoundingRect();

			for (FLandscapeFileResolution& PossibleResolution : HeightmapInfo.PossibleResolutions)
			{
				if ((PossibleResolution.Width == (ComponentsRect.Width() + 1)) && (PossibleResolution.Height == (ComponentsRect.Height() + 1)))
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool FT4WorldTileModel::IsLandscapeProxy() const
{
	return (Landscape.IsValid() && Landscape.Get()->IsA(ALandscapeStreamingProxy::StaticClass()));
}

bool FT4WorldTileModel::IsInLayersList(const TArray<FWorldTileLayer>& InLayerList) const
{
	if (InLayerList.Num() > 0)
	{
		return InLayerList.Contains(TileDetails->Layer);
	}
	
	return true;
}

ALandscapeProxy* FT4WorldTileModel::GetLandscape() const
{
	return Landscape.Get();
}

void FT4WorldTileModel::AssignToLayer(const FWorldTileLayer& InLayer)
{
	if (LevelCollectionModel.IsReadOnly())
	{
		return;
	}
	
	if (!IsRootTile() && IsLoaded())
	{
		TileDetails->Layer = InLayer;
		OnLevelInfoUpdated();
	}
}

FBox FT4WorldTileModel::GetLevelBounds() const
{
	// Level local bounding box
	FBox Bounds = TileDetails->Bounds;

	if (Bounds.IsValid)
	{
		// Current level position in the world
		FVector LevelPosition = GetLevelCurrentPosition();
		FVector LevelExtent = Bounds.GetExtent();
		// Calculate bounding box in world space
		Bounds.Min = LevelPosition - LevelExtent;
		Bounds.Max = LevelPosition + LevelExtent;
	}
	
	return Bounds;
}

FIntVector FT4WorldTileModel::CalcAbsoluteLevelPosition() const
{
	TSharedPtr<FT4WorldTileModel> ParentModel = StaticCastSharedPtr<FT4WorldTileModel>(GetParent());
	if (ParentModel.IsValid())
	{
		return TileDetails->Position + ParentModel->CalcAbsoluteLevelPosition();
	}

	return IsRootTile() ? FIntVector::ZeroValue : TileDetails->Position;
}

FIntVector FT4WorldTileModel::GetAbsoluteLevelPosition() const
{
	return IsRootTile() ? FIntVector::ZeroValue : TileDetails->AbsolutePosition;
}
	
FIntVector FT4WorldTileModel::GetRelativeLevelPosition() const
{
	return IsRootTile() ? FIntVector::ZeroValue : TileDetails->Position;
}

FVector FT4WorldTileModel::GetLevelCurrentPosition() const
{
	if (TileDetails->Bounds.IsValid)
	{
		UWorld* CurrentWorld = (LevelCollectionModel.IsSimulating() ? LevelCollectionModel.GetSimulationWorld() : LevelCollectionModel.GetWorld());

		FVector LevelLocalPosition(TileDetails->Bounds.GetCenter());
		FIntVector LevelOffset = GetAbsoluteLevelPosition();
			
		return LevelLocalPosition + FVector(LevelOffset - CurrentWorld->OriginLocation); 
	}

	return FVector::ZeroVector;
}

void FT4WorldTileModel::SetLevelPosition(const FIntVector& InPosition, const FIntPoint* InLandscapeSectionOffset)
{
	// Parent absolute position
	TSharedPtr<FT4WorldTileModel> ParentModel = StaticCastSharedPtr<FT4WorldTileModel>(GetParent());
	FIntVector ParentAbsolutePostion = ParentModel.IsValid() ? ParentModel->GetAbsoluteLevelPosition() : FIntVector::ZeroValue;

	// Actual offset
	FIntVector Offset = InPosition - TileDetails->AbsolutePosition;
	FIntPoint LandscapeOffset = InLandscapeSectionOffset ? *InLandscapeSectionOffset : FIntPoint(Offset.X, Offset.Y);

	TileDetails->Modify();

	// Update absolute position
	TileDetails->AbsolutePosition = InPosition;

	// Assign new position as relative to parent
	TileDetails->Position = TileDetails->AbsolutePosition - ParentAbsolutePostion;

	// Flush changes to level package
	OnLevelInfoUpdated();

	// Move actors if necessary
	ULevel* Level = GetLevelObject();
	if (Level != nullptr && Level->bIsVisible)
	{
		// Shelve level, if during this translation level will end up out of Editable area
		if (!ShouldBeVisible(LevelCollectionModel.EditableWorldArea()))
		{
			Shelve();
		}

		// Move actors
		if (Offset != FIntVector::ZeroValue)
		{
			Level->ApplyWorldOffset(FVector(Offset), false);

			for (AActor* Actor : Level->Actors)
			{
				if (Actor != nullptr)
				{
					GEditor->BroadcastOnActorMoved(Actor);
				}
			}
		}
	}

	if (IsLandscapeBased())
	{
		UpdateLandscapeSectionsOffset(LandscapeOffset); // section offset is 2D 
		bool bShowWarnings = true;
		ULandscapeInfo::RecreateLandscapeInfo(LevelCollectionModel.GetWorld(), bShowWarnings);
	}

	// Transform child levels
	for (auto It = AllChildren.CreateIterator(); It; ++It)
	{
		TSharedPtr<FT4WorldTileModel> ChildModel = StaticCastSharedPtr<FT4WorldTileModel>(*It);
		FIntVector ChildPosition = TileDetails->AbsolutePosition + ChildModel->GetRelativeLevelPosition();
		ChildModel->SetLevelPosition(ChildPosition);
	}
}

void FT4WorldTileModel::UpdateLandscapeSectionsOffset(FIntPoint LevelOffset)
{
	ALandscapeProxy* LandscapeProxy = GetLandscape();
	if (LandscapeProxy)
	{
		// Calculate new section coordinates for landscape
		FVector	DrawScale = LandscapeProxy->GetRootComponent()->GetRelativeScale3D();
		FIntPoint QuadsSpaceOffset;
		QuadsSpaceOffset.X = FMath::RoundToInt(LevelOffset.X / DrawScale.X);
		QuadsSpaceOffset.Y = FMath::RoundToInt(LevelOffset.Y / DrawScale.Y);
		LandscapeProxy->SetAbsoluteSectionBase(QuadsSpaceOffset + LandscapeProxy->LandscapeSectionOffset);
	}
}

void FT4WorldTileModel::Update()
{
	if (!IsRootTile())
	{
		Landscape = NULL;

		ULevel* Level = GetLevelObject();

		// #91 : World Single
		bool bWorldCompositionEnabled = (nullptr != LevelCollectionModel.GetWorld()->WorldComposition) ? true : false;
		if (bWorldCompositionEnabled)
		{
			// Receive tile info from world composition
			FWorldTileInfo Info = LevelCollectionModel.GetWorld()->WorldComposition->GetTileInfo(TileDetails->PackageName);
			TileDetails->SetInfo(Info, Level);
		}

		if (Level != nullptr && Level->bIsVisible)
		{
			if (Level->LevelBoundsActor.IsValid())
			{
				if (!bWorldCompositionEnabled) // #91
				{
					// WARN : World Sinle 임으로 필요한 Actor의 Bound 만 계산하도록 처리
					TileDetails->Bounds = T4EditorUtil::CalculateLevelBounds(Level);
				}
				else
				{
					TileDetails->Bounds = Level->LevelBoundsActor.Get()->GetComponentsBoundingBox();
				}
			}

			// True level bounds without offsets applied
			if (TileDetails->Bounds.IsValid)
			{
				FBox LevelWorldBounds = TileDetails->Bounds;
				FIntVector LevelAbolutePosition = GetAbsoluteLevelPosition();
				FIntVector LevelOffset = LevelAbolutePosition - LevelCollectionModel.GetWorld()->OriginLocation;

				TileDetails->Bounds = LevelWorldBounds.ShiftBy(-FVector(LevelOffset));
			}

			OnLevelInfoUpdated();

			// Cache landscape information
			for (int32 ActorIndex = 0; ActorIndex < Level->Actors.Num(); ++ActorIndex)
			{
				AActor* Actor = Level->Actors[ActorIndex];
				if (Actor)
				{
					ALandscapeProxy* LandscapeActor = Cast<ALandscapeProxy>(Actor);
					if (LandscapeActor)
					{
						Landscape = LandscapeActor;
						break;
					}
				}
			}
		}
	}

	FT4LevelModel::Update();
}

void FT4WorldTileModel::LoadLevel()
{
	// Level is currently loading or has been loaded already
	if (bLoadingLevel || LoadedLevel.IsValid())
	{
		return;
	}

	// Create transient level streaming object and add to persistent level
	ULevelStreaming* LevelStreaming = GetAssociatedStreamingLevel();
	// should be clean level streaming object here
	check(LevelStreaming && LevelStreaming->GetLoadedLevel() == nullptr);
	
	bLoadingLevel = true;

	// Load level package 
	{
		FName LevelPackageName = LevelStreaming->GetWorldAssetPackageFName();
		
		ULevel::StreamedLevelsOwningWorld.Add(LevelPackageName, LevelCollectionModel.GetWorld());
		UWorld::WorldTypePreLoadMap.FindOrAdd(LevelPackageName) = LevelCollectionModel.GetWorld()->WorldType;

		UPackage* LevelPackage = LoadPackage(nullptr, *LevelPackageName.ToString(), LOAD_None);

		ULevel::StreamedLevelsOwningWorld.Remove(LevelPackageName);
		UWorld::WorldTypePreLoadMap.Remove(LevelPackageName);

		// Find world object and use its PersistentLevel pointer.
		UWorld* LevelWorld = UWorld::FindWorldInPackage(LevelPackage);
		// Check for a redirector. Follow it, if found.
		if (LevelWorld == nullptr)
		{
			LevelWorld = UWorld::FollowWorldRedirectorInPackage(LevelPackage);
		}

		if (LevelWorld && LevelWorld->PersistentLevel)
		{
			// LevelStreaming is transient object so world composition stores color in ULevel object
			LevelStreaming->LevelColor = LevelWorld->PersistentLevel->LevelColor;
		}
	}

	// Our level package should be loaded at this point, so level streaming will find it in memory
	LevelStreaming->SetShouldBeLoaded(true);
	LevelStreaming->SetShouldBeVisible(false); // Should be always false in the Editor
	LevelStreaming->SetShouldBeVisibleInEditor(false);
	LevelCollectionModel.GetWorld()->FlushLevelStreaming();

	LoadedLevel = LevelStreaming->GetLoadedLevel();
	
	bWasShelved = false;
	// Bring level to world
	if (LoadedLevel.IsValid())
	{
		// SetLevelVisibility will attempt to mark level as dirty for Undo purposes 
		// We don't want to undo sub-level loading operation, and in general loading sub-level should not make it Dirty
		FT4UnmodifiableObject ImmuneLevel(LoadedLevel.Get());

		// Whether this tile should be made visible at current world bounds
		const bool bShouldBeVisible = ShouldBeVisible(LevelCollectionModel.EditableWorldArea());
		EditorLevelUtils::SetLevelVisibility(LoadedLevel.Get(), bShouldBeVisible, true);
		
		// Mark tile as shelved in case it is hidden(does not fit to world bounds)
		bWasShelved = !bShouldBeVisible;
	}

	bLoadingLevel = false;
	
	// Enable tile properties
	TileDetails->bTileEditable = LoadedLevel.IsValid();

	if ((GEditor != nullptr) && (GEditor->Trans != nullptr))
	{
		GEditor->Trans->Reset(LOCTEXT("Loaded", "Discard undo history."));
	}
}

ULevelStreaming* FT4WorldTileModel::GetAssociatedStreamingLevel()
{
	FName PackageName = TileDetails->PackageName;
	UWorld* PersistentWorld = LevelCollectionModel.GetWorld();
				
	// Try to find existing object first
	auto Predicate = [&](ULevelStreaming* StreamingLevel) 
	{
		return (StreamingLevel && StreamingLevel->GetWorldAssetPackageFName() == PackageName && StreamingLevel->HasAnyFlags(RF_Transient));
	};

	ULevelStreaming* AssociatedStreamingLevel = nullptr;

	if (ULevelStreaming*const* FoundStreamingLevel = PersistentWorld->GetStreamingLevels().FindByPredicate(Predicate))
	{
		AssociatedStreamingLevel = *FoundStreamingLevel;
	}
	
	if (AssociatedStreamingLevel == nullptr)
	{
		// Create new streaming level
		AssociatedStreamingLevel = NewObject<ULevelStreamingDynamic>(PersistentWorld, NAME_None, RF_Transient);

		AssociatedStreamingLevel->SetWorldAssetByPackageName(PackageName);
		AssociatedStreamingLevel->LevelColor		= GetLevelColor();
		AssociatedStreamingLevel->LevelTransform	= FTransform::Identity;
		AssociatedStreamingLevel->PackageNameToLoad	= PackageName;

		PersistentWorld->AddStreamingLevel(AssociatedStreamingLevel);
	}

	return AssociatedStreamingLevel;
}

void FT4WorldTileModel::OnLevelAddedToWorld(ULevel* InLevel)
{
	if (!LoadedLevel.IsValid())
	{
		LoadedLevel = InLevel;
	}
		
	FT4LevelModel::OnLevelAddedToWorld(InLevel);

	EnsureLevelHasBoundsActor();

	// Manually call Update to make sure WorldTileModel is properly initialized (don't rely on Level Bounds changes as it will not be called if bAutoUpdateBounds is set to false).
	Update();

	LoadedLevel.Get()->LevelBoundsActorUpdated().AddRaw(this, &FT4WorldTileModel::OnLevelBoundsActorUpdated);
}

void FT4WorldTileModel::OnLevelRemovedFromWorld()
{
	FT4LevelModel::OnLevelRemovedFromWorld();

	ULevel* ThisLevel = LoadedLevel.Get();
	if (ThisLevel)
	{
		ThisLevel->LevelBoundsActorUpdated().RemoveAll(this);
	}
}

void FT4WorldTileModel::OnParentChanged()
{
	TileDetails->Modify();

	// Transform level offset to absolute
	TileDetails->Position = GetAbsoluteLevelPosition();
	// Remove link to parent	
	TileDetails->ParentPackageName = NAME_None;
	
	// Attach to new parent
	TSharedPtr<FT4WorldTileModel> NewParentTileModel = StaticCastSharedPtr<FT4WorldTileModel>(GetParent());
	if (!NewParentTileModel->IsRootTile())
	{
		// Transform level offset to relative
		TileDetails->Position-= NewParentTileModel->GetAbsoluteLevelPosition();
		// Setup link to parent 
		TileDetails->ParentPackageName = NewParentTileModel->TileDetails->PackageName;
	}
		
	OnLevelInfoUpdated();
}

bool FT4WorldTileModel::IsVisibleInCompositionView() const
{
	return !TileDetails->bHideInTileView && LevelCollectionModel.PassesAllFilters(*this);
}

FLinearColor FT4WorldTileModel::GetLevelColor() const
{
	ULevel* LevelObject = GetLevelObject();
	if (LevelObject)
	{
		return LevelObject->LevelColor;
	}
	else
	{
		return FT4LevelModel::GetLevelColor();
	}
}

void FT4WorldTileModel::SetLevelColor(FLinearColor InColor)
{
	ULevel* LevelObject = GetLevelObject();
	if (LevelObject)
	{
		ULevelStreaming* StreamingLevel = GetAssociatedStreamingLevel();
		if (StreamingLevel)
		{
			LevelObject->MarkPackageDirty();
			LevelObject->LevelColor = InColor;
			StreamingLevel->LevelColor = InColor; // this is transient object, but components fetch color from it
			LevelObject->MarkLevelComponentsRenderStateDirty();
		}
	}
}

void FT4WorldTileModel::OnLevelBoundsActorUpdated()
{
	Update();
}

void FT4WorldTileModel::EnsureLevelHasBoundsActor()
{
	ULevel* Level = GetLevelObject();
	if (Level && !Level->LevelBoundsActor.IsValid())
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.OverrideLevel = Level;

		LevelCollectionModel.GetWorld()->SpawnActor<ALevelBounds>(SpawnParameters);
	}
}

void FT4WorldTileModel::SortRecursive()
{
	AllChildren.Sort(FCompareByLongPackageName());
	FilteredChildren.Sort(FCompareByLongPackageName());
	
	for (auto It = AllChildren.CreateIterator(); It; ++It)
	{
		StaticCastSharedPtr<FT4WorldTileModel>(*It)->SortRecursive();
	}
}

void FT4WorldTileModel::OnLevelInfoUpdated()
{
	// #91 : World Single
	bool bWorldCompositionEnabled = (nullptr != LevelCollectionModel.GetWorld()->WorldComposition) ? true : false;

	if (!IsRootTile() && bWorldCompositionEnabled)
	{
		LevelCollectionModel.GetWorld()->WorldComposition->OnTileInfoUpdated(TileDetails->PackageName, TileDetails->GetInfo());
		ULevel* Level = GetLevelObject();
		if (Level)
		{
			bool bMarkDirty = false;
			bMarkDirty|= !(Level->LevelSimplification[0] == TileDetails->LOD1.SimplificationDetails);
			bMarkDirty|= !(Level->LevelSimplification[1] == TileDetails->LOD2.SimplificationDetails);
			bMarkDirty|= !(Level->LevelSimplification[2] == TileDetails->LOD3.SimplificationDetails);
			bMarkDirty|= !(Level->LevelSimplification[3] == TileDetails->LOD4.SimplificationDetails);
			
			if (bMarkDirty)
			{
				Level->LevelSimplification[0] = TileDetails->LOD1.SimplificationDetails;
				Level->LevelSimplification[1] = TileDetails->LOD2.SimplificationDetails;
				Level->LevelSimplification[2] = TileDetails->LOD3.SimplificationDetails;
				Level->LevelSimplification[3] = TileDetails->LOD4.SimplificationDetails;
				Level->MarkPackageDirty();
			}
		}
	}
}

void FT4WorldTileModel::OnPostUndoEvent()
{
	// #91 : World Single
	bool bWorldCompositionEnabled = (nullptr != LevelCollectionModel.GetWorld()->WorldComposition) ? true : false;
	if (!bWorldCompositionEnabled)
	{
		return;
	}

	FWorldTileInfo Info = LevelCollectionModel.GetWorld()->WorldComposition->GetTileInfo(TileDetails->PackageName);
	if (GetLevelObject())
	{
		// Level position changes
		FIntVector NewAbsolutePosition = TileDetails->AbsolutePosition;
		if (Info.AbsolutePosition != NewAbsolutePosition)
		{
			// SetLevelPosition will update AbsolutePosition to an actual value once level is moved
			TileDetails->AbsolutePosition = Info.AbsolutePosition; 
			SetLevelPosition(NewAbsolutePosition);
		}
		
		// Level attachment changes
		FName NewParentName = TileDetails->ParentPackageName;
		if (Info.ParentTilePackageName != NewParentName.ToString())
		{
			OnParentPackageNamePropertyChanged();
		}
	}
	
	OnLevelInfoUpdated();
}

void FT4WorldTileModel::OnPositionPropertyChanged()
{
	// #91 : World Single
	bool bWorldCompositionEnabled = (nullptr != LevelCollectionModel.GetWorld()->WorldComposition) ? true : false;
	if (!bWorldCompositionEnabled)
	{
		return;
	}

	FWorldTileInfo Info = LevelCollectionModel.GetWorld()->WorldComposition->GetTileInfo(TileDetails->PackageName);

	if (GetLevelObject())
	{
		// Get the delta
		FIntVector Delta = TileDetails->Position - Info.Position;

		// Snap the delta
		FT4LevelModelList LevelsList; LevelsList.Add(this->AsShared());
		FVector2D SnappedDelta2D = LevelCollectionModel.SnapTranslationDelta(LevelsList, FVector2D(Delta.X, Delta.Y), false, 0.f);

		// Set new level position
		SetLevelPosition(Info.AbsolutePosition + FIntVector(SnappedDelta2D.X, SnappedDelta2D.Y, Delta.Z));
		return;
	}
	
	// Restore original value
	TileDetails->Position = Info.Position;
}

void FT4WorldTileModel::OnParentPackageNamePropertyChanged()
{	
	// #91 : World Single
	bool bWorldCompositionEnabled = (nullptr != LevelCollectionModel.GetWorld()->WorldComposition) ? true : false;
	if (!bWorldCompositionEnabled)
	{
		return;
	}

	if (GetLevelObject())
	{
		TSharedPtr<FT4LevelModel> NewParent = LevelCollectionModel.FindLevelModel(TileDetails->ParentPackageName);
		// Assign to a root level if new parent is not found or we assigning to ourself 
		if (!NewParent.IsValid() || NewParent.Get() == this) 
		{
			NewParent = static_cast<FT4WorldTileCollectionModel&>(LevelCollectionModel).GetWorldRootModel();
		}

		FT4LevelModelList LevelList; LevelList.Add(this->AsShared());
		LevelCollectionModel.AssignParent(LevelList, NewParent);
		return;
	}
	
	// Restore original parent
	FWorldTileInfo Info = LevelCollectionModel.GetWorld()->WorldComposition->GetTileInfo(TileDetails->PackageName);
	TileDetails->ParentPackageName = FName(*Info.ParentTilePackageName);
}

void FT4WorldTileModel::OnLODSettingsPropertyChanged()
{
	OnLevelInfoUpdated();
}

void FT4WorldTileModel::OnZOrderPropertyChanged()
{
	OnLevelInfoUpdated();
}

void FT4WorldTileModel::OnHideInTileViewChanged()
{
	OnLevelInfoUpdated();
}

bool FT4WorldTileModel::CreateAdjacentLandscapeProxy(ALandscapeProxy* SourceLandscape, FT4WorldTileModel::EWorldDirections InWhere)
{
	if (!IsLoaded())	
	{
		return false;
	}

	// Determine import parameters from source landscape
	FBox SourceLandscapeBounds = SourceLandscape->GetComponentsBoundingBox(true);
	FVector SourceLandscapeScale = SourceLandscape->GetRootComponent()->GetComponentToWorld().GetScale3D();
	FIntRect SourceLandscapeRect = SourceLandscape->GetBoundingRect();
	FIntPoint SourceLandscapeSize = SourceLandscapeRect.Size();
	FIntPoint LandscapeSectionOffset = FIntPoint((SourceLandscape->LandscapeSectionOffset.X + SourceLandscapeRect.Min.X) * SourceLandscapeScale.X,
		(SourceLandscape->LandscapeSectionOffset.Y + SourceLandscapeRect.Min.Y) * SourceLandscapeScale.Y);

	FLandscapeImportSettings ImportSettings = {};
	ImportSettings.LandscapeGuid = SourceLandscape->GetLandscapeGuid();
	ImportSettings.LandscapeMaterial = SourceLandscape->GetLandscapeMaterial();
	ImportSettings.ComponentSizeQuads = SourceLandscape->ComponentSizeQuads;
	ImportSettings.QuadsPerSection = SourceLandscape->SubsectionSizeQuads;
	ImportSettings.SectionsPerComponent = SourceLandscape->NumSubsections;
	ImportSettings.SizeX = SourceLandscapeRect.Width() + 1;
	ImportSettings.SizeY = SourceLandscapeRect.Height() + 1;

	// Initialize with blank heightmap data
	ImportSettings.HeightData.AddUninitialized(ImportSettings.SizeX * ImportSettings.SizeY);
	for (auto& HeightSample : ImportSettings.HeightData)
	{
		HeightSample = 32768;
	}

	// Set proxy location at landscape bounds Min point
	ImportSettings.LandscapeTransform.SetLocation(FVector(0.f, 0.f, SourceLandscape->GetActorLocation().Z));
	ImportSettings.LandscapeTransform.SetScale3D(SourceLandscapeScale);

	// Create new landscape object
	ALandscapeProxy* AdjacentLandscape = ImportLandscapeTile(ImportSettings);
	if (AdjacentLandscape)
	{
		// Copy source landscape properties 
		AdjacentLandscape->GetSharedProperties(SourceLandscape);

		// Refresh level model bounding box
		FBox AdjacentLandscapeBounds = AdjacentLandscape->GetComponentsBoundingBox(true);
		TileDetails->Bounds = AdjacentLandscapeBounds;

		// Calculate proxy offset from source landscape actor
		FVector ProxyOffset(SourceLandscapeBounds.GetCenter() - AdjacentLandscapeBounds.GetCenter());

		// Add offset by chosen direction
		switch (InWhere)
		{
		case FT4WorldTileModel::XNegative:
			LandscapeSectionOffset += FIntPoint(-SourceLandscapeScale.X * SourceLandscapeSize.X, 0);
			ProxyOffset += FVector(-SourceLandscapeScale.X * SourceLandscapeSize.X, 0.f, 0.f);
			break;
		case FT4WorldTileModel::XPositive:
			LandscapeSectionOffset += FIntPoint(+SourceLandscapeScale.X * SourceLandscapeSize.X, 0);
			ProxyOffset += FVector(+SourceLandscapeScale.X * SourceLandscapeSize.X, 0.f, 0.f);
			break;
		case FT4WorldTileModel::YNegative:
			LandscapeSectionOffset += FIntPoint(0, -SourceLandscapeScale.Y * SourceLandscapeSize.Y);
			ProxyOffset += FVector(0.f, -SourceLandscapeScale.Y * SourceLandscapeSize.Y, 0.f);
			break;
		case FT4WorldTileModel::YPositive:
			LandscapeSectionOffset += FIntPoint(0, +SourceLandscapeScale.Y * SourceLandscapeSize.Y);
			ProxyOffset += FVector(0.f, +SourceLandscapeScale.Y * SourceLandscapeSize.Y, 0.f);
			break;
		}

		// Add source level position
		FIntVector IntOffset = FIntVector(ProxyOffset) + LevelCollectionModel.GetWorld()->OriginLocation;

		// Move level with landscape proxy to desired position
		SetLevelPosition(IntOffset, &LandscapeSectionOffset);
		return true;
	}

	return false;
}

ALandscapeProxy* FT4WorldTileModel::ImportLandscapeTile(const FLandscapeImportSettings& Settings)
{
	if (!IsLoaded())
	{
		return nullptr;
	}
	
	check(Settings.LandscapeGuid.IsValid())
	
	ALandscapeProxy* LandscapeProxy = Cast<UWorld>(LoadedLevel->GetOuter())->SpawnActor<ALandscapeStreamingProxy>();
	LandscapeProxy->SetActorTransform(Settings.LandscapeTransform);
		
	if (Settings.LandscapeMaterial)
	{
		LandscapeProxy->LandscapeMaterial = Settings.LandscapeMaterial;
	}
	
	// Cache pointer to landscape in the level model
	Landscape = LandscapeProxy;

	TMap<FGuid, TArray<uint16>> HeightmapDataPerLayers;
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayer;

	HeightmapDataPerLayers.Add(FGuid(), Settings.HeightData);
	MaterialLayerDataPerLayer.Add(FGuid(), Settings.ImportLayers);

	// Create landscape components
	LandscapeProxy->Import(Settings.LandscapeGuid, 0, 0, Settings.SizeX - 1, Settings.SizeY - 1, Settings.SectionsPerComponent, Settings.QuadsPerSection, HeightmapDataPerLayers, *Settings.HeightmapFilename,
		MaterialLayerDataPerLayer, Settings.ImportLayerType);

	return LandscapeProxy;
}

#undef LOCTEXT_NAMESPACE
