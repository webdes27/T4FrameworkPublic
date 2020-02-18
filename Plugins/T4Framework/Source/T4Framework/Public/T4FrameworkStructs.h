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

class IT4GameFramework;

#if WITH_EDITOR
DECLARE_DELEGATE_OneParam(FT4OnCreateEditorPlayerController, IT4GameFramework*); // #42
#endif

// #42
DECLARE_DELEGATE_OneParam(FT4OnRegisterGameplayLayerInstancce, IT4GameFramework*); // #42
class T4FRAMEWORK_API FT4FrameworkDelegates
{
public:
	static FT4OnRegisterGameplayLayerInstancce OnRegisterGameplayLayerInstancce;

#if WITH_EDITOR
	static FT4OnCreateEditorPlayerController OnCreateEditorPlayerController;
#endif

private:
	FT4FrameworkDelegates() {}
};
