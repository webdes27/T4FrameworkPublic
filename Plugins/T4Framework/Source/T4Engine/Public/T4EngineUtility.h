// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

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