// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionEquipWeaponNode.h"

#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Object/T4GameObject.h"

#include "T4Asset/Classes/Entity/T4WeaponEntityAsset.h"
#include "T4Asset/Public/Entity/T4Entity.h"

#include "T4EngineInternal.h"

/**
  * #107
 */
FT4ActionEquipWeaponNode::FT4ActionEquipWeaponNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionNodeBase(InControl, InKey)
	, bAttached(false)
	, bChangeStanceInEntity(false) // #110 : Weapon Entity 에 설정된 Stance 도 함께 설정해준다.
	, bChangeStanceSync(false) // #111 : 스탠스 변경과 동기화한다.
{
}

FT4ActionEquipWeaponNode::~FT4ActionEquipWeaponNode()
{
}

FT4ActionEquipWeaponNode* FT4ActionEquipWeaponNode::CreateNode(
	FT4ActionNodeControl* InControl,
	const FT4EquipWeaponAction& InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	check(ET4ActionType::EquipWeapon == InAction.ActionType);
	FT4ActionEquipWeaponNode* NewNode = new FT4ActionEquipWeaponNode(InControl, InAction.ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionEquipWeaponNode::Create(const FT4ActionCommand* InAction)
{
	check(ET4ActionType::EquipWeapon == InAction->ActionType);
	const FT4EquipWeaponAction& ConvAction = *(static_cast<const FT4EquipWeaponAction*>(InAction));
	const UT4WeaponEntityAsset* EntityAsset = T4AssetEntityManagerGet()->GetWeaponEntity(
		ConvAction.MainWeaponData.WeaponEntityAsset.ToSoftObjectPath()
	);
	if (nullptr == EntityAsset)
	{
		T4_LOG(
			Error,
			TEXT("Main Weapon Entity (%s) not found"),
			*(ConvAction.MainWeaponData.WeaponEntityAsset.ToString())
		);
		return false;
	}
	EquipmentActionKey = ConvAction.EquipmentActionKey; // #111
	MainWeaponData = ConvAction.MainWeaponData; // #111
	SubWeaponDatas = ConvAction.SubWeaponDatas; // #111
	bChangeStanceInEntity = ConvAction.bChangeStanceInEntity; // #110 : Weapon Entity 에 설정된 Stance 도 함께 설정해준다.
	bChangeStanceSync = ConvAction.bChangeStanceSync;// #111 : 스탠스 변경과 동기화한다.
#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionEquipWeaponNode::Destroy()
{
	if (!bAttached)
	{
		PlayInternal(0.0f); // #111 : 장비 관련 처리는 무조건 호출되어야 한다.
	}
}

bool FT4ActionEquipWeaponNode::Play()
{
	bool bResult = PlayInternal(GetOffsetTimeSec()); // #56
	return bResult;
}

void FT4ActionEquipWeaponNode::Stop()
{
}

bool FT4ActionEquipWeaponNode::PlayInternal(float InOffsetTimeSec)
{
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);

	FT4EntityKey WeaponEntityKey;
	WeaponEntityKey.Type = ET4EntityType::Weapon;
	WeaponEntityKey.Value = *(MainWeaponData.WeaponEntityAsset.ToSoftObjectPath().ToString());
	bool bMainResult = OwnerObject->AttachEquipment(
		EquipmentActionKey,
		true,
		WeaponEntityKey,
		MainWeaponData.OverrideEquipPoint,
		bChangeStanceInEntity,
		bChangeStanceSync // #111 : Stance 변경에 있는 AnimNotify_Equipment Mount 에 따라 Hide => Show 처리가 됨
	);
	if (!bMainResult)
	{
		T4_LOG(
			Error,
			TEXT("Main WeaponEntity '%s' Attachment Failed"),
			*(WeaponEntityKey.ToString())
		);
		return false;
	}

	for (const FT4EquipWeaponEntityData& SubWeaponData : SubWeaponDatas) // #111
	{
		if (!SubWeaponData.WeaponEntityAsset.IsNull())
		{
			WeaponEntityKey.Type = ET4EntityType::Weapon;
			WeaponEntityKey.Value = *(SubWeaponData.WeaponEntityAsset.ToSoftObjectPath().ToString());

			bool bSubResult = OwnerObject->AttachEquipment(
				EquipmentActionKey,
				false ,
				WeaponEntityKey,
				SubWeaponData.OverrideEquipPoint,
				bChangeStanceInEntity,
				bChangeStanceSync // #111 : Stance 변경에 있는 AnimNotify_Equipment Mount 에 따라 Hide => Show 처리가 됨
			);

			if (!bSubResult)
			{
				T4_LOG(
					Warning,
					TEXT("Sub WeaponEntity '%s' Attachment Failed"),
					*(WeaponEntityKey.ToString())
				);
			}
		}
	}

	bAttached = true;
	return true;
}