// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionNodeUtility.h"

#include "Object/Action/T4ActionControl.h"
#include "Object/T4GameObject.h"

#include "Public/T4EngineUtility.h"

#include "T4EngineInternal.h"

/**
  * #58, #102
 */
FT4ActionEasingCurveBlender::FT4ActionEasingCurveBlender()
	: BlendInCurve(ET4BuiltInEasing::Linear)
	, BlendInTimeSec(0.0f)
	, BlendOutCurve(ET4BuiltInEasing::Linear)
	, BlendOutTimeSec(0.0f)
	, BlendInOffsetTimeSec(0.0f) // #58 : 시작 시점이 필요에 의해 늦어질 경우 (ex. CameraWork)
	, BlendOutOffsetTimeSec(0.0f) // #58 : 끝 시점이 필요에 의해 늦어질 경우 (ex. CameraWork) 
{
}

FT4ActionEasingCurveBlender::~FT4ActionEasingCurveBlender()
{
}

void FT4ActionEasingCurveBlender::Initialize(
	ET4BuiltInEasing InBlendInCurve,
	float InBlendInTimeSec,
	ET4BuiltInEasing InBlendOutCurve,
	float InBlendOutTimeSec
)
{
	BlendInCurve = InBlendInCurve;
	BlendInTimeSec = InBlendInTimeSec;
	BlendOutCurve = InBlendOutCurve;
	BlendOutTimeSec = InBlendOutTimeSec;
}

void FT4ActionEasingCurveBlender::SetOffsetTimeSec(
	float InBlendInOffsetTimeSec,
	float InBlendOutOffsetTimeSec
)
{
	BlendInOffsetTimeSec = InBlendInOffsetTimeSec;
	BlendOutOffsetTimeSec = InBlendOutOffsetTimeSec;
}

float FT4ActionEasingCurveBlender::GetBlendWeight(float InPlayingTimeSec, float InPlayTimeLeft) const // #100, #58, #102 : 0 ~ 1
{
	ET4BlendType BlendType = ET4BlendType::BT_Default;
	float CurrentWeight = GetCurrentWeight(InPlayingTimeSec, InPlayTimeLeft, BlendType);
	if (ET4BlendType::BT_Default == BlendType)
	{
		return CurrentWeight;
	}
	ET4BuiltInEasing EasingType = (ET4BlendType::BT_In == BlendType) ? BlendInCurve : BlendOutCurve;
	if (ET4BuiltInEasing::Linear == EasingType)
	{
		return CurrentWeight;
	}
	float EvaluateWeight = T4EngineUtility::Evaluate(
		EasingType,
		CurrentWeight
	);
	return EvaluateWeight;
}

float FT4ActionEasingCurveBlender::GetCurrentWeight(
	float InPlayingTimeSec,
	float InPlayTimeLeft,
	ET4BlendType& OutBlendType
) const // #100, #58 : 0 ~ 1
{
	OutBlendType = ET4BlendType::BT_Default;
	float CurrPlayingTime = InPlayingTimeSec;
	if (CurrPlayingTime < BlendInOffsetTimeSec)
	{
		return 0.0f; // #58 : 시작 시점이 필요에 의해 늦어질 경우 (ex. CameraWork)
	}
	CurrPlayingTime = FMath::Max(0.0f, CurrPlayingTime - BlendInOffsetTimeSec);
	if (CurrPlayingTime < BlendInTimeSec)
	{
		if (0.0f >= BlendInTimeSec)
		{
			return 1.0f;
		}
		OutBlendType = ET4BlendType::BT_In;
		return FMath::Clamp(CurrPlayingTime / BlendInTimeSec, 0.0f, 1.0f);
	}
	if (0.0f >= BlendOutTimeSec)
	{
		return 1.0f;
	}
	float CurrPlayingTimeLeft = InPlayTimeLeft;
	CurrPlayingTimeLeft = FMath::Max(0.0f, CurrPlayingTimeLeft - BlendOutOffsetTimeSec);
	if (CurrPlayingTimeLeft < BlendOutTimeSec)
	{
		OutBlendType = ET4BlendType::BT_Out;
		return FMath::Clamp(CurrPlayingTimeLeft / BlendOutTimeSec, 0.0f, 1.0f);
	}
	return 1.0f;
}

namespace T4ActionNodeUtility
{
	bool CheckPlayTarget(
		ET4PlayTarget InPlayTarget,
		FT4ActionControl* InControl,
		const FT4ActionStruct* InAction,
		const FT4ActionParameters* InParameters // #54
	) // #101 : PlayTarget 옵션에 따라 스스로 처리할지 Player 에게 전달할지를 결정!
	{
		AT4GameObject* OwnerGameObject = InControl->GetGameObject();
		if (nullptr == OwnerGameObject)
		{
			return false;
		}
		const bool bPlayer = OwnerGameObject->IsPlayer();
		if (bPlayer) // 내가 플레이어면 어떤 조건이든 플레이...
		{
			return true;
		}
		else if (ET4PlayTarget::Player == InPlayTarget ||
				 ET4PlayTarget::Default == InPlayTarget) // #100
		{
			if (!bPlayer) // #100 : 플레이어가 아니면 플레이하지 않는다.
			{
				return false;
			}
		}
		else
		{
			// #100 : All 이면 Player 에게 Action 을 던진다.
			//        참고로 모든 유저에게 던지는 것이기 때문에, Player 에게 액션을 보내는 것...(Player 화면!)
			IT4GameWorld* GameWorld = InControl->GetGameWorld();
			if (nullptr == GameWorld || !GameWorld->HasPlayerObject())
			{
				return false;
			}
			IT4GameObject* PlayerObject = GameWorld->GetPlayerObject();
			if (nullptr == PlayerObject)
			{
				return false;
			}
			PlayerObject->DoExecuteAction(InAction, InParameters); // 플레이어에게 던진다.
			return false;
		}
		return true;
	}
}