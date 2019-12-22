// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4Engine/Public/T4EngineTypes.h"

/**
  *
 */
class IT4RehearsalViewModel;
class UTexture2D;
class UT4EditorActionPlaybackController;
class FT4RehearsalEditor
{
public:
	FT4RehearsalEditor();
	virtual ~FT4RehearsalEditor();

	virtual ET4LayerType GetLayerType() const { return ET4LayerType::Max; }

	virtual IT4RehearsalViewModel* GetMainViewModelInterface() const { return nullptr; } // #104

	virtual bool HasActionPlaybackController() const { return false; } // #68
	virtual UT4EditorActionPlaybackController* GetActionPlaybackController() const { return nullptr; } // #68

	void CommonSetupCommands(TSharedRef<FUICommandList> InCommandList);
	void CommonExtendToolbar(FToolBarBuilder& InToolbarBuilder);

	void HandleOnThumbnailCaptured(UObject* InOwner, UTexture2D* InThumbnail);

	// #68
	void HandleOnActionPlaybackRec();

	void HandleOnActionPlaybackPlay();
	TSharedRef<SWidget> HandleOnActionPlaybackPlayMenu();
	void HandleOnToogleActionPlaybackRepeatEnable();
	bool HandleOnIsActionPlaybackRepeatEnabled() const;
	void HandleOnToogleActionPlaybackPlayerPossessed();
	bool HandleOnIsActionPlaybackPlayerPossessed() const;

	void HandleOnActionPlaybackPause();
	void HandleOnActionPlaybackStop();

	bool HandleOnCanExecuteActionPlaybackRec();
	bool HandleOnCanExecuteActionPlaybackPlay();
	bool HandleOnCanExecuteActionPlaybackPause();
	bool HandleOnCanExecuteActionPlaybackStop();

	void HandleOnDespawnAll();
	// ~#68
};