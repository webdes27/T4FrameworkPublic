// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4PacketHandler_SC.h"
#include "T4GameplayDefinitions.h"

#include "Public/Protocol/T4PacketSC_World.h"
#include "GameDB/T4GameDB.h"

#include "Classes/Controller/AI/T4GameplayCreatureAIController.h" // #50
#include "Classes/Controller/AI/T4GameplayFOAIController.h" // #41, #50
#include "Classes/Controller/AI/T4GameplayItemAIController.h" // #41, #50

#include "T4Engine/Public/Action/T4ActionCodeMinimal.h"
#include "T4Engine/Public/T4Engine.h"
#include "T4Frame/Public/T4Frame.h"

#include "Classes/Engine/World.h"

#include "T4GameplayInternal.h"

/**
  *
 */
// #27
// #T4_ADD_PACKET_TAG_SC

void FT4PacketHandlerSC::HandleSC_WorldTravel(const FT4PacketWorldTravelSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::WorldTravel == InPacket->PacketSC);
	FT4GameDB& GameDB = GetGameDB();
	const FT4GameWorldData* WorldData = GameDB.GetGameData<FT4GameWorldData>(InPacket->WorldDataID);
	if (nullptr == WorldData)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_WorldTravel '%' failed. WorldData '%s' not found."),
			uint32(LayerType),
			*(InPacket->ToString()),
			*(InPacket->WorldDataID.ToString())
		);
		return;
	}

	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);

	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();;
	check(nullptr != GameWorld);

	FT4WorldTravelAction NewAction;
	NewAction.MapEntityOrLevelObjectPath = WorldData->RawData.EntityAsset.ToSoftObjectPath();
	GameWorld->DoExecuteAction(&NewAction);
}

void FT4PacketHandlerSC::HandleSC_MyPCEnter(const FT4PacketMyPCEnterSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::MyPCEnter == InPacket->PacketSC);
	FT4GameDB& GameDB = GetGameDB();
	const FT4GamePlayerData* PlayerData = GameDB.GetGameData<FT4GamePlayerData>(InPacket->PlayerDataID);
	if (nullptr == PlayerData)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("FT4PacketHandlerSC : failed to MyPC enter. PlayerDataID '%s' Not Found."),
			*(InPacket->PlayerDataID.ToString())
		);
		return;
	}

	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);

	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();;
	check(nullptr != GameWorld);

	IT4GameObject* EnteredObject = GameWorld->GetContainer()->FindGameObject(InPacket->EnterObjectID);
	if (nullptr != EnteredObject)
	{
		check(false); // WARN: 이미 스폰되어 있다?
	}

	FT4SpawnObjectAction NewAction;
	NewAction.ObjectID = InPacket->EnterObjectID;
	NewAction.Name = TEXT("T4MyPCObject");
	NewAction.EntityType = ET4EntityType::Character;
	NewAction.EntityAssetPath = PlayerData->RawData.EntityAsset.ToSoftObjectPath();
	NewAction.GameDataIDName = InPacket->PlayerDataID.RowName;
	NewAction.SpawnLocation = InPacket->SpawnLocation;
	NewAction.SpawnRotation = InPacket->SpawnRotation;
	NewAction.bPlayer = true;
	bool bResult = GameWorld->DoExecuteAction(&NewAction);
	if (bResult)
	{
		IT4PlayerController* MyPC = GetPlayerController();
		check(nullptr != MyPC);
		MyPC->SetGameObject(InPacket->EnterObjectID);
	}
}

void FT4PacketHandlerSC::HandleSC_MyPCChange(const FT4PacketMyPCChangeSC* InPacket) // #11, #52
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::MyPCChange == InPacket->PacketSC);

	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);

	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();;
	check(nullptr != GameWorld);

	IT4GameObject* NewPlayerObject = GameWorld->GetContainer()->FindGameObject(InPacket->NewPlayerObjectID);
	if (nullptr == NewPlayerObject)
	{
		return; // WARN: 대상이 없다?
	}

	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	PlayerController->SetGameObject(InPacket->NewPlayerObjectID);
}

void FT4PacketHandlerSC::HandleSC_PCEnter(const FT4PacketPCEnterSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::PCEnter == InPacket->PacketSC);
	FT4GameDB& GameDB = GetGameDB();
	const FT4GamePlayerData* PlayerData = GameDB.GetGameData<FT4GamePlayerData>(InPacket->PlayerDataID);
	if (nullptr == PlayerData)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("FT4PacketHandlerSC : failed to player enter. PlayerDataID '%s' Not Found."),
			*(InPacket->PlayerDataID.ToString())
		);
		return;
	}

	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);

	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();;
	check(nullptr != GameWorld);

	IT4GameObject* EnteredObject = GameWorld->GetContainer()->FindGameObject(InPacket->EnterObjectID);
	if (nullptr != EnteredObject)
	{
		check(false); // WARN: 이미 스폰되어 있다?
	}
	FT4SpawnObjectAction NewAction;
	NewAction.ObjectID = InPacket->EnterObjectID;
	NewAction.Name = TEXT("T4OtherPCObject");
	NewAction.EntityType = ET4EntityType::Character;
	NewAction.EntityAssetPath = PlayerData->RawData.EntityAsset.ToSoftObjectPath();
	NewAction.StanceName = InPacket->StanceName; // #73
	NewAction.GameDataIDName = InPacket->PlayerDataID.RowName;
	NewAction.SpawnLocation = InPacket->SpawnLocation;
	NewAction.SpawnRotation = InPacket->SpawnRotation;
	GameWorld->DoExecuteAction(&NewAction);
}

void FT4PacketHandlerSC::HandleSC_PCLeave(const FT4PacketPCLeaveSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::PCLeave == InPacket->PacketSC);

	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);

	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();;
	check(nullptr != GameWorld);

	IT4GameObject* EnteredObject = GameWorld->GetContainer()->FindGameObject(InPacket->LeaveObjectID);
	if (nullptr == EnteredObject)
	{
		check(false); // WARN: 없다? 지금은 에러!
	}
	bool bIsMyPC = GameWorld->IsPlayerObject(EnteredObject);
	if (bIsMyPC)
	{
		IT4PlayerController* MyPC = GetPlayerController();
		check(nullptr != MyPC);
		MyPC->ClearGameObject(true);
	}
	FT4DespawnObjectAction NewAction;
	NewAction.ObjectID = InPacket->LeaveObjectID;
	GameWorld->DoExecuteAction(&NewAction);
}

void FT4PacketHandlerSC::HandleSC_NPCEnter(const FT4PacketNPCEnterSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::NPCEnter == InPacket->PacketSC);
	FT4GameDB& GameDB = GetGameDB();
	const FT4GameNPCData* NPCData = GameDB.GetGameData<FT4GameNPCData>(InPacket->NPCDataID);
	if (nullptr == NPCData)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("FT4PacketHandlerSC : failed to NPC enter. NPCData '%s' Not Found."),
			*(InPacket->NPCDataID.ToString())
		);
		return;
	}

	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);

	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();;
	check(nullptr != GameWorld);

	IT4GameObject* EnteredObject = GameWorld->GetContainer()->FindGameObject(InPacket->EnterObjectID);
	if (nullptr != EnteredObject)
	{
		check(false); // WARN: 이미 스폰되어 있다?
	}

	FT4SpawnObjectAction NewAction;
	NewAction.ObjectID = InPacket->EnterObjectID;
	NewAction.Name = TEXT("T4NPCObject");
	NewAction.EntityType = ET4EntityType::Character;
	NewAction.EntityAssetPath = NPCData->RawData.EntityAsset.ToSoftObjectPath();
	NewAction.GameDataIDName = InPacket->NPCDataID.RowName;
	NewAction.SpawnLocation = InPacket->SpawnLocation;
	NewAction.SpawnRotation = InPacket->SpawnRotation;
	GameWorld->DoExecuteAction(&NewAction);

#if (WITH_EDITOR || WITH_SERVER_CODE)
	EnteredObject = GameWorld->GetContainer()->FindGameObject(InPacket->EnterObjectID);
	if (nullptr != EnteredObject)
	{
		IT4PlayerController* MyPC = GetPlayerController();
		if (T4EngineLayer::IsServer(LayerType) || (nullptr != MyPC && MyPC->CheckAuthority())) // #15 : Only HasAuthority
		{
			IT4GameAIController* GameAIController = GameFrame->FindGameAIController(InPacket->NetID);
			if (nullptr != GameAIController)
			{
				AT4GameplayCreatureAIController* NPCController = Cast<AT4GameplayCreatureAIController>(
					GameAIController->GetAIController()
				);
				check(nullptr != NPCController);
				NPCController->SetGameObject(InPacket->EnterObjectID);
			}
		}
	}
#endif
}

void FT4PacketHandlerSC::HandleSC_NPCLeave(const FT4PacketNPCLeaveSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::NPCLeave == InPacket->PacketSC);

	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);

	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();;
	check(nullptr != GameWorld);

	IT4GameObject* EnteredObject = GameWorld->GetContainer()->FindGameObject(InPacket->LeaveObjectID);
	if (nullptr == EnteredObject)
	{
		check(false); // WARN: 없다? 지금은 에러!
	}

	FT4DespawnObjectAction NewAction;
	NewAction.ObjectID = InPacket->LeaveObjectID;
	GameWorld->DoExecuteAction(&NewAction);
}

void FT4PacketHandlerSC::HandleSC_FOEnter(const FT4PacketFOEnterSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::FOEnter == InPacket->PacketSC);
	FT4GameDB& GameDB = GetGameDB();
	const FT4GameFOData* FOData = GameDB.GetGameData<FT4GameFOData>(InPacket->FODataID);
	if (nullptr == FOData)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("FT4PacketHandlerSC : failed to FO enter. FOData '%s' Not Found."),
			*(InPacket->FODataID.ToString())
		);
		return;
	}

	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);

	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();;
	check(nullptr != GameWorld);

	IT4GameObject* EnteredObject = GameWorld->GetContainer()->FindGameObject(InPacket->EnterObjectID);
	if (nullptr != EnteredObject)
	{
		check(false); // WARN: 이미 스폰되어 있다?
	}

	FT4SpawnObjectAction NewAction;
	NewAction.ObjectID = InPacket->EnterObjectID;
	NewAction.Name = TEXT("T4FOObject");
	NewAction.EntityType = ET4EntityType::Prop;
	NewAction.EntityAssetPath = FOData->RawData.EntityAsset.ToSoftObjectPath();
	NewAction.GameDataIDName = InPacket->FODataID.RowName;
	NewAction.SpawnLocation = InPacket->SpawnLocation;
	NewAction.SpawnRotation = InPacket->SpawnRotation;
	GameWorld->DoExecuteAction(&NewAction);

#if (WITH_EDITOR || WITH_SERVER_CODE)
	EnteredObject = GameWorld->GetContainer()->FindGameObject(InPacket->EnterObjectID);
	if (nullptr != EnteredObject)
	{
		IT4PlayerController* MyPC = GetPlayerController();
		if (T4EngineLayer::IsServer(LayerType) || (nullptr != MyPC && MyPC->CheckAuthority())) // #15 : Only HasAuthority
		{
			// #41
			IT4GameAIController* GameAIController = GameFrame->FindGameAIController(InPacket->NetID);
			if (nullptr != GameAIController)
			{
				AT4GameplayFOAIController* FOController = Cast<AT4GameplayFOAIController>(
					GameAIController->GetAIController()
				);
				check(nullptr != FOController);
				FOController->SetGameObject(InPacket->EnterObjectID);
			}
		}
	}
#endif
}

void FT4PacketHandlerSC::HandleSC_FOLeave(const FT4PacketFOLeaveSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::FOLeave == InPacket->PacketSC);
	IT4GameWorld* GameWorld = T4EngineWorldGet(LayerType);
	check(nullptr != GameWorld);
	IT4GameObject* EnteredObject = GameWorld->GetContainer()->FindGameObject(InPacket->LeaveObjectID);
	if (nullptr == EnteredObject)
	{
		check(false); // WARN: 없다? 지금은 에러!
	}
	FT4DespawnObjectAction NewAction;
	NewAction.ObjectID = InPacket->LeaveObjectID;
	GameWorld->DoExecuteAction(&NewAction);
}

// #41
void FT4PacketHandlerSC::HandleSC_ItemEnter(const FT4PacketItemEnterSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::ItemEnter == InPacket->PacketSC);
	FT4GameDB& GameDB = GetGameDB();
	const FT4GameItemWeaponData* ItemData = GameDB.GetGameData<FT4GameItemWeaponData>(InPacket->ItemDataID);
	if (nullptr == ItemData)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("FT4PacketHandlerSC : failed to Item enter. ItemData '%s' Not Found."),
			*(InPacket->ItemDataID.ToString())
		);
		return;
	}

	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);

	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();;
	check(nullptr != GameWorld);

	IT4GameObject* EnteredObject = GameWorld->GetContainer()->FindGameObject(InPacket->EnterObjectID);
	if (nullptr != EnteredObject)
	{
		check(false); // WARN: 이미 스폰되어 있다?
	}

	FT4SpawnObjectAction NewAction;
	NewAction.ObjectID = InPacket->EnterObjectID;
	NewAction.Name = TEXT("T4ItemObject");
	NewAction.EntityType = ET4EntityType::Item;
	NewAction.EntityAssetPath = ItemData->RawData.EntityAsset.ToSoftObjectPath();
	NewAction.GameDataIDName = InPacket->ItemDataID.RowName;
	NewAction.SpawnLocation = InPacket->SpawnLocation;
	NewAction.SpawnRotation = InPacket->SpawnRotation;
	GameWorld->DoExecuteAction(&NewAction);

#if (WITH_EDITOR || WITH_SERVER_CODE)
	EnteredObject = GameWorld->GetContainer()->FindGameObject(InPacket->EnterObjectID);
	if (nullptr != EnteredObject)
	{
		IT4PlayerController* MyPC = GetPlayerController();
		if (T4EngineLayer::IsServer(LayerType) || (nullptr != MyPC && MyPC->CheckAuthority())) // #15 : Only HasAuthority
		{
			// #41
			IT4GameAIController* GameAIController = GameFrame->FindGameAIController(InPacket->NetID);
			if (nullptr != GameAIController)
			{
				AT4GameplayItemAIController* ItemController = Cast<AT4GameplayItemAIController>(
					GameAIController->GetAIController()
				);
				check(nullptr != ItemController);
				ItemController->SetGameObject(InPacket->EnterObjectID);
			}
		}
	}
#endif
}

// #41
void FT4PacketHandlerSC::HandleSC_ItemLeave(const FT4PacketItemLeaveSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::ItemLeave == InPacket->PacketSC);
	IT4GameWorld* GameWorld = T4EngineWorldGet(LayerType);
	check(nullptr != GameWorld);
	IT4GameObject* EnteredObject = GameWorld->GetContainer()->FindGameObject(InPacket->LeaveObjectID);
	if (nullptr == EnteredObject)
	{
		check(false); // WARN: 없다? 지금은 에러!
	}
	FT4DespawnObjectAction NewAction;
	NewAction.ObjectID = InPacket->LeaveObjectID;
	GameWorld->DoExecuteAction(&NewAction);
}
