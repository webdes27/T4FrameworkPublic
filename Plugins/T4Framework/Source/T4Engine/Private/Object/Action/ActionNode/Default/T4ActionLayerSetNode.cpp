// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionLayerSetNode.h"

#include "Object/Action/T4ActionControl.h"
#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "T4EngineInternal.h"

/**
  * #81
 */
FT4ActionLayerSetNode::FT4ActionLayerSetNode(FT4ActionControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionNode(InControl, InKey)
	, LayerTagName(NAME_None)
	, LayerTagType(ET4LayerTagType::All)
{
}

FT4ActionLayerSetNode::~FT4ActionLayerSetNode()
{
}

FT4ActionLayerSetNode* FT4ActionLayerSetNode::CreateNode(
	FT4ActionControl* InControl,
	const FT4LayerSetAction& InAction,
	const FT4ActionParameters* InParameters
)
{
	check(ET4ActionType::LayerSet == InAction.ActionType);
	const FT4ActionKey ActionKey = FindActionKeyInParameter(InParameters); // #100 : Conti Action 류는 Param 에서 Key 를 받거나 empty 로 사용한다.
	FT4ActionLayerSetNode* NewNode = new FT4ActionLayerSetNode(InControl, ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionLayerSetNode::Create(const FT4ActionStruct* InAction)
{
	check(ET4ActionType::LayerSet == InAction->ActionType);
	const FT4LayerSetAction& ConvAction = *(static_cast<const FT4LayerSetAction*>(InAction));

	LayerTagName = ConvAction.LayerTagName;
	LayerTagType = ConvAction.LayerTagType;

	LayerSetActionKey = FString::Printf(
		TEXT("LayerSet_%u_%s_u"),
		ConvAction.HeaderKey,
		*(LayerTagName.ToString()),
		uint8(LayerTagType)
	);

#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionLayerSetNode::Destroy()
{
}

void FT4ActionLayerSetNode::Advance(const FT4UpdateTime& InUpdateTime)
{
}

bool FT4ActionLayerSetNode::Play()
{
	bool bResult = PlayInternal(GetOffsetTimeSec()); // #56
	return bResult;
}

void FT4ActionLayerSetNode::Stop()
{
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);
	OwnerObject->StopLayerTag(LayerSetActionKey);
}

bool FT4ActionLayerSetNode::PlayInternal(float InOffsetTimeSec)
{
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);
	OwnerObject->PlayLayerTag(LayerTagName, LayerTagType, LayerSetActionKey);
	return true;
}
