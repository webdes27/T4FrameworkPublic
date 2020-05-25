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
	Swing,
	Throw,
	Launch, // #135, 특수, 발사체와 반동이 함께 있음 (TODO : 적당한 이름이 있다면 수정할 것)

	Air, // #135 : Jump Attack
	Dash, // #135

	None,
};

UENUM()
enum class ET4GameplayEffectType : uint8 // #68
{
	DirectDamage,
	AreaDamage,

	Knockback, // CC
	Airborne, // CC
	Stun, // CC

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
