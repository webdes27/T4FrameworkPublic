// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4GameAITypes.generated.h"

/**
  * #114
 */
UENUM()
enum class ET4GameAIState : uint8 // #114
{
	Ready,
	Invisible,
	Active,
	Dead,
};

UENUM()
enum class ET4GameAITaskState : uint8 // #114
{
	None,
	Dead,
	Abnormal, // #135
	FindNearestEnemy,
	Approach,
	Attack,
	Roaming,
	Wait,
};
