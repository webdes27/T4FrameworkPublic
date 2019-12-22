// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4PacketHandler_CS.h"

#include "Classes/Controller/Player/T4GameplayPlayerController.h" // #42

#include "Public/Protocol/T4PacketCSMinimal.h"

#include "Gameplay/T4GameplayInstance.h"

#include "T4Engine/Public/T4Engine.h"

#include "T4GameplayInternal.h"

/**
  *
 */
FT4PacketHandlerCS::FT4PacketHandlerCS(ET4LayerType InLayerType)
	: LayerType(InLayerType)
{
	check(ET4LayerType::Max != LayerType);
}

FT4PacketHandlerCS::~FT4PacketHandlerCS()
{
}

void FT4PacketHandlerCS::Reset()
{
}

UWorld* FT4PacketHandlerCS::GetWorld() const
{
	check(ET4LayerType::Max > LayerType);
	IT4GameWorld* GameWorld = T4EngineWorldGet(LayerType);
	check(nullptr != GameWorld);
	return GameWorld->GetWorld();
}

IT4GameWorld* FT4PacketHandlerCS::GetGameWorld() const // #52
{
	check(ET4LayerType::Max > LayerType);
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	return GameFrame->GetGameWorld();
}

IT4PlayerController* FT4PacketHandlerCS::GetPlayerController() const
{
	check(ET4LayerType::Max > LayerType);
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	return GameFrame->GetPlayerController();
}

IT4GameObject* FT4PacketHandlerCS::GetGameObjectForServer(const FT4ObjectID& InObjectID) const
{
	check(ET4LayerType::Max > LayerType);
	IT4GameWorld* GameWorld = T4EngineWorldGet(LayerType);
	check(nullptr != GameWorld);
	return GameWorld->GetContainer()->FindGameObject(InObjectID);
}

AT4GameplayPlayerController* FT4PacketHandlerCS::CastGameplayPlayerController(
	IT4GameObject* InGameObject
) // #52
{
	check(nullptr != InGameObject);
	IT4ObjectController* GameplayControl = InGameObject->GetObjectController();
	if (nullptr == GameplayControl)
	{
		return nullptr;
	}
	AT4GameplayPlayerController* CastGameplayPlayerController = Cast<AT4GameplayPlayerController>(
		GameplayControl->GetAController()
	);
	if (nullptr == CastGameplayPlayerController)
	{
		return nullptr;
	}
	return CastGameplayPlayerController;
}

IT4PacketHandlerSC* FT4PacketHandlerCS::GetPacketHandlerSC() const
{
	check(ET4LayerType::Max > LayerType);
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	FT4GameplayInstance* GameplayInstance = FT4GameplayInstance::CastFrom(
		GameFrame->GetGameplayInstance()
	);
	if (nullptr == GameplayInstance)
	{
		return nullptr;
	}
	return GameplayInstance->GetPacketHandlerSC();
}

bool FT4PacketHandlerCS::DoSendPacket(FT4PacketCtoS* InPacket)
{
	FString OutString;
	if (!InPacket->Validate(OutString))
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] DoSendPacket '%s' failed. error msg '%s'"),
			uint32(LayerType),
			*(InPacket->ToString()),
			*OutString
		);
		return false;
	}

	bool bResult = true;

	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);

#if WITH_EDITOR
	if (T4EngineLayer::IsToolSide(LayerType))
	{
		// WARN : #29, #17 : Only Editor LayerType
		if (OnRecvPacket_Validation(InPacket))
		{
			bResult = OnRecvPacket(InPacket, PlayerController);
		}
		return bResult;
	}
#endif

	AT4GameplayPlayerController* MyPC = static_cast<AT4GameplayPlayerController*>(PlayerController);
	check(nullptr != MyPC);

	const ET4PacketCtoS PacketCS = InPacket->PacketCS;
	switch (PacketCS)
	{
		// #27
		// #T4_ADD_PACKET_TAG_CS
		case ET4PacketCtoS::Move:
			MyPC->CS_RecvPacket_Move(*(static_cast<const FT4PacketMoveCS*>(InPacket)));
			break;

		case ET4PacketCtoS::Jump:
			MyPC->CS_RecvPacket_Jump(*(static_cast<const FT4PacketJumpCS*>(InPacket)));
			break;

		case ET4PacketCtoS::Roll: // #46
			MyPC->CS_RecvPacket_Roll(*(static_cast<const FT4PacketRollCS*>(InPacket)));
			break;

		case ET4PacketCtoS::Turn: // #40
			MyPC->CS_RecvPacket_Turn(*(static_cast<const FT4PacketTurnCS*>(InPacket)));
			break;

		case ET4PacketCtoS::LockOn:
			MyPC->CS_RecvPacket_LockOn(*(static_cast<const FT4PacketLockOnCS*>(InPacket)));
			break;

		case ET4PacketCtoS::LockOff:
			MyPC->CS_RecvPacket_LockOff(*(static_cast<const FT4PacketLockOffCS*>(InPacket)));
			break;

		case ET4PacketCtoS::Stance: // #73
			MyPC->CS_RecvPacket_Stance(*(static_cast<const FT4PacketStanceCS*>(InPacket)));
			break;

		case ET4PacketCtoS::Equip:
			MyPC->CS_RecvPacket_Equip(*(static_cast<const FT4PacketEquipCS*>(InPacket)));
			break;

		case ET4PacketCtoS::UnEquip:
			MyPC->CS_RecvPacket_UnEquip(*(static_cast<const FT4PacketUnEquipCS*>(InPacket)));
			break;

		case ET4PacketCtoS::Exchange: // #37
			MyPC->CS_RecvPacket_Exchange(*(static_cast<const FT4PacketExchangeCS*>(InPacket)));
			break;

		case ET4PacketCtoS::Attack:
			MyPC->CS_RecvPacket_Attack(*(static_cast<const FT4PacketAttackCS*>(InPacket)));
			break;

		case ET4PacketCtoS::CmdWorldTravel:
			MyPC->CS_RecvPacket_CmdWorldTravel(*(static_cast<const FT4PacketCmdWorldTravelCS*>(InPacket)));
			break;

		case ET4PacketCtoS::CmdChangePlayer: // #11, #52
			MyPC->CS_RecvPacket_CmdChangePlayer(*(static_cast<const FT4PacketCmdChangePlayerCS*>(InPacket)));
			break;

		case ET4PacketCtoS::CmdPCEnter:
			MyPC->CS_RecvPacket_CmdPCEnter(*(static_cast<const FT4PacketCmdPCEnterCS*>(InPacket)));
			break;

		case ET4PacketCtoS::CmdNPCEnter:
			MyPC->CS_RecvPacket_CmdNPCEnter(*(static_cast<const FT4PacketCmdNPCEnterCS*>(InPacket)));
			break;

		case ET4PacketCtoS::CmdFOEnter:
			MyPC->CS_RecvPacket_CmdFOEnter(*(static_cast<const FT4PacketCmdFOEnterCS*>(InPacket)));
			break;

		case ET4PacketCtoS::CmdItemEnter: // #41
			MyPC->CS_RecvPacket_CmdItemEnter(*(static_cast<const FT4PacketCmdItemEnterCS*>(InPacket)));
			break;

		case ET4PacketCtoS::CmdLeave: // #68
			MyPC->CS_RecvPacket_CmdLeave(*(static_cast<const FT4PacketCmdLeaveCS*>(InPacket)));
			break;

		case ET4PacketCtoS::CmdTeleport:
			MyPC->CS_RecvPacket_CmdTeleport(*(static_cast<const FT4PacketCmdTeleportCS*>(InPacket)));
			break;

		default:
			{
				UE_LOG(
					LogT4Gameplay,
					Error,
					TEXT("[SL:%u] OnSendPacket '%s' failed. no implementation."),
					uint32(LayerType),
					*(InPacket->ToString())
				);
				bResult = false;
			}
			break;
	}
	return bResult;
}

bool FT4PacketHandlerCS::OnRecvPacket_Validation(const FT4PacketCtoS* InPacket)
{
	check(ET4LayerType::Max != LayerType);
	check(nullptr != InPacket);

	// TODO

	return true;
}

bool FT4PacketHandlerCS::OnRecvPacket(
	const FT4PacketCtoS* InPacket,
	IT4PlayerController* InSenderPC
)
{
	check(ET4LayerType::Max != LayerType);
	check(nullptr != InSenderPC);
	check(nullptr != GetPacketHandlerSC());
	check(nullptr != InPacket);
	bool bResult = true;
	const ET4PacketCtoS PacketCS = InPacket->PacketCS;
	switch (PacketCS)
	{
		// #27
		// #T4_ADD_PACKET_TAG_CS
		case ET4PacketCtoS::Move:
			HandleCS_Move(static_cast<const FT4PacketMoveCS*>(InPacket));
			break;

		case ET4PacketCtoS::Jump:
			HandleCS_Jump(static_cast<const FT4PacketJumpCS*>(InPacket));
			break;

		case ET4PacketCtoS::Roll: // #46
			HandleCS_Roll(static_cast<const FT4PacketRollCS*>(InPacket));
			break;

		case ET4PacketCtoS::Turn:
			HandleCS_Turn(static_cast<const FT4PacketTurnCS*>(InPacket));
			break;

		case ET4PacketCtoS::LockOn:
			HandleCS_LockOn(static_cast<const FT4PacketLockOnCS*>(InPacket));
			break;

		case ET4PacketCtoS::LockOff:
			HandleCS_LockOff(static_cast<const FT4PacketLockOffCS*>(InPacket));
			break;

		case ET4PacketCtoS::Stance: // #73
			HandleCS_Stance(static_cast<const FT4PacketStanceCS*>(InPacket));
			break;

		case ET4PacketCtoS::Equip:
			HandleCS_Equip(static_cast<const FT4PacketEquipCS*>(InPacket));
			break;

		case ET4PacketCtoS::UnEquip:
			HandleCS_UnEquip(static_cast<const FT4PacketUnEquipCS*>(InPacket));
			break;

		case ET4PacketCtoS::Exchange: // #37
			HandleCS_Exchange(static_cast<const FT4PacketExchangeCS*>(InPacket));
			break;

		case ET4PacketCtoS::Attack:
			HandleCS_Attack(static_cast<const FT4PacketAttackCS*>(InPacket));
			break;

		case ET4PacketCtoS::CmdWorldTravel:
			HandleCS_CmdWorldTravel(static_cast<const FT4PacketCmdWorldTravelCS*>(InPacket));
			break;

		case ET4PacketCtoS::CmdChangePlayer: // #11, #52
			HandleCS_CmdChangePlayer(static_cast<const FT4PacketCmdChangePlayerCS*>(InPacket), InSenderPC);
			break;

		case ET4PacketCtoS::CmdPCEnter:
			HandleCS_CmdPCEnter(static_cast<const FT4PacketCmdPCEnterCS*>(InPacket), InSenderPC);
			break;

		case ET4PacketCtoS::CmdNPCEnter:
			HandleCS_CmdNPCEnter(static_cast<const FT4PacketCmdNPCEnterCS*>(InPacket));
			break;

		case ET4PacketCtoS::CmdFOEnter:
			HandleCS_CmdFOEnter(static_cast<const FT4PacketCmdFOEnterCS*>(InPacket));
			break;

		case ET4PacketCtoS::CmdItemEnter:
			HandleCS_CmdItemEnter(static_cast<const FT4PacketCmdItemEnterCS*>(InPacket));
			break;

		case ET4PacketCtoS::CmdLeave: // #68
			HandleCS_CmdLeave(static_cast<const FT4PacketCmdLeaveCS*>(InPacket), InSenderPC);
			break;

		case ET4PacketCtoS::CmdTeleport:
			HandleCS_CmdTeleport(static_cast<const FT4PacketCmdTeleportCS*>(InPacket));
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
	return bResult;
}
