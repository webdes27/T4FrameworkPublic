// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Products/Common/ViewModel/T4BaseViewModel.h"

#include "UObject/GCObject.h"
#include "TickableEditorObject.h"
#include "EditorUndoClient.h"

/**
  * #83
 */
class UWorld;
class UT4MapEntityAsset;
struct FT4WorldPreviewViewModelOptions
{
	FT4WorldPreviewViewModelOptions();

	UT4MapEntityAsset* MapEntityAsset; // #104
	FString ActionPlaybackAssetName; // #104
	FString ActionPlaybackFolderName; // #104
};

class UT4EditorGameplaySettingObject; // #60, #104
struct FT4ActionParameters;
class FT4WorldPreviewViewModel
	: public TSharedFromThis<FT4WorldPreviewViewModel>
	, public FGCObject
	, public FEditorUndoClient
	, public FTickableEditorObject
	, public FT4BaseViewModel
{
public:
	DECLARE_MULTICAST_DELEGATE(FT4OnSubLevelUpdate); // #86

public:
	FT4WorldPreviewViewModel(const FT4WorldPreviewViewModelOptions& InOptions);
	~FT4WorldPreviewViewModel();

	//~ FGCObject interface
	void AddReferencedObjects(FReferenceCollector& Collector) override;

	//~ FEditorUndoClient interface
	void PostUndo(bool bSuccess) override;
	void PostRedo(bool bSuccess) override { PostUndo(bSuccess); }

	// ~ FTickableEditorObject
	void Tick(float DeltaTime) override;
	bool IsTickable() const override { return true; }
	TStatId GetStatId() const override;

	// FT4BaseViewModel
	ET4ViewModelEditMode GetEditMode() const override { return ET4ViewModelEditMode::WorldPreview; }

	void NotifyActionPlaybackRec() override; // #104
	void NotifyActionPlaybackPlay() override; // #104

public:
	FT4OnSubLevelUpdate& GetOnSubLevelUpdate() { return OnSubLevelUpdate; } // #86

	void OnRefreshWorld(); // #83
	void OnDisplayEditorWorldModified(); // #83
	void OnSubLevelSelection(const TArray<FName>& InSubLevelPackageNames, bool bFlushLevelStreaming); // #83, #86

	void OnToggleSimulation(); // #86

	void SetMapEntityAsset(UT4MapEntityAsset* InMapEntityAsset) { MapEntityAsset = InMapEntityAsset; } // #104

	void SetCameraLookAt(const FVector& InLocation, const FBox& InBoundingBox); // #85
	void SetCameraLocation(const FVector2D& InLocation); // #90
	void SetCameraLocationAndRotation(const FVector& InLocation, const FRotator& InRotation); // #90, #103

	bool GetPlayerViewPoint(
		FVector& OutCameraLocation,
		FRotator& OutCameraRotation,
		FVector& OutPlayerLocation
	);
	bool GetGameObjectLocations(TArray<FVector2D>& OutGameObjectLocations); // #104

	void HandleOnUpdateSubLevel(ULevel* InSubLevel, UWorld* InWorld); // #86

protected:
	void Cleanup() override; // #85
	void Reset() override; // #79
	void StartPlay() override; // #76, #86
	void RestartPlay(); // #94, #104 : 월드 이동후 호출

	void DrawHUD(FViewport* InViewport, FCanvas* InCanvas, FT4HUDDrawInfo* InOutDrawInfo) override; // #59, #83

	// #87 : ViewModel 시작시 특정 레벨을 열고 싶다면, MapEntityAssetPath 를 채울 것!
	void SetupStartWorld(FT4WorldConstructionValues& InWorldConstructionValues) override;

	FString GetActionPlaybackAssetName() const override { return ActionPlaybackAssetName; } // #68, #104
	FString GetActionPlaybackFolderName() const override { return ActionPlaybackFolderName; } // #68, #104
	
	void SetSimulationMode(bool bInSimulating); // #86

private:
	void UpdateManualLevelStreaming(UWorld* InPersistentWorld, float InDeltaTime); // #86

private:
	UT4MapEntityAsset* MapEntityAsset;
	UT4EditorGameplaySettingObject* EditorPlaySettingObject; // #60, #104

	TArray<FName> VisibleSubLevelPackageNames;

	FT4OnSubLevelUpdate OnSubLevelUpdate; // #86

	FDelegateHandle LevelAddedToWorldHandle; // #86
	FDelegateHandle LevelRemovedFromWorldHandle; // #86

	bool bSimulating; // #86
	bool bUpdatingReload;

	bool bDisplayEditorWorldModified;
	FName LastValidSubLevelPackageName; // #85

	bool bPendingRefreshWorld; // #86
	bool bPendingPlayerRespawn;
	FT4EntityKey PlayerEntityKey; // #83
	FName PlayerStanceName; // #83
	FVector PlayerLocationCached; // #83
	FRotator PlayerRotationCached; // #83
	
	FString ActionPlaybackAssetName; // #104
	FString ActionPlaybackFolderName; // #104

	float CachedTimeHour; // #104 : RefreshWorld 시 Time 복구
	float CachedTimeScale; // #104 : RefreshWorld 시 Time 복구
};

