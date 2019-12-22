// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Products/Common/ViewModel/T4BaseViewModel.h"

/**
  * #83
 */
class ULevel;
class UWorld;
class ST4WorldMap; // #83
struct FT4LevelThumbnailData;
class FT4WorldPreviewViewModel;
class FT4LevelCollectionModel;
class UT4MapEntityAsset; // #104
class FT4WorldMapViewModel
	: public FT4BaseViewModel
{
public:
	DECLARE_MULTICAST_DELEGATE_TwoParams(FT4OnSubLevelSelection, const TArray<FName>&, bool); // #83, #86
	DECLARE_MULTICAST_DELEGATE(FT4OnSubLevelChanged); // #83

public:
	FT4WorldMapViewModel(TSharedPtr<FT4WorldPreviewViewModel> PreviewViewModelPtr);
	~FT4WorldMapViewModel();

public:
	UT4MapEntityAsset* GetMapEntityAsset() const; // #104 : WorldAsset 의 Tile 을 MapEntity 로 이전!
	TSharedRef<ST4WorldMap> GetWorldMapRef(); // #104

	bool IsSimulating() const { return bSimulating; } // #86
	bool IsWorldCompositionEnabled() const; // #91

	UWorld* GetPreviewWorld() const; // #85

	TSharedPtr<FT4LevelCollectionModel> GetWorldModelPtr();

	bool GetTileThumbnailSize(int32& OutTileThumbnailSize, int32& OutTileThumbnailAtlasSize); // #91
	const FT4LevelThumbnailData* GetSubLevelThumbnail(const FName InLevelAssetName); // #84

	FT4OnSubLevelSelection& GetOnSubLevelSelection() { return OnSubLevelSelection; } // #83
	FT4OnSubLevelChanged& GetOnSubLevelChanged() { return OnSubLevelChanged; } // #83
	FT4OnSubLevelChanged& GetOnEditorSubLevelChanged() { return OnEditorSubLevelChanged; } // #104

	void NotifyEditorWorldModified(); // #83, #92 : Editor / Preview World 가 틀릴 경우 화면에 노티!

	void UpdatePreviewWorldSubLevel(); // #86
	void ResetSubLevelSelection(); // #86

	void SelectSubLevelOfWorldMap(const TArray<FName>& InSubLevelNames); // #104

	void LoadEditorWorldSubLevel(const TArray<FName>& InSubLevelNames); // #104
	void LoadPreviewWorldSubLevel(const TArray<FName>& InSubLevelNames); // #104
	void UnloadPreviewWorldSubLevel(const TArray<FName>& InSubLevelNames); // #104

	void SetPreviewWorldActorSelected(const FVector& InLocation, const FBox& InBoundingBox); // #104

	void SetPreviewWorldUpdateCamera(const FVector& InLocation, const FBox& InBoundingBox, bool bWorldMapSet); // #85
	void SetPreviewWorldUpdateCamera(const FVector2D& InLocation); // #90

	void SetEditorWorldUpdateCamera(const FVector2D& InLocation); // #103
	void SetEditorWorldUpdateCamera(const FVector& InLocation, const FRotator& InRotation); // #103

	const TSet<FName>& GetPreviewLoadedLevels() const; // #104

	bool GetPlayerViewOnPreviewWorld(FVector& OutCameraLocation, FRotator& OutCameraRotation, FVector& OutPlayerLocation); // #86
	bool GetPreviewGameObjectLocations(TArray<FVector2D>& OutGameObjectLocations); // #104

	void HandleOnDetailsPropertiesChanged(const FName& InPropertyName); // #76
	void HandleOnWorldPropertiesChanged();

	void HandleOnSelectionChanged();
	void HandleOnPreviewLoadedLevelChanged(); // #104
	void HandleOnEditorLoadedLevelChanged(); // #104
	void HandleOnSubLevelChanged();

protected:
	void WorldMapInitialize(); // #104
	void WorldMapTick(float InDeltaTime); // #104
	void WorldMapCleanup(); // #85
	void WorldMapSetEntity(UT4MapEntityAsset* InMapEntityAsset); // #104

protected:
	TWeakObjectPtr<UT4MapEntityAsset> MapEntityAssetPtr; // #104 : WorldAsset 의 Tile 을 MapEntity 로 이전!

	TSharedPtr<ST4WorldMap> WorldMapPtr; // #90
	TSharedPtr<FT4WorldPreviewViewModel> PreviewViewModelPtr;

	TWeakPtr<FT4LevelCollectionModel> WorldCollectionModelPtr;

	FT4OnSubLevelSelection OnSubLevelSelection; // #83
	FT4OnSubLevelChanged OnSubLevelChanged; // #83
	FT4OnSubLevelChanged OnEditorSubLevelChanged; // #104

	bool bUpdatePreviewLoadedLevels; // #83
	bool bSimulating; // #86

	FString ActionPlaybackAssetName; // #68, #104
	FString ActionPlaybackFolderName; // #68, #104
};
