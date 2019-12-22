// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Products/Common/ViewModel/T4BaseViewModel.h"

#include "Products/Common/Helper/T4EditorViewTargetSelector.h" // #57
#include "Products/Common/Helper/T4EditorAnimSetAssetSelector.h"

#include "T4Engine/Public/T4EngineTypes.h"
#include "T4Engine/Public/T4EngineConstants.h" // #39

#include "UObject/GCObject.h"
#include "TickableEditorObject.h"
#include "EditorUndoClient.h"

/**
  *
 */
class USkeleton;
class UAnimSequence;
class UBlendSpaceBase;
class UT4EntityAsset;
class UT4AnimSetAsset;
class FT4RehearsalEntityEditor;
struct FT4EntityViewModelOptions
{
	FT4EntityViewModelOptions();

	UT4EntityAsset* EntityAsset;
	FT4RehearsalEntityEditor* EntityEditor;
};

struct FT4EntityCharacterStanceData;
class FT4RehearsalViewportClient;
class UT4ContiAsset;
class UT4CostumeEntityAsset;
class UT4WeaponEntityAsset;
struct FT4ActionParameters;
class UWorld; // #94
class UT4EnvironmentDetailObject; // #94
class FT4EntityViewModel
	: public TSharedFromThis<FT4EntityViewModel>
	, public FGCObject
	, public FEditorUndoClient
	, public FTickableEditorObject
	, public FT4BaseViewModel
{
public:
	FT4EntityViewModel(const FT4EntityViewModelOptions& InOptions);
	~FT4EntityViewModel();

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
	ET4ViewModelEditMode GetEditMode() const override { return ET4ViewModelEditMode::Entity; }
	bool IsEditWidgetMode() const override; // #94

	const FString GetAssetPath() override; // #79

	UT4EnvironmentDetailObject* GetEnvironmentDetailObject() override { return EnvironmentDetailObjectOwner; } // #90, #94

	AActor* GetEditWidgetModeTarget() const override; // #94

	void ChangeWorldEnvironment(FName InTimeTagName) override; // #94

public:
	void RefreshAll();

	UT4EntityAsset* GetEntityAsset() const { return EntityAsset; }

	TSharedRef<FT4EditorViewTargetSelector> GetViewTargetSelector() { return ViewTargetSelectorPtr->AsShared(); } // #74
	TSharedRef<FT4EditorAnimSetAssetSelector> GetAnimSetAssetSelector() { return AnimSetAssetSelectorPtr->AsShared(); }

	bool DoSave(FString& OutErrorMessage); // #39

	bool DoAnimSetSaveAll(FString& OutErrorMessage); // #39
	bool DoAnimSetUpdateAll(FString& OutErrorMessage); // #39
	bool DoUpdateThumbnailAnimSetAsset(FString& OutErrorMessage); // #39

	bool DoEntitySpawn(UT4EntityAsset* InEntityAsset); // #79

	void ReloadPlayerSpawn(); // #87

	bool ClientSpawnObject(UT4EntityAsset* InEntityAsset, const FName InStanceName) override; // #36, #30
	void ClientChangeStance(FName InStanceName) override; // #73

	void ClientEditorAction(ET4EditorAction InEditorActionType) override; // #71

	void HandleOnDetailsPropertiesChanged(const FName& InPropertyName); // #76
	void HandleOnAnimSetDetailsPropertiesChanged(const FName& InPropertyName); // #39, #77

	void HandleOnEntityPropertiesChanged();

protected:
	// FT4BaseViewModel
	void Cleanup() override; // #85
	void Reset() override; // #79
	void StartPlay() override; // #76, #86
	void RestartPlay() override; // #94 : 월드 이동후 호출

	void DrawHUD(FViewport* InViewport, FCanvas* InCanvas, FT4HUDDrawInfo* InOutDrawInfo) override; // #94

	// #87 : ViewModel 시작시 특정 레벨을 열고 싶다면, MapEntityAssetPath 를 채울 것!
	void SetupStartWorld(FT4WorldConstructionValues& InWorldConstructionValues) override;

	void NotifyViewTargetChanged(IT4GameObject* InViewTarget) override; // #87

	UObject* GetEditObject() const override; // #103
	FT4EditorTestAutomation* GetTestAutomation() const override; // #103

	FString GetActionPlaybackAssetName() const override; // #68, #104
	FString GetActionPlaybackFolderName() const override; // #68, #104

private:
	void SetupInternal();
	void CheckAndSpawnEntity(); // #79

	UT4AnimSetAsset* GetAnimSetAssetByStance() const; // #39

	void SetPropertiesChangedDelegate(bool bRegister);

private:
	UT4EntityAsset* EntityAsset;
	UT4EnvironmentDetailObject* EnvironmentDetailObjectOwner; // #94

	TSharedPtr<FT4EditorViewTargetSelector> ViewTargetSelectorPtr; // #74
	TSharedPtr<FT4EditorAnimSetAssetSelector> AnimSetAssetSelectorPtr;

	FT4RehearsalEntityEditor* EntityEditor;

	FT4ObjectID ZoneEntityObjectID; // #94 : Zone Entity 만 사용!

	bool bPlayerSpawnd; // #71
	bool bUpdatingStartPlay;
};
