// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4PacketHandler_SC.h"

#include "Public/Protocol/T4PacketSCMinimal.h"
#include "Classes/Controller/Player/T4GameplayPlayerController.h" // #42

#include "T4Engine/Public/T4Engine.h"
#include "T4Frame/Public/T4Frame.h"

#include "Classes/Engine/World.h"

#include "T4GameplayInternal.h"

/**
  *
 */
FT4PacketHandlerSC::FT4PacketHandlerSC(ET4LayerType InLayerType)
	: LayerType(InLayerType)
{
	check(ET4LayerType::Max != LayerType);
}

FT4PacketHandlerSC::~FT4PacketHandlerSC()
{
}

void FT4PacketHandlerSC::Reset()
{
}

UWorld* FT4PacketHandlerSC::GetWorld() const
{
	check(ET4LayerType::Max > LayerType);
	IT4GameWorld* GameWorld = T4EngineWorldGet(LayerType);
	check(nullptr != GameWorld);
	return GameWorld->GetWorld();
}

IT4GameWorld* FT4PacketHandlerSC::GetGameWorld() const // #52
{
	check(ET4LayerType::Max > LayerType);
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	return GameFrame->GetGameWorld();
}

IT4PlayerController* FT4PacketHandlerSC::GetPlayerController() const
{
	check(ET4LayerType::Max > LayerType);
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	return GameFrame->GetPlayerController();
}

IT4GameObject* FT4PacketHandlerSC::GetGameObjectForClient(const FT4ObjectID& InObjectID) const
{
	check(ET4LayerType::Max > LayerType);
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	check(nullptr != GameWorld);
	return GameWorld->GetContainer()->FindGameObject(InObjectID);
}

bool FT4PacketHandlerSC::SendPacketInternal(
	FT4PacketStoC* InPacket,
	AT4GameplayPlayerController* InRecvPC
)
{
	check(nullptr != InRecvPC);

	bool bResult = true;
	const ET4PacketStoC PacketSC = InPacket->PacketSC;
	switch (PacketSC)
	{
		// #27
		// #T4_ADD_PACKET_TAG_SC
		case ET4PacketStoC::WorldTravel:
			InRecvPC->SC_RecvPacket_WorldTravel(*(static_cast<const FT4PacketWorldTravelSC*>(InPacket)));
			break;

		case ET4PacketStoC::MyPCEnter:
			InRecvPC->SC_RecvPacket_MyPCEnter(*(static_cast<const FT4PacketMyPCEnterSC*>(InPacket)));
			break;

		case ET4PacketStoC::MyPCChange: // #11, #52
			InRecvPC->SC_RecvPacket_MyPCChange(*(static_cast<const FT4PacketMyPCChangeSC*>(InPacket)));
			break;

		case ET4PacketStoC::PCEnter:
			InRecvPC->SC_RecvPacket_PCEnter(*(static_cast<const FT4PacketPCEnterSC*>(InPacket)));
			break;

		case ET4PacketStoC::PCLeave:
			InRecvPC->SC_RecvPacket_PCLeave(*(static_cast<const FT4PacketPCLeaveSC*>(InPacket)));
			break;

		case ET4PacketStoC::NPCEnter:
			InRecvPC->SC_RecvPacket_NPCEnter(*(static_cast<const FT4PacketNPCEnterSC*>(InPacket)));
			break;

		case ET4PacketStoC::NPCLeave:
			InRecvPC->SC_RecvPacket_NPCLeave(*(static_cast<const FT4PacketNPCLeaveSC*>(InPacket)));
			break;

		case ET4PacketStoC::FOEnter:
			InRecvPC->SC_RecvPacket_FOEnter(*(static_cast<const FT4PacketFOEnterSC*>(InPacket)));
			break;

		case ET4PacketStoC::FOLeave:
			InRecvPC->SC_RecvPacket_FOLeave(*(static_cast<const FT4PacketFOLeaveSC*>(InPacket)));
			break;

		case ET4PacketStoC::ItemEnter: // #41
			InRecvPC->SC_RecvPacket_ItemEnter(*(static_cast<const FT4PacketItemEnterSC*>(InPacket)));
			break;

		case ET4PacketStoC::ItemLeave: // #41
			InRecvPC->SC_RecvPacket_ItemLeave(*(static_cast<const FT4PacketItemLeaveSC*>(InPacket)));
			break;

		case ET4PacketStoC::MoveTo:
			InRecvPC->SC_RecvPacket_MoveTo(*(static_cast<const FT4PacketMoveToSC*>(InPacket)));
			break;

		case ET4PacketStoC::JumpTo:
			InRecvPC->SC_RecvPacket_JumpTo(*(static_cast<const FT4PacketJumpToSC*>(InPacket)));
			break;

		case ET4PacketStoC::RollTo: // #46
			InRecvPC->SC_RecvPacket_RollTo(*(static_cast<const FT4PacketRollToSC*>(InPacket)));
			break;

		case ET4PacketStoC::TurnTo: // #40
			InRecvPC->SC_RecvPacket_TurnTo(*(static_cast<const FT4PacketTurnToSC*>(InPacket)));
			break;

		case ET4PacketStoC::TeleportTo:
			InRecvPC->SC_RecvPacket_TeleportTo(*(static_cast<const FT4PacketTeleportToSC*>(InPacket)));
			break;

		case ET4PacketStoC::MoveStop: // #52
			InRecvPC->SC_RecvPacket_MoveStop(*(static_cast<const FT4PacketMoveStopSC*>(InPacket)));
			break;

		case ET4PacketStoC::MoveSpeedSync: // #52
			InRecvPC->SC_RecvPacket_MoveSpeedSync(*(static_cast<const FT4PacketMoveSpeedSyncSC*>(InPacket)));
			break;

		case ET4PacketStoC::LockOn:
			InRecvPC->SC_RecvPacket_LockOn(*(static_cast<const FT4PacketLockOnSC*>(InPacket)));
			break;

		case ET4PacketStoC::LockOff:
			InRecvPC->SC_RecvPacket_LockOff(*(static_cast<const FT4PacketLockOffSC*>(InPacket)));
			break;

		case ET4PacketStoC::Stance: // #73
			InRecvPC->SC_RecvPacket_Stance(*(static_cast<const FT4PacketStanceSC*>(InPacket)));
			break;

		case ET4PacketStoC::Equip:
			InRecvPC->SC_RecvPacket_Equip(*(static_cast<const FT4PacketEquipSC*>(InPacket)));
			break;

		case ET4PacketStoC::UnEquip:
			InRecvPC->SC_RecvPacket_UnEquip(*(static_cast<const FT4PacketUnEquipSC*>(InPacket)));
			break;

		case ET4PacketStoC::Exchange: // #37
			InRecvPC->SC_RecvPacket_Exchange(*(static_cast<const FT4PacketExchangeSC*>(InPacket)));
			break;

		case ET4PacketStoC::Attack:
			InRecvPC->SC_RecvPacket_Attack(*(static_cast<const FT4PacketAttackSC*>(InPacket)));
			break;

		case ET4PacketStoC::EffectDirect:
			InRecvPC->SC_RecvPacket_EffectDirect(*(static_cast<const FT4PacketEffectDirectSC*>(InPacket)));
			break;

		case ET4PacketStoC::EffectArea: // #68
			InRecvPC->SC_RecvPacket_EffectArea(*(static_cast<const FT4PacketEffectAreaSC*>(InPacket)));
			break;

		case ET4PacketStoC::Die: // #76
			InRecvPC->SC_RecvPacket_Die(*(static_cast<const FT4PacketDieSC*>(InPacket)));
			break;

		case ET4PacketStoC::Resurrect: // #76
			InRecvPC->SC_RecvPacket_Resurrect(*(static_cast<const FT4PacketResurrectSC*>(InPacket)));
			break;

		default:
			{
				UE_LOG(
					LogT4Gameplay,
					Error,
					TEXT("[SL:%u] SendPacketInternal '%s' failed. no implementation."),
					uint32(LayerType),
					*(InPacket->ToString())
				);
				bResult = false;
			}
			break;
	}
	return bResult;
}

#if (WITH_EDITOR || WITH_SERVER_CODE)
bool FT4PacketHandlerSC::DoSendPacketForServer(
	FT4PacketStoC* InPacket,
	IT4PlayerController* InRecvPC
)
{
	FString OutString;
	if (!InPacket->Validate(OutString))
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] DoSendPacketForServer '%s' failed. error msg '%s'"),
			uint32(LayerType),
			*(InPacket->ToString()),
			*OutString
		);
		return false;
	}
	AT4GameplayPlayerController* TargetPC = static_cast<AT4GameplayPlayerController*>(InRecvPC);
	check(nullptr != TargetPC);
	bool bResult = SendPacketInternal(InPacket, TargetPC);
	return bResult;
}

bool FT4PacketHandlerSC::DoBroadcastPacketForServer(
	FT4PacketStoC* InPacket, 
	bool bProcessServerPacket
)
{
	FString OutString;
	if (!InPacket->Validate(OutString))
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] DoBroadcastPacketForServer '%s' failed. error msg '%s'"),
			uint32(LayerType),
			*(InPacket->ToString()),
			*OutString
		);
		return false;
	}
	bool bResult = true;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AT4GameplayPlayerController* TargetPC = Cast<AT4GameplayPlayerController>(*It);
		if (nullptr != TargetPC && TargetPC->HasAuthority())
		{
			bResult &= SendPacketInternal(InPacket, TargetPC);
		}
	}
	// #49 : WARN : 클라이언트로 패킷 전송 후 서버 로직이 처리되도록 순서 변경.
	//       NPC Spawn 후 EquipWeapon 처리가 패킷 순서가 바뀌며 처리되지 못하는 문제가 있었음.
	if (bProcessServerPacket && T4EngineLayer::IsServer(LayerType))
	{
		bResult = DoProcessPacketOnlyServer(InPacket, false);
	}
	return bResult;
}

bool FT4PacketHandlerSC::DoProcessPacketOnlyServer(
	FT4PacketStoC* InPacket,
	bool bCheckValidate
) // #52
{
	check(T4EngineLayer::IsServer(LayerType)); // Only Server
	FString OutString;
	if (bCheckValidate && !InPacket->Validate(OutString))
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] DoProcessPacketOnlyServer '%s' failed. error msg '%s'"),
			uint32(LayerType),
			*(InPacket->ToString()),
			*OutString
		);
		return false;
	}
	bool bResult = OnRecvPacket(InPacket); // #15 : 서버 World 로 전달
	return bResult;
}

#endif

bool FT4PacketHandlerSC::OnRecvPacket(const FT4PacketStoC* InPacket)
{
	check(ET4LayerType::Max != LayerType);
	check(nullptr != InPacket);
	bool bResult = true;
	const ET4PacketStoC PacketSC = InPacket->PacketSC;
	switch (PacketSC)
	{
		// #27
		// #T4_ADD_PACKET_TAG_SC
		case ET4PacketStoC::WorldTravel:
			HandleSC_WorldTravel(static_cast<const FT4PacketWorldTravelSC*>(InPacket));
			break;

		case ET4PacketStoC::MyPCEnter:
			HandleSC_MyPCEnter(static_cast<const FT4PacketMyPCEnterSC*>(InPacket));
			break;

		case ET4PacketStoC::MyPCChange: // #11, #52
			HandleSC_MyPCChange(static_cast<const FT4PacketMyPCChangeSC*>(InPacket));
			break;

		case ET4PacketStoC::PCEnter:
			HandleSC_PCEnter(static_cast<const FT4PacketPCEnterSC*>(InPacket));
			break;

		case ET4PacketStoC::PCLeave:
			HandleSC_PCLeave(static_cast<const FT4PacketPCLeaveSC*>(InPacket));
			break;

		case ET4PacketStoC::NPCEnter:
			HandleSC_NPCEnter(static_cast<const FT4PacketNPCEnterSC*>(InPacket));
			break;

		case ET4PacketStoC::NPCLeave:
			HandleSC_NPCLeave(static_cast<const FT4PacketNPCLeaveSC*>(InPacket));
			break;

		case ET4PacketStoC::FOEnter:
			HandleSC_FOEnter(static_cast<const FT4PacketFOEnterSC*>(InPacket));
			break;

		case ET4PacketStoC::FOLeave:
			HandleSC_FOLeave(static_cast<const FT4PacketFOLeaveSC*>(InPacket));
			break;

		case ET4PacketStoC::ItemEnter: // #41
			HandleSC_ItemEnter(static_cast<const FT4PacketItemEnterSC*>(InPacket));
			break;

		case ET4PacketStoC::ItemLeave: // #41
			HandleSC_ItemLeave(static_cast<const FT4PacketItemLeaveSC*>(InPacket));
			break;

		case ET4PacketStoC::MoveTo:
			HandleSC_MoveTo(static_cast<const FT4PacketMoveToSC*>(InPacket));
			break;

		case ET4PacketStoC::JumpTo:
			HandleSC_JumpTo(static_cast<const FT4PacketJumpToSC*>(InPacket));
			break;

		case ET4PacketStoC::RollTo: // #46
			HandleSC_RollTo(static_cast<const FT4PacketRollToSC*>(InPacket));
			break;

		case ET4PacketStoC::TurnTo: // #40
			HandleSC_TurnTo(static_cast<const FT4PacketTurnToSC*>(InPacket));
			break;

		case ET4PacketStoC::TeleportTo:
			HandleSC_TeleportTo(static_cast<const FT4PacketTeleportToSC*>(InPacket));
			break;

		case ET4PacketStoC::MoveStop: // #52
			HandleSC_MoveStop(static_cast<const FT4PacketMoveStopSC*>(InPacket));
			break;

		case ET4PacketStoC::MoveSpeedSync: // #52
			HandleSC_MoveSpeedSync(static_cast<const FT4PacketMoveSpeedSyncSC*>(InPacket));
			break;

		case ET4PacketStoC::LockOn:
			HandleSC_LockOn(static_cast<const FT4PacketLockOnSC*>(InPacket));
			break;

		case ET4PacketStoC::LockOff:
			HandleSC_LockOff(static_cast<const FT4PacketLockOffSC*>(InPacket));
			break;

		case ET4PacketStoC::Stance: // #73
			HandleSC_Stance(static_cast<const FT4PacketStanceSC*>(InPacket));
			break;

		case ET4PacketStoC::Equip:
			HandleSC_Equip(static_cast<const FT4PacketEquipSC*>(InPacket));
			break;

		case ET4PacketStoC::UnEquip:
			HandleSC_UnEquip(static_cast<const FT4PacketUnEquipSC*>(InPacket));
			break;

		case ET4PacketStoC::Exchange:
			HandleSC_Exchange(static_cast<const FT4PacketExchangeSC*>(InPacket)); // #37
			break;

		case ET4PacketStoC::Attack:
			HandleSC_Attack(static_cast<const FT4PacketAttackSC*>(InPacket));
			break;

		case ET4PacketStoC::EffectDirect:
			HandleSC_EffectDirect(static_cast<const FT4PacketEffectDirectSC*>(InPacket));
			break;

		case ET4PacketStoC::EffectArea: // #68
			HandleSC_EffectArea(static_cast<const FT4PacketEffectAreaSC*>(InPacket)); // #68
			break;

		case ET4PacketStoC::Die: // #76
			HandleSC_Die(static_cast<const FT4PacketDieSC*>(InPacket));
			break;

		case ET4PacketStoC::Resurrect: // #76
			HandleSC_Resurrect(static_cast<const FT4PacketResurrectSC*>(InPacket)); // #68
			break;

		default:
			{
				UE_LOG(
					LogT4Gameplay,
					Error,
					TEXT("[SL:%u] OnRecvPacket '%s' failed. no implementation."),
					uint32(LayerType),
					*(InPacket->ToString())
				);
				bResult = false;
			}
			break;
	}
	return true;
}
