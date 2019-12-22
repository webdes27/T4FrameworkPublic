// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionTimeScaleNode.h"

#include "Object/Action/T4ActionControl.h"
#include "Object/T4GameObject.h"

#include "T4EngineInternal.h"

/**
  * #102
 */
FT4ActionTimeScaleNode::FT4ActionTimeScaleNode(FT4ActionControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionNode(InControl, InKey)
	, TimeScale(1.0f)
{
}

FT4ActionTimeScaleNode::~FT4ActionTimeScaleNode()
{
}

FT4ActionTimeScaleNode* FT4ActionTimeScaleNode::CreateNode(
	FT4ActionControl* InControl,
	const FT4TimeScaleAction& InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	check(ET4ActionType::TimeScale == InAction.ActionType);
	if (!T4ActionNodeUtility::CheckPlayTarget(InAction.PlayTarget, InControl, &InAction, InParameters))
	{
		return nullptr; // #101 : PlayTarget 옵션에 따라 스스로 처리할지 Player 에게 전달할지를 결정!
	}
	const FT4ActionKey ActionKey = FindActionKeyInParameter(InParameters); // #100 : Conti Action 류는 Param 에서 Key 를 받거나 empty 로 사용한다.
	FT4ActionTimeScaleNode* NewNode = new FT4ActionTimeScaleNode(InControl, ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionTimeScaleNode::Create(const FT4ActionStruct* InAction)
{
	check(ET4ActionType::TimeScale == InAction->ActionType);
	const FT4TimeScaleAction& ConvAction = *(static_cast<const FT4TimeScaleAction*>(InAction));

	TimeScale = ConvAction.TimeScale;

	EasingCurveBlender.Initialize(
		ConvAction.BlendInCurve,
		ConvAction.BlendInTimeSec,
		ConvAction.BlendOutCurve,
		ConvAction.BlendOutTimeSec
	);

#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionTimeScaleNode::Destroy()
{
}

void FT4ActionTimeScaleNode::Destroying()
{
	// #102 : 삭제 대기로 들어갈 경우 한프레임 늦게 삭제되기 때문에
	//		  TimeScale 원복이 되지 않는 문제가 있어 Destroying 시점에 Stop 처리함
	Stop();
}

void FT4ActionTimeScaleNode::Advance(const FT4UpdateTime& InUpdateTime)
{
	if (!IsPlayed())
	{
		return;
	}
	AT4GameObject* GameObject = GetGameObject();
	if (nullptr == GameObject)
	{
		return;
	}
	const float BlendWeight = EasingCurveBlender.GetBlendWeight(GetPlayingTime(), GetPlayTimeLeft());
	const float TimeScaleValue = FMath::Lerp(1.0f, TimeScale, BlendWeight);
	GameObject->SetTimeScale(TimeScaleValue);
#if 0
	UE_LOG(
		LogT4Engine, 
		Display, 
		TEXT("PlayTime = %.2f sec (Local = %.2f), TimeScaleValue = %.2f, BlendWeight = %.2f"), 
		GetElapsedTimeSec(),
		GetPlayingTime(), 
		TimeScaleValue, 
		BlendWeight
	);
#endif
}

bool FT4ActionTimeScaleNode::Play()
{
	bool bResult = PlayInternal(GetOffsetTimeSec()); // #56
	return bResult;
}

void FT4ActionTimeScaleNode::Stop()
{
	AT4GameObject* GameObject = GetGameObject();
	if (nullptr != GameObject)
	{
		GameObject->SetTimeScale(1.0f);
	}
}

bool FT4ActionTimeScaleNode::IsAutoFinished() const
{
	check(IsPlayed());
	check(ET4LifecycleType::Auto == LifecycleType);
	// #54 : Conti 에서 설정된 MaxPlayTime 값으로 종료를 체크한다. 만약 Conti 를 사용하지 않았다면 무조건 true 다.
	return CheckAbsoluteMaxPlayTime();
}

bool FT4ActionTimeScaleNode::PlayInternal(float InOffsetTimeSec)
{ 
	float CurrentDurationSec = FMath::Max(0.0f, DurationSec - EasingCurveBlender.GetBlendInTimeSec());
	if (0.0f >= CurrentDurationSec)
	{
		EasingCurveBlender.SetBlendInTimeSec(DurationSec);
		EasingCurveBlender.SetBlendOutTimeSec(0.0f);
	}
	else
	{
		if (CurrentDurationSec < EasingCurveBlender.GetBlendOutTimeSec())
		{
			EasingCurveBlender.SetBlendOutTimeSec(CurrentDurationSec);
		}
	}
	return true; 
} // #56