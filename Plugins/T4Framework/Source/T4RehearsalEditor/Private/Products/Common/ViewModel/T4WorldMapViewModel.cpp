// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WorldMapViewModel.h"
#include "T4WorldPreviewViewModel.h"

#include "Products/T4RehearsalEditorUtils.h"

#include "Products/T4RehearsalEditorUtils.h" // #104
#include "Products/Common/Viewport/T4RehearsalViewportClient.h"

#include "Products/Common/WorldMap/T4LevelModel.h"
#include "Products/Common/WorldMap/T4LevelCollectionModel.h"

#include "Products/Common/WorldMap/ST4WorldMap.h" // #90

#include "T4Asset/Public/T4AssetUtils.h" // #39, #62
#include "T4Asset/Classes/Entity/T4MapEntityAsset.h" // #104
#include "T4Engine/Public/T4Engine.h"

#include "Engine/LevelStreaming.h"
#include "LevelEditorViewport.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "FT4WorldMapViewModel"

/**
  * #83
 */
FT4WorldMapViewModel::FT4WorldMapViewModel(TSharedPtr<FT4WorldPreviewViewModel> InPreviewViewModelPtr)
	: WorldMapPtr(SNew(ST4WorldMap))
	, PreviewViewModelPtr(InPreviewViewModelPtr)
	, bUpdatePreviewLoadedLevels(false)
	, bSimulating(false) // #86
{
}

FT4WorldMapViewModel::~FT4WorldMapViewModel()
{
}

void FT4WorldMapViewModel::WorldMapInitialize()
{
	WorldMapPtr->OnInitialize(this); // #90
}

void FT4WorldMapViewModel::WorldMapCleanup()
{
	if (MapEntityAssetPtr.IsValid())
	{
		MapEntityAssetPtr->RemoveFromRoot();
		MapEntityAssetPtr.Reset();
	}
	WorldMapPtr.Reset();
	PreviewViewModelPtr.Reset();
	WorldCollectionModelPtr.Reset();
}

void FT4WorldMapViewModel::WorldMapSetEntity(UT4MapEntityAsset* InMapEntityAsset) // #104
{ 
	if (MapEntityAssetPtr.IsValid())
	{
		MapEntityAssetPtr->RemoveFromRoot();
		MapEntityAssetPtr.Reset();
	}
	MapEntityAssetPtr = InMapEntityAsset;
	if (MapEntityAssetPtr.IsValid())
	{
		MapEntityAssetPtr->AddToRoot();
	}
	if (PreviewViewModelPtr.IsValid())
	{
		PreviewViewModelPtr->SetMapEntityAsset(MapEntityAssetPtr.Get());
	}
}

void FT4WorldMapViewModel::WorldMapTick(float DeltaTime)
{
	if (bUpdatePreviewLoadedLevels)
	{
		if (WorldCollectionModelPtr.IsValid())
		{
			TArray<FName> SubLevelSelections;
#if 1
			const TSet<FName>& PreviewLoadedLevels = WorldCollectionModelPtr.Pin()->GetPreviewLoadedLevelLists();
			for (const FName PackageName : PreviewLoadedLevels)
			{
				SubLevelSelections.Add(PackageName);
			}
#else
			FT4LevelModelList SelectedLevels = WorldCollectionModelPtr.Pin()->GetPreviewLoadedLevelLists();
			for (auto It = SelectedLevels.CreateConstIterator(); It; ++It)
			{
				FT4LevelModel* LevelModel = It->Get();
				check(nullptr != LevelModel);
				SubLevelSelections.Add(LevelModel->GetLongPackageName());
			}
#endif
			GetOnSubLevelSelection().Broadcast(SubLevelSelections, false);
		}
		bUpdatePreviewLoadedLevels = false;
	}
}

UT4MapEntityAsset* FT4WorldMapViewModel::GetMapEntityAsset() const
{ 
	return MapEntityAssetPtr.Get(); // #104 : WorldAsset 의 Tile 을 MapEntity 로 이전!
}

TSharedRef<ST4WorldMap> FT4WorldMapViewModel::GetWorldMapRef() // #104
{
	return WorldMapPtr.ToSharedRef();
}

bool FT4WorldMapViewModel::IsWorldCompositionEnabled() const // #91
{
	UWorld* EditorWorld = T4EditorUtil::GetWorld(ET4LayerType::LevelEditor);
	if (nullptr == EditorWorld)
	{
		return false;
	}
	return (nullptr != EditorWorld->WorldComposition) ? true : false;
}

UWorld* FT4WorldMapViewModel::GetPreviewWorld() const  // #85
{ 
	if (nullptr == PreviewViewModelPtr)
	{
		return nullptr;
	}
	return PreviewViewModelPtr->GetWorld();
} 

TSharedPtr<FT4LevelCollectionModel> FT4WorldMapViewModel::GetWorldModelPtr()
{
	UWorld* EditorWorld = T4EditorUtil::GetWorld(ET4LayerType::LevelEditor);
	check(nullptr != EditorWorld);
	TSharedPtr<FT4LevelCollectionModel> SharedWorldModel = WorldCollectionModelPtr.Pin();
	if (!SharedWorldModel.IsValid() || SharedWorldModel->GetWorld() != EditorWorld)
	{
		// #91 : 단일 World 도 사용할 수 있도록 수정
		if (nullptr != EditorWorld)
		{
			SharedWorldModel = FT4WorldTileCollectionModel::Create(this);
		}

		// Hold weak reference to shared world model
		WorldCollectionModelPtr = SharedWorldModel;
		if (WorldCollectionModelPtr.IsValid())
		{
			WorldCollectionModelPtr.Pin()->SelectionChanged.AddRaw(this, &FT4WorldMapViewModel::HandleOnSelectionChanged);
			WorldCollectionModelPtr.Pin()->PreviewLoadedLevelChanged.AddRaw(this, &FT4WorldMapViewModel::HandleOnPreviewLoadedLevelChanged); // #104
			WorldCollectionModelPtr.Pin()->EditorLoadedLevelChanged.AddRaw(this, &FT4WorldMapViewModel::HandleOnEditorLoadedLevelChanged); // #104
			WorldCollectionModelPtr.Pin()->OnSubLevelChanged.AddRaw(this, &FT4WorldMapViewModel::HandleOnSubLevelChanged); // #83
		}
	}

	return SharedWorldModel;
}

bool FT4WorldMapViewModel::GetTileThumbnailSize(
	int32& OutTileThumbnailSize,
	int32& OutTileThumbnailAtlasSize
) // #91
{
	if (!MapEntityAssetPtr.IsValid())
	{
		return false;
	}
	UWorld* EditorWorld = T4EditorUtil::GetWorld(ET4LayerType::LevelEditor);
	check(nullptr != EditorWorld);
	if (nullptr != EditorWorld->WorldComposition)
	{
		// #84 : UE4 의 World Browser 에서 사용하는 SubLevel Thumbnail Size 를 기준 * 4배로 설정
		OutTileThumbnailSize = 512; // #84 : WorldBrowser def TileThumbnailSize
		OutTileThumbnailAtlasSize = 2048; // #84 : WorldBrowser def TileThumbnailAtlasSize
	}
	else
	{
		// #91 : World Single 은 World Asset 에 Thumbnail 을 사용하지 않고, 런타임에 생성, 사용함으로
		//       큰 해상도를 사용한다. World BBox 를 얻이 비율을 통해 크기를 결정한다.
		FBox WorldBox = T4EditorUtil::CalculateLevelBounds(EditorWorld->PersistentLevel);
		float WorldLength = WorldBox.GetSize().X;
		if (WorldBox.GetSize().X < WorldBox.GetSize().Y)
		{
			WorldLength = WorldBox.GetSize().Y;
		}
		int32 TileResolution = FMath::CeilToInt((WorldLength / 100.0f) * 6.0f); // 1m => 6.0 pixel
		OutTileThumbnailSize = FMath::Min((int32)(GetMax2DTextureDimension() / 4), TileResolution); // #91 : WorldBrowser def TileThumbnailSize
		OutTileThumbnailAtlasSize = OutTileThumbnailSize * 4; // #91 : WorldBrowser def TileThumbnailSize
	}
	return true;
}

const FT4LevelThumbnailData* FT4WorldMapViewModel::GetSubLevelThumbnail(
	const FName InLevelAssetName
) // #84
{
	if (!MapEntityAssetPtr.IsValid())
	{
		return nullptr;
	}
	if (!MapEntityAssetPtr->LevelThumbnailDatas.Contains(InLevelAssetName))
	{
		return nullptr;
	}
	FT4LevelThumbnailData& SubLevelThumbnailData = MapEntityAssetPtr->LevelThumbnailDatas[InLevelAssetName];
	return &SubLevelThumbnailData;
}

void FT4WorldMapViewModel::NotifyEditorWorldModified() // #83
{
	if (PreviewViewModelPtr.IsValid())
	{
		PreviewViewModelPtr->OnDisplayEditorWorldModified();
	}
}

void FT4WorldMapViewModel::SelectSubLevelOfWorldMap(const TArray<FName>& InSubLevelNames) // #104
{
	if (WorldMapPtr.IsValid())
	{
		WorldMapPtr->OnSubLevelSelected(InSubLevelNames);
	}
}

void FT4WorldMapViewModel::LoadEditorWorldSubLevel(const TArray<FName>& InSubLevelNames) // #104
{
	if (WorldMapPtr.IsValid())
	{
		WorldMapPtr->OnEditorSubLevelLoad(InSubLevelNames);
	}
}

void FT4WorldMapViewModel::LoadPreviewWorldSubLevel(const TArray<FName>& InSubLevelNames) // #104
{
	if (WorldMapPtr.IsValid())
	{
		WorldMapPtr->OnPreviewSubLevelLoad(InSubLevelNames);
	}
}

void FT4WorldMapViewModel::UnloadPreviewWorldSubLevel(const TArray<FName>& InSubLevelNames) // #104
{
	if (WorldMapPtr.IsValid())
	{
		WorldMapPtr->OnPreviewSubLevelUnload(InSubLevelNames);
	}
}

void FT4WorldMapViewModel::SetPreviewWorldActorSelected(
	const FVector& InLocation, 
	const FBox& InBoundingBox
) // #104
{
	if (WorldMapPtr.IsValid())
	{
		WorldMapPtr->OnActorSelected(InLocation, InBoundingBox);
	}
}

void FT4WorldMapViewModel::SetPreviewWorldUpdateCamera(
	const FVector& InLocation,
	const FBox& InBoundingBox,
	bool bWorldMapSet
) // #85
{
	if (PreviewViewModelPtr.IsValid())
	{
		PreviewViewModelPtr->SetCameraLookAt(InLocation, InBoundingBox);
	}
	if (bWorldMapSet && WorldMapPtr.IsValid()) // #90
	{
		WorldMapPtr->OnRequestScrollTo(InLocation, InBoundingBox);
	}
}

void FT4WorldMapViewModel::SetPreviewWorldUpdateCamera(
	const FVector2D& InLocation
) // #90
{
	if (PreviewViewModelPtr.IsValid())
	{
		PreviewViewModelPtr->SetCameraLocation(InLocation);
	}
}

void FT4WorldMapViewModel::SetEditorWorldUpdateCamera(
	const FVector2D& InLocation
) // #90
{
	if (GCurrentLevelEditingViewportClient)
	{
		if (nullptr != GCurrentLevelEditingViewportClient->Viewport)
		{
			bool bRealtimeCached = GCurrentLevelEditingViewportClient->IsRealtime();
			if (!bRealtimeCached)
			{
				GCurrentLevelEditingViewportClient->SetRealtime(true);
			}
			{
				const FVector CameraViewLocation = GCurrentLevelEditingViewportClient->GetViewLocation();

				FVector CameraEyeLocation = CameraViewLocation;
				CameraEyeLocation.X = InLocation.X;
				CameraEyeLocation.Y = InLocation.Y;

				GCurrentLevelEditingViewportClient->SetViewLocation(CameraEyeLocation);
			}
			if (!bRealtimeCached)
			{
				GCurrentLevelEditingViewportClient->SetRealtime(false);
			}
		}
	}
}

void FT4WorldMapViewModel::SetEditorWorldUpdateCamera(
	const FVector& InLocation, 
	const FRotator& InRotation
) // #103
{
	if (GCurrentLevelEditingViewportClient)
	{
		if (nullptr != GCurrentLevelEditingViewportClient->Viewport)
		{
			bool bRealtimeCached = GCurrentLevelEditingViewportClient->IsRealtime();
			if (!bRealtimeCached)
			{
				GCurrentLevelEditingViewportClient->SetRealtime(true);
			}
			{
				GCurrentLevelEditingViewportClient->SetViewLocation(InLocation);
				GCurrentLevelEditingViewportClient->SetViewRotation(InRotation);
			}
			if (!bRealtimeCached)
			{
				GCurrentLevelEditingViewportClient->SetRealtime(false);
			}
		}
	}
}

void FT4WorldMapViewModel::UpdatePreviewWorldSubLevel() // #86
{
	if (bSimulating)
	{
		UWorld* PreviewWorld = GetPreviewWorld();
		if (nullptr != PreviewWorld)
		{
			if (WorldCollectionModelPtr.IsValid())
			{
				// #86 : PreviewSimulating 이 되면 PreviewWorld 의 활성화된 Streaming Level 을 표시해준다.
				check(WorldCollectionModelPtr.Pin()->IsSimulating());
				FT4LevelModelList LevelsToSelect;
				const TArray<ULevelStreaming*>& StreamingLevels = PreviewWorld->GetStreamingLevels();
				for (ULevelStreaming* LevelStreaming : StreamingLevels)
				{
					if (LevelStreaming->HasLoadedLevel())
					{
						const FName LongPackageName = LevelStreaming->PackageNameToLoad;
						TSharedPtr<FT4LevelModel> LevelModel = WorldCollectionModelPtr.Pin()->FindLevelModel(LongPackageName);
						if (LevelModel.IsValid())
						{
							LevelsToSelect.Add(LevelModel);
						}
					}
				}
				WorldCollectionModelPtr.Pin()->SetSelectedLevels(LevelsToSelect);
			}
		}
	}
	if (WorldMapPtr.IsValid())
	{
		WorldMapPtr->OnRefreshSelection();
	}
}

void FT4WorldMapViewModel::ResetSubLevelSelection()
{
	// #86 : Editor World 편집 이슈로 Preview World 에서 로드한 SubLevel 을 리로드 처리해준다.
	TArray<FName> SubLevelSelections;
	GetOnSubLevelSelection().Broadcast(SubLevelSelections, true);
	bUpdatePreviewLoadedLevels = true;
}

const TSet<FName>& FT4WorldMapViewModel::GetPreviewLoadedLevels() const // #104
{
	if (!WorldMapPtr.IsValid())
	{
		static TSet<FName> EmptySet;
		return EmptySet;
	}
	return WorldCollectionModelPtr.Pin()->GetPreviewLoadedLevelLists();
}

bool FT4WorldMapViewModel::GetPlayerViewOnPreviewWorld(
	FVector& OutCameraLocation,
	FRotator& OutCameraRotation,
	FVector& OutPlayerLocation
) // #86
{
	if (nullptr == PreviewViewModelPtr)
	{
		return false;
	}
	return PreviewViewModelPtr->GetPlayerViewPoint(OutCameraLocation, OutCameraRotation, OutPlayerLocation);
}

bool FT4WorldMapViewModel::GetPreviewGameObjectLocations(TArray<FVector2D>& OutGameObjectLocations) // #104
{
	if (nullptr == PreviewViewModelPtr)
	{
		return false;
	}
	return PreviewViewModelPtr->GetGameObjectLocations(OutGameObjectLocations);
}

void FT4WorldMapViewModel::HandleOnDetailsPropertiesChanged(const FName& InPropertyName)
{
	if (GetOnViewModelDetailPropertyChanged().IsBound())
	{
		GetOnViewModelDetailPropertyChanged().Broadcast();
	}
}

void FT4WorldMapViewModel::HandleOnWorldPropertiesChanged()
{
}

void FT4WorldMapViewModel::HandleOnSelectionChanged()
{
	//bUpdatePreviewLoadedLevels = true; 
	// Preview Detail 에서 현재 선택된 레벨 선택 지원
}

void FT4WorldMapViewModel::HandleOnPreviewLoadedLevelChanged() // #104
{
	bUpdatePreviewLoadedLevels = true;
}

void FT4WorldMapViewModel::HandleOnEditorLoadedLevelChanged() // #104
{
	GetOnEditorSubLevelChanged().Broadcast();
}

void FT4WorldMapViewModel::HandleOnSubLevelChanged()
{
	GetOnSubLevelChanged().Broadcast();
}

#undef LOCTEXT_NAMESPACE