// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Products/T4RehearsalEditor.h"

#include "Toolkits/AssetEditorToolkit.h"
#include "Toolkits/IToolkitHost.h"

/**
  *
 */
class ISequencer;
class IToolkitHost;
class IDetailsView;
class ST4EntityBrowserWidget;
class ST4ContiBrowserWidget;
class ST4RehearsalViewport;
class FT4ContiViewModel;
class FT4PreviewEntityViewModel;
class UT4EntityAsset;
class UT4ContiAsset;
class FT4RehearsalContiEditor : public FAssetEditorToolkit, public FT4RehearsalEditor
{
public:
	FT4RehearsalContiEditor();
	virtual ~FT4RehearsalContiEditor();

	void InitializeWithConti(
		const EToolkitMode::Type InMode, 
		const TSharedPtr<IToolkitHost>& InInitToolkitHost,
		UT4ContiAsset* InContiAsset
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

public:
	void SetPreviewViewTarget(UT4EntityAsset* EntityAsset); // #60

protected:
	ET4LayerType GetLayerType() const override; // #68

	IT4RehearsalViewModel* GetMainViewModelInterface() const override; // #104

	bool HasActionPlaybackController() const override; // #68
	UT4EditorActionPlaybackController* GetActionPlaybackController() const override; // #68

	bool CanSaveAsset() const override;
	void SaveAsset_Execute() override;

	bool CanSaveAssetAs() const override;
	void SaveAssetAs_Execute() override;

	void HandleOnSaveThumbnailImage();
	void HandleOnMirrorToPIE(); // #59

	void HandleSelectContiAsset(UObject* InAsset);
	void HandleOpenContiAsset(UObject* InAsset);

	void HandleOnGoPreviewScene(); // #87

	void HandleOnHotKeyJumpToPlay(); // #99
	void HandleOnHotKeyJumpToEnd(); // #99
	void HandleOnHotKeyTogglePlay(); // #99

	void HandleOnToggleSimulation(); // #102

private:
	void SetupCommands();

	void ExtendMenu();
	void ExtendToolbar();

	void GetSequencerAddMenuContent(FMenuBuilder& MenuBuilder, TSharedRef<ISequencer> Sequencer);
	TSharedRef<SWidget> CreateAddContiMenuContent();

	TSharedRef<SDockTab> SpawnTab_Preview(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Detail(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_EditorPlaySettings(const FSpawnTabArgs& Args); // #60
	TSharedRef<SDockTab> SpawnTab_Sequencer(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_ContiBrowser(const FSpawnTabArgs& Args); // #30

private:
	static const FName AppIdentifier;
	static const FName PreviewTabId;
	static const FName MainViewportTabId;
	static const FName DetailTabId;
	static const FName EditorPlaySettingsTabId; // #60
	static const FName SequencerTabId;
	static const FName ContiBrowserTabId; // #30

	TSharedPtr<ST4RehearsalViewport> MainViewportPtr;
	TSharedPtr<FT4ContiViewModel> MainViewModelPtr;

	TSharedPtr<ST4RehearsalViewport> PreviewViewportPtr;
	TSharedPtr<FT4PreviewEntityViewModel> PreviewViewModelPtr;

	TSharedPtr<ST4ContiBrowserWidget> ContiBrowserPtr; // #30

	TWeakObjectPtr<UT4ContiAsset> ContiAssetPtr;
};