// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Products/T4RehearsalEditor.h"

#include "Toolkits/AssetEditorToolkit.h"
#include "Toolkits/IToolkitHost.h"

/**
  *
 */
class IToolkitHost;
class ST4RehearsalViewport;
class FT4EntityViewModel;
class FT4PreviewEntityViewModel;
class ST4EntityBrowserWidget;
class ST4ContiBrowserWidget;
class UT4EntityAsset;
class UT4EditorActionPlaybackController;

class FT4RehearsalEntityEditor : public FAssetEditorToolkit, public FT4RehearsalEditor
{
public:
	FT4RehearsalEntityEditor();
	virtual ~FT4RehearsalEntityEditor();

	void InitializeWithEntity(
		const EToolkitMode::Type InMode, 
		const TSharedPtr<IToolkitHost>& InInitToolkitHost,
		UT4EntityAsset* InEntityAsset
	);

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

	bool HasActionPlaybackController() const override; // #68
	UT4EditorActionPlaybackController* GetActionPlaybackController() const override; // #68

	bool CanSaveAsset() const override;
	void SaveAsset_Execute() override;

	bool CanSaveAssetAs() const override;
	void SaveAssetAs_Execute() override;

	void HandleOnSaveThumbnailImage(); // #79
	void HandleOnReload();

	void HandleSelectAsset(UObject* InAsset);
	void HandleSpawnAsset(UObject* InAsset);
	void HandleOpenAsset(UObject* InAsset);
	void HandleSaveAsset(UObject* InAsset);
	void HandleOnUpdateThumbnail(UObject* InAsset);
	void HandleOnSavePreviewCameraInfo();
	void HandleOnDoubleClickedEntityAsset(UObject* InAsset);
	void HandleOnEquipItem(UObject* InAsset, FName InActionPointName, bool bEquip); // #72
	void HandleOnExchangeItem(UObject* InAsset, FName InCompositePartName, bool bSet); // #72

	void HandleSelectContiAsset(UObject* InAsset); // #71
	void HandleOpenContiAsset(UObject* InAsset); // #71

	void HandleOnGoPreviewScene(); // #87

private:
	void SetupCommands();

	void ExtendMenu();
	void ExtendToolbar();

	TSharedRef<SDockTab> SpawnTab_Preview(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_EntityBrowser(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_EntityDetail(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_AnimSetDetail(const FSpawnTabArgs& Args); // #39
	TSharedRef<SDockTab> SpawnTab_ContiBrowser(const FSpawnTabArgs& Args); // #71

private:
	static const FName AppIdentifier;
	static const FName PreviewTabId;
	static const FName EntityBrowserTabId; // #36
	static const FName ContiBrowserTabId; // #71
	static const FName MainViewportTabId;
	static const FName EntityDetailTabId;
	static const FName AnimSetDetailTabId; // #39

	TSharedPtr<ST4RehearsalViewport> MainViewportPtr;
	TSharedPtr<FT4EntityViewModel> MainViewModelPtr;

	TSharedPtr<ST4RehearsalViewport> PreviewViewportPtr;
	TSharedPtr<FT4PreviewEntityViewModel> PreviewViewModelPtr;

	TSharedPtr<ST4EntityBrowserWidget> EntityBrowserPtr;
	TSharedPtr<ST4ContiBrowserWidget> ContiBrowserPtr; // #71

	TWeakObjectPtr<UT4EntityAsset> EntityAssetPtr;
};