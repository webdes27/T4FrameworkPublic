// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Public/T4Frame.h"

#include "T4FrameInternal.h"

/**
  * #42
 */
FT4OnRegisterGameplayLayerInstancce FT4FrameDelegates::OnRegisterGameplayLayerInstancce;
#if WITH_EDITOR
FT4OnCreateEditorPlayerController FT4FrameDelegates::OnCreateEditorPlayerController;
#endif

// #52 : temp
static bool bT4EditorModeAISystemPause = false;
void SetT4EditorModeAISystemPaused(bool bInPaused)
{
	bT4EditorModeAISystemPause = bInPaused;
}

bool HasT4EditorModeAISystemPaused()
{
	return bT4EditorModeAISystemPause;
}
// ~#52 : temp