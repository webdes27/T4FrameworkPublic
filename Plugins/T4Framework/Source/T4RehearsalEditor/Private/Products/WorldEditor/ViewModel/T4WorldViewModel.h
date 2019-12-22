// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Products/Common/ViewModel/T4WorldMapViewModel.h"
#include "Products/Common/WorldMap/ST4WorldMap.h" // #90

#include "T4Engine/Public/T4EngineTypes.h"
#include "T4Engine/Public/T4EngineConstants.h" // #39

#include "UObject/GCObject.h"
#include "TickableEditorObject.h"
#include "EditorUndoClient.h"

/**
  * #83, #104
 */
struct FT4EditorTestAutomation;
class IT4GameWorld;
class AT4MapZoneVolume; // #92
class UT4WorldAsset;
class UT4WorldPreviewLevelDetailObject;
class UT4EnvironmentDetailObject; // #90

struct FT4WorldViewModelOptions
{
	FT4WorldViewModelOptions();

	UT4WorldAsset* WorldAsset;
	TSharedPtr<FT4WorldPreviewViewModel> PreviewViewModelPtr;
};

class FT4WorldViewModel
	: public TSharedFromThis<FT4WorldViewModel>
	, public FGCObject
	, public FEditorUndoClient
	, public FTickableEditorObject
	, public FT4WorldMapViewModel
{
public:
	DECLARE_DELEGATE(FT4OnWorldEditorRefresh); // #90

public:
	FT4WorldViewModel(const FT4WorldViewModelOptions& InOptions);
	~FT4WorldViewModel();

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
	ET4ViewModelEditMode GetEditMode() const override { return ET4ViewModelEditMode::World; }

	const FString GetAssetPath() override; // #79

	UT4EnvironmentDetailObject* GetEnvironmentDetailObject() override { return EnvironmentDetailObjectOwner; } // #90, #94

	void ChangeWorldEnvironment(FName InTimeTagName) override; // #94

public:
	UT4WorldAsset* GetWorldAsset() const { return WorldAssetOwner; }
	UT4WorldPreviewLevelDetailObject* GetWorldLevelDetailObject() { return WorldPreviewLevelDetailObjectOwner; } // #85

	FT4OnWorldEditorRefresh& GetOnWorldEditorRefresh() { return OnWorldEditorRefresh; } // #90

	AT4MapZoneVolume* GetMapZoneSelectedOnEditorWorld(); // #92
	AT4MapZoneVolume* GetMapZoneSelectedOnPreviewWorld(); // #92

	void ToggleSimulation(); // #86

	void UpdateWorldEnvironment(); // #90

	void ChangeMapEntityAsset(); // #90

	void SelectEditorWorldMapZone(FName InMapZoneName); // #92

	void HandleOnMapZonePropertyChanged(); // #92
	void HandleOnGameWorldTimeTransition(IT4GameWorld* InGameWorld, const FName InTimeName); // #93

protected:
	// FT4BaseViewModel
	void Cleanup() override; // #85
	void Reset() override; // #79
	void StartPlay() override; // #76, #86

	UObject* GetEditObject() const override; // #103

	FT4EditorTestAutomation* GetTestAutomation() const override; // #103

	void TravelPOI(FT4EditorPointOfInterest* InPOIData) override; // #100, #103
	bool GetPOIInfo(FT4EditorPointOfInterest* OutPOIData) override; // #100, #103

private:
	void SetPropertiesChangedDelegate(bool bRegister);

private:
	UT4WorldAsset* WorldAssetOwner;
	UT4WorldPreviewLevelDetailObject* WorldPreviewLevelDetailObjectOwner;
	UT4EnvironmentDetailObject* EnvironmentDetailObjectOwner; // #90

	FT4OnWorldEditorRefresh OnWorldEditorRefresh; // #90

	FName MapZoneSelectedOnEditorWorld; // #92
	FName TimeTagNameSelected; // #93
};
