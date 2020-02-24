// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4GameBuiltin_AITypes.generated.h"

/**
  * #114
 */
UENUM()
enum class ET4GameBuiltin_AIState : uint8 // #114
{
	Ready,
	Invisible,
	Active,
	Dead,
};

UENUM()
enum class ET4GameBuiltin_AITaskState : uint8 // #114
{
	None,
	FindNearestEnemy,
	Approach,
	Attack,
	Roaming,
	Wait,
};
