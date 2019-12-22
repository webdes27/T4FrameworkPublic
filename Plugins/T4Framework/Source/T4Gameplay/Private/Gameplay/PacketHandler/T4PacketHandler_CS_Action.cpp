// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4PacketHandler_CS.h"

#include "Classes/Controller/Player/T4GameplayPlayerController.h" // #50

#include "Public/Protocol/T4PacketCS_Action.h"
#include "Public/Protocol/T4PacketSC_Action.h"

#include "GameDB/T4GameDB.h"

#include "T4Engine/Public/T4Engine.h"

#include "T4GameplayInternal.h"

/**
  *
 */
// #27
// #T4_ADD_PACKET_TAG_CS

void FT4PacketHandlerCS::HandleCS_Attack(const FT4PacketAttackCS* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketCtoS::Attack == InPacket->PacketCS);

	IT4GameObject* AttackerObject = GetGameObjectForServer(InPacket->SenderID);
	if (nullptr == AttackerObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleCS_Attack '%' failed. AttackerObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	// WARN: CtoS 는 플레이어만 보낼 수 있다.
	AT4GameplayPlayerController* GameplayPlayerController = CastGameplayPlayerController(AttackerObject);
	check(nullptr != GameplayPlayerController);
	GameplayPlayerController->DoAttackForServer(*InPacket);
}
