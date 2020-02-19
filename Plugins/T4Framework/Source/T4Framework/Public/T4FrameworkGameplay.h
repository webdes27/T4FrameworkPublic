// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4FrameworkGameplay.generated.h"

/**
  * #114 : 에디터에서 전투 시뮬레이션으로 사용해야 할 경우 Type 지정, 이 타입을 그대로 사용하던, 별도로 정의해 사용할 것!
 */
UENUM()
enum class ET4GameplayAttackType : uint8 // #63
{
	Melee,
	Ranged,
	Area,

	None,
};

UENUM()
enum class ET4GameplayEffectType : uint8 // #68
{
	Direct,
	Area,

	None UMETA(Hidden),
};

UENUM()
enum class ET4GameplayFindTarget : uint8 // #117 : 공객 대상을 찾을 경우에 대한 옵션 (TODO : Tribe or Enemy)
{
	All,
	Static,
	Dynamic,

	None UMETA(Hidden),
};
