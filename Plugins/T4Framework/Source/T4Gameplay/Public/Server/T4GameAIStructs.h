// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameAITypes.h"
#include "Public/T4GameDataTypes.h"
#include "T4Asset/Public/T4AssetDefinitions.h"
#include "T4Framework/Public/T4FrameworkTypes.h"
#include "T4GameAIStructs.generated.h"

/**
  * #114
 */
USTRUCT()
struct FT4GameAIStat // #114
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4ContentStatTableRow
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
	FT4GameAIStat()
#if WITH_EDITOR
		: Health_Point(1000000000.0f) // #135 : 아무 세팅이 없으면 죽지 말자
#else
		: Health_Point(0.0f)
#endif
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
struct FT4GameAIBehaviorData // #114
{
	GENERATED_USTRUCT_BODY()

public:
	// #114
	// FT4GameAIBehaviorData : AIController (Runtime)
	// FT4GameNPCBehaviorData : NPC DataTable (Original Set)
	// FT4GameOverrideNPCBehaviorData : Spawn Asset (Instance Set)

	UPROPERTY(VisibleAnywhere)
	ET4GameEnemyType EnemyType; // #104

	UPROPERTY(VisibleAnywhere)
	bool bAggressive; // #50

	UPROPERTY(VisibleAnywhere)
	float ActiveOrKeepAggroTimeSec; // #50

	UPROPERTY(VisibleAnywhere)
	float AgentRadius; // #114 : Agent 크기 및 Attack/Stop Distance 에서 겹치지 않기 위한 값으로 사용 (WorldActor = BoundRadius)

	UPROPERTY(VisibleAnywhere)
	float SensoryRange; // #50

	UPROPERTY(VisibleAnywhere)
	float RomaingRange; // #50

	UPROPERTY(VisibleAnywhere)
	float RoamingRate; // #50

public:
	FT4GameAIBehaviorData()
		: EnemyType(ET4GameEnemyType::NoEnemy) // #104
		, bAggressive(false)
		, ActiveOrKeepAggroTimeSec(5.0f/*60.0f * 5.0f*/)
		, AgentRadius(50.0f) // #114 : Agent 크기 및 Attack/Stop Distance 에서 겹치지 않기 위한 값으로 사용 (WorldActor = BoundRadius)
		, SensoryRange(1000.0f)
		, RomaingRange(1000.0f)
		, RoamingRate(30.0f)
	{
	}
};

struct FT4GameAIMemory // #50 : 필요하다면 Blackboard 로 변경하겠지만, 현재는 장점이 없어보인다.
{
	FT4GameAIMemory()
	{
		Reset();
	}

	void Reset()
	{
		AIState = ET4GameAIState::Ready;
		AITaskState = ET4GameAITaskState::None;

		TargetLocation = FVector::ZeroVector;
		TargetObjectID.Empty();

		IdleWaitTime = 5.0f;

		StanceName = T4Const_DefaultStanceName; // #140
		PostureName = T4Const_DefaultPostureName; // #106
		MoveSpeedSelected = 0.0f;

		bActiveAbnormalStatus = false; // #135
		ActiveAbnormalStatusClearTimeLeft = 0.0f; // #135 : bActiveAbnormalStatus Clear 시간

		bActiveSkillCasting = false;
		bActiveSkillPending = false;
		SkillCastingDurationSec = 0.0f;
		ActiveSkillCastingClearTimeLeft = 0.0f;

		bActiveSkillUsing = false;
		SkillUsingDurationSec = 0.0f;
		ActiveSkillUsingClearTimeLeft = 0.0f;

		bActiveApproaching = false; // #135
		ActiveApproachingClearTimeLeft = 0.0f; // #135 : 타겟 위치로 지정 시간동안 도착하지 못하면 재타겟 하도록 처리 (제자리 걸음)

		bActiveAggro = false;
		ActiveOrKeepAggroTimeLeft = 0.0f;

#if WITH_EDITOR
		Health_Point = 1000000000.0f; // #135 : 아무 세팅이 없으면 죽지 말자
#else
		Health_Point = 0.0f; // #135 : 아무 세팅이 없으면 죽지 말자
#endif
	}

	ET4GameAIState AIState;
	ET4GameAITaskState AITaskState;

	FVector TargetLocation;
	FT4ObjectID TargetObjectID;

	float IdleWaitTime;

	FName StanceName; // #140
	FName PostureName; // #106
	float MoveSpeedSelected; // #106

	bool bActiveAbnormalStatus; // #135
	float ActiveAbnormalStatusClearTimeLeft; // #135 : bActiveAbnormalStatus Clear 시간

	FT4GameSkillDataID SkillDataIDSelected; // #135

	bool bActiveSkillCasting;
	bool bActiveSkillPending;
	float SkillCastingDurationSec; // #114 : Skill Duration
	float ActiveSkillCastingClearTimeLeft; // #114 : bActiveAttack Clear 시간

	bool bActiveSkillUsing;
	float SkillUsingDurationSec; // #114 : Skill Duration
	float ActiveSkillUsingClearTimeLeft; // #114 : bActiveAttack Clear 시간

	bool bActiveApproaching; // #135
	float ActiveApproachingClearTimeLeft; // #135 : 타겟 위치로 지정 시간동안 도착하지 못하면 재타겟 하도록 처리 (제자리 걸음)

	bool bActiveAggro; // #114 : Hit 를 당할 경우
	float ActiveOrKeepAggroTimeLeft; // #114 : Aggro 유지 시간

	float Health_Point; // 피, 체력
};
