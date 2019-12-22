// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4FrameGameplayTypes.generated.h"

/**
  *
 */
static const FName DefaultPlayerClassName = TEXT("Player"); // #104
static const FName DefaultCreatureClassName = TEXT("Creature"); // #104
static const FName DefaultFOClassName = TEXT("FO"); // #104
static const FName DefaultItemClassName = TEXT("Item"); // #104

UENUM()
enum class ET4GameAttackType : uint8 // #63
{
	Melee,
	Ranged,
	Area,

	None,
};

UENUM()
enum class ET4GameEffectType : uint8 // #68
{
	Direct,
	Area,

	None UMETA(Hidden),
};

UENUM()
enum class ET4GameTribeType : uint8 // #104
{
	Team_Red,
	Team_Blue,

	Neutral,
};

UENUM()
enum class ET4GameEnemyType : uint8 // #104
{
	Player,
	Hostile,
	PlayerAndHostile,

	All,

	NoEnemy,
};
