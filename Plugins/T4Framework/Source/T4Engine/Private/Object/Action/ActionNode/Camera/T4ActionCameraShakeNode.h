// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionCameraNode.h"
#include "Public/T4Engine.h"

/**
  * #101
 */
class UCameraShake;
class FT4ActionControl;
class FT4ActionCameraShakeNode : public FT4ActionCameraNode
{
public:
	explicit FT4ActionCameraShakeNode(FT4ActionControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionCameraShakeNode();

	static FT4ActionCameraShakeNode* CreateNode(
		FT4ActionControl* InControl,
		const FT4CameraShakeAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::CameraShake; }

protected:
	bool Create(const FT4ActionStruct* InAction) override;
	void Destroy() override;

	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play() override;
	void Stop() override;

	bool IsAutoFinished() const override;

	bool PlayInternal(float InOffsetTimeSec) override;

private:
	float PlayScale;
	TEnumAsByte<ECameraAnimPlaySpace::Type> PlaySpace;
	FRotator UserDefinedPlaySpace;

	FT4CameraShakeOscillationData OscillationData;
	FT4CameraShakeAnimData AnimData;

	TWeakObjectPtr<UCameraShake> CameraShakePtr;

#if WITH_EDITOR
	FT4CameraShakeAction ActionCached;
#endif
};
