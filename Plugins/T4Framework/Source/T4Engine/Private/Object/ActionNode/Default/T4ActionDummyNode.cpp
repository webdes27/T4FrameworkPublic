// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionDummyNode.h"

#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Object/T4GameObject.h"

#include "T4EngineInternal.h"

/**
  * #56 : Conti Editor 에서 Invisible or Isolate 로 출력을 제어할 때 더미용으로 사용 (delay, duration 동작 보장)
 */
FT4ActionDummyNode::FT4ActionDummyNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionNodeBase(InControl, InKey)
	, SourceAction(ET4ActionType::Dummy)
{
}

FT4ActionDummyNode::~FT4ActionDummyNode()
{
}

FT4ActionDummyNode* FT4ActionDummyNode::CreateNode(
	FT4ActionNodeControl* InControl,
	const FT4ActionCommand& InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	FT4ActionDummyNode* NewNode = new FT4ActionDummyNode(
		InControl,
		(ET4ActionCommandType::Code == InAction.GetActionStructType()) ? // #62
			(static_cast<const FT4ActionCodeCommand*>(&InAction))->ActionKey : FT4ActionKey::EmptyActionKey
	);
	check(nullptr != NewNode);
	// switch
	return NewNode;
}

bool FT4ActionDummyNode::Create(const FT4ActionCommand* InAction)
{
	SourceAction = InAction->ActionType;
	return true;
}

void FT4ActionDummyNode::Destroy()
{
}

void FT4ActionDummyNode::Advance(const FT4UpdateTime& InUpdateTime)
{
}

bool FT4ActionDummyNode::Play()
{
	bool bResult = PlayInternal(GetOffsetTimeSec()); // #56
	return bResult;
}

void FT4ActionDummyNode::Stop()
{
}

bool FT4ActionDummyNode::IsAutoFinished() const
{
	check(IsPlayed());
	check(ET4LifecycleType::Auto == LifecycleType);
	// #54 : Conti 에서 설정된 MaxPlayTime 값으로 종료를 체크한다. 만약 Conti 를 사용하지 않았다면 무조건 true 다.
	return CheckAbsoluteMaxPlayTime();
}
