// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4PacketHandler_CS.h"

#include "Public/Protocol/T4PacketCS_Move.h"
#include "Public/Protocol/T4PacketSC_Move.h"

#include "Classes/Controller/Player/T4GameplayPlayerController.h" // #50

#include "T4GameplaySettings.h" // #52

#include "T4Engine/Public/T4Engine.h"

#include "T4GameplayInternal.h"

/**
  *
 */
// #27
// #T4_ADD_PACKET_TAG_CS

void FT4PacketHandlerCS::HandleCS_Move(const FT4PacketMoveCS* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketCtoS::Move == InPacket->PacketCS);

	IT4GameObject* SenderObject = GetGameObjectForServer(InPacket->SenderID);
	if (nullptr == SenderObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleCS_Move '%' failed. SenderObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	FVector MoveVelocity = InPacket->MoveToLocation - SenderObject->GetNavPoint();
	if (MoveVelocity.IsNearlyZero())
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleCS_Move '%' failed. Valocity is zero."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	// #52 : 클라에서 MoveToLocation 을 레이턴시를 감안한 거리로 보냄으로 검증 역시 레이턴시를 감안해 처리한다.
	const float DefaultNetworkLatencySec = GetDefault<UT4GameplaySettings>()->GameplayDefaultNetworkLatencySec; // #52 : 200ms

	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);

	FVector MoveToLocation = FVector::ZeroVector;
	const FVector MoveStartLocation = SenderObject->GetNavPoint();
	FVector NewTargetLocation = MoveStartLocation;

	const float CheckMoveSpeed = MoveVelocity.Size();
	FT4ServerGameObjectAttribute& ObjectAttribute = SenderObject->GetServerAttribute(); // #46
	const float MaxMoveSpeed = (ObjectAttribute.MaxRunSpeed * DefaultNetworkLatencySec);
	if (MaxMoveSpeed < CheckMoveSpeed)
	{
		// #52 : 만약 최대 스피드보다 먼 거리 이동 요청이라면, 어뷰징을 고려해 최대 거리를 조정해 이동시킨다.
		FVector MoveDirection = MoveVelocity / CheckMoveSpeed;
		NewTargetLocation += (MoveDirection * MaxMoveSpeed);
	}
	else
	{
		NewTargetLocation = InPacket->MoveToLocation;
	}

	IT4WorldNavigationSystem* WorldNavigationSystem = GameWorld->GetNavigationSystem();
	check(nullptr != WorldNavigationSystem);

	if (!WorldNavigationSystem->ProjectPoint(NewTargetLocation, T4_INVALID_NAVEXTENT, MoveToLocation))
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleCS_Move '%' failed. ProjectPoint to Navigation."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}
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

	IT4PacketHandlerSC* PacketHandlerSC = GetPacketHandlerSC();
	check(nullptr != PacketHandlerSC);

	FT4PacketMoveToSC NewPacketSC;
	NewPacketSC.ObjectID = InPacket->SenderID;
	NewPacketSC.MoveToLocation = MoveToLocation;
	NewPacketSC.HeadYawAngle = InPacket->HeadYawAngle; // #40
	NewPacketSC.bForceMaxSpeed = false; // #52 : MovementComponet::MaxSpeed 를 사용할지에 대한 Flag, 기본값이 false 로 Velocity 에서 Speed 를 얻는다. 동기화 이슈!!
#if WITH_EDITOR
	NewPacketSC.ServerNavPoint = SenderObject->GetNavPoint(); // #52
	NewPacketSC.ServerDirection = SenderObject->GetFrontVector(); // #52
#endif
	GetPacketHandlerSC()->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
}

void FT4PacketHandlerCS::HandleCS_Jump(const FT4PacketJumpCS* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketCtoS::Jump == InPacket->PacketCS);

	IT4GameObject* SenderObject = GetGameObjectForServer(InPacket->SenderID);
	if (nullptr == SenderObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleCS_Jump '%' failed. SenderObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	FT4ServerGameObjectAttribute& ObjectAttribute = SenderObject->GetServerAttribute(); // #46

	FT4PacketJumpToSC NewPacketSC;
	NewPacketSC.ObjectID = InPacket->SenderID;
	NewPacketSC.JumpVelocity = InPacket->JumpDirection;
	NewPacketSC.JumpVelocity.Z = ObjectAttribute.MaxJumpZVelocity;
	GetPacketHandlerSC()->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
}

// #46
void FT4PacketHandlerCS::HandleCS_Roll(const FT4PacketRollCS* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketCtoS::Roll == InPacket->PacketCS);

	IT4GameObject* SenderObject = GetGameObjectForServer(InPacket->SenderID);
	if (nullptr == SenderObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleCS_Roll '%' failed. SenderObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	FT4ServerGameObjectAttribute& ObjectAttribute = SenderObject->GetServerAttribute(); // #46

	FT4PacketRollToSC NewPacketSC;
	NewPacketSC.ObjectID = InPacket->SenderID;
	NewPacketSC.RollVelocity = InPacket->RollDirection;
	NewPacketSC.RollVelocity.Z = ObjectAttribute.MaxRollZVelocity;
	GetPacketHandlerSC()->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
}

void FT4PacketHandlerCS::HandleCS_Turn(const FT4PacketTurnCS* InPacket)
{
	// #40
	check(nullptr != InPacket);
	check(ET4PacketCtoS::Turn == InPacket->PacketCS);

	IT4GameObject* SenderObject = GetGameObjectForServer(InPacket->SenderID);
	if (nullptr == SenderObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleCS_Turn '%' failed. SenderObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	FT4PacketTurnToSC NewPacketSC;
	NewPacketSC.ObjectID = InPacket->SenderID;
	NewPacketSC.TargetYawAngle = InPacket->TargetYawAngle;
	GetPacketHandlerSC()->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
}

void FT4PacketHandlerCS::HandleCS_LockOn(const FT4PacketLockOnCS* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketCtoS::LockOn == InPacket->PacketCS);

	IT4GameObject* SenderObject = GetGameObjectForServer(InPacket->SenderID);
	if (nullptr == SenderObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleCS_LockOn '%' failed. SenderObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	FT4PacketLockOnSC NewPacketSC;
	NewPacketSC.ObjectID = InPacket->SenderID;
	NewPacketSC.HeadYawAngle = InPacket->HeadYawAngle; // #40
	GetPacketHandlerSC()->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
}

void FT4PacketHandlerCS::HandleCS_LockOff(const FT4PacketLockOffCS* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketCtoS::LockOff == InPacket->PacketCS);

	IT4GameObject* SenderObject = GetGameObjectForServer(InPacket->SenderID);
	if (nullptr == SenderObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleCS_LockOff '%' failed. SenderObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	FT4PacketLockOffSC NewPacketSC;
	NewPacketSC.ObjectID = InPacket->SenderID;
	NewPacketSC.HeadYawAngle = InPacket->HeadYawAngle; // #38, #40
	GetPacketHandlerSC()->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
}

void FT4PacketHandlerCS::HandleCS_CmdTeleport(const FT4PacketCmdTeleportCS* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketCtoS::CmdTeleport == InPacket->PacketCS);

	IT4GameObject* SenderObject = GetGameObjectForServer(InPacket->SenderID);
	if (nullptr == SenderObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleCS_CmdTeleport '%' failed. SenderObject not found."),
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
			TEXT("[SL:%u] HandleCS_CmdTeleport '%' failed. ProjectPoint to Navigation."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	FT4PacketTeleportToSC NewPacketSC;
	NewPacketSC.ObjectID = InPacket->SenderID;
	NewPacketSC.TargetLocation = TeleportLocation;
	GetPacketHandlerSC()->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
}
