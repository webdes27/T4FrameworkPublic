// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Asset/Public/Action/T4ActionTypes.h"

/**
  * #58, #102
 */
class FT4ActionEasingCurveBlender
{
public:
	explicit FT4ActionEasingCurveBlender();
	virtual ~FT4ActionEasingCurveBlender();

	void Initialize(
		ET4BuiltInEasing InBlendInCurve,
		float InBlendInTimeSec,
		ET4BuiltInEasing InBlendOutCurve,
		float InBlendOutTimeSec
	);

	void SetOffsetTimeSec(
		float InBlendInOffsetTimeSec,
		float InBlendOutOffsetTimeSec
	);

	float GetBlendInTimeSec() const { return BlendInTimeSec; }
	void SetBlendInTimeSec(float InBlendTimeSec) { BlendInTimeSec = InBlendTimeSec; }

	float GetBlendOutTimeSec() const { return BlendOutTimeSec; }
	void SetBlendOutTimeSec(float InBlendTimeSec) { BlendOutTimeSec = InBlendTimeSec; }

	float GetBlendWeight(float InPlayingTimeSec, float InPlayTimeLeft) const; // #100, #58, #102 : 0 ~ 1

private:
	enum ET4BlendType {
		BT_In,
		BT_Out,
		BT_Default,
	};
	float GetCurrentWeight(
		float InPlayingTimeSec, 
		float InPlayTimeLeft,
		ET4BlendType& OutBlendType
	) const; // #100, #58 : 0 ~ 1

private:
	ET4BuiltInEasing BlendInCurve;
	float BlendInTimeSec;

	ET4BuiltInEasing BlendOutCurve;
	float BlendOutTimeSec;

	float BlendInOffsetTimeSec; // #58 : 시작 시점이 필요에 의해 늦어질 경우 (ex. CameraWork)
	float BlendOutOffsetTimeSec; // #58 : 끝 시점이 필요에 의해 늦어질 경우 (ex. CameraWork)
};

namespace T4ActionNodeUtility
{
	bool CheckPlayTarget(
		ET4PlayTarget InPlayTarget,
		FT4ActionControl* InControl,
		const FT4ActionStruct* InAction,
		const FT4ActionParameters* InParameters // #54
	); // #101 : PlayTarget 옵션에 따라 스스로 처리할지 Player 에게 전달할지를 결정!
}