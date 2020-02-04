// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionPlayTagNode.h"

#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "T4EngineInternal.h"

/**
  * #81
 */
FT4ActionPlayTagNode::FT4ActionPlayTagNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionNodeBase(InControl, InKey)
	, PlayTagName(NAME_None)
	, PlayTagType(ET4PlayTagType::All)
{
}

FT4ActionPlayTagNode::~FT4ActionPlayTagNode()
{
}

FT4ActionPlayTagNode* FT4ActionPlayTagNode::CreateNode(
	FT4ActionNodeControl* InControl,
	const FT4PlayTagAction& InAction,
	const FT4ActionParameters* InParameters
)
{
	check(ET4ActionType::PlayTag == InAction.ActionType);
	const FT4ActionKey ActionKey = FindActionKeyInParameter(InParameters); // #100 : Conti Action 류는 Param 에서 Key 를 받거나 empty 로 사용한다.
	FT4ActionPlayTagNode* NewNode = new FT4ActionPlayTagNode(InControl, ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionPlayTagNode::Create(const FT4ActionCommand* InAction)
{
	check(ET4ActionType::PlayTag == InAction->ActionType);
	const FT4PlayTagAction& ConvAction = *(static_cast<const FT4PlayTagAction*>(InAction));

	PlayTagName = ConvAction.PlayTagName;
	PlayTagType = ConvAction.PlayTagType;

	PlayTagActionKey = FString::Printf(
		TEXT("PlayTag_%u_%s_u"),
		ConvAction.HeaderKey,
		*(PlayTagName.ToString()),
		uint8(PlayTagType)
	);

#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionPlayTagNode::Destroy()
{
}

void FT4ActionPlayTagNode::Advance(const FT4UpdateTime& InUpdateTime)
{
}

bool FT4ActionPlayTagNode::Play()
{
	bool bResult = PlayInternal(GetOffsetTimeSec()); // #56
	return bResult;
}

void FT4ActionPlayTagNode::Stop()
{
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);
	OwnerObject->InactivePlayTag(PlayTagType, PlayTagActionKey);
}

bool FT4ActionPlayTagNode::PlayInternal(float InOffsetTimeSec)
{
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);
	OwnerObject->ActivePlayTag(PlayTagName, PlayTagType, PlayTagActionKey);
	return true;
}
