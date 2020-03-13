// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4GameTypes.h" // #114
#include "Public/T4GameDataTypes.h" // #48
#include "T4Framework/Classes/Object/T4GameObjectBase.h"
#include "T4Framework/Public/T4FrameworkTypes.h"
#include "T4GameClientObject.generated.h"

/**
  * #114 : BP 로 노출해서 게임 로직에서 사용한다.
 */
class IT4Framework;
class IT4WorldSystem;
class IT4WorldActor;
class IT4PlayerController;
struct FT4GamePlayerData;
struct FT4GameNPCData;
struct FT4GameWeaponData;
struct FT4GameCostumeData;
struct FT4GameSkillData;
struct FT4GameEffectData;
struct FT4GamePacketCS_Base;
UCLASS()
class T4GAMEPLAY_API UT4GameClientObject : public UT4GameObjectBase
{
	GENERATED_UCLASS_BODY()
		
public:
	// IT4GameObject
	ET4ControllerType GetControllerType() const override { return ET4ControllerType::Controller_Player; } // #114
	IT4ObjectController* GetObjectController() const override; // #114 : Server All, Client Player Only

public:
	// UT4GameObjectBase
	bool IsClientObject() const override { return true; }

public:
	const FT4ActorID& GetControlActorID() const { return ControlActorID; } // #114 : ActorID 기억! 현재는 ObjectID.Value 와 같다. 이후 교체가 되어야 할 수 있음

	void SetMainWeaponDataID(const FT4GameDataID& InMainWeaponDataID) { MainWeaponDataID = InMainWeaponDataID; } // #48
	const FT4GameDataID& GetMainWeaponDataID() const { return MainWeaponDataID; } // #48

#if WITH_EDITOR
	void SetClientMode(bool bInClientMode) { bClientModeOnly = bInClientMode; } // #118 : 서버로의 패킷 전송을 막는다.
#endif

public:
	// Send Packet Process
	//
	bool OnSendPacket(FT4GamePacketCS_Base* InPacketCS); // #114

public:
	// Recv Packet Process
	//
	bool OnRecvLeave(float InFadeOutSec);

#if WITH_EDITOR
	bool OnRecvEnterWithEditor(
		const FT4EntityKey& InEntityKey,
		const FVector& InSpawnLocation,
		const FRotator& InSpawnRotation,
		bool bInPossess
	); // #114
#endif

	bool OnRecvEnterPlayer(
		const FT4GameDataID& InPlayerDataID,
		const FVector& InSpawnLocation,
		const FRotator& InSpawnRotation,
		bool bInPossess
	);
	bool OnRecvEnterNPC(const FT4GameDataID& InNPCDataID, const FVector& InSpawnLocation, const FRotator& InSpawnRotation);
	bool OnRecvEnterItem(const FT4GameDataID& InNPCDataID, const FVector& InSpawnLocation, const FRotator& InSpawnRotation);

	bool OnRecvMove(
		const FVector& InMoveToLocation,
		float InHeadYawAngle,
		bool bInForceMaxSpeed,
		const FVector& InServerNavPoint,
		const FVector& InServerDirection
	);
	bool OnRecvMoveStop(const FVector& InStopLocation, float InHeadYawAngle, bool bInSyncLocation);
	bool OnRecvMoveSpeedSync(float InMoveSpeed);

	bool OnRecvJump(const FVector& InJumpVelocity);
	bool OnRecvRoll(const FVector& InRollVelocity);
	bool OnRecvTurn(float InTargetYawAngle);
	bool OnRecvTeleport(const FVector& InTargetLocation);

	bool OnRecvLockOn(float InHeadYawAngle);
	bool OnRecvLockOff(float InHeadYawAngle);

	bool OnRecvAimSet(const FT4GameDataID& InSkillDataID, bool bInAimingStart, const FVector& InTargetLocation);
	bool OnRecvAimClear();

	bool OnRecvSkillTarget(
		const FT4GameDataID& InSkillDataID,
		ET4GameAttackTarget InTargetType,
		const FT4ObjectID& InTargetObjectID, // #63 : 타겟이 있으면 먼저 체크! 없으면 Direct 을 사용한다.
		FName InTargetHitBone, // #112 : TargetActorID Valid 일 경우만, 현재는 순수 비쥬얼 용도
		const FVector& InTargetLocationOrDirection, // #49, #68, #112
		float InProjectileDurationSec // #63 : Range Attack 이라면 ProjectileSpeed 로 계산된 Duration 시간이 넘어온다.
	);
	bool OnRecvEffectDirect(const FT4GameDataID& InEffectDataID, const FT4ObjectID& InAttackerObjectID);

	bool OnRecvChangeStance(FName InStanceName);
	bool OnRecvChangeSubStance(FName InSubStanceName);

	bool OnRecvEquipItem(
		const FT4GameDataID& InWeaponDataID, 
		bool bInMainWeapon,
		const FT4GameDataID& InUnequipItemWeaponDataID // #114, #116 : 이전에 장착한 무기가 있다면 해제 후 장착
	);
	bool OnRecvUnequipItem(const FT4GameDataID& InWeaponDataID, bool bInMainWeapon);
	bool OnRecvExchangeItem(const FT4GameDataID& InCostumeDataID);

	bool OnRecvDie(FName InReactionName, const FT4ObjectID& InAttackerObjectID);
	bool OnRecvResurrect();

protected:
	void Initialize() override;
	void Finalize() override;

	IT4WorldActor* GetControlActor() const;
	IT4PlayerController* GetPlayerController() const;

	IT4WorldActor* FindWorldActor(const FT4ObjectID& InObjectID) const;

	const FT4GamePlayerData* GetPlayerData();
	const FT4GameNPCData* GetNPCData();
	const FT4GameWeaponData* GetWeaponData();
	const FT4GameCostumeData* GetCostumeData();

	const FT4GameSkillData* GetSkillData(const FT4GameDataID& InSkillDataID);
	const FT4GameEffectData* GetEffectData(const FT4GameDataID& InEffectDataID);
	const FT4GameWeaponData* GetWeaponData(const FT4GameDataID& InWeaponDataID);
	const FT4GameCostumeData* GetCostumeData(const FT4GameDataID& InCostumeDataID);

private:
	bool bEntered;
#if WITH_EDITOR
	bool bClientModeOnly; // #118 : 툴용 flag 로 서버로의 패킷 전송을 막는다.
#endif
	FT4ActorID ControlActorID; // #114 : ActorID 기억! 현재는 ObjectID.Value 와 같다. 이후 교체가 되어야 할 수 있음
	FT4GameDataID GameDataID;
	FT4GameDataID MainWeaponDataID;
};
