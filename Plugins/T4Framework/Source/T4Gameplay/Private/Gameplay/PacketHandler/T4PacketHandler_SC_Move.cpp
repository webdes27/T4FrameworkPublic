// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4PacketHandler_SC.h"
#include "T4GameplayDefinitions.h"

#include "Public/Protocol/T4PacketSC_Move.h"
#include "GameDB/T4GameDB.h"

#include "T4GameplaySettings.h" // #52

#include "T4Engine/Public/Action/T4ActionCodeMinimal.h"
#include "T4Engine/Public/T4Engine.h"

#include "T4GameplayInternal.h"

/**
  *
 */
 // #27
 // #T4_ADD_PACKET_TAG_SC

void FT4PacketHandlerSC::HandleSC_MoveTo(const FT4PacketMoveToSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::MoveTo == InPacket->PacketSC);
	IT4GameObject* TargetObject = GetGameObjectForClient(InPacket->ObjectID);
	if (nullptr == TargetObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_MoveTo '%' failed. TargetObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);

	IT4WorldNavigationSystem* WorldNavigationSystem = GameWorld->GetNavigationSystem();
	check(nullptr != WorldNavigationSystem);

	FVector MoveToLocation = FVector::ZeroVector;
	if (!WorldNavigationSystem->ProjectPoint(InPacket->MoveToLocation, T4_INVALID_NAVEXTENT, MoveToLocation))
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleSC_MoveTo '%' failed. ProjectPoint to Navigation."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}
	const FVector MoveStartLocation = TargetObject->GetNavPoint();
	if (!WorldNavigationSystem->HasReached(MoveStartLocation, MoveToLocation))
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleCS_Move '%s' failed. has not reached. (%s => %s)"),
			uint32(LayerType),
			*(InPacket->ToString()),
			*(MoveStartLocation.ToString()),
			*(MoveToLocation.ToString())
		);
		return;
	}

	FVector MoveVelocity = MoveToLocation - TargetObject->GetNavPoint();
	if (MoveVelocity.IsNearlyZero())
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleSC_MoveTo '%' failed. Valocity is zero."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	// #52 : InPacket->MoveToLocation 은 DefaultNetworkLatencySec 를 감안한 거리로 서버에서 보내기 때문에
	//       클라이언트에서 필요한 Velocity (per sec) 에 맞게 복원 해서 넘겨준다.
	const float DefaultNetworkLatencySec = GetDefault<UT4GameplaySettings>()->GameplayDefaultNetworkLatencySec; // #52 : 200ms
	MoveVelocity *= (1.0f / DefaultNetworkLatencySec);

	FT4MoveSyncAction NewAction;
	// #52 : C/S 레이턴시로 인해 오차가 발생할 수 밖에 없음으로 StoC 에서는 TargetLocation 을 받고,
	//       Velocity 로 변환해 처리하도록 수정
	NewAction.MoveVelocity = MoveVelocity;
	NewAction.HeadYawAngle = InPacket->HeadYawAngle; // #40
	NewAction.bForceMaxSpeed = InPacket->bForceMaxSpeed; // #52 : MovementComponet::MaxSpeed 를 사용할지에 대한 Flag, 기본값이 false 로 Velocity 에서 Speed 를 얻는다. 동기화 이슈!!
#if WITH_EDITOR
	NewAction.ServerNavPoint = InPacket->ServerNavPoint; // #52
	NewAction.ServerDirection = InPacket->ServerDirection; // #52
#endif
	TargetObject->DoExecuteAction(&NewAction);
}

void FT4PacketHandlerSC::HandleSC_JumpTo(const FT4PacketJumpToSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::JumpTo == InPacket->PacketSC);
	IT4GameObject* TargetObject = GetGameObjectForClient(InPacket->ObjectID);
	if (nullptr == TargetObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_JumpTo '%' failed. TargetObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}
	FT4JumpAction NewAction;
	NewAction.ActionKey = T4ActionJumpPKey;
	NewAction.JumpVelocity = InPacket->JumpVelocity;
	TargetObject->DoExecuteAction(&NewAction);
}

void FT4PacketHandlerSC::HandleSC_RollTo(const FT4PacketRollToSC* InPacket) // #46
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::RollTo == InPacket->PacketSC);
	IT4GameObject* TargetObject = GetGameObjectForClient(InPacket->ObjectID);
	if (nullptr == TargetObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_RollTo '%' failed. TargetObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}
	const bool bIsLockOn = TargetObject->HasPublicAction(T4ActionLockOnPKey);
	FT4RollAction NewAction;
	NewAction.ActionKey = T4ActionRollPKey;
	NewAction.RollVelocity = InPacket->RollVelocity;
	TargetObject->DoExecuteAction(&NewAction);
}

void FT4PacketHandlerSC::HandleSC_TurnTo(const FT4PacketTurnToSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::TurnTo == InPacket->PacketSC);
	IT4GameObject* TargetObject = GetGameObjectForClient(InPacket->ObjectID);
	if (nullptr == TargetObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_TurnTo '%' failed. TargetObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}
	if (TargetObject->IsPlayer())
	{
		return; // #40 : MyPC 는 Turn 처리는 패스한다.
	}
	FT4TurnAction NewAction;
	NewAction.ActionKey = T4ActionTurnPKey;
	NewAction.TurnType = ET4TargetType::TargetCustom;
	NewAction.TargetYawAngle = InPacket->TargetYawAngle; // #40
	TargetObject->DoExecuteAction(&NewAction);
}

void FT4PacketHandlerSC::HandleSC_TeleportTo(const FT4PacketTeleportToSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::TeleportTo == InPacket->PacketSC);
	IT4GameObject* TargetObject = GetGameObjectForClient(InPacket->ObjectID);
	if (nullptr == TargetObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_TeleportTo '%' failed. TargetObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);

	IT4WorldNavigationSystem* NavigationSystem = GameWorld->GetNavigationSystem();
	if (nullptr == NavigationSystem)
	{
		return;
	}

	FVector TeleportLocation = FVector::ZeroVector;
	if (!NavigationSystem->ProjectPoint(InPacket->TargetLocation, T4_INVALID_NAVEXTENT, TeleportLocation))
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleSC_TeleportTo '%' failed. ProjectPoint to Navigation."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}
	
	FT4TeleportAction NewAction;
	NewAction.TargetLocation = TeleportLocation;
	TargetObject->DoExecuteAction(&NewAction);
}

void FT4PacketHandlerSC::HandleSC_MoveStop(const FT4PacketMoveStopSC* InPacket) // #52
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::MoveStop == InPacket->PacketSC);
	IT4GameObject* TargetObject = GetGameObjectForClient(InPacket->ObjectID);
	if (nullptr == TargetObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_MoveStop '%' failed. TargetObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}
	FT4MoveStopAction NewAction;
	NewAction.StopLocation = InPacket->StopLocation;
	NewAction.HeadYawAngle = InPacket->HeadYawAngle;
	NewAction.bSyncLocation = InPacket->bSyncLocation;
	TargetObject->DoExecuteAction(&NewAction);
}

void FT4PacketHandlerSC::HandleSC_MoveSpeedSync(const FT4PacketMoveSpeedSyncSC* InPacket) // #52
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::MoveSpeedSync == InPacket->PacketSC);
	IT4GameObject* TargetObject = GetGameObjectForClient(InPacket->ObjectID);
	if (nullptr == TargetObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_MoveSpeedSync '%' failed. TargetObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}
	FT4MoveSpeedSyncAction NewAction;
	NewAction.MoveSpeed = InPacket->MoveSpeed;
	TargetObject->DoExecuteAction(&NewAction);
}

void FT4PacketHandlerSC::HandleSC_LockOn(const FT4PacketLockOnSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::LockOn == InPacket->PacketSC);
	IT4GameObject* TargetObject = GetGameObjectForClient(InPacket->ObjectID);
	if (nullptr == TargetObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_LockOn '%' failed. TargetObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}
	FT4LockOnAction NewAction;
	NewAction.ActionKey = T4ActionLockOnPKey;
	NewAction.bSetLocked = true;
	NewAction.HeadYawAngle = InPacket->HeadYawAngle;
	NewAction.LifecycleType = ET4LifecycleType::Looping;
	TargetObject->DoExecuteAction(&NewAction);
}

void FT4PacketHandlerSC::HandleSC_LockOff(const FT4PacketLockOffSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::LockOff == InPacket->PacketSC);
	IT4GameObject* TargetObject = GetGameObjectForClient(InPacket->ObjectID);
	if (nullptr == TargetObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_LockOff '%' failed. TargetObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}
	FT4LockOnAction NewAction;
	NewAction.ActionKey = T4ActionLockOnPKey;
	NewAction.bSetLocked = false;
	NewAction.HeadYawAngle = InPacket->HeadYawAngle; // #38
	NewAction.LifecycleType = ET4LifecycleType::Default;
	TargetObject->DoExecuteAction(&NewAction);
}
