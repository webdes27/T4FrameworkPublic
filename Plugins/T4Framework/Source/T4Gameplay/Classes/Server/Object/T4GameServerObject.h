// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4GameTypes.h"
#include "Public/T4GameDataTypes.h" // #48
#include "Public/Server/T4GameAIStructs.h" // #114
#include "T4Framework/Classes/Object/T4GameObjectBase.h"
#include "T4Framework/Public/T4FrameworkTypes.h"
#include "T4GameServerObject.generated.h"

/**
  * #114 : BP 로 노출해서 게임 로직에서 사용한다.
 */
struct FT4GameUseSkillInfo // #60
{
	FT4GameSkillDataID SkillDataID; // #50
	FT4GameEffectDataID ResultEffectDataID; // #63

	ET4GameplayAttackType AttackType; // #63

	bool bMoveable;
	bool bLockOn;
	bool bAiming; // #113
	float SkillDataHitDelayTimeSec;
	float SkillDataDurationSec;
	float ProjectileSpeed; // #63
};

struct FT4GameSkillTargetInfo
{
	ET4GameAttackTarget TargetType; // #112
	FT4ObjectID TargetObjectID; // #63 : 타겟이 있으면 먼저 체크! 없으면 Direct 을 사용한다.
	FName TargetHitBone; // #112 : TargetActorID Valid 일 경우만, 현재는 순수 비쥬얼 용도
	FVector TargetLocationOrDirection; // #49, #68 : Area, #112

	float TargetDistance; // #63
};

class IT4WorldActor;
class IT4PlayerController;
class IT4NPCAIControllerBase;
struct FT4GameOverrideBehaviorData;
struct FT4GamePlayerData;
struct FT4GameNPCData;
struct FT4GameWeaponData;
struct FT4GameCostumeData;
struct FT4GameSkillData;
struct FT4GameEffectData;
struct FT4GameStatData;
struct FT4GamePacketSC_Base;
UCLASS()
class T4GAMEPLAY_API UT4GameServerObject : public UT4GameObjectBase
{
	GENERATED_UCLASS_BODY()

public:
	// IT4GameObject
	ET4ControllerType GetControllerType() const override { return ControllerType; } // #114
	IT4ObjectController* GetObjectController() const override { return ObjectControllerRef; } // #114 : Server All, Client Player Only

	bool IsServerObject() const override { return true; }

public:
	const FT4ActorID& GetControlActorID() const { return ControlActorID; } // #114 : ActorID 기억! 현재는 ObjectID.Value 와 같다. 이후 교체가 되어야 할 수 있음

	const FT4GameAIMemory& GetAIMemoryConst() const { return AIMemory; } // #50
	const FT4GameAIStat& GetAIStatConst() const { return AIStat; } // #114
	const FT4GameAIBehaviorData& GetAIBehaviorDataConst() const { return AIBehaviorData; } // #114

	FName GetAIStateName() const; // #114
	FName GetRaceName() const;
	float GetMaxMoveSpeed();
	float GetMaxAttackRange() const; // #114 : 기본 캐릭터 공격 거리에 무기를 더한 최대 공격 거리

	const FT4GameDataID& GetGameDataID() const { return GameDataID; }

	void SetOverrideNPCBehaviorData(const FT4GameOverrideBehaviorData* InBehaviorData); // #114

	bool ValidTargetObjectID() const; // #114
	void SetTargetObjectID(const FT4ObjectID& InTargetObjectID); // #52
	void ClearTargetObjectID(); // #114

	void SetTargetLocation(const FVector& InTargetLocation); // #52
	void ClearTargetLocation(); // #114

	void SetMainWeaponDataID(const FT4GameDataID& InMainWeaponDataID) { MainWeaponDataID = InMainWeaponDataID; } // #48
	const FT4GameDataID& GetMainWeaponDataID() const { return MainWeaponDataID; } // #4

	void ActiveOrKeepAggro(); // #114

	IT4WorldActor* GetControlActor() const;

	IT4WorldActor* FindWorldActor(const FT4ObjectID& InObjectID) const;
	IT4WorldActor* FindNearestActor(float InMaxDistance); // #50
	
	bool FindNearestWorldActors(float InMaxDistance, TArray<IT4WorldActor*>& OutActors); // #114

	bool IsDead() const { return (ET4GameAIState::Dead == AIMemory.AIState) ? true : false; }
	bool IsAttacking() const; // #114 : 공격 중인가...
	bool IsAttackable() const; // #114 : 공격이 가능한 상황, bAggressive || bAggravated
	
	bool CheckAttackDistance(const FT4ObjectID& InTargetObjectID, float InOffsetDistance); // #104 : AttackTarget 이 Normal Attack 이 가능한 거리인지 체크!
	bool CheckValidTarget(IT4WorldActor* InTargetActor); // #104 : tribe 와 enemy 설정을 보고 hit 전달 여부를 결정해야 한다.

	IT4WorldActor* FindNearestEnemyByAttackRange(); // #50
	IT4WorldActor* FindNearestEnemyBySensoryRange(); // #34, #50

	bool GetApproachLocation(const FT4ObjectID& InTargetObjectID, FVector& OutLocation); // #50, #114
	bool GetApproachLocation(
		const FVector& InStart, 
		const FVector& InEnd, 
		float InSourceRadius, // #114 : AgnetRadius or AgentRadius + Weapon Length
		float InTargetRadius, 
		FVector& OutLocation
	); // #50
	bool GetRoamingLocation(FVector& OutLocation); // #114

	bool OnAttack(const FT4ObjectID& InTargetObjectID);

	void OnUpdateMoveSpeed(bool bMoving);

	void HandleOnHitOverlap(const FName& InEventName, IT4WorldActor* InHitWorldActor, const FHitResult& InSweepResult); // #49
	void ClearHitOverlapEvent(); // #49

	bool IsServerRunning() const;
	bool HasServerGameplayCustomSettings() const;
#if WITH_EDITOR
	IT4EditorGameplayContoller* GetEditorGameplayController() const; // #60
#endif

public:
	// Send Packet Process
	//
	bool OnBroadcastPacket(FT4GamePacketSC_Base* InPacketCS); // #114 : 브로드캐스팅
	bool OnSendPacket(IT4PlayerController* TargetPC, FT4GamePacketSC_Base* InPacketCS); // #114 : 자기 자신에게만 Packet 전송
	
	bool OnPacketProcess(FT4GamePacketSC_Base* InPacketCS); // #114 : 패킷 전송을 하지 않고, 로컬 실행

public:
	// Recv Packet Process
	//
	bool OnRecvLeave(IT4PlayerController* InSenderPC);
	
#if WITH_EDITOR
	bool OnRecvEnterWithEditor(bool bInPossess); // #114
#endif

	bool OnRecvEnterPlayer(
		IT4PlayerController* InSenderPC,
		const FT4GameDataID& InPlayerDataID,
		const FVector& InSpawnLocation,
		const FRotator& InSpawnRotation,
		bool bInPossess
	);
	bool OnRecvEnterNPC(const FT4GameDataID& InNPCDataID, const FVector& InSpawnLocation, const FRotator& InSpawnRotation);
	bool OnRecvEnterItem(const FT4GameDataID& InItemDataID, const FVector& InSpawnLocation, const FRotator& InSpawnRotation);

	bool OnRecvChangePlayer(IT4PlayerController* InSenderPC, const FT4ActorID& InChangeActorID);

	bool OnRecvMove(const FVector& InLocation, float InHeadYawAngle); // #114 : Player : 어뷰징 대비
	bool OnRecvMove(const FVector& InVelocity, bool bForceMaxSpeed); // #42, #34, #114 : NPC Only
	bool OnRecvMoveStop(bool bSyncLocation);
	bool OnRecvMoveSpeedSync(float InMoveSpeed); // #114 : NPC Only

	bool OnRecvJump(const FVector& InJumpDirection);
	bool OnRecvRoll(const FVector& InRollDirection);
	bool OnRecvTurn(float InTargetYawAngle);

	bool OnRecvLockOn(float InHeadYawAngle);
	bool OnRecvLockOff(float InHeadYawAngle);

	bool OnRecvTeleport(const FVector& InTargetLocation);

	bool OnRecvAimSet(const FT4GameDataID& InSkillDataID, bool bInAimingStart, const FVector& InTargetLocation);
	bool OnRecvAimClear();

	bool OnProcessSkillTarget(
		const FT4GameUseSkillInfo& InUseSkillInfo,
		FT4GameSkillTargetInfo& InSkillTargetInfo
	); // #114
	bool OnRecvSkillTarget(
		const FT4GameDataID& InSkillDataID,
		ET4GameAttackTarget InTargetType, // #112
		const FT4ObjectID& InTargetObjectID, // #63 : 타겟이 있으면 먼저 체크! 없으면 Direct 을 사용한다.
		FName InTargetHitBone, // #112 : TargetActorID Valid 일 경우만, 현재는 순수 비쥬얼 용도
		const FVector& InTargetLocationOrDirection // #49, #68 : Area, #112
	);
	
	bool OnRecvEffectDirect(const FT4GameDataID& InEffectDataID, const FT4ObjectID& InAttackerID);

	bool OnRecvChangeStance(FName InStanceName);
	bool OnRecvChangeSubStance(FName InSubStanceName);

	bool OnRecvEquipItem(const FT4GameDataID& InWeaponDataID, bool bInMainWeapon);
	bool OnRecvUnequipItem(const FT4GameDataID& InWeaponDataID, bool bInMainWeapon);
	bool OnRecvExchangeItem(const FT4GameDataID& InCostumeDataID);

	bool OnRecvDie(const FName InReactionName, const FT4ObjectID& InAttackerID); // #114

protected:
	friend class AT4GameNPCAIController;

	void Initialize() override;
	void Finalize() override;

	void Process(float InDeltaTime) override;

	bool EnterBeginCommon();
	void EnterEndCommon();
	void LeaveCommon();

	bool UpdateAIBehaviorData(); // #114
	bool UpdateAIStat(); // #114

	void AIUpdate(float InDeltaTime); // #114 : AIController 의 AIUpdate 에서 호출
	void AIStart(); // #114 : AIController 의 NotifyAIStart 에서 호출
	void AIEnd(); // #114 : AIController 의 NotifyAIEnd 에서 호출

	FT4GameAIMemory& GetAIMemory() { return AIMemory; } // #114 : Only

	UT4GameServerObject* GetServerObject(const FT4ObjectID& InObjectID) const;

	const FT4GamePlayerData* GetPlayerData() const;
	const FT4GameNPCData* GetNPCData() const;
	const FT4GameWeaponData* GetWeaponData() const;
	const FT4GameCostumeData* GetCostumeData() const;

	const FT4GameSkillData* GetSkillData(const FT4GameDataID& InSkillDataID) const;
	const FT4GameEffectData* GetEffectData(const FT4GameDataID& InEffectDataID) const;
	const FT4GameWeaponData* GetWeaponData(const FT4GameDataID& InWeaponDataID) const;
	const FT4GameCostumeData* GetCostumeData(const FT4GameDataID& InCostumeDataID) const;
	const FT4GameStatData* GetStatData(const FT4GameDataID& InStatDataID) const;

	void SetDefaultStat(const FT4GameDataID& InStatDataID); // #114
	void AddWeaponStat(const FT4GameDataID& InWeaponDataID); // #114

	bool GetSkillInfoFromEditor(const FT4GameDataID& InSkillDataID, FT4GameUseSkillInfo& OutSkillInfo);
	bool GetSkillInfoFromGameData(const FT4GameDataID& InSkillDataID, FT4GameUseSkillInfo& OutSkillInfo);
	bool GetSkillDataIDSelected(const FName InSubStanceName, FT4GameSkillDataID& OutSkillData); // TODO

private:
	bool bEntered;

	ET4ControllerType ControllerType; // #114
	IT4ObjectController* ObjectControllerRef; // #114

	FT4ActorID ControlActorID; // #114 : ActorID 기억! 현재는 ObjectID.Value 와 같다. 이후 교체가 되어야 할 수 있음
	FT4GameDataID GameDataID;
	FT4GameDataID MainWeaponDataID;

	FT4GameAIMemory AIMemory; // #50 : 필요하다면 Blackboard 로 변경하겠지만, 현재는 장점이 없어보인다.
	FT4GameAIStat AIStat; // #114
	FT4GameAIBehaviorData AIBehaviorData; // #114

protected:
	bool bHitOverlapEventStarted;
	FT4GameSkillDataID UseSkillDataID;
	float HitOverlapEventDelayTimeSec;
	float HitOverlapEventClearTimeLeft;
	FDelegateHandle HitOverlapDelegateHandle; // #49
};
