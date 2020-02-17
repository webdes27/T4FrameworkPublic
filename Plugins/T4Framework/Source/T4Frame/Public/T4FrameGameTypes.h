// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4FrameGameTypes.generated.h"

/**
  *
 */
static const FName DefaultPlayerClassName = TEXT("Player"); // #104
static const FName DefaultNPCClassName = TEXT("NPC"); // #104
static const FName DefaultItemClassName = TEXT("Item"); // #104

UENUM()
enum class ET4GameStatCategory : uint8 // #114
{
	Player,
	NPC,
	Item,

	Skill,
	Effect,

	None UMETA(Hidden),
};

UENUM()
enum class ET4GameStatLevel : uint8 // #114
{
	Level_1,
	Level_2,
	Level_3,
	Level_4,
	Level_5,
	Level_6,
	Level_7,
	Level_8,
	Level_9,
	Level_10,

	Max UMETA(Hidden),
};

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

UENUM()
enum class ET4GameTargetType : uint8 // #112
{
	ObjectID,
	Location,
	Direction,

	None,
};

UENUM()
enum class ET4GameFindTargetType : uint8 // #117 : 공객 대상을 찾을 경우에 대한 옵션 (TODO : Tribe or Enemy)
{
	All,
	Static,
	Dynamic,

	None UMETA(Hidden),
};