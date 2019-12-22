// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4Engine/Public/T4EngineTypes.h"

/**
  * 
 */
class IT4GameFrame;
class FT4GameplayLauncher
{
public:
	FT4GameplayLauncher() {}
	~FT4GameplayLauncher() {}

	bool Initialize();
	void Finalize();

	void HandleOnRegisterGameplayLayerInstancce(IT4GameFrame* InGameFramework);
#if WITH_EDITOR
	void HandleOnCreateEditorPlayerController(IT4GameFrame* InGameFramework); // #42
#endif
};

FT4GameplayLauncher& GetGameplayLauncher();