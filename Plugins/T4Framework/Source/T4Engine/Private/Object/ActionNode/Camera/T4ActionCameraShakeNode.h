// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionCameraNodeBase.h"
#include "Public/T4Engine.h"

/**
  * #101
 */
class UCameraShake;
class FT4ActionNodeControl;
class FT4ActionCameraShakeNode : public FT4ActionCameraNodeBase
{
public:
	explicit FT4ActionCameraShakeNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionCameraShakeNode();

	static FT4ActionCameraShakeNode* CreateNode(
		FT4ActionNodeControl* InControl,
		const FT4CameraShakeAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::CameraShake; }

protected:
	bool Create(const FT4ActionCommand* InAction) override;
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
