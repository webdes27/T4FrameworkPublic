// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4EngineTypes.h"

/**
  *
 */
struct FT4GameObjectState // #76
{
	FT4GameObjectState()
		: bLoadComplated(false)
		, bResetting(false) // #111 : Reset 이 호출될 경우 불필요한 설정을 방지하기 위한 Flag
		, bGhost(false) // #54
		, GhostOutTimeLeft(0.0f) // #54
		, bPaused(false)
		, TimeScale(1.0f) // #102
		, LifeTimeSec(0.0f) // #102
		, bInvisible(false) // #117
		, bTransitionOpacity(false)
		, OpacityValue(1.0f)
		, TargetOpacityValue(1.0f)
		, TransitionTimeMaxSec(0.0f)
		, TransitionTimeSec(0.0f)
		, bTurning(false) // #44
		, TurnGoalRotation(FRotator::ZeroRotator) // #44
		, TurnRotationYawRate(0.0f) // #44
		, bAiming(false) // #113
		, AimTargetDirection(FVector::ZeroVector) // #113
#if WITH_EDITOR
		, bDebugPaused(false) // #63
#endif
	{
	}

	bool bLoadComplated;
	bool bResetting; // #111 : Reset 이 호출될 경우 불필요한 설정을 방지하기 위한 Flag

	bool bGhost; // #54
	float GhostOutTimeLeft; // #54

	bool bPaused;
	float TimeScale; // #102
	float LifeTimeSec; // #102

	bool bInvisible; // #117

	// #78 : Fade In/Out
	bool bTransitionOpacity;
	float OpacityValue;
	float TargetOpacityValue;
	float TransitionTimeMaxSec;
	float TransitionTimeSec;

	// #44 : Turn 에서 호출 (0 == InRotationYawRate) ? immediate
	bool bTurning;
	FRotator TurnGoalRotation;
	float TurnRotationYawRate;

	// #113 : Aim
	bool bAiming;
	FVector AimTargetDirection;

#if !UE_BUILD_SHIPPING
	bool bDebugPaused; // #63
#endif
};
