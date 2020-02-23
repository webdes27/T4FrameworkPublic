// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4GameBuiltin_Structs.generated.h"

/**
  * #114
 */
USTRUCT()
struct FT4GameBuiltin_AIStat // #114
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4GameBuiltin_StatTableRow
	// Total Stat = InitializeNPCStatDataID + MainWeaponDataID (Stat)

	UPROPERTY(VisibleAnywhere)
	float Health_Point; // 피, 체력

	UPROPERTY(VisibleAnywhere)
	float Mana_Point; // 마력, 기

	UPROPERTY(VisibleAnywhere)
	float Striking_Power; // 공격력

	UPROPERTY(VisibleAnywhere)
	float Defensive_Power; // 방어력

	UPROPERTY(VisibleAnywhere)
	float Hit_Ratio; // 명중률

	UPROPERTY(VisibleAnywhere)
	float Dodge_Ratio; // 회피률

	UPROPERTY(VisibleAnywhere)
	float Result_EXP; // 경험치

public:
	FT4GameBuiltin_AIStat()
		: Health_Point(0.0f)
		, Mana_Point(0.0f)
		, Striking_Power(0.0f)
		, Defensive_Power(0.0f)
		, Hit_Ratio(0.0f)
		, Dodge_Ratio(0.0f)
		, Result_EXP(0.0f)
	{
	}
};

USTRUCT()
struct FT4GameBuiltin_AIBehaviorData // #114
{
	GENERATED_USTRUCT_BODY()

public:
	// #114
	// FT4GameBuiltin_AIBehaviorData : AIController (Runtime)
	// FT4GameBuiltin_NPCBehaviorData : NPC DataTable (Original Set)
	// FT4GameBuiltin_OverrideNPCBehaviorData : Spawn Asset (Instance Set)

	UPROPERTY(VisibleAnywhere)
	ET4GameBuiltin_EnemyType EnemyType; // #104

	UPROPERTY(VisibleAnywhere)
	bool bAggressive; // #50

	UPROPERTY(VisibleAnywhere)
	float PassiveApproachTimeSec; // #50

	UPROPERTY(VisibleAnywhere)
	float AgentRadius; // #114 : Agent 크기 및 Attack/Stop Distance 에서 겹치지 않기 위한 값으로 사용. WorldActor 의 CapsuleRadius 보다 커야함

	UPROPERTY(VisibleAnywhere)
	float SensoryRange; // #50

	UPROPERTY(VisibleAnywhere)
	float RomaingRange; // #50

	UPROPERTY(VisibleAnywhere)
	float RoamingRateRatio; // #50

public:
	FT4GameBuiltin_AIBehaviorData()
		: EnemyType(ET4GameBuiltin_EnemyType::NoEnemy) // #104
		, bAggressive(false)
		, PassiveApproachTimeSec(5.0f/*60.0f * 5.0f*/)
		, AgentRadius(50.0f) // #114 : Agent 크기 및 Attack/Stop Distance 에서 겹치지 않기 위한 값으로 사용. WorldActor 의 CapsuleRadius 보다 커야함
		, SensoryRange(1000.0f)
		, RomaingRange(1000.0f)
		, RoamingRateRatio(30.0f)
	{
	}
};