// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
  *
 */
struct FT4HUDDrawInfo // #68
{
	FT4HUDDrawInfo()
		: NoticeLineOffset(0.0f)
	{
	}
	float NoticeLineOffset;
};

class IT4GameFrame;

#if WITH_EDITOR
DECLARE_DELEGATE_OneParam(FT4OnCreateEditorPlayerController, IT4GameFrame*); // #42
#endif

// #42
DECLARE_DELEGATE_OneParam(FT4OnRegisterGameplayLayerInstancce, IT4GameFrame*); // #42
class T4FRAME_API FT4FrameDelegates
{
public:
	static FT4OnRegisterGameplayLayerInstancce OnRegisterGameplayLayerInstancce;

#if WITH_EDITOR
	static FT4OnCreateEditorPlayerController OnCreateEditorPlayerController;
#endif

private:
	FT4FrameDelegates() {}
};
