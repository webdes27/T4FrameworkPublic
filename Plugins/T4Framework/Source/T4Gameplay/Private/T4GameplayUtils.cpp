// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameplayUtils.h"

#include "Gameplay/T4GameplayInstance.h"
#include "Public/Protocol/T4PacketCSMinimal.h"
#include "GameDB/T4GameDB.h"

#include "T4Engine/Public/T4Engine.h"
#include "T4Frame/Public/T4Frame.h"

#include "T4GameplayInternal.h"

/**
  *
 */
namespace T4GameplayUtil
{
	IT4PlayerController* GetPlayerController(ET4LayerType InLayerType)
	{
		IT4GameFrame* GameFrame = T4FrameGet(InLayerType);
		check(nullptr != GameFrame);
		IT4PlayerController* PlayerController = GameFrame->GetPlayerController();
		check(nullptr != PlayerController);
		return PlayerController;
	}

	IT4PacketHandlerCS* GetPacketHandler(ET4LayerType InLayerType)
	{
		check(ET4LayerType::Max > InLayerType);
		IT4GameFrame* GameFrame = T4FrameGet(InLayerType);
		check(nullptr != GameFrame);
		FT4GameplayInstance* GameplayInstance = FT4GameplayInstance::CastFrom(
			GameFrame->GetGameplayInstance()
		);
		if (nullptr == GameplayInstance)
		{
			return nullptr;
		}
		return GameplayInstance->GetPacketHandlerCS();
	}

	bool DoPlayerSpawn(ET4LayerType InLayerType, const FT4GameDataID& InPlayerDataID)
	{
		// #43	
		FT4GameDB& GameDB = GetGameDB();
		const FT4GamePlayerData* PlayerData = GameDB.GetGameData<FT4GamePlayerData>(InPlayerDataID);
		if (nullptr == PlayerData)
		{
			UE_LOG(
				LogT4Gameplay,
				Warning,
				TEXT("T4GameplayUtil : failed to player spawn. PlayerDataID '%s' Not Found."),
				*(InPlayerDataID.ToString())
			);
			return false;
		}
		IT4GameFrame* GameFrame = T4FrameGet(InLayerType);
		if (nullptr == GameFrame)
		{
			return false;
		}
		FVector PickingLocation;
		if (!GameFrame->GetMousePickingLocation(PickingLocation))
		{
			UE_LOG(
				LogT4Gameplay,
				Warning,
				TEXT("T4GameplayUtil : failed to player spawn. Invalid PickingLocation.")
			);
			return false;
		}
		IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandler(InLayerType);
		if (nullptr == PacketHandlerCS)
		{
			return false;
		}
		IT4PlayerController* PlayerController = GetPlayerController(InLayerType);
		check(nullptr != PlayerController);
		FT4PacketCmdPCEnterCS NewPacketCS; // #27
		NewPacketCS.PlayerDataID = InPlayerDataID;
		NewPacketCS.SpawnLocation = PickingLocation;
		NewPacketCS.bSetViewTarget = !PlayerController->HasGameObject(); // WARN : 빙의된 캐릭터가 없으면 MyPC로 간주하도록 조치
		PacketHandlerCS->DoSendPacket(&NewPacketCS);
		return true;
	}

	bool DoNPCSpawn(ET4LayerType InLayerType, const FT4GameDataID& InNPCDataID)
	{
		IT4GameFrame* GameFrame = T4FrameGet(InLayerType);
		if (nullptr == GameFrame)
		{
			return false;
		}
		FVector PickingLocation;
		if (!GameFrame->GetMousePickingLocation(PickingLocation))
		{
			UE_LOG(
				LogT4Gameplay,
				Warning,
				TEXT("T4GameplayUtil : failed to npc spawn. Invalid PickingLocation.")
			);
			return false;
		}
		return DoNPCSpawn(InLayerType, InNPCDataID, PickingLocation);
	}

	bool DoNPCSpawn(
		ET4LayerType InLayerType,
		const FT4GameDataID& InNPCDataID,
		const FVector& InSpawnLocation
	) // #60
	{
		// #43	
		FT4GameDB& GameDB = GetGameDB();
		const FT4GameNPCData* NPCData = GameDB.GetGameData<FT4GameNPCData>(InNPCDataID);
		if (nullptr == NPCData)
		{
			UE_LOG(
				LogT4Gameplay,
				Warning,
				TEXT("T4GameplayUtil : failed to npc spawn. NPCDataID '%s' Not Found."),
				*(InNPCDataID.ToString())
			);
			return false;
		}
		IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandler(InLayerType);
		if (nullptr == PacketHandlerCS)
		{
			return false;
		}
		IT4PlayerController* PlayerController = GetPlayerController(InLayerType);
		check(nullptr != PlayerController);
		FT4PacketCmdNPCEnterCS NewPacketCS; // #31
		NewPacketCS.NPCDataID = InNPCDataID;
		NewPacketCS.SpawnLocation = InSpawnLocation;
		PacketHandlerCS->DoSendPacket(&NewPacketCS);
		return true;
	}

	bool DoDespawnAll(
		ET4LayerType InLayerType,
		bool bClearPlayerObject
	) // #68
	{
		IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandler(InLayerType);
		if (nullptr == PacketHandlerCS)
		{
			return false;
		}
		IT4PlayerController* PlayerController = GetPlayerController(InLayerType);
		check(nullptr != PlayerController);
		IT4GameFrame* GameFrame = T4FrameGet(InLayerType);
		if (nullptr == GameFrame)
		{
			return false;
		}
		TArray<IT4GameObject*> GameObjects;
		IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
		check(nullptr != GameWorld);
		if (!GameWorld->GetContainer()->GetGameObjects(ET4SpawnMode::All, GameObjects))
		{
			return false;
		}
		for (IT4GameObject* GameObject : GameObjects)
		{
			check(nullptr != GameObject);
			if (!bClearPlayerObject && GameObject->IsPlayer())
			{
				continue;
			}
			FT4PacketCmdLeaveCS NewPacketCS;
			NewPacketCS.LeaveObjectID = GameObject->GetObjectID();
			PacketHandlerCS->DoSendPacket(&NewPacketCS);
		}
		return true;
	}

	bool DoEquipWeapon(
		ET4LayerType InLayerType,
		const FT4GameDataID& InWeaponDataID,
		bool bUnEquip,
		bool bInMainWeapon
	) // #48
	{
		FT4GameDB& GameDB = GetGameDB();
		const FT4GameItemWeaponData* WeaponData = GameDB.GetGameData<FT4GameItemWeaponData>(InWeaponDataID);
		if (nullptr == WeaponData)
		{
			UE_LOG(
				LogT4Gameplay,
				Warning,
				TEXT("T4GameplayUtil : failed to Equip or UnEquip Item. ItemWeaponDataID '%s' Not Found."),
				*(InWeaponDataID.ToString())
			);
			return false;
		}
		IT4PlayerController* PlayerController = GetPlayerController(InLayerType);
		if (nullptr == PlayerController)
		{
			UE_LOG(
				LogT4Gameplay,
				Warning,
				TEXT("T4GameplayUtil : failed to Equip or UnEquip Item. PlayerObject is Not set."),
				*(InWeaponDataID.ToString())
			);
			return false;
		}
		IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandler(InLayerType);
		if (nullptr == PacketHandlerCS)
		{
			return false;
		}
		const FT4ObjectID PlayerObjectID = PlayerController->GetGameObjectID();
		if (bUnEquip)
		{
			FT4PacketUnEquipCS NewPacketCS; // #27
			NewPacketCS.SenderID = PlayerObjectID;
			NewPacketCS.ItemWeaponDataID = InWeaponDataID;
			NewPacketCS.bMainWeapon = bInMainWeapon; // #48
			PacketHandlerCS->DoSendPacket(&NewPacketCS);
		}
		else
		{
			FT4PacketEquipCS NewPacketCS; // #27
			NewPacketCS.SenderID = PlayerObjectID;
			NewPacketCS.ItemWeaponDataID = InWeaponDataID;
			NewPacketCS.bMainWeapon = bInMainWeapon; // #48
			PacketHandlerCS->DoSendPacket(&NewPacketCS);
		}
		return true;
	}

	FT4ServerEventManager* GetServerEventManager(const ET4LayerType InLayerType) // #63
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
		return GameplayInstance->GetServerEventManager();
	}
}
