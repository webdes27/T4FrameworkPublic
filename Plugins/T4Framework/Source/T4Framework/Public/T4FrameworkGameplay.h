// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4FrameworkGameplay.generated.h"

/**
  * // #114
 */
static const FName DefaultPlayerClassName = TEXT("Player"); // #104
static const FName DefaultNPCClassName = TEXT("NPC"); // #104
static const FName DefaultItemClassName = TEXT("Item"); // #104

UENUM()
enum class ET4GameplayStatCategory : uint8 // #114
{
	Player,
	NPC,
	Item,

	Skill,
	Effect,

	None UMETA(Hidden),
};

UENUM()
enum class ET4GameplayStatLevel : uint8 // #114
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
enum class ET4GameplayAIState : uint8 // #114
{
	Invisible,
	Active,
	Dead,
};

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
enum class ET4GameplayTribeType : uint8 // #104
{
	Team_Red,
	Team_Blue,

	Neutral,
};

UENUM()
enum class ET4GameplayEnemyType : uint8 // #104
{
	Player,
	Hostile,
	PlayerAndHostile,

	All,

	NoEnemy,
};

UENUM()
enum class ET4GameplayAttackTarget : uint8 // #112
{
	ObjectID,
	Location,
	Direction,

	None,
};

UENUM()
enum class ET4GameplayFindTarget : uint8 // #117 : 공객 대상을 찾을 경우에 대한 옵션 (TODO : Tribe or Enemy)
{
	All,
	Static,
	Dynamic,

	None UMETA(Hidden),
};

struct FT4GameplayAIStat // #114
{
	FT4GameplayAIStat()
		: Health_Point(0.0f)
		, Mana_Point(0.0f)
		, Striking_Power(0.0f)
		, Defensive_Power(0.0f)
		, Hit_Ratio(0.0f)
		, Dodge_Ratio(0.0f)
		, Result_EXP(0.0f)
	{
	}

	// FT4GameBuiltin_StatTableRow
	// Total Stat = InitializeNPCStatDataID + MainWeaponDataID (Stat)

	float Health_Point; // 피, 체력
	float Mana_Point; // 마력, 기
	float Striking_Power; // 공격력
	float Defensive_Power; // 방어력
	float Hit_Ratio; // 명중률
	float Dodge_Ratio; // 회피률
	float Result_EXP; // 경험치
};