// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4RehearsalEditorCommands.h"
#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4RehearsalEditorCommands"

/**
  * 
 */
void FT4RehearsalEditorCommands::RegisterCommands()
{
	UI_COMMAND(ReloadSpawnObject, "Reload", "Reload SpawnObject", EUserInterfaceActionType::Button, FInputChord()); // #38
	UI_COMMAND(SaveThumbnailImage, "Thumbnail", "Generate Thumbnail", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(MirrorToPIE, "MirrorToPIE", "Viewport Mirroring to PIE Viewport", EUserInterfaceActionType::Button, FInputChord()); // #59

	UI_COMMAND(ActionPlaybackRec, "ActionPlaybackRec", "Action Playback Rec", EUserInterfaceActionType::Button, FInputChord()); // #68
	UI_COMMAND(ActionPlaybackPlay, "ActionPlaybackPlay", "Action Playback Play", EUserInterfaceActionType::Button, FInputChord()); // #68
	UI_COMMAND(ActionPlaybackPause, "ActionPlaybackPause", "Action Playback Pause", EUserInterfaceActionType::Button, FInputChord()); // #68
	UI_COMMAND(ActionPlaybackStop, "ActionPlaybackStop", "Action Playback Stop", EUserInterfaceActionType::Button, FInputChord()); // #68

	UI_COMMAND(DespawnAll, "DespawnAll", "Despawn all Objects", EUserInterfaceActionType::Button, FInputChord()); // #68
	UI_COMMAND(GoPreviewScene, "GoPreviewScene", "Go to the PreviewScene", EUserInterfaceActionType::Button, FInputChord()); // #87

	UI_COMMAND(SaveAllModifiedLevels, "SaveAll", "Save all modified assets", EUserInterfaceActionType::Button, FInputChord()); // #86

	UI_COMMAND(UseDefaultShowFlags, "Use Defaults", "Resets all show flags to default", EUserInterfaceActionType::Button, FInputChord()); // #94

	// #76
	UI_COMMAND(ViewportShowCapsule, "Capsule", "Toggle Show capsule a player", EUserInterfaceActionType::ToggleButton, FInputChord());
	UI_COMMAND(ViewportAlwaysTick, "Always Tick", "Toggle Always tick a viewport", EUserInterfaceActionType::ToggleButton, FInputChord());
	// ~#76

	// #99
	UI_COMMAND(ViewportJumpToPlay, "Jump to Start", "Jump to the start of the playback range", EUserInterfaceActionType::None, FInputChord(EKeys::Up));
	UI_COMMAND(ViewportJumpToEnd, "Jump to End", "Jump to the end of the playback range", EUserInterfaceActionType::None, FInputChord(EModifierKey::Control, EKeys::Up));
	UI_COMMAND(ViewportTogglePlay, "Toggle Play", "Toggle the timeline playing", EUserInterfaceActionType::None, FInputChord(EKeys::Down));
	// ~#99
}

#undef LOCTEXT_NAMESPACE