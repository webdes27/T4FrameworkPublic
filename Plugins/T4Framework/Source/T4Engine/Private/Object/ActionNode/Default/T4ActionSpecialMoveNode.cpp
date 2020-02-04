// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionSpecialMoveNode.h"

#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionSpecialMoveNode::FT4ActionSpecialMoveNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionNodeBase(InControl, InKey)
{
}

FT4ActionSpecialMoveNode::~FT4ActionSpecialMoveNode()
{
}

FT4ActionSpecialMoveNode* FT4ActionSpecialMoveNode::CreateNode(
	FT4ActionNodeControl* InControl,
	const FT4SpecialMoveAction& InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	check(ET4ActionType::SpecialMove == InAction.ActionType);
	const FT4ActionKey ActionKey = FindActionKeyInParameter(InParameters); // #100 : Conti Action 류는 Param 에서 Key 를 받거나 empty 로 사용한다.
	FT4ActionSpecialMoveNode* NewNode = new FT4ActionSpecialMoveNode(InControl, ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionSpecialMoveNode::Create(const FT4ActionCommand* InAction)
{
	check(ET4ActionType::SpecialMove == InAction->ActionType);
	const FT4SpecialMoveAction& ConvAction = *(static_cast<const FT4SpecialMoveAction*>(InAction));

#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionSpecialMoveNode::Destroy()
{
}

void FT4ActionSpecialMoveNode::Advance(const FT4UpdateTime& InUpdateTime)
{
}

bool FT4ActionSpecialMoveNode::Play()
{
	bool bResult = PlayInternal(GetOffsetTimeSec()); // #56
	return bResult;
}

void FT4ActionSpecialMoveNode::Stop()
{
}

bool FT4ActionSpecialMoveNode::IsAutoFinished() const
{
	check(IsPlayed());
	check(ET4LifecycleType::Auto == LifecycleType);
	// #54 : Conti 에서 설정된 MaxPlayTime 값으로 종료를 체크한다. 만약 Conti 를 사용하지 않았다면 무조건 true 다.
	return CheckAbsoluteMaxPlayTime();
}
