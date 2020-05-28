// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4EngineLayer.h"

/**
  * #113
 */
 struct T4ENGINE_API FT4FloatInterpolator
{
	FT4FloatInterpolator()
		: TranslateTimeSec(0.0f)
		, CurrentTimeSec(0.0f)
		, CurrentValue(0.0f)
		, GoalValue(0.0f)
	{
	}

	void Initialize(float InTranslateTimeSec, float InInitValue)
	{
		TranslateTimeSec = InTranslateTimeSec;
		CurrentTimeSec = 0.0f;
		GoalValue = InInitValue;
		CurrentValue = InInitValue;
	}

	float GetGoalValue() const { return GoalValue; }
	float GetCurrentValue() const { return CurrentValue; }

	void SetGoalAndCurrentValue(float InNewValue)
	{
		GoalValue = InNewValue;
		CurrentValue = InNewValue;
	}

	void TrySetGoalValue(float InGoalValue)
	{
		if (GoalValue == InGoalValue)
		{
			return;
		}
		GoalValue = InGoalValue;
		CurrentTimeSec = 0.0f;
	}

	float UpdateValue(float InDeltaTimeSec)
	{
		if (GoalValue == CurrentValue)
		{
			return GoalValue;
		}
		CurrentTimeSec += InDeltaTimeSec;
		if (CurrentTimeSec >= TranslateTimeSec)
		{
			CurrentValue = GoalValue;
		}
		else
		{
			CurrentValue = FMath::Lerp(CurrentValue, GoalValue, CurrentTimeSec / TranslateTimeSec);
		}
		return CurrentValue;
	}

	float TranslateTimeSec;
	float CurrentTimeSec;
	float CurrentValue;
	float GoalValue;
};

struct T4ENGINE_API FT4HalfAngleInterpolator
{
	FT4HalfAngleInterpolator()
		: TranslateYawRate(0.0f)
		, TranslateYawRateScale(1.0f)
		, CurrentAngle(0.0f)
		, GoalAngle(0.0f)
	{
	}

	void Initialize(float InYawRate, float InInitAngle)
	{
		TranslateYawRate = InYawRate;
		GoalAngle = InInitAngle;
		CurrentAngle = InInitAngle;
	}

	float GetGoalAngle() const { return GoalAngle; }
	float GetCurrentAngle() const { return CurrentAngle; }

	void SetTranslateYawRateScale(float InScale)
	{
		TranslateYawRateScale = InScale;
	}

	void SetGoalAndCurrentAngle(float InNewValue)
	{
		GoalAngle = InNewValue;
		CurrentAngle = InNewValue;
	}

	void TrySetGoalAngle(float InGoalValue)
	{
		if (GoalAngle == InGoalValue)
		{
			return;
		}
		GoalAngle = InGoalValue;
	}

	float UpdateAngle(float InDeltaTimeSec)
	{
		if (GoalAngle == CurrentAngle)
		{
			return GoalAngle;
		}
		const float UpdateYaqAngle = (TranslateYawRate * TranslateYawRateScale) * InDeltaTimeSec;
		CurrentAngle = FMath::FixedTurn(CurrentAngle, GoalAngle, UpdateYaqAngle);
		CurrentAngle = FMath::UnwindDegrees(CurrentAngle);
		if (0.1f >= FMath::Abs(GoalAngle - CurrentAngle))
		{
			CurrentAngle = GoalAngle;
		}
		return CurrentAngle;
	}

	float TranslateYawRate;
	float TranslateYawRateScale;
	float CurrentAngle;
	float GoalAngle;
};

class IT4WorldActor;

namespace T4EngineUtility
{
	T4ENGINE_API bool GetMovementStartParameter(
		IT4WorldActor* InOwnerActor,
		const FVector& InJumpVelocity, // 
		float InMaxHeight, // 최대 높이
		float InMaxSpeedXY, // 점프 최대 속도 XY
		FVector& OutGoalLocation,
		float& OutDurationSec,
		FVector& OutCollideLocation, // 점프시 첫번째 부딪히는 지점이 있을 경우. 없으면 Zero
		float& OutCollideTimeSec // 점프시 첫번째 부딪히는 지점까지의 시간. 없으면 Zero
	); // #140 : Goal Location 을 찾아 서버로 전송 또는 클라 테스트

	T4ENGINE_API bool GetMovementParameterValidation(
		ET4LayerType InLayerType,
		const FVector& InStartLocation, // NavPoint or Actor Root Location (바닥)
		const FVector& InGoalLocation, // 서버에서 넘겨주었다.
		float InMaxHeight, // 최대 높이
		float InMaxDistanceXY, // 최대 이동 가능한 최대 거리
		FVector& OutGoalLocation,
		float& OutDurationSec
	); // #140 : 클라에게 받은 GoalLocation 검증, 성공시 파라미터 리턴
}