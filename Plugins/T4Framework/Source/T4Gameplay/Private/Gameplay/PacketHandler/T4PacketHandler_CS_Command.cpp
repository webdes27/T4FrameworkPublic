// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4PacketHandler_CS.h"

#include "Gameplay/T4GameplayAIControllerFactory.h"

#include "Classes/Controller/Player/T4GameplayPlayerController.h" // #42

#include "Public/Protocol/T4PacketCS_Command.h"
#include "Public/Protocol/T4PacketSC_World.h"

#include "GameDB/T4GameDB.h"

#include "T4Engine/Public/T4Engine.h"
#include "T4Frame/Public/T4Frame.h"

#include "Classes/Engine/World.h"

#include "T4GameplayInternal.h"

/**
  *
 */
// #27
// #T4_ADD_PACKET_TAG_CS

void FT4PacketHandlerCS::HandleCS_CmdWorldTravel(const FT4PacketCmdWorldTravelCS* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketCtoS::CmdWorldTravel == InPacket->PacketCS);
	FT4GameDB& GameDB = GetGameDB();
	const FT4GameWorldData* WorldData = GameDB.GetGameData<FT4GameWorldData>(InPacket->WorldDataID);
	if (nullptr == WorldData)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleCS_CmdWorldTravel '%' failed. WorldData '%s' not found."),
			uint32(LayerType),
			*(InPacket->ToString()),
			*(InPacket->WorldDataID.ToString())
		);
		return;
	}

	FT4PacketWorldTravelSC NewPacketSC;
	NewPacketSC.WorldDataID = InPacket->WorldDataID;
	GetPacketHandlerSC()->DoBroadcastPacketForServer(&NewPacketSC, true);
}

void FT4PacketHandlerCS::HandleCS_CmdChangePlayer(
	const FT4PacketCmdChangePlayerCS* InPacket,
	IT4PlayerController* InSenderPC
) // #11, #52
{
	check(nullptr != InPacket);
	check(nullptr != InSenderPC);
	check(ET4PacketCtoS::CmdChangePlayer == InPacket->PacketCS);

	IT4GameObject* NewPlayerObject = GetGameObjectForServer(InPacket->NewPlayerObjectID);
	if (nullptr == NewPlayerObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleCS_CmdChangePlayer '%' failed. NewPlayerObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	if (T4EngineLayer::IsServer(LayerType))
	{
		IT4ObjectController* ObjectController = NewPlayerObject->GetObjectController();
		if (nullptr == ObjectController)
		{
			return;
		}
		if (!ObjectController->HasPlayerController())
		{
			UE_LOG(
				LogT4Gameplay,
				Error,
				TEXT("[SL:%u] HandleCS_CmdChangePlayer '%' failed. Only Player GameObject."),
				uint32(LayerType),
				*(InPacket->ToString())
			);
			return;
		}
		InSenderPC->SetGameObject(InPacket->NewPlayerObjectID);
	}

	FT4PacketMyPCChangeSC NewPacketSC;
	NewPacketSC.NewPlayerObjectID = InPacket->NewPlayerObjectID;
	GetPacketHandlerSC()->DoSendPacketForServer(&NewPacketSC, InSenderPC); // 자기 자신에게만 보내면 된다.
}

void FT4PacketHandlerCS::HandleCS_CmdPCEnter(
	const FT4PacketCmdPCEnterCS* InPacket,
	IT4PlayerController* InSenderPC
)
{
	check(nullptr != InPacket);
	check(nullptr != InSenderPC);
	check(ET4PacketCtoS::CmdPCEnter == InPacket->PacketCS);
	FT4GameDB& GameDB = GetGameDB();
	const FT4GamePlayerData* PlayerData = GameDB.GetGameData<FT4GamePlayerData>(InPacket->PlayerDataID);
	if (nullptr == PlayerData)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("FT4PacketHandlerCS : failed to player enter. PlayerDataID '%s' Not Found."),
			*(InPacket->PlayerDataID.ToString())
		);
		return;
	}

	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);

	const FT4ObjectID NewSpawnObjectID = GameFrame->GenerateObjectIDForServer();
	check(NewSpawnObjectID.IsValid());

	const bool bMyPCEntered = InPacket->bSetViewTarget;

	FT4PacketPCEnterSC NewPCEnterPacketSC;
	NewPCEnterPacketSC.EnterObjectID = NewSpawnObjectID;
	NewPCEnterPacketSC.PlayerDataID = InPacket->PlayerDataID;
	NewPCEnterPacketSC.SpawnLocation = InPacket->SpawnLocation;
	NewPCEnterPacketSC.SpawnRotation = InPacket->SpawnRotation;

	check(NewPCEnterPacketSC.EnterObjectID.IsValid());

	IT4PacketHandlerSC* PacketHandlerSC = GetPacketHandlerSC();
	check(nullptr != PacketHandlerSC);

	if (T4EngineLayer::IsServer(LayerType))
	{
		PacketHandlerSC->OnRecvPacket(&NewPCEnterPacketSC); // #15
		if (bMyPCEntered)
		{
			InSenderPC->SetGameObject(NewSpawnObjectID);
		}
	}

	// #15 : 클라이언트 전파!
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AT4GameplayPlayerController* TargetPC = Cast<AT4GameplayPlayerController>(*It);
		if (nullptr != TargetPC && TargetPC->HasAuthority())
		{
			if (bMyPCEntered && TargetPC == InSenderPC)
			{
				// WARN : MyPC 의 경우 별도의 패킷 처리가 필요함으로 분리
				FT4PacketMyPCEnterSC NewMyPCEnterPacketSC;
				NewMyPCEnterPacketSC.EnterObjectID = NewSpawnObjectID;
				NewMyPCEnterPacketSC.PlayerDataID = InPacket->PlayerDataID;
				NewMyPCEnterPacketSC.SpawnLocation = InPacket->SpawnLocation;
				NewMyPCEnterPacketSC.SpawnRotation = InPacket->SpawnRotation;
				PacketHandlerSC->DoSendPacketForServer(&NewMyPCEnterPacketSC, TargetPC);
			}
			else
			{
				PacketHandlerSC->DoSendPacketForServer(&NewPCEnterPacketSC, TargetPC);
			}
		}
	}
}

// #31
void FT4PacketHandlerCS::HandleCS_CmdNPCEnter(const FT4PacketCmdNPCEnterCS* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketCtoS::CmdNPCEnter == InPacket->PacketCS);
	FT4GameDB& GameDB = GetGameDB();
	const FT4GameNPCData* NPCData = GameDB.GetGameData<FT4GameNPCData>(InPacket->NPCDataID);
	if (nullptr == NPCData)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("FT4PacketHandlerCS : failed to npc enter. NPCData '%s' Not Found."),
			*(InPacket->NPCDataID.ToString())
		);
		return;
	}

	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);

	FT4NetID NewNetID; // #41

#if (WITH_EDITOR || WITH_SERVER_CODE)
	// #31 : NPCAI :
	NewNetID = GetGameplayAIControllerFactory().CreateCreatureAIController(
		LayerType,
		InPacket->NPCDataID,
		InPacket->SpawnLocation,
		InPacket->SpawnRotation
	);
#endif

	FT4PacketNPCEnterSC NewPacketSC;
	NewPacketSC.NetID = NewNetID;
	NewPacketSC.EnterObjectID = GameFrame->GenerateObjectIDForServer();
	NewPacketSC.NPCDataID = InPacket->NPCDataID;
	NewPacketSC.SpawnLocation = InPacket->SpawnLocation;
	NewPacketSC.SpawnRotation = InPacket->SpawnRotation;
	GetPacketHandlerSC()->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
}

// #31
void FT4PacketHandlerCS::HandleCS_CmdFOEnter(const FT4PacketCmdFOEnterCS* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketCtoS::CmdFOEnter == InPacket->PacketCS);
	FT4GameDB& GameDB = GetGameDB();
	const FT4GameFOData* FOData = GameDB.GetGameData<FT4GameFOData>(InPacket->FODataID);
	if (nullptr == FOData)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("FT4PacketHandlerCS : failed to FO enter. FOData '%s' Not Found."),
			*(InPacket->FODataID.ToString())
		);
		return;
	}

	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);

	FT4NetID NewNetID; // #41

#if (WITH_EDITOR || WITH_SERVER_CODE)
	// #41 : Item/FO AI
	NewNetID = GetGameplayAIControllerFactory().CreateFOAIController(
		LayerType,
		InPacket->FODataID,
		InPacket->SpawnLocation,
		InPacket->SpawnRotation
	);
#endif

	const FT4ObjectID NewSpawnObjectID = GameFrame->GenerateObjectIDForServer();
	FT4PacketFOEnterSC NewPacketSC;
	NewPacketSC.NetID = NewNetID; // #41
	NewPacketSC.EnterObjectID = NewSpawnObjectID;
	NewPacketSC.FODataID = InPacket->FODataID;
	NewPacketSC.SpawnLocation = InPacket->SpawnLocation;
	NewPacketSC.SpawnRotation = InPacket->SpawnRotation;
	GetPacketHandlerSC()->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
}

// #41
void FT4PacketHandlerCS::HandleCS_CmdItemEnter(const FT4PacketCmdItemEnterCS* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketCtoS::CmdItemEnter == InPacket->PacketCS);
	FT4GameDB& GameDB = GetGameDB();

	// #48
	bool bResult = false;
	if (ET4GameDataType::Item_Weapon == InPacket->ItemDataID.Type)
	{
		const FT4GameItemWeaponData* ItemWeaponData = GameDB.GetGameData<FT4GameItemWeaponData>(InPacket->ItemDataID);
		if (nullptr != ItemWeaponData)
		{
			bResult = true;
		}
	}
	else
	{
		const FT4GameItemCostumeData* ItemCostumeData = GameDB.GetGameData<FT4GameItemCostumeData>(InPacket->ItemDataID);
		if (nullptr != ItemCostumeData)
		{
			bResult = true;
		}
	}
	if (!bResult)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("FT4PacketHandlerCS : failed to Item enter. ItemData '%s' Not Found."),
			*(InPacket->ItemDataID.ToString())
		);
		return;
	}

	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);

	FT4NetID NewNetID; // #41

#if (WITH_EDITOR || WITH_SERVER_CODE)
	// #41 : Item/FO AI
	NewNetID = GetGameplayAIControllerFactory().CreateFOAIController(
		LayerType,
		InPacket->ItemDataID,
		InPacket->SpawnLocation,
		InPacket->SpawnRotation
	);
#endif

	const FT4ObjectID NewSpawnObjectID = GameFrame->GenerateObjectIDForServer();
	FT4PacketItemEnterSC NewPacketSC;
	NewPacketSC.NetID = NewNetID; // #41
	NewPacketSC.EnterObjectID = NewSpawnObjectID;
	NewPacketSC.ItemDataID = InPacket->ItemDataID;
	NewPacketSC.SpawnLocation = InPacket->SpawnLocation;
	NewPacketSC.SpawnRotation = InPacket->SpawnRotation;
	GetPacketHandlerSC()->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
}

// #68
void FT4PacketHandlerCS::HandleCS_CmdLeave(
	const FT4PacketCmdLeaveCS* InPacket,
	IT4PlayerController* InSenderPC
)
{
	check(nullptr != InPacket);
	check(ET4PacketCtoS::CmdLeave == InPacket->PacketCS);
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	check(nullptr != GameWorld);
	IT4GameObject* EnteredObject = GameWorld->GetContainer()->FindGameObject(InPacket->LeaveObjectID);
	if (nullptr == EnteredObject)
	{
		check(false); // WARN: 없다? 지금은 에러!
	}
	{
		// TODO : 검증!
	}

	IT4PacketHandlerSC* PacketHandlerSC = GetPacketHandlerSC();
	check(nullptr != PacketHandlerSC);

	IT4ObjectController* ObjectController = EnteredObject->GetObjectController();
	check(nullptr != ObjectController);
	const FName ObjectClassTypeName = ObjectController->GetClassTypeName();
	if (DefaultPlayerClassName == ObjectClassTypeName)
	{
		FT4PacketPCLeaveSC NewPacketSC;
		NewPacketSC.LeaveObjectID = InPacket->LeaveObjectID;

		if (T4EngineLayer::IsServer(LayerType))
		{
			if (InPacket->LeaveObjectID == InSenderPC->GetGameObjectID())
			{
				InSenderPC->ClearGameObject(true);
			}
		}

		PacketHandlerSC->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
	}
#if (WITH_EDITOR || WITH_SERVER_CODE) // #68 : 클라에서는 GameplayControl 은 오직 MyPC 밖에 없다.
	else if (DefaultCreatureClassName == ObjectClassTypeName)
	{
		GetGameplayAIControllerFactory().DestroyCreatureAIController(
			LayerType,
			ObjectController->GetAController()
		); // #68

		FT4PacketNPCLeaveSC NewPacketSC;
		NewPacketSC.LeaveObjectID = InPacket->LeaveObjectID;
		PacketHandlerSC->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
	}
	else if (DefaultFOClassName == ObjectClassTypeName)
	{
		GetGameplayAIControllerFactory().DestroyFOAIController(
			LayerType,
			ObjectController->GetAController()
		); // #68

		FT4PacketFOLeaveSC NewPacketSC;
		NewPacketSC.LeaveObjectID = InPacket->LeaveObjectID;
		PacketHandlerSC->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
	}
	else if (DefaultItemClassName == ObjectClassTypeName)
	{
		GetGameplayAIControllerFactory().DestroyItemAIController(
			LayerType,
			ObjectController->GetAController()
		); // #68

		FT4PacketItemLeaveSC NewPacketSC;
		NewPacketSC.LeaveObjectID = InPacket->LeaveObjectID;
		PacketHandlerSC->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
	}
#endif
	else
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("FT4PacketHandlerCS:HandleCS_CmdLeave '%s' failed. no implementation."),
			*(ObjectClassTypeName.ToString())
		);
	}
}