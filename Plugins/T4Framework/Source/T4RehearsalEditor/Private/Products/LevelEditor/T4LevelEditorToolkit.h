// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Layout/Visibility.h"
#include "Input/Reply.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Misc/NotifyHook.h"
#include "Toolkits/IToolkitHost.h"
#include "Toolkits/BaseToolkit.h"

/**
  * #68
 */

class IDetailsView;
class SButton;
class SCheckBox;
class SUniformGridPanel;
class UGeomModifier;

/** Geometry Mode widget for controls */
class ST4LevelEditorModeControls : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(ST4LevelEditorModeControls) {}
	SLATE_END_ARGS()

public:
	void SelectionChanged();

	void Construct(const FArguments& InArgs);

protected:
	FReply HandleOnDespawnAllSpawnObjectsClicked();

	FReply HandleOnActionPlaybackRecClicked();
	FReply HandleOnActionPlaybackPlayClicked();
	FReply HandleOnActionPlaybackPauseClicked();
	FReply HandleOnActionPlaybackStopClicked();

private:
	void CreateLayout();

	TSharedRef<SVerticalBox> CreateDespawnAllObjectButtons();
	TSharedRef<SHorizontalBox> CreateActionPlaybackButtons();
	TSharedRef<class IDetailsView> CreateEditorActionPlaybackController();

	class UT4EditorActionPlaybackController* GetActionPlaybackController() const;

private:
	TSharedPtr<class IDetailsView> PropertiesControl;
};

/**
 * Mode Toolkit for the Geometry Tools
 */
class FT4LevelEditorToolkit : public FModeToolkit
{
public:
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;

	/** Initializes the geometry mode toolkit */
	virtual void Init(const TSharedPtr< class IToolkitHost >& InitToolkitHost) override;

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual class FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<class SWidget> GetInlineContent() const override;

	/** Method called when the selection */
	virtual void SelectionChanged();

private:
	/** Geometry tools widget */
	TSharedPtr<class ST4LevelEditorModeControls> LevelEditorModeWidget;
};
