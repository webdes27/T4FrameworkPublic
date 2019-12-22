// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4PacketHandler_CS.h"

#include "Public/Protocol/T4PacketCS_Status.h"
#include "Public/Protocol/T4PacketSC_Status.h"

#include "GameDB/T4GameDB.h"

#include "T4Engine/Public/T4Engine.h"

#include "T4GameplayInternal.h"

/**
  *
 */
// #27
// #T4_ADD_PACKET_TAG_CS

void FT4PacketHandlerCS::HandleCS_Stance(const FT4PacketStanceCS* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketCtoS::Stance == InPacket->PacketCS);

	IT4GameObject* SenderObject = GetGameObjectForServer(InPacket->SenderID);
	if (nullptr == SenderObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleCS_Stance '%' failed. SenderObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	FT4PacketStanceSC NewPacketSC;
	NewPacketSC.ObjectID = InPacket->SenderID;
	NewPacketSC.StanceName = InPacket->StanceName;
	GetPacketHandlerSC()->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
}

void FT4PacketHandlerCS::HandleCS_Equip(const FT4PacketEquipCS* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketCtoS::Equip == InPacket->PacketCS);

	IT4GameObject* SenderObject = GetGameObjectForServer(InPacket->SenderID);
	if (nullptr == SenderObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleCS_Equip '%' failed. SenderObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	FT4GameDB& GameDB = GetGameDB();
	if (!GameDB.HasGameData(InPacket->ItemWeaponDataID))
	{
		return;
	}

	FT4PacketEquipSC NewPacketSC;
	NewPacketSC.ObjectID = InPacket->SenderID;
	NewPacketSC.ItemWeaponDataID = InPacket->ItemWeaponDataID;
	NewPacketSC.bMainWeapon = InPacket->bMainWeapon; // #48
	GetPacketHandlerSC()->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
}

void FT4PacketHandlerCS::HandleCS_UnEquip(const FT4PacketUnEquipCS* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketCtoS::UnEquip == InPacket->PacketCS);

	IT4GameObject* SenderObject = GetGameObjectForServer(InPacket->SenderID);
	if (nullptr == SenderObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleCS_UnEquip '%' failed. SenderObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	FT4GameDB& GameDB = GetGameDB();
	if (!GameDB.HasGameData(InPacket->ItemWeaponDataID))
	{
		return;
	}

	FT4PacketUnEquipSC NewPacketSC;
	NewPacketSC.ObjectID = InPacket->SenderID;
	NewPacketSC.ItemWeaponDataID = InPacket->ItemWeaponDataID;
	NewPacketSC.bMainWeapon = InPacket->bMainWeapon; // #48
	GetPacketHandlerSC()->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
}

void FT4PacketHandlerCS::HandleCS_Exchange(const FT4PacketExchangeCS* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketCtoS::Exchange == InPacket->PacketCS);

	IT4GameObject* SenderObject = GetGameObjectForServer(InPacket->SenderID);
	if (nullptr == SenderObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] HandleCS_Exchange '%' failed. SenderObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	FT4GameDB& GameDB = GetGameDB();
	if (!GameDB.HasGameData(InPacket->ItemCostumeDataID))
	{
		return;
	}

	FT4PacketExchangeSC NewPacketSC;
	NewPacketSC.ObjectID = InPacket->SenderID;
	NewPacketSC.ItemCostumeDataID = InPacket->ItemCostumeDataID;
	GetPacketHandlerSC()->DoBroadcastPacketForServer(&NewPacketSC, true); // #15, #17, #29
}