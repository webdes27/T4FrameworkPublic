// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionUnequipWeaponNode.h"

#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Object/T4GameObject.h"

#include "T4Asset/Classes/Entity/T4WeaponEntityAsset.h"
#include "T4Asset/Public/Entity/T4Entity.h"

#include "T4EngineInternal.h"

/**
  * #48, #111
 */
FT4ActionUnequipWeaponNode::FT4ActionUnequipWeaponNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionNodeBase(InControl, InKey)
	, bChangeDefaultStance(false)
	, bChangeStanceSync(false)
{
}

FT4ActionUnequipWeaponNode::~FT4ActionUnequipWeaponNode()
{
}

FT4ActionUnequipWeaponNode* FT4ActionUnequipWeaponNode::CreateNode(
	FT4ActionNodeControl* InControl,
	const FT4UnequipWeaponAction& InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	check(ET4ActionType::UnequipWeapon == InAction.ActionType);
	FT4ActionUnequipWeaponNode* NewNode = new FT4ActionUnequipWeaponNode(InControl, InAction.ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionUnequipWeaponNode::Create(const FT4ActionCommand* InAction)
{
	check(ET4ActionType::UnequipWeapon == InAction->ActionType);
	const FT4UnequipWeaponAction& ConvAction = *(static_cast<const FT4UnequipWeaponAction*>(InAction));
	EquipmentActionKey = ConvAction.EquipmentActionKey; // #111
	bChangeDefaultStance = ConvAction.bChangeDefaultStance;
	bChangeStanceSync = ConvAction.bChangeStanceSync;
#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionUnequipWeaponNode::Destroy()
{
	if (EquipmentActionKey.IsValid())
	{
		PlayInternal(0.0f); // #111 : 장비 관련 처리는 무조건 호출되어야 한다.
	}
}

bool FT4ActionUnequipWeaponNode::Play()
{
	bool bResult = PlayInternal(GetOffsetTimeSec()); // #56
	return bResult;
}

void FT4ActionUnequipWeaponNode::Stop()
{
}

bool FT4ActionUnequipWeaponNode::PlayInternal(float InOffsetTimeSec)
{
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);
	OwnerObject->DetachEquipment(
		EquipmentActionKey,
		(bChangeDefaultStance) ? T4Const_DefaultStanceName : NAME_None,
		bChangeStanceSync // #111 : Stance 변경에 있는 AnimNotify_Equipment Unmount 에 따라 Show => Hide 처리가 됨
	);
	EquipmentActionKey.Reset();
	return true;
}