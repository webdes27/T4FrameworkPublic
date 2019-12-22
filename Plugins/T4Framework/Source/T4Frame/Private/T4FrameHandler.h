// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/EngineBaseTypes.h"

#if WITH_EDITOR
#include "UnrealEdMisc.h"
#endif

/**
 * 
 */
class AHUD;
class UCanvas;
class UWorld;
class FT4FrameHandler
{
public:
	FT4FrameHandler();
	virtual ~FT4FrameHandler();

	void Initialize();
	void Finalize();

private:
	void HandleOnWorldPreActorTick(UWorld* InWorld, ELevelTick InTickType, float InDeltaTime); // #34
	void HandleOnWorldPostActorTick(UWorld* InWorld, ELevelTick InTickType, float InDeltaTime); // #34

	void HandleOnHUDPostRender(AHUD* InHUD, UCanvas* InCanvas); // #68

#if WITH_EDITOR
	void HandleOnWorldCleanup(UWorld* InWorld, bool bSessionEnded, bool bCleanupResources); // #29
	void HandleOnMapChangedEvent(UWorld* InWorld, EMapChangeType InMapChangeType); // #17
#endif

private:
	FDelegateHandle OnWorldPreActorTickHandle;
	FDelegateHandle OnWorldPostActorTickHandle;
	FDelegateHandle OnHUDPostRenderHandle;
#if WITH_EDITOR
	FDelegateHandle OnWorldCleanupHandle;
	FDelegateHandle OnMapChangedEventHandle;
#endif
};

FT4FrameHandler& GetFrameHandler();