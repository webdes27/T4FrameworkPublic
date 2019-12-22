// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionAnimationNode.h"

#include "Object/Action/T4ActionControl.h"
#include "Object/T4GameObject.h"

#include "Public/Action/T4ActionContiStructs.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionAnimationNode::FT4ActionAnimationNode(FT4ActionControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionNode(InControl, InKey)
	, bAutoFinished(false) // #60
	, AutoDurationSec(0.0f) // #54
	, PlayAnimInstanceID(INDEX_NONE)
{
}

FT4ActionAnimationNode::~FT4ActionAnimationNode()
{
}

FT4ActionAnimationNode* FT4ActionAnimationNode::CreateNode(
	FT4ActionControl* InControl,
	const FT4AnimationAction& InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	check(ET4ActionType::Animation == InAction.ActionType);
	const FT4ActionKey ActionKey = FindActionKeyInParameter(InParameters); // #100 : Conti Action 류는 Param 에서 Key 를 받거나 empty 로 사용한다.
	FT4ActionAnimationNode* NewNode = new FT4ActionAnimationNode(InControl, ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionAnimationNode::Create(const FT4ActionStruct* InAction)
{
	check(ET4ActionType::Animation == InAction->ActionType);
	const FT4AnimationAction& ConvAction = *(static_cast<const FT4AnimationAction*>(InAction));
	AnimParameters.AnimMontageName = T4AnimSetAnimMontageSkillName;
	AnimParameters.SectionName = ConvAction.SectionName;
	AnimParameters.PlayRate = ConvAction.PlayRate;
	AnimParameters.BlendInTimeSec = ConvAction.BlendInTimeSec;
	AnimParameters.BlendOutTimeSec = ConvAction.BlendOutTimeSec;
	AnimParameters.LoopCount = ConvAction.LoopCount;
#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionAnimationNode::Destroy()
{
	ensure(INDEX_NONE == PlayAnimInstanceID);
}

void FT4ActionAnimationNode::Advance(const FT4UpdateTime& InUpdateTime)
{
	if (CheckPlayState(APS_Ready))
	{
		// #56
		AddOffsetTimeSec(InUpdateTime.ScaledTimeSec); // #54 : Case-2
		AT4GameObject* OwnerGameObject = GetGameObject();
		check(nullptr != OwnerGameObject);
		if (!OwnerGameObject->IsLoaded())
		{
			return;
		}
		PlayInternal(GetOffsetTimeSec());
	}
	if (INDEX_NONE == PlayAnimInstanceID)
	{
		return;
	}
	if (ET4LifecycleType::Auto == LifecycleType)
	{
		// #54 : Auto 는 플레이시 Duration 을 계산하고, 해당 시간이 끝나면 자동 종료하도록 처리한다.
		if (AutoDurationSec <= GetPlayingTime())
		{
			PlayAnimInstanceID = INDEX_NONE;
			bAutoFinished = true;
		}
	}
}

bool FT4ActionAnimationNode::Play()
{
	AT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	if (!OwnerGameObject->IsLoaded())
	{
		// #54 : 리소스가 로드가 안되어 Offset Time 적용이 필요할 경우,
		//       PlayState 를 APS_Ready 로 바꾼 후 로딩 완료 후 OffsetTimeSec 로 플레이 되도록 처리
		SetPlayState(APS_Ready);
		return false;
	}
	bool bResult = PlayInternal(GetOffsetTimeSec()); // #56
	return bResult;
}

void FT4ActionAnimationNode::Stop()
{
	if (INDEX_NONE == PlayAnimInstanceID)
	{
		return;
	}
	// #54 : 일반적인 애니는 루핑이 아니면 자동 종료되나, Stop 은 명시적으로 종료를 가정함으로
	//       Node 종료시 Stop 을 호출해준다. 
	// TODO : 이후 반대로 Stop 호출하지 않는 옵션을 제공해주어야 할 것이다.
	AT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	if (OwnerGameObject->IsLoaded())
	{
		IT4AnimControl* AnimControl = OwnerGameObject->GetAnimControl();
		check(nullptr != AnimControl);
		AnimControl->StopAnimation(PlayAnimInstanceID, T4AnimSetBlendTimeSec);
	}
	PlayAnimInstanceID = INDEX_NONE;
}

bool FT4ActionAnimationNode::IsAutoFinished() const
{
	check(IsPlayed());
	check(ET4LifecycleType::Auto == LifecycleType);
	return bAutoFinished;
}

bool FT4ActionAnimationNode::PlayInternal(float InOffsetTimeSec)
{
	// #56
	check(INDEX_NONE == PlayAnimInstanceID);
	if (AnimParameters.SectionName == NAME_None)
	{
		UE_LOG(
			LogT4Engine,
			Warning,
			TEXT("FT4ActionAnimationNode : AnimSequence SectionName is empty.")
		);
		OnStop();
		return false;
	}

	AT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);

	IT4AnimControl* AnimControl = OwnerGameObject->GetAnimControl();
	check(nullptr != AnimControl);

	// #56
	if (ET4LifecycleType::Duration == LifecycleType && 0.0f < DurationSec)
	{
		float AnimDurationSec = AnimControl->GetDurationSec(
			AnimParameters.AnimMontageName,
			AnimParameters.SectionName
		);
		if (0.0f < AnimDurationSec)
		{
			AnimDurationSec *= (1.0f / AnimParameters.PlayRate);
			if (AnimDurationSec != DurationSec)
			{
				// 플레이ㅣ 속도를 맞춰준다. PlayRate = 2 는 두배 빠르게 이다.
				float ApplyPlayRate = AnimDurationSec / DurationSec;
				AnimParameters.PlayRate = ApplyPlayRate;
			}
		}
	}
	else if (ET4LifecycleType::Auto == LifecycleType)
	{
		AutoDurationSec = AnimControl->GetDurationSec(
			AnimParameters.AnimMontageName,
			AnimParameters.SectionName
		);
		AutoDurationSec = FMath::Max(0.0f, AutoDurationSec - InOffsetTimeSec);
		if (0.0f >= AutoDurationSec)
		{
			// #54 : OffsetTime 이 넘어가서 더이상 플레이 할 필요가 없다. 
			//       ActionNode 는 Duration Policy 라면 노드 생성 자체를 하지 않는다.
			OnStop();
			return false;
		}
		AutoDurationSec *= (1.0f / AnimParameters.PlayRate);
	}

	AnimParameters.OffsetTimeSec = InOffsetTimeSec; // #54, #56 : TODO : 애니쪽 OffsetTime 아직 처리 하지 않음

	if (ActionParameterPtr.IsValid())
	{
		// #54 : 애니 BlendIn Time 을 없앤다. (현재는 툴용)
		if (0.0f < InOffsetTimeSec && 
			ActionParameterPtr->CheckBits(ET4AnimationParamBits::NoBlendInTimeWithOffsetPlayBit))
		{
			AnimParameters.BlendInTimeSec = 0.0f;
		}
	}

	PlayAnimInstanceID = AnimControl->PlayAnimation(AnimParameters);
	if (INDEX_NONE == PlayAnimInstanceID)
	{
		OnStop();
		return false;
	}

	SetPlayState(APS_Playing);
	return true;
}
