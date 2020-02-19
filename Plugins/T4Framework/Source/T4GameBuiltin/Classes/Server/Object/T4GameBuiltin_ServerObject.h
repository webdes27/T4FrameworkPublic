// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4GameBuiltin_Types.h"
#include "Public/T4GameBuiltin_GameDataTypes.h" // #48
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
	IT4ObjectController* GetController() const override; // #114 : Server All, Client Player Only

public:
	// UT4GameObject
	virtual bool IsPlayer() const override;

	bool IsServerObject() const override { return true; }

public:
	const FT4ActorID& GetWorldActorID() const { return WorldActorID; } // #114 : ActorID 기억! 현재는 ObjectID.Value 와 같다. 이후 교체가 되어야 할 수 있음

	bool OnLeave(IT4PlayerController* InSenderPC);
	
#if WITH_EDITOR
	bool OnEnterWithEditor(bool bInPossess); // #114
#endif

	bool OnEnterPlayer(
		IT4PlayerController* InSenderPC,
		const FT4GameBuiltin_GameDataID& InPlayerDataID,
		const FVector& InSpawnLocation,
		const FRotator& InSpawnRotation,
		bool bInPossess
	);
	bool OnEnterNPC(const FT4GameBuiltin_GameDataID& InNPCDataID, const FVector& InSpawnLocation, const FRotator& InSpawnRotation);
	bool OnEnterItem(const FT4GameBuiltin_GameDataID& InItemDataID, const FVector& InSpawnLocation, const FRotator& InSpawnRotation);

	bool OnChangePlayer(IT4PlayerController* InSenderPC, const FT4ActorID& InChangeActorID);

	bool OnMove(const FVector& InMoveToLocation, float InHeadYawAngle);
	bool OnJump(const FVector& InJumpDirection);
	bool OnRoll(const FVector& InRollDirection);
	bool OnTurn(float InTargetYawAngle);
	bool OnLockOn(float InHeadYawAngle);
	bool OnLockOff(float InHeadYawAngle);
	bool OnTeleport(const FVector& InTargetLocation);

	bool OnAimSet(const FT4GameBuiltin_GameDataID& InSkillDataID, bool bInAimingStart, const FVector& InTargetLocation);
	bool OnAimClear();

	bool OnSkillTarget(
		const FT4GameBuiltin_GameDataID& InSkillDataID,
		ET4GameBuiltin_AttackTarget InTargetType, // #112
		const FT4ObjectID& InTargetObjectID, // #63 : 타겟이 있으면 먼저 체크! 없으면 Direct 을 사용한다.
		FName InTargetHitBone, // #112 : TargetActorID Valid 일 경우만, 현재는 순수 비쥬얼 용도
		const FVector& InTargetLocationOrDirection // #49, #68 : Area, #112
	);

	bool OnChangeStance(FName InStanceName);
	bool OnChangeSubStance(FName InSubStanceName);

	bool OnEquipItem(const FT4GameBuiltin_GameDataID& InWeaponDataID, bool bInMainWeapon);
	bool OnUnequipItem(const FT4GameBuiltin_GameDataID& InWeaponDataID, bool bInMainWeapon);
	bool OnExchangeItem(const FT4GameBuiltin_GameDataID& InCostumeDataID);

public:
	void SetMainWeaponDataID(const FT4GameBuiltin_GameDataID& InMainWeaponDataID) { MainWeaponDataID = InMainWeaponDataID; } // #48
	const FT4GameBuiltin_GameDataID& GetMainWeaponDataID() const { return MainWeaponDataID; } // #4

	void HandleOnHitOverlap(const FName& InEventName, IT4WorldActor* InHitWorldActor, const FHitResult& InSweepResult); // #49 : Only Server
	void ClearHitOverlapEvent(); // #49 : Only Server

protected:
	void Initialize() override;
	void Finalize() override;

	void Process(float InDeltaTime) override;

	void EnterCommon();
	void LeaveCommon();

	IT4WorldActor* GetWorldActor() const;
	IT4NPCAIController* GetGameAIController() const;

	IT4GameBuiltin_ClientPacketHandler* GetClientPacketHandler() const;

	IT4WorldActor* FindWorldActor(const FT4ObjectID& InObjectID) const;

	const FT4GameBuiltin_GamePlayerData* GetGamePlayerData();
	const FT4GameBuiltin_GameNPCData* GetGameNPCData();
	const FT4GameBuiltin_GameWeaponData* GetGameWeaponData();
	const FT4GameBuiltin_GameCostumeData* GetGameCostumeData();

	const FT4GameBuiltin_GameSkillData* GetGameSkillData(const FT4GameBuiltin_GameDataID& InSkillDataID);
	const FT4GameBuiltin_GameWeaponData* GetGameWeaponData(const FT4GameBuiltin_GameDataID& InWeaponDataID);
	const FT4GameBuiltin_GameCostumeData* GetGameCostumeData(const FT4GameBuiltin_GameDataID& InCostumeDataID);

private:
	bool bEntered;
	FT4ActorID WorldActorID; // #114 : ActorID 기억! 현재는 ObjectID.Value 와 같다. 이후 교체가 되어야 할 수 있음
	FT4GameBuiltin_GameDataID GameDataID;
	FT4GameBuiltin_GameDataID MainWeaponDataID;

protected:
	bool bHitOverlapEventStarted;
	FT4GameBuiltin_GameSkillDataID UseSkillDataID;
	float HitOverlapEventDelayTimeSec;
	float HitOverlapEventClearTimeLeft;
	FDelegateHandle HitOverlapDelegateHandle; // #49
};
