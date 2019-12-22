// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4PacketHandler_SC.h"
#include "T4GameplayDefinitions.h"

#include "Public/Protocol/T4PacketSC_Action.h"
#include "GameDB/T4GameDB.h"

#include "T4Engine/Public/Action/T4ActionCodeMinimal.h"
#include "T4Engine/Public/Action/T4ActionParameters.h"
#include "T4Engine/Public/T4Engine.h"
#include "T4Frame/Public/T4Frame.h"

#include "T4GameplayInternal.h"

/**
  *
 */
 // #27
 // #T4_ADD_PACKET_TAG_SC

void FT4PacketHandlerSC::HandleSC_Attack(const FT4PacketAttackSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::Attack == InPacket->PacketSC);

	IT4GameObject* AttackerObject = GetGameObjectForClient(InPacket->ObjectID);
	if (nullptr == AttackerObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_Attack '%' failed. AttackerObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	FT4ActionParameters ActionParameters;

	float TargetDistance = 0.0f; // #63

	// #63 : 타겟이 있으면 먼저 사용!
	ET4TargetType TargetTypeSelected = ET4TargetType::Default;
	if (InPacket->TargetObjectID.IsValid())
	{
		IT4GameObject* TargetObject = GetGameObjectForClient(InPacket->TargetObjectID);
		if (nullptr != TargetObject)
		{
			ActionParameters.SetTargetObjectID(InPacket->TargetObjectID);
			TargetTypeSelected = ET4TargetType::TargetObject;

			FVector TargetVector = TargetObject->GetNavPoint() - AttackerObject->GetNavPoint();
			TargetDistance = TargetVector.Size(); // #63

			// #63 : 나중에는 Attacker 의 발사지점, Target 의 피격 지점으로 거리에 따른 속도를 계산해야 함.
			//       현재는 미관상, 양쪽의 Radius 를 반영해 ProjSpeed 를 계산하도록 처리
			TargetDistance -= TargetObject->GetPropertyConst().CapsuleRadius;
			TargetDistance -= AttackerObject->GetPropertyConst().CapsuleRadius;
		}
	}
	if (ET4TargetType::Default == TargetTypeSelected)
	{
		ActionParameters.SetTargetDirection(InPacket->TargetDirection);
		TargetTypeSelected = ET4TargetType::TargetDirection;
	}

	FSoftObjectPath PlayContiAssetPath;

	float OriginalProjectileSpeed = 0.0f; // #63

#if WITH_EDITOR
	if (InPacket->SkillDataID.RowName == EditorSkillDataNameID) // #60
	{
		IT4GameFrame* ClientFramework = T4FrameGet(LayerType);
		check(nullptr != ClientFramework);
		IT4EditorGameplayHandler* EditorGameplayHandler = ClientFramework->GetEditorGameplayCustomHandler();
		if (nullptr != EditorGameplayHandler)
		{
			if (EditorGameplayHandler->IsUsedGameplaySettings()) // #104
			{
				// #63 : 에디터에 공격자로 되어 있을 경우는 에디터의 Conti 를 플레이 할 수 있도록 처리한다.
				const FT4EditorSkillDataInfo& OverrideSkillDataInfo = EditorGameplayHandler->GetOverrideSkillDataInfo();
				PlayContiAssetPath = OverrideSkillDataInfo.ContiAsset.ToSoftObjectPath();
				OriginalProjectileSpeed = OverrideSkillDataInfo.ProjectileSpeed; // #63
				if (EditorGameplayHandler->IsOverrideSkillData())
				{
					PlayContiAssetPath = EditorGameplayHandler->GetOverridePlayContiPath();
				}
			}
		}
	}
#endif

	if (!PlayContiAssetPath.IsValid())
	{
		FT4GameDB& GameDB = GetGameDB();
		const FT4GameSkillData* SkillData = GameDB.GetGameData<FT4GameSkillData>(InPacket->SkillDataID);
		if (nullptr == SkillData)
		{
			UE_LOG(
				LogT4Gameplay,
				Warning,
				TEXT("[SL:%u] HandleSC_Attack '%' failed. SkillData '%s' not found."),
				uint32(LayerType),
				*(InPacket->ToString()),
				*(InPacket->SkillDataID.ToString())
			);
			return;
		}
		PlayContiAssetPath = SkillData->RawData.ContiAsset.ToSoftObjectPath();
		OriginalProjectileSpeed = SkillData->RawData.ProjectileSpeed; // #63
	}

	if (0.0f < TargetDistance && 0.0f < InPacket->ProjectileDurationSec) // #63
	{
		// #63 : C/S 간 위치 동기화 문제와 정확한 타이밍에서의 Hit 처리를 위해 ProjectileDurationSec 를
		//       ProjectileSpeed 를 계산해 처리한다.
		float ClientProjectileSpeed = TargetDistance * (1.0f / InPacket->ProjectileDurationSec);
		ActionParameters.SetProjectileSpeed(ClientProjectileSpeed);
		ActionParameters.SetProjectileDurationSec(InPacket->ProjectileDurationSec);
	}
	else
	{
		ActionParameters.SetProjectileSpeed(OriginalProjectileSpeed); // #63
	}

	FT4TurnAction NewTurnAction; // #49
	NewTurnAction.TurnType = TargetTypeSelected;
	NewTurnAction.RotationYawRate = AttackerObject->GetPropertyConst().RotationYawRate; // #46
	AttackerObject->DoExecuteAction(&NewTurnAction, &ActionParameters);

	FT4ContiAction NewAction;
	NewAction.ActionKey = InPacket->SkillDataID.ToPrimaryActionKey();
	NewAction.ActionKey.bOverrideExisting = true; // #49 : 동기화가 중요하니 무조건 플레이를 보장한다.
	NewAction.ContiAsset = PlayContiAssetPath;
	AttackerObject->DoExecuteAction(&NewAction, &ActionParameters);
}

void FT4PacketHandlerSC::HandleSC_EffectDirect(const FT4PacketEffectDirectSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::EffectDirect == InPacket->PacketSC);

	IT4GameObject* TargetObject = GetGameObjectForClient(InPacket->ObjectID);
	if (nullptr == TargetObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_EffectDirect '%' failed. TargetObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	FSoftObjectPath PlayContiAssetPath;

#if WITH_EDITOR
	// #60, #63
	IT4GameFrame* ClientFramework = T4FrameGet(LayerType);
	check(nullptr != ClientFramework);
	IT4EditorGameplayHandler* EditorGameplayHandler = ClientFramework->GetEditorGameplayCustomHandler();
	if (nullptr != EditorGameplayHandler)
	{
		if (EditorGameplayHandler->IsUsedGameplaySettings()) // #104
		{
			if (EditorGameplayHandler->IsOverrideEffectData())
			{
				// #63 : 에디터에 설정된 Effect 의 ContiAsset 을 그대로 사용한다.
				const FT4EditorEffectDataInfo& OverrideEffectDataInfo = EditorGameplayHandler->GetOverrideEffectDataInfo();
				PlayContiAssetPath = OverrideEffectDataInfo.ContiAsset.ToSoftObjectPath();
			}
		}
	}
#endif

	if (!PlayContiAssetPath.IsValid())
	{
		FT4GameDB& GameDB = GetGameDB();
		const FT4GameEffectData* EffectData = GameDB.GetGameData<FT4GameEffectData>(InPacket->EffectDataID);
		if (nullptr == EffectData)
		{
			UE_LOG(
				LogT4Gameplay,
				Warning,
				TEXT("[SL:%u] HandleSC_EffectDirect '%' failed. EffectData '%s' not found."),
				uint32(LayerType),
				*(InPacket->ToString()),
				*(InPacket->EffectDataID.ToString())
			);
			return;
		}
		PlayContiAssetPath = EffectData->RawData.ContiAsset.ToSoftObjectPath();
	}

	FT4ActionParameters ActionParameters;
	if (InPacket->AttackerObjectID.IsValid())
	{
		ActionParameters.SetTargetObjectID(InPacket->AttackerObjectID);
	}

	FT4ContiAction NewAction;
	NewAction.ActionKey = InPacket->EffectDataID.ToOverlapActionKey(); // #49 : 이팩트는 중첩과 관계 없이 플레이
	NewAction.ContiAsset = PlayContiAssetPath;
	TargetObject->DoExecuteAction(&NewAction, &ActionParameters);
}

void FT4PacketHandlerSC::HandleSC_EffectArea(const FT4PacketEffectAreaSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::EffectArea == InPacket->PacketSC);

	if (InPacket->TargetLocation.IsNearlyZero())
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_EffectArea '%' failed. TargetLocation is zero."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	FSoftObjectPath PlayContiAssetPath;

	IT4GameFrame* ClientFramework = T4FrameGet(LayerType);
	check(nullptr != ClientFramework);

#if WITH_EDITOR
	// #60, #63
	IT4EditorGameplayHandler* EditorGameplayHandler = ClientFramework->GetEditorGameplayCustomHandler();
	if (nullptr != EditorGameplayHandler)
	{
		if (EditorGameplayHandler->IsUsedGameplaySettings()) // #104
		{
			if (EditorGameplayHandler->IsOverrideEffectData())
			{
				// #63 : 에디터에 설정된 Effect 의 ContiAsset 을 그대로 사용한다.
				const FT4EditorEffectDataInfo& OverrideEffectDataInfo = EditorGameplayHandler->GetOverrideEffectDataInfo();
				PlayContiAssetPath = OverrideEffectDataInfo.ContiAsset.ToSoftObjectPath();
			}
		}
	}
#endif

	if (!PlayContiAssetPath.IsValid())
	{
		FT4GameDB& GameDB = GetGameDB();
		const FT4GameEffectData* EffectData = GameDB.GetGameData<FT4GameEffectData>(InPacket->EffectDataID);
		if (nullptr == EffectData)
		{
			UE_LOG(
				LogT4Gameplay,
				Warning,
				TEXT("[SL:%u] HandleSC_EffectArea '%' failed. EffectData '%s' not found."),
				uint32(LayerType),
				*(InPacket->ToString()),
				*(InPacket->EffectDataID.ToString())
			);
			return;
		}
		PlayContiAssetPath = EffectData->RawData.ContiAsset.ToSoftObjectPath();
	}

	IT4GameWorld* GameWorld = ClientFramework->GetGameWorld();
	check(nullptr != GameWorld);
	IT4GameObject* ClientObject = GameWorld->GetContainer()->PlayClientObject(
		ET4ObjectType::World_Default,
		InPacket->EffectDataID.RowName,
		InPacket->TargetLocation,
		FRotator::ZeroRotator,
		FVector::OneVector
	);
	if (nullptr != ClientObject)
	{
		FT4ContiAction NewAction;
		NewAction.ActionKey = InPacket->EffectDataID.ToOverlapActionKey(); // #49 : 이팩트는 중첩과 관계 없이 플레이
		NewAction.ContiAsset = PlayContiAssetPath;
		ClientObject->DoExecuteAction(&NewAction);
	}
}
