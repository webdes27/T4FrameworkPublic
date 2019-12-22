// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Products/Common/ViewModel/T4BaseViewModel.h"

#include "UObject/GCObject.h"
#include "TickableEditorObject.h"
#include "EditorUndoClient.h"

/**
  *
 */
class UT4EntityAsset;
class UT4ContiAsset;
struct FT4PreviewEntityViewModelOptions
{
	FT4PreviewEntityViewModelOptions();

	UT4EntityAsset* EntityAsset;
};

struct FT4ActionParameters;
class FT4PreviewEntityViewModel
	: public TSharedFromThis<FT4PreviewEntityViewModel>
	, public FGCObject
	, public FEditorUndoClient
	, public FTickableEditorObject
	, public FT4BaseViewModel
{
public:
	FT4PreviewEntityViewModel(const FT4PreviewEntityViewModelOptions& InOptions);
	~FT4PreviewEntityViewModel();

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
	ET4ViewModelEditMode GetEditMode() const override { return ET4ViewModelEditMode::Preview; }

public:
	UT4EntityAsset* GetSelectedEntityAsset() { return EntityAsset; }

	void OnChangeViewTarget(UObject* InEntityAsset, float InRuntimeDurationSec);
	void ClientPlayConti(UT4ContiAsset* InContiAsset, const FT4ActionParameters* InActionParameters) override;

	void HandleOnEntityPropertiesChanged();

public:
	void RefreshAll();

	void SaveThumbnailCameraInfo();

	void SetPropertiesChangedDelegate(bool bRegister);

protected:
	void Cleanup() override; // #85
	void Reset() override; // #79
	void StartPlay() override; // #76, #86

private:
	UT4EntityAsset* EntityAsset;
};
