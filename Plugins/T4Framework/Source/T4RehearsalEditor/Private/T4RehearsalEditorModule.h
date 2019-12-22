// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "IAssetTools.h"

/**
  *
 */
class UT4ContiAsset;
class UT4EntityAsset;
class UT4WorldAsset;
class FUICommandList;
class USequencerSettings;
class IAssetTypeActions;
class FT4RehearsalEditorModule : 
	public IModuleInterface,
	public IHasMenuExtensibility, 
	public IHasToolBarExtensibility,
	public FGCObject
{
public:
	FT4RehearsalEditorModule();
	~FT4RehearsalEditorModule();

	static inline IModuleInterface& Get()
	{
		return FModuleManager::LoadModuleChecked<IModuleInterface>("T4RehearsalEditor");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("T4RehearsalEditor");
	}

	void StartupModule() override;
	void ShutdownModule() override;

	/** Gets the extensibility managers for outside entities to extend static mesh editor's menus and toolbars */
	virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager() override { return MenuExtensibilityManager; }
	virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager() override { return ToolBarExtensibilityManager; }

	void HandleOnContiEditorLaunch(
		UT4ContiAsset* InContiAsset,
		const TSharedPtr<IToolkitHost>& InEditWithinLevelEditor
	);

	void HandleOnEntityEditorLaunch(
		UT4EntityAsset* InEntityAsset,
		const TSharedPtr<IToolkitHost>& InEditWithinLevelEditor
	);

	void HandleOnWorldEditorLaunch(
		UT4WorldAsset* InWorldAsset,
		const TSharedPtr<IToolkitHost>& InEditWithinLevelEditor
	); // #83

	TSharedRef<FExtender> HandleOnLevelEditorPlayMenu(
		const TSharedRef<FUICommandList> CommandList
	); // #61

protected:
	// #61
	void CreateLevelEditorT4PlayOptionMenu(FMenuBuilder& InBuilder); // #61
	
	void HandleOnToogleT4GameplayEnable();
	bool HandleOnIsT4GameplayEnabled() const;
	// ~#61

private:
	/** FGCObject interface */
	void AddReferencedObjects(FReferenceCollector& Collector) override;

	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action);

private:
	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;
	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;

	USequencerSettings* SequencerSettings;
	FDelegateHandle SequencerTrackEditorHandle;

	TArray<TSharedPtr<IAssetTypeActions>> CreatedAssetTypeActions;

	// #61
	FDelegateHandle LevelEditorPlayExtenderHandle;
	FSoftClassPath CachedEngineGameInstanceClassName;
	FSoftClassPath CachedEngineGlobalGameModeClassName;
	// ~#61
};
