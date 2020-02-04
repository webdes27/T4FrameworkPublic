// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionPostProcessNode.h"

#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"
#include "Classes/Camera/T4CameraModifier.h" // #100

#include "Camera/PlayerCameraManager.h"

#include "T4EngineInternal.h"

/**
  * #100
 */
FT4ActionPostProcessNode::FT4ActionPostProcessNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionCameraNodeBase(InControl, InKey)
{
}

FT4ActionPostProcessNode::~FT4ActionPostProcessNode()
{
}

FT4ActionPostProcessNode* FT4ActionPostProcessNode::CreateNode(
	FT4ActionNodeControl* InControl,
	const FT4PostProcessAction& InAction,
	const FT4ActionParameters* InParameters
)
{
	check(ET4ActionType::PostProcess == InAction.ActionType);
	if (!T4ActionNodeUtility::CheckPlayTarget(InAction.PlayTarget, InControl, &InAction, InParameters))
	{
		return nullptr; // #101 : PlayTarget 옵션에 따라 스스로 처리할지 Player 에게 전달할지를 결정!
	}
	const FT4ActionKey ActionKey = FindActionKeyInParameter(InParameters); // #100 : Conti Action 류는 Param 에서 Key 를 받거나 empty 로 사용한다.
	FT4ActionPostProcessNode* NewNode = new FT4ActionPostProcessNode(InControl, ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionPostProcessNode::Create(const FT4ActionCommand* InAction)
{
	check(ET4ActionType::PostProcess == InAction->ActionType);
	const FT4PostProcessAction& ConvAction = *(static_cast<const FT4PostProcessAction*>(InAction));

	AT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	check(OwnerGameObject->HasPlayer()); // PP 는 플레이어만 동작한다!!

	EasingCurveBlender.Initialize(
		ConvAction.BlendInCurve,
		ConvAction.BlendInTimeSec,
		ConvAction.BlendOutCurve,
		ConvAction.BlendOutTimeSec
	);

	PostProcessSettings = ConvAction.PostProcessSettings;

#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionPostProcessNode::Destroy()
{
}

void FT4ActionPostProcessNode::Advance(const FT4UpdateTime& InUpdateTime)
{
	if (!IsPlayed())
	{
		return;
	}
	const float BlendWeight = GetBlendWeight();
	if (0.0f < BlendWeight)
	{
		UT4CameraModifier* CameraModifier = GetCameraModifier();// #100
		if (nullptr != CameraModifier)
		{
			CameraModifier->PostProcessBlendWeights.Add(BlendWeight);
			CameraModifier->PostProcessSettings.Add(&PostProcessSettings);
		}
	}
	// T4_LOG(Display, TEXT("BlendWeight = %.2f"), BlendWeight);
}

bool FT4ActionPostProcessNode::Play()
{
	bool bResult = PlayInternal(GetOffsetTimeSec()); // #56
	return bResult;
}

void FT4ActionPostProcessNode::Stop()
{
}

bool FT4ActionPostProcessNode::PlayInternal(float InOffsetTimeSec)
{
	const float BlendWeight = GetBlendWeight();
	if (0.0f >= BlendWeight)
	{
		return true;
	}
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);
	check(OwnerObject->HasPlayer());
	UT4CameraModifier* CameraModifier = GetCameraModifier();// #100
	if (nullptr == CameraModifier)
	{
		return false;
	}
	CameraModifier->PostProcessBlendWeights.Add(BlendWeight);
	CameraModifier->PostProcessSettings.Add(&PostProcessSettings);
	return true;
}
