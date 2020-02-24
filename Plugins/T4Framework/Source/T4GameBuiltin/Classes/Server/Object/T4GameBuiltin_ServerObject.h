// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4GameBuiltin_Types.h"
#include "Public/T4GameBuiltin_GameDataTypes.h" // #48
#include "Public/Server/T4GameBuiltin_AIStructs.h" // #114
#include "T4Framework/Classes/Object/T4GameObject.h"
#include "T4Framework/Public/T4FrameworkTypes.h"
#include "T4GameBuiltin_ServerObject.generated.h"

/**
  * #114 : BP 로 노출해서 게임 로직에서 사용한다.
 */
class IT4WorldActor;
class IT4PlayerController;
class IT4GameBuiltin_ClientPacketHandler;
class IT4NPCAIController;
struct FT4GameBuiltin_OverrideNPCBehaviorData;
struct FT4GameBuiltin_GamePlayerData;
struct FT4GameBuiltin_GameNPCData;
struct FT4GameBuiltin_GameWeaponData;
struct FT4GameBuiltin_GameCostumeData;
struct FT4GameBuiltin_GameSkillData;
struct FT4GameBuiltin_PacketSC_Base;
UCLASS()
class T4GAMEBUILTIN_API UT4GameBuiltin_ServerObject : public UT4GameObject
{
	GENERATED_UCLASS_BODY()

public:
	// IT4GameObject
	ET4ControllerType GetControllerType() const override { return ControllerType; } // #114
	IT4ObjectController* GetObjectController() const override { return ObjectControllerRef; } // #114 : Server All, Client Player Only

	bool IsServerObject() const override { return true; }

public:
	// AIContoller
	//
	const FT4ActorID& GetControlActorID() const { return ControlActorID; } // #114 : ActorID 기억! 현재는 ObjectID.Value 와 같다. 이후 교체가 되어야 할 수 있음

	const FT4GameBuiltin_AIMemory& GetAIMemoryConst() const { return AIMemory; } // #50
	const FT4GameBuiltin_AIStat& GetAIStatConst() const { return AIStat; } // #114
	const FT4GameBuiltin_AIBehaviorData& GetAIBehaviorDataConst() const { return AIBehaviorData; } // #114

	FName GetAIStateName() const; // #114

	FName GetRaceName() const;
	float GetMaxMoveSpeed();
	float GetMaxAttackRange() const; // #114 : 기본 캐릭터 공격 거리에 무기를 더한 최대 공격 거리

	const FT4GameBuiltin_GameDataID& GetGameDataID() const { return GameDataID; }

	void SetOverrideNPCBehaviorData(const FT4GameBuiltin_OverrideNPCBehaviorData* InBehaviorData); // #114
	
	bool ValidTargetObjectID() const; // #114
	void SetTargetObjectID(const FT4ObjectID& InTargetObjectID); // #52
	void ClearTargetObjectID(); // #114

	void SetTargetLocation(const FVector& InTargetLocation); // #52
	void ClearTargetLocation(); // #114

	void SetMainWeaponDataID(const FT4GameBuiltin_GameDataID& InMainWeaponDataID) { MainWeaponDataID = InMainWeaponDataID; } // #48
	const FT4GameBuiltin_GameDataID& GetMainWeaponDataID() const { return MainWeaponDataID; } // #4

	void ActiveOrKeepAggro(); // #114

	IT4WorldActor* GetControlActor() const;

	IT4WorldActor* FindWorldActor(const FT4ObjectID& InObjectID) const;
	IT4WorldActor* FindNearestActor(float InMaxDistance); // #50
	
	bool FindNearestWorldActors(float InMaxDistance, TArray<IT4WorldActor*>& OutActors); // #114

	bool HasAttacking() const; // #114 : 공격 중인가...
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

	bool GetUseSkillDataIDSelected(const FName InSubStanceName, FT4GameBuiltin_GameSkillDataID& OutSkillData); // TODO : #114 : 연속기 처리! 현재는 랜덤

	bool IsServerRunning() const;
	bool HasServerGameplayCustomSettings() const;
#if WITH_EDITOR
	IT4EditorGameplayContoller* GetEditorGameplayController() const; // #60
#endif

public:
	// Send Packet Process
	//
	bool OnBroadcastPacket(FT4GameBuiltin_PacketSC_Base* InPacketCS); // #114 : 브로드캐스팅
	bool OnSendPacket(IT4PlayerController* TargetPC, FT4GameBuiltin_PacketSC_Base* InPacketCS); // #114 : 자기 자신에게만 Packet 전송
	
	bool OnPacketProcess(FT4GameBuiltin_PacketSC_Base* InPacketCS); // #114 : 패킷 전송을 하지 않고, 로컬 실행

public:
	// Recv Packet Process
	//
	bool OnRecvLeave(IT4PlayerController* InSenderPC);
	
#if WITH_EDITOR
	bool OnRecvEnterWithEditor(bool bInPossess); // #114
#endif

	bool OnRecvEnterPlayer(
		IT4PlayerController* InSenderPC,
		const FT4GameBuiltin_GameDataID& InPlayerDataID,
		const FVector& InSpawnLocation,
		const FRotator& InSpawnRotation,
		bool bInPossess
	);
	bool OnRecvEnterNPC(const FT4GameBuiltin_GameDataID& InNPCDataID, const FVector& InSpawnLocation, const FRotator& InSpawnRotation);
	bool OnRecvEnterItem(const FT4GameBuiltin_GameDataID& InItemDataID, const FVector& InSpawnLocation, const FRotator& InSpawnRotation);

	bool OnRecvChangePlayer(IT4PlayerController* InSenderPC, const FT4ActorID& InChangeActorID);

	bool OnRecvMove(const FVector& InLocation, float InHeadYawAngle); // #114 : Player : 어뷰징 대비
	bool OnRecvMove(const FVector& InVelocity, bool bForceMaxSpeed); // #42, #34, #114 : NPC Only
	bool OnRecvMoveStop(bool bSyncLocation); // #114 : NPC Only
	bool OnRecvMoveSpeedSync(float InMoveSpeed); // #114 : NPC Only

	bool OnRecvJump(const FVector& InJumpDirection);
	bool OnRecvRoll(const FVector& InRollDirection);
	bool OnRecvTurn(float InTargetYawAngle);

	bool OnRecvLockOn(float InHeadYawAngle);
	bool OnRecvLockOff(float InHeadYawAngle);

	bool OnRecvTeleport(const FVector& InTargetLocation);

	bool OnRecvAimSet(const FT4GameBuiltin_GameDataID& InSkillDataID, bool bInAimingStart, const FVector& InTargetLocation);
	bool OnRecvAimClear();

	bool OnRecvSkillTarget(
		const FT4GameBuiltin_GameDataID& InSkillDataID,
		ET4GameBuiltin_AttackTarget InTargetType, // #112
		const FT4ObjectID& InTargetObjectID, // #63 : 타겟이 있으면 먼저 체크! 없으면 Direct 을 사용한다.
		FName InTargetHitBone, // #112 : TargetActorID Valid 일 경우만, 현재는 순수 비쥬얼 용도
		const FVector& InTargetLocationOrDirection // #49, #68 : Area, #112
	);

	bool OnRecvChangeStance(FName InStanceName);
	bool OnRecvChangeSubStance(FName InSubStanceName);

	bool OnRecvEquipItem(const FT4GameBuiltin_GameDataID& InWeaponDataID, bool bInMainWeapon);
	bool OnRecvUnequipItem(const FT4GameBuiltin_GameDataID& InWeaponDataID, bool bInMainWeapon);
	bool OnRecvExchangeItem(const FT4GameBuiltin_GameDataID& InCostumeDataID);

protected:
	friend class AT4GameBuiltin_NPCAIController;

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

	FT4GameBuiltin_AIMemory& GetAIMemory() { return AIMemory; } // #114 : Only

	UT4GameBuiltin_ServerObject* GetServerObject(const FT4ObjectID& InObjectID) const;
	IT4GameBuiltin_ClientPacketHandler* GetClientPacketHandler() const;

	const FT4GameBuiltin_GamePlayerData* GetGamePlayerData() const;
	const FT4GameBuiltin_GameNPCData* GetGameNPCData() const;
	const FT4GameBuiltin_GameWeaponData* GetGameWeaponData() const;
	const FT4GameBuiltin_GameCostumeData* GetGameCostumeData() const;

	const FT4GameBuiltin_GameSkillData* GetGameSkillData(const FT4GameBuiltin_GameDataID& InSkillDataID) const;
	const FT4GameBuiltin_GameWeaponData* GetGameWeaponData(const FT4GameBuiltin_GameDataID& InWeaponDataID) const;
	const FT4GameBuiltin_GameCostumeData* GetGameCostumeData(const FT4GameBuiltin_GameDataID& InCostumeDataID) const;

	void SetDefaultStat(const FT4GameBuiltin_GameDataID& InStatDataID); // #114
	void AddWeaponStat(const FT4GameBuiltin_GameDataID& InWeaponDataID); // #114

private:
	bool bEntered;

	ET4ControllerType ControllerType; // #114
	IT4ObjectController* ObjectControllerRef; // #114

	FT4ActorID ControlActorID; // #114 : ActorID 기억! 현재는 ObjectID.Value 와 같다. 이후 교체가 되어야 할 수 있음
	FT4GameBuiltin_GameDataID GameDataID;
	FT4GameBuiltin_GameDataID MainWeaponDataID;

	FT4GameBuiltin_AIMemory AIMemory; // #50 : 필요하다면 Blackboard 로 변경하겠지만, 현재는 장점이 없어보인다.
	FT4GameBuiltin_AIStat AIStat; // #114
	FT4GameBuiltin_AIBehaviorData AIBehaviorData; // #114

protected:
	bool bHitOverlapEventStarted;
	FT4GameBuiltin_GameSkillDataID UseSkillDataID;
	float HitOverlapEventDelayTimeSec;
	float HitOverlapEventClearTimeLeft;
	FDelegateHandle HitOverlapDelegateHandle; // #49
};
