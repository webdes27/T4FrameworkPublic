// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Products/T4RehearsalEditor.h"

#include "Toolkits/AssetEditorToolkit.h"
#include "Toolkits/IToolkitHost.h"

/**
  * #83
 */
class IToolkitHost;
class IDetailsView;
class UT4WorldAsset;
class ST4RehearsalViewport;
class FT4WorldViewModel;
class FT4WorldPreviewViewModel;
class ST4WorldObjectWidget; // #85
class ST4WorldPreviewObjectWidget; // #85
class UT4EditorActionPlaybackController;
class FT4RehearsalWorldEditor : public FAssetEditorToolkit, public FT4RehearsalEditor, public FTickableEditorObject
{
public:
	FT4RehearsalWorldEditor();
	virtual ~FT4RehearsalWorldEditor();

	void InitializeWithWorld(
		const EToolkitMode::Type InMode,
		const TSharedPtr<IToolkitHost>& InInitToolkitHost,
		UT4WorldAsset* InWorldAsset
	);

	// ~ FTickableEditorObject
	void Tick(float DeltaTime) override;
	bool IsTickable() const override { return true; }
	TStatId GetStatId() const override;

	/** IToolkit interface */
	void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	FName GetToolkitFName() const override;
	FText GetBaseToolkitName() const override;
	FString GetWorldCentricTabPrefix() const override;
	FLinearColor GetWorldCentricTabColorScale() const override;
	FString GetDocumentationLink() const override
	{
		return TEXT("NotAvailable");
	}

	void DoUpdateThumbnail(UObject* InAsset); // #39

protected:
	ET4LayerType GetLayerType() const override; // #68

	IT4RehearsalViewModel* GetMainViewModelInterface() const override; // #104

	bool HasActionPlaybackController() const override; // #68
	UT4EditorActionPlaybackController* GetActionPlaybackController() const override; // #68

	bool IsWorldCompositionEnabled() const;// #91 : World Single

	// #83, #104 : World 툴이 오픈된 상황에서 일반 레벨을 로드하거나 다른 World 를 열 경우 현재의
	//             World Editor 를 Close 하도록 처리한다.
	void HandleOnAssetEditorRequestedOpen(UObject* InWorldObject);

	void HandleOnSaveThumbnailImage();
	void HandleOnSaveAllModifiedLevels(); // #86

	void HandleOnSubLevelAddOrRemovePreviewWorld(); // #86
	void HandleOnRefreshPreviewWorld(); // #86
	void HandleToggleSimulation(); // #86
	void HandleOnEditorSubLevelSelection(const TArray<FName>& InSubLevelPackageNames, bool bFlushLevelStreaming); // #86
	void HandleOnSubLevelChanged(); // #86
	void HandleOnEditorSubLevelChanged(); // #104
	void HandleOnWorldEditorRefresh(); // #90

private:
	void CleanUp(); // #90
	void SetupCommands();

	void ExtendMenu();
	void ExtendToolbar();

	TSharedRef<SDockTab> SpawnTab_PreviewViewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_WorldMap(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_WorldDetail(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_PreviewDetail(const FSpawnTabArgs& Args);

private:
	static const FName AppIdentifier;
	static const FName PreviewViewportTabId;
	static const FName PreviewDetailTabId;
	static const FName WorldMapTabId;
	static const FName WorldDetailTabId;

	TSharedPtr<FT4WorldViewModel> WorldViewModelPtr;

	TSharedPtr<ST4RehearsalViewport> PreviewViewportPtr;
	TSharedPtr<FT4WorldPreviewViewModel> PreviewViewModelPtr;

	TSharedPtr<ST4WorldObjectWidget> WorldObjectDetailsPtr; // #85
	TSharedPtr<ST4WorldPreviewObjectWidget> PreviewObjectDetailsPtr; // #85

	TWeakObjectPtr<UT4WorldAsset> WorldAssetPtr;

	bool bInitializing; // #83, #104
	bool bUpdatePreviewSubLevels; // #86
};