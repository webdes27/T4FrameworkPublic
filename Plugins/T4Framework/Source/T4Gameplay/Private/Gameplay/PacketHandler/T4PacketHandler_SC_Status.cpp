// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4PacketHandler_SC.h"
#include "T4GameplayDefinitions.h"

#include "Public/Protocol/T4PacketSC_Status.h"
#include "GameDB/T4GameDB.h"

#include "Classes/Controller/Player/T4GameplayPlayerController.h" // #42

#include "T4Engine/Public/Action/T4ActionCodeMinimal.h"
#include "T4Engine/Public/Action/T4ActionParameters.h" // #76
#include "T4Engine/Public/T4Engine.h"

#include "T4GameplayInternal.h"

/**
  *
 */
// #27
// #T4_ADD_PACKET_TAG_SC
void FT4PacketHandlerSC::HandleSC_Stance(const FT4PacketStanceSC* InPacket) // #73
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::Stance == InPacket->PacketSC);
	IT4GameObject* TargetObject = GetGameObjectForClient(InPacket->ObjectID);
	if (nullptr == TargetObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_Stance '%' failed. TargetObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}
	FT4ChangeStanceAction NewAction;
	NewAction.StanceName = InPacket->StanceName;
	TargetObject->DoExecuteAction(&NewAction);
}

void FT4PacketHandlerSC::HandleSC_Equip(const FT4PacketEquipSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::Equip == InPacket->PacketSC);
	FT4GameDB& GameDB = GetGameDB();
	const FT4GameItemWeaponData* ItemData = GameDB.GetGameData<FT4GameItemWeaponData>(InPacket->ItemWeaponDataID);
	if (nullptr == ItemData)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_Equip '%' failed. ItemWeaponData '%s' not found."),
			uint32(LayerType),
			*(InPacket->ToString()),
			*(InPacket->ItemWeaponDataID.ToString())
		);
		return;
	}
	IT4GameObject* TargetObject = GetGameObjectForClient(InPacket->ObjectID);
	if (nullptr == TargetObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_Equip '%' failed. TargetObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}
	if (TargetObject->IsPlayer() && InPacket->bMainWeapon) // #48
	{
		AT4GameplayPlayerController* PlayerController = Cast<AT4GameplayPlayerController>(
			GetPlayerController()->GetAController()
		);
		check(nullptr != PlayerController);
		PlayerController->SetMainWeaponDataID(InPacket->ItemWeaponDataID);
	}
	FT4EquipWeaponAction NewAction;
	NewAction.ActionKey = InPacket->ItemWeaponDataID.ToPrimaryActionKey(); // #48
	NewAction.WeaponEntityAsset = ItemData->RawData.EntityAsset.ToSoftObjectPath();
	NewAction.EquipPoint = ItemData->RawData.EquipBoneOrSocketName; // TEXT("hand_r"); // #57 : BoneOrSocketName
	NewAction.LifecycleType = ET4LifecycleType::Looping;
	TargetObject->DoExecuteAction(&NewAction);
}

void FT4PacketHandlerSC::HandleSC_UnEquip(const FT4PacketUnEquipSC* InPacket)
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::UnEquip == InPacket->PacketSC);
	FT4GameDB& GameDB = GetGameDB();
	const FT4GameItemWeaponData* ItemData = GameDB.GetGameData<FT4GameItemWeaponData>(InPacket->ItemWeaponDataID);
	if (nullptr == ItemData)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_UnEquip '%' failed. ItemWeaponData '%s' not found."),
			uint32(LayerType),
			*(InPacket->ToString()),
			*(InPacket->ItemWeaponDataID.ToString())
		);
		return;
	}
	IT4GameObject* TargetObject = GetGameObjectForClient(InPacket->ObjectID);
	if (nullptr == TargetObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_UnEquip '%' failed. TargetObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}
	if (TargetObject->IsPlayer() && InPacket->bMainWeapon) // #48
	{
		AT4GameplayPlayerController* PlayerController = Cast<AT4GameplayPlayerController>(
			GetPlayerController()->GetAController()
		);
		PlayerController->SetMainWeaponDataID(InvalidGameDataID);
	}
	FT4UnEquipWeaponAction NewAction;
	NewAction.ActionKey = InPacket->ItemWeaponDataID.ToPrimaryActionKey(); // #48
	NewAction.StartTimeSec = 1.0f;
	TargetObject->DoExecuteAction(&NewAction);
}

void FT4PacketHandlerSC::HandleSC_Exchange(const FT4PacketExchangeSC* InPacket) // #72
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::Exchange == InPacket->PacketSC);
	FT4GameDB& GameDB = GetGameDB();
	const FT4GameItemCostumeData* ItemData = GameDB.GetGameData<FT4GameItemCostumeData>(InPacket->ItemCostumeDataID);
	if (nullptr == ItemData)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_Exchange '%' failed. ItemData '%s' not found."),
			uint32(LayerType),
			*(InPacket->ToString()),
			*(InPacket->ItemCostumeDataID.ToString())
		);
		return;
	}
	IT4GameObject* TargetObject = GetGameObjectForClient(InPacket->ObjectID);
	if (nullptr == TargetObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_Exchange '%' failed. TargetObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}
	FT4ExchangeCostumeAction NewAction;
	NewAction.ActionKey = InPacket->ItemCostumeDataID.ToPrimaryActionKey();// #48
	NewAction.CostumeEntityAsset = ItemData->RawData.EntityAsset.ToSoftObjectPath();
	NewAction.TargetPartsName = ItemData->RawData.ExchangePartName;
	NewAction.LifecycleType = ET4LifecycleType::Default;
	TargetObject->DoExecuteAction(&NewAction);
}

void FT4PacketHandlerSC::HandleSC_Die(const FT4PacketDieSC* InPacket) // #76
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::Die == InPacket->PacketSC);
	IT4GameObject* TargetObject = GetGameObjectForClient(InPacket->ObjectID);
	if (nullptr == TargetObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Warning,
			TEXT("[SL:%u] HandleSC_Die '%' failed. TargetObject not found."),
			uint32(LayerType),
			*(InPacket->ToString())
		);
		return;
	}

	FVector ShotDirection = FVector::UpVector;
	IT4GameObject* AttackerObject = GetGameObjectForClient(InPacket->AttackerObjectID);
	if (nullptr != AttackerObject)
	{
		ShotDirection = TargetObject->GetNavPoint() - AttackerObject->GetNavPoint();
		ShotDirection.Normalize();

		FT4ActionParameters NewActionParameters;
		NewActionParameters.SetTargetDirection(-ShotDirection);

		FT4TurnAction NewAction;
		NewAction.ActionKey = T4ActionTurnPKey;
		NewAction.TurnType = ET4TargetType::TargetDirection;
		TargetObject->DoExecuteAction(&NewAction, &NewActionParameters);
	}

	FT4DieAction NewAction;
	NewAction.ReactionName = InPacket->ReactionName;
	NewAction.ShotDirection = ShotDirection;
	TargetObject->DoExecuteAction(&NewAction);
}

void FT4PacketHandlerSC::HandleSC_Resurrect(const FT4PacketResurrectSC* InPacket) // #76
{
	check(nullptr != InPacket);
	check(ET4PacketStoC::Resurrect == InPacket->PacketSC);
}