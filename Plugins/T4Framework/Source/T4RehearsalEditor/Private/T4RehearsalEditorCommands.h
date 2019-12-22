// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "T4RehearsalEditorStyle.h"

/**
  *
 */
class FT4RehearsalEditorCommands : public TCommands<FT4RehearsalEditorCommands>
{
public:
	FT4RehearsalEditorCommands() : TCommands<FT4RehearsalEditorCommands>
	(
		TEXT("T4RehearsalEditor"),
		NSLOCTEXT("T4RehearsalEditorCommands", "T4RehearsalEditorCommands", "T4Rehearsal Commands"),
		NAME_None,
		FT4RehearsalEditorStyle::GetStyleSetName()
	)
	{
	}

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> ReloadSpawnObject; // #38
	TSharedPtr<FUICommandInfo> SaveThumbnailImage;
	TSharedPtr<FUICommandInfo> MirrorToPIE; // #59

	// #68
	TSharedPtr<FUICommandInfo> ActionPlaybackRec;
	TSharedPtr<FUICommandInfo> ActionPlaybackPlay;
	TSharedPtr<FUICommandInfo> ActionPlaybackPause;
	TSharedPtr<FUICommandInfo> ActionPlaybackStop;

	TSharedPtr<FUICommandInfo> DespawnAll;
	// ~#68

	TSharedPtr<FUICommandInfo> GoPreviewScene; // #87

	TSharedPtr<FUICommandInfo> SaveAllModifiedLevels; // #86

	TSharedPtr<FUICommandInfo> UseDefaultShowFlags; // #94

	TSharedPtr<FUICommandInfo> ViewportShowCapsule; // #76
	TSharedPtr<FUICommandInfo> ViewportAlwaysTick; // #76

	TSharedPtr<FUICommandInfo> ViewportJumpToPlay; // #99 : Keys::Up
	TSharedPtr<FUICommandInfo> ViewportJumpToEnd; // #99 : Keys::Up + CTRL
	TSharedPtr<FUICommandInfo> ViewportTogglePlay; // #99 : Keys::Down
};