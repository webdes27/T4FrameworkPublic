// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Controller/Player/T4GameplayPlayerController.h"

#include "T4GameplayUtils.h" // #63
#include "T4GameplayDefinitions.h" // #63

#include "GameDB/T4GameDB.h"

#include "Gameplay/Server/T4ServerEventManager.h" // #49
#include "Gameplay/T4GameplayInstance.h"

#include "T4Engine/Public/T4Engine.h"
#include "T4Engine/Public/Action/T4ActionCodeMove.h"
#include "T4Frame/Public/T4Frame.h"

#include "T4GameplayInternal.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Framework/Controller/PlayerController/index.html
 */

FORCEINLINE IT4PacketHandlerCS* GetPacketHandlerCS(const ET4LayerType InLayerType)
{
	IT4GameFrame* GameFrame = T4FrameGet(InLayerType);
	if (nullptr == GameFrame)
	{
		return nullptr;
	}
	FT4GameplayInstance* GameplayInstance = FT4GameplayInstance::CastFrom(
		GameFrame->GetGameplayInstance()
	);
	if (nullptr == GameplayInstance)
	{
		return nullptr;
	}
	return GameplayInstance->GetPacketHandlerCS();
}

AT4GameplayPlayerController::AT4GameplayPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
#if (WITH_EDITOR || WITH_SERVER_CODE)
	, bHitOverlapEventStarted(false)
	, HitOverlapEventDelayTimeSec(0.0f)
	, HitOverlapEventClearTimeLeft(0.0f)
#endif
{
	bReplicates = true;
}

void AT4GameplayPlayerController::NotifyAdvance(float InDeltaTime) // #49
{
#if (WITH_EDITOR || WITH_SERVER_CODE)
	if (bHitOverlapEventStarted)
	{
		IT4GameObject* OwnerGameObject = GetGameObject();
		if (nullptr != OwnerGameObject)
		{
			float ScaledDeltaTime = InDeltaTime;
#if WITH_EDITOR
			// #102 : 서버 로직이라면 Player 가 없음으로 TimeScale 을 고려할 필요가 없임.
			//        에디터에서의 테스트를 위해 TimeScale 을 감안해줌!
			ScaledDeltaTime = ScaledDeltaTime * OwnerGameObject->GetTimeScale();
#endif
			HitOverlapEventDelayTimeSec -= ScaledDeltaTime;
			if (!OwnerGameObject->IsWeaponHitOverlapEventEnabled() && HitOverlapEventDelayTimeSec <= 0.0f)
			{
				OwnerGameObject->BeginWeaponHitOverlapEvent(UseSkillDataID.RowName);
			}
			HitOverlapEventClearTimeLeft -= ScaledDeltaTime;
			if (HitOverlapEventClearTimeLeft <= 0.0f)
			{
				ClearHitOverlapEventForServer();
			}
		}
		else
		{
			bHitOverlapEventStarted = false;
		}
	}
#endif
}

bool AT4GameplayPlayerController::DoAttackForServer(const FT4PacketAttackCS& InPacket) // #49
{
#if (WITH_EDITOR || WITH_SERVER_CODE)

	// #60
	FT4GameSkillDataID SelectSkillDataID(InPacket.SkillDataID.RowName);

	bool bUpdateSkillData = true;

	bool bMoveable = false;
	float SkillDataHitDelayTimeSec = 0.0f;
	float SkillDataDurationSec = 0.0f;

	ET4GameAttackType AttackType = ET4GameAttackType::Melee; // #63
	float ProjectileSpeed = 0.0f; // #63
	FT4GameEffectDataID ResultEffectDataID; // #63

#if WITH_EDITOR
	IT4EditorGameplayHandler* EditorGameplayHandler = GetEditorGameplayCustomHandler();
	if (nullptr != EditorGameplayHandler)
	{
		if (EditorGameplayHandler->IsUsedGameplaySettings()) // #104
		{
			// #60 : Editor AI Handler 가 있다면 AI를 별도로 제어함
			SelectSkillDataID.RowName = EditorGameplayHandler->GetOverrideSkillDataNameID();
			if (SelectSkillDataID.RowName == NAME_None)
			{
				return false;
			}
			if (EditorGameplayHandler->IsOverrideSkillData())
			{
				const FT4EditorSkillDataInfo& EditorSkillData = EditorGameplayHandler->GetOverrideSkillDataInfo();
				AttackType = EditorSkillData.AttackType; // #63
				SkillDataHitDelayTimeSec = EditorSkillData.HitDelayTimeSec;
				SkillDataDurationSec = EditorSkillData.DurationSec;
				ProjectileSpeed = EditorSkillData.ProjectileSpeed; // #63
				bMoveable = EditorSkillData.bMoveable;
				SelectSkillDataID.RowName = EditorSkillDataNameID; // #63 ; 에디터용 스킬이 성절되도록 처리
				ResultEffectDataID.RowName = EditorSkillData.ResultEffectDataID; // #63 : Effect 설정
				if (SelectSkillDataID.RowName == EditorSkillDataNameID && 0.0f >= SkillDataDurationSec)
				{
					return false;
				}
				bUpdateSkillData = false;
			}
		}
	}
#endif

	if (bUpdateSkillData)
	{
		FT4GameDB& GameDB = GetGameDB();
		const FT4GameSkillData* SkillData = GameDB.GetGameData<FT4GameSkillData>(SelectSkillDataID);
		if (nullptr == SkillData)
		{
			return false;
		}
		AttackType = SkillData->RawData.AttackType; // #63
		SkillDataHitDelayTimeSec = SkillData->RawData.HitDelayTimeSec;
		SkillDataDurationSec = SkillData->RawData.DurationSec;
		ProjectileSpeed = SkillData->RawData.ProjectileSpeed; // #63
		bMoveable = SkillData->RawData.bMoveable;
		ResultEffectDataID = SkillData->RawData.ResultEffectDataID; // #63
	}

	if (!SelectSkillDataID.IsValid())
	{
		return false;
	}

	// #63
	float TargetDistance = 0.0f;
	FVector TargetDirection = InPacket.TargetDirection;
	if (ET4GameAttackType::Ranged == AttackType) // #63
	{
		if (InPacket.TargetObjectID.IsValid())
		{
			IT4GameObject* TargetObject = FindGameObject(InPacket.TargetObjectID);
			if (nullptr != TargetObject)
			{
				FVector TargetVector = TargetObject->GetNavPoint() - GetGameObject()->GetNavPoint();
				TargetDistance = TargetVector.Size();
				TargetDirection = (FMath::Abs(TargetDistance) > KINDA_SMALL_NUMBER)
					? TargetVector / TargetDistance : GetGameObject()->GetFrontVector();
			}
		}
	}

	IT4PacketHandlerSC* PacketHandlerSC = GetPacketHandlerSC(LayerType);
	check(nullptr != PacketHandlerSC);

	FT4PacketAttackSC NewPacketSC;
	NewPacketSC.ObjectID = GetGameObjectID();
	NewPacketSC.SkillDataID = SelectSkillDataID;
	NewPacketSC.TargetObjectID = InPacket.TargetObjectID;
	NewPacketSC.TargetDirection = TargetDirection; // #49

	if (ET4GameAttackType::Ranged == AttackType) // #63
	{
		if (0.0f < ProjectileSpeed && 0.1f < TargetDistance)
		{
			NewPacketSC.ProjectileDurationSec = TargetDistance / ProjectileSpeed;

			FT4ServerEventManager* ServerEventManager = T4GameplayUtil::GetServerEventManager(GetLayerType()); // #63
			if (nullptr != ServerEventManager)
			{
				FEffectDamageData NewEffectDamageData;
				NewEffectDamageData.EventName = T4GameplayAIEventEffectDamage; // #63
				NewEffectDamageData.TargetObjectID = InPacket.TargetObjectID;
				NewEffectDamageData.EffectDataID = ResultEffectDataID;
				NewEffectDamageData.AttackerObjectID = GetGameObjectID();
				ServerEventManager->AddEffectDamage(
					NewPacketSC.ProjectileDurationSec + SkillDataHitDelayTimeSec, // HitDelay 를 더해줘야 한다! 
					NewEffectDamageData
				);
			}
		}
	}
	else if (ET4GameAttackType::Area == AttackType) // #68
	{
		FT4ServerEventManager* ServerEventManager = T4GameplayUtil::GetServerEventManager(GetLayerType()); // #68
		if (nullptr != ServerEventManager)
		{
			FEffectDamageData NewDamageData;
			NewDamageData.EventName = T4GameplayAIEventEffectDamage; // #68
			NewDamageData.EffectDataID = ResultEffectDataID;
			NewDamageData.TargetLocation = InPacket.TargetLocation;
			NewDamageData.AttackerObjectID = GetGameObjectID();
			ServerEventManager->AddEffectDamage(
				SkillDataHitDelayTimeSec, // HitDelay 를 더해줘야 한다!
				NewDamageData
			);
		}
	}

	check(NewPacketSC.ObjectID.IsValid());
	PacketHandlerSC->DoBroadcastPacketForServer(&NewPacketSC, true);

	ClearHitOverlapEventForServer(); // #49

	if (ET4GameAttackType::Melee == AttackType) // #63
	{
		bHitOverlapEventStarted = true;
		UseSkillDataID = SelectSkillDataID;
		HitOverlapEventDelayTimeSec = SkillDataHitDelayTimeSec;
		HitOverlapEventClearTimeLeft = SkillDataDurationSec;
		if (0.0f >= HitOverlapEventDelayTimeSec)
		{
			GetGameObject()->BeginWeaponHitOverlapEvent(UseSkillDataID.RowName);
		}
	}
#endif
	return true;
}

void AT4GameplayPlayerController::NotifyPossess(
	IT4GameObject* InNewGameObject
) // #49
{
	if (nullptr == InNewGameObject)
	{
		return;
	}
#if (WITH_EDITOR || WITH_SERVER_CODE)
	FT4ServerGameObjectDelegates& GameObjectDelegates = InNewGameObject->GetServerDelegates();
	HitOverlapDelegateHandle = GameObjectDelegates.OnHitOverlap.AddUObject(
		this, 
		&AT4GameplayPlayerController::HandleOnHitOverlapForServer
	);
#endif
}

void AT4GameplayPlayerController::NotifyUnPossess(
	IT4GameObject* InOldGameObject
) // #49
{
	if (nullptr == InOldGameObject)
	{
		return;
	}
#if (WITH_EDITOR || WITH_SERVER_CODE)
	if (HitOverlapDelegateHandle.IsValid())
	{
		HitOverlapDelegateHandle.Reset();
	}
#endif
}

#if (WITH_EDITOR || WITH_SERVER_CODE)
void AT4GameplayPlayerController::HandleOnHitOverlapForServer(
	const FName& InEventName,
	IT4GameObject* InHitGameObject,
	const FHitResult& InSweepResult
) // #49
{
	if (nullptr == InHitGameObject)
	{
		return;
	}
	if (!bHitOverlapEventStarted)
	{
		//ensure(false); // ?
		GetGameObject()->EndWeaponHitOverlapEvent();
		return;
	}

	FT4GameEffectDataID ResultEffectDataID;

#if WITH_EDITOR
	if (InEventName == EditorSkillDataNameID)
	{
		IT4EditorGameplayHandler* EditorGameplayHandler = GetEditorGameplayCustomHandler();
		if (nullptr != EditorGameplayHandler) // #60
		{
			if (EditorGameplayHandler->IsUsedGameplaySettings()) // #104
			{
				const FT4EditorSkillDataInfo& EditorSkillData = EditorGameplayHandler->GetOverrideSkillDataInfo();
				ResultEffectDataID.RowName = EditorSkillData.ResultEffectDataID; // #63 : Effect 설정
			}
		}
	}
#endif

	if (!ResultEffectDataID.IsValid())
	{
		FT4GameSkillDataID SkillDataID(InEventName);
		FT4GameDB& GameDB = GetGameDB();
		const FT4GameSkillData* SkillData = GameDB.GetGameData<FT4GameSkillData>(SkillDataID);
		if (nullptr == SkillData)
		{
			return;
		}
		ResultEffectDataID = SkillData->RawData.ResultEffectDataID;
	}

	if (!ResultEffectDataID.IsValid())
	{
		return;
	}

	FT4ServerEventManager* ServerEventManager = T4GameplayUtil::GetServerEventManager(GetLayerType()); // #63
	if (nullptr != ServerEventManager)
	{
		FEffectDamageData NewEffectDamageData;
		NewEffectDamageData.EventName = T4GameplayAIEventEffectDamage; // #63
		NewEffectDamageData.TargetObjectID = InHitGameObject->GetObjectID();
		NewEffectDamageData.EffectDataID = ResultEffectDataID;
		NewEffectDamageData.AttackerObjectID = GetGameObjectID();
		ServerEventManager->AddEffectDamage(0.0f, NewEffectDamageData);
	}
}

void AT4GameplayPlayerController::ClearHitOverlapEventForServer() // #49
{
	IT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	if (OwnerGameObject->IsWeaponHitOverlapEventEnabled())
	{
		OwnerGameObject->EndWeaponHitOverlapEvent();
		bHitOverlapEventStarted = false;
	}
}
#endif

bool AT4GameplayPlayerController::CS_RecvPacket_Validate(const FT4PacketCtoS* InPacket)
{
	IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandlerCS(LayerType);
	if (nullptr == PacketHandlerCS)
	{
		return false;
	}
	return PacketHandlerCS->OnRecvPacket_Validation(InPacket);
}

void AT4GameplayPlayerController::CS_RecvPacket_Implementation(const FT4PacketCtoS* InPacket)
{
	IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandlerCS(LayerType);
	if (nullptr == PacketHandlerCS)
	{
		return;
	}
	PacketHandlerCS->OnRecvPacket(InPacket, static_cast<IT4PlayerController*>(this));
}

#define DEFINE_CS_VALIDATION(x)																		\
bool AT4GameplayPlayerController::CS_RecvPacket_##x##_Validate(const FT4Packet##x##CS& InPacket)	\
{																									\
	return CS_RecvPacket_Validate(&InPacket);														\
}

#define DEFINE_CS_IMPLEMENTATION(x)																		\
void AT4GameplayPlayerController::CS_RecvPacket_##x##_Implementation(const FT4Packet##x##CS& InPacket)	\
{																										\
	CS_RecvPacket_Implementation(&InPacket);															\
}

#define DEFINE_CS_PACKET_MACRO(x)	\
DEFINE_CS_VALIDATION(x)				\
DEFINE_CS_IMPLEMENTATION(x)

// #T4_ADD_PACKET_TAG_CS
DEFINE_CS_PACKET_MACRO(Move)
DEFINE_CS_PACKET_MACRO(Jump)
DEFINE_CS_PACKET_MACRO(Roll) // #46
DEFINE_CS_PACKET_MACRO(Turn)
DEFINE_CS_PACKET_MACRO(LockOn)
DEFINE_CS_PACKET_MACRO(LockOff)
DEFINE_CS_PACKET_MACRO(Stance) // #73
DEFINE_CS_PACKET_MACRO(Equip)
DEFINE_CS_PACKET_MACRO(UnEquip)
DEFINE_CS_PACKET_MACRO(Exchange) // #37
DEFINE_CS_PACKET_MACRO(Attack)
DEFINE_CS_PACKET_MACRO(CmdWorldTravel)
DEFINE_CS_PACKET_MACRO(CmdChangePlayer) // #11, #52
DEFINE_CS_PACKET_MACRO(CmdPCEnter)
DEFINE_CS_PACKET_MACRO(CmdNPCEnter)
DEFINE_CS_PACKET_MACRO(CmdFOEnter)
DEFINE_CS_PACKET_MACRO(CmdItemEnter) // #41
DEFINE_CS_PACKET_MACRO(CmdLeave) // #68
DEFINE_CS_PACKET_MACRO(CmdTeleport)

void AT4GameplayPlayerController::SC_RecvPacket_Implementation(const FT4PacketStoC* InPacket)
{
	IT4PacketHandlerSC* PacketHandlerSC = GetPacketHandlerSC(LayerType);
	if (nullptr == PacketHandlerSC)
	{
		return;
	}
	PacketHandlerSC->OnRecvPacket(InPacket);
}

// #25, #29, #17
#define DEFINE_SC_PACKET_MACRO(x)																		\
void AT4GameplayPlayerController::SC_RecvPacket_##x##_Implementation(const FT4Packet##x##SC& InPacket)	\
{																										\
	SC_RecvPacket_Implementation(&InPacket);															\
}

// #T4_ADD_PACKET_TAG_SC
DEFINE_SC_PACKET_MACRO(WorldTravel)
DEFINE_SC_PACKET_MACRO(MyPCEnter)
DEFINE_SC_PACKET_MACRO(MyPCChange) // #11, #52
DEFINE_SC_PACKET_MACRO(PCEnter)
DEFINE_SC_PACKET_MACRO(PCLeave)
DEFINE_SC_PACKET_MACRO(NPCEnter)
DEFINE_SC_PACKET_MACRO(NPCLeave)
DEFINE_SC_PACKET_MACRO(FOEnter)
DEFINE_SC_PACKET_MACRO(FOLeave)
DEFINE_SC_PACKET_MACRO(ItemEnter) // #41
DEFINE_SC_PACKET_MACRO(ItemLeave) // #41
DEFINE_SC_PACKET_MACRO(MoveTo)
DEFINE_SC_PACKET_MACRO(JumpTo)
DEFINE_SC_PACKET_MACRO(RollTo) // #46
DEFINE_SC_PACKET_MACRO(TurnTo)
DEFINE_SC_PACKET_MACRO(TeleportTo)
DEFINE_SC_PACKET_MACRO(MoveStop) // #52
DEFINE_SC_PACKET_MACRO(MoveSpeedSync) // #52
DEFINE_SC_PACKET_MACRO(LockOn)
DEFINE_SC_PACKET_MACRO(LockOff)
DEFINE_SC_PACKET_MACRO(Stance) // #73
DEFINE_SC_PACKET_MACRO(Equip)
DEFINE_SC_PACKET_MACRO(UnEquip)
DEFINE_SC_PACKET_MACRO(Exchange) // #37
DEFINE_SC_PACKET_MACRO(Attack)
DEFINE_SC_PACKET_MACRO(EffectDirect)
DEFINE_SC_PACKET_MACRO(EffectArea) // #68
DEFINE_SC_PACKET_MACRO(Die) // #76
DEFINE_SC_PACKET_MACRO(Resurrect) // #76
