// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4GameBuiltin_Types.h"
#include "Public/T4GameBuiltin_GameDataTypes.h" // #48

#include "T4Engine/Public/Asset/T4AssetLoader.h" // #50, #42
#include "T4Framework/Classes/Controller/AI/T4NPCAIController.h"
#include "T4GameBuiltin_CreatureAIController.generated.h"

/**
  * WARN : AI Controller 는 서버에서만 사용하고, 클라리언트에서는 사용하지 않음에 유의할 것!
  * #114
 */
struct FT4CreatureAIMemory // #50
{
	FT4CreatureAIMemory()
	{
		Reset();
	}

	void Reset()
	{
		AIState = ET4GameBuiltin_AIState::Ready;
		HealthPoint = 0.0f;

		// TODO : #114 : 임시!! 아래 프로퍼티는 모두 정리!!

		InitSpawnLocation = FVector::ZeroVector;
		MoveTargetLocation = FVector::ZeroVector;
		AttackTargetObjectID.Empty();

		IdleWaitTime = 5.0f;

		bActiveSkill = false;
		SkillPlayTimeLeft = 0.0f;

		bActiveAggressive = false;
		AggressiveClearTimeLeft = 0.0f;

		SubStanceName = T4Const_DefaultSubStanceName; // #106
		MoveSpeedSelected = 0.0f;
	}

	ET4GameBuiltin_AIState AIState;

	float HealthPoint; // #114

	// TODO : #114 : 임시!! 아래 프로퍼티는 모두 정리!!

	FVector InitSpawnLocation;
	FVector MoveTargetLocation;

	FT4ObjectID AttackTargetObjectID;

	float IdleWaitTime;

	bool bActiveSkill;
	float SkillPlayTimeLeft;

	bool bActiveAggressive;
	float AggressiveClearTimeLeft;

	FName SubStanceName; // #106
	float MoveSpeedSelected; // #106
};

class UBehaviorTree;
class IT4WorldActor;
class UT4GameBuiltin_ServerObject;
UCLASS()
class T4GAMEBUILTIN_API AT4GameBuiltin_CreatureAIController : public AT4NPCAIController
{
	GENERATED_UCLASS_BODY()

public:
	// class IT4ObjectController
#if (WITH_EDITOR || WITH_SERVER_CODE)
	void OnNotifyAIEvent(const FName& InEventName, const FT4ObjectID& InSenderObjectID) override; // #63
#endif

public:
	bool Bind(const FT4GameBuiltin_GameDataID& InNPCGameDataID); // #31, #50

	bool IsDead() const { return (ET4GameBuiltin_AIState::Dead == AIMemory.AIState) ? true : false; } // #114

	bool IsMoving() const; // #50
	bool IsAttacking() const; // #50
	bool IsCurrentAggressive() const; // #50
	bool IsOriginAggressive() const; // #104 : 테이블 설정

	bool DoRoaming(FVector& OutTargetLocation); // #50
	bool DoAttack(const FT4ObjectID& InTargetObjectID); // #50
	bool DoMoveTo(const FVector& InMoveVelocity, bool bForceMaxSpeed); // #52
	bool DoMoveStop(bool bSyncLocation); // #52

	bool DoDie(); // 임시! Die Test

	bool DoUpdateMoveSpeed(bool bMoving); // #52
	bool DoUpdateSubStance(FName InSubStanceName); // #108

	const FT4CreatureAIMemory& GetAIMemoryConst() const { return AIMemory; } // #50

	void SetMoveTargetLocation(const FVector& InMoveTargetLocation) // #52
	{
		AIMemory.MoveTargetLocation = InMoveTargetLocation;
	}
	void SetAttackTargetObjectID(const FT4ObjectID& InTargetObjectID) // #52
	{
		AIMemory.AttackTargetObjectID = InTargetObjectID;
	}

	void UpdateAggressive();

	UT4GameBuiltin_ServerObject* GetServerObject() const; // #114
	IT4ObjectController* GetObjectController(const FT4ObjectID& InObjectID) const; // #114

protected:
	void NotifyAdvance(float InDeltaTime) override; // #114
	void NotifyBeginPlay() override; // #50
	void NotifyEndPlay() override; // #50
	void NotifyAIStart() override; // #50
	void NotifyAIEnd() override; // #50

	void HandleOnHitOverlap(const FName& InEventName, IT4WorldActor* InHitWorldActor, const FHitResult& InSweepResult); // #49
	void ClearHitOverlapEvent(); // #49

	void HandleOnCallbackMoveTo(const FVector& InMoveVelocity, bool bForceMaxSpeed); // #42, #34, #52 : MovementComponet::MaxSpeed 를 사용할지에 대한 Flag, 기본값이 false 로 Velocity 에서 Speed 를 얻는다. 동기화 이슈!!
	void HandleOnCallbackMoveStop(); // #52

private:
	bool CheckAsyncLoading();

private:
	ET4AIDataLoadState AIDataLoadState; // #50
	FT4BehaviorTreeAssetLoader BehaviorTreeAssetLoader;

	/* Cached BT component */
	UPROPERTY(transient)
	UBehaviorTree* BehaviorTreeAsset;

	FT4CreatureAIMemory AIMemory; // #50 : 필요하다면 Blackboard 로 변경하겠지만, 현재는 장점이 없어보인다.

	bool bHitOverlapEventStarted;
	FT4GameBuiltin_GameSkillDataID UseSkillDataID;
	float HitOverlapEventDelayTimeSec;
	float HitOverlapEventClearTimeLeft;
	FDelegateHandle HitOverlapDelegateHandle; // #49
};
