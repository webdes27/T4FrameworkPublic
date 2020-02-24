// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameBuiltin_AITypes.h"
#include "Public/T4GameBuiltin_GameDataTypes.h"
#include "T4Asset/Public/T4AssetDefinitions.h"
#include "T4Framework/Public/T4FrameworkTypes.h"
#include "T4GameBuiltin_AIStructs.generated.h"

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
	float Hit_Rate; // 명중률

	UPROPERTY(VisibleAnywhere)
	float Dodge_Rate; // 회피률

	UPROPERTY(VisibleAnywhere)
	float Result_EXP; // 경험치

public:
	FT4GameBuiltin_AIStat()
		: Health_Point(0.0f)
		, Mana_Point(0.0f)
		, Striking_Power(0.0f)
		, Defensive_Power(0.0f)
		, Hit_Rate(0.0f)
		, Dodge_Rate(0.0f)
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
	float ActiveOrKeepAggroTimeSec; // #50

	UPROPERTY(VisibleAnywhere)
	float AgentRadius; // #114 : Agent 크기 및 Attack/Stop Distance 에서 겹치지 않기 위한 값으로 사용. WorldActor 의 CapsuleRadius 보다 커야함

	UPROPERTY(VisibleAnywhere)
	float SensoryRange; // #50

	UPROPERTY(VisibleAnywhere)
	float RomaingRange; // #50

	UPROPERTY(VisibleAnywhere)
	float RoamingRate; // #50

public:
	FT4GameBuiltin_AIBehaviorData()
		: EnemyType(ET4GameBuiltin_EnemyType::NoEnemy) // #104
		, bAggressive(false)
		, ActiveOrKeepAggroTimeSec(5.0f/*60.0f * 5.0f*/)
		, AgentRadius(50.0f) // #114 : Agent 크기 및 Attack/Stop Distance 에서 겹치지 않기 위한 값으로 사용. WorldActor 의 CapsuleRadius 보다 커야함
		, SensoryRange(1000.0f)
		, RomaingRange(1000.0f)
		, RoamingRate(30.0f)
	{
	}
};

struct FT4GameBuiltin_AIMemory // #50 : 필요하다면 Blackboard 로 변경하겠지만, 현재는 장점이 없어보인다.
{
	FT4GameBuiltin_AIMemory()
	{
		Reset();
	}

	void Reset()
	{
		AIState = ET4GameBuiltin_AIState::Ready;
		AITaskState = ET4GameBuiltin_AITaskState::None;

		TargetLocation = FVector::ZeroVector;
		TargetObjectID.Empty();

		IdleWaitTime = 5.0f;

		SubStanceName = T4Const_DefaultSubStanceName; // #106
		MoveSpeedSelected = 0.0f;

		bActiveAttack = false;
		AttackDurationSec = 0.0f;
		ActiveAttackClearTimeLeft = 0.0f;

		bActiveAggro = false;
		ActiveOrKeepAggroTimeLeft = 0.0f;
	}

	ET4GameBuiltin_AIState AIState;
	ET4GameBuiltin_AITaskState AITaskState;

	FVector TargetLocation;
	FT4ObjectID TargetObjectID;

	float IdleWaitTime;

	FName SubStanceName; // #106
	float MoveSpeedSelected; // #106

	bool bActiveAttack;
	float AttackDurationSec; // #114 : Skill Duration
	float ActiveAttackClearTimeLeft; // #114 : bActiveAttack Clear 시간

	bool bActiveAggro; // #114 : Hit 를 당할 경우
	float ActiveOrKeepAggroTimeLeft; // #114 : Aggro 유지 시간
};
