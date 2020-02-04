// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/ActionNode/T4ActionNodeUtility.h"
#include "Object/ActionNode/T4ActionNodeBase.h"
#include "Public/T4Engine.h"

/**
  * #102
 */
class FT4ActionNodeControl;
class FT4ActionTimeScaleNode : public FT4ActionNodeBase
{
public:
	explicit FT4ActionTimeScaleNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionTimeScaleNode();

	static FT4ActionTimeScaleNode* CreateNode(
		FT4ActionNodeControl* InControl,
		const FT4TimeScaleAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::TimeScale; }

protected:
	bool Create(const FT4ActionCommand* InAction) override;
	void Destroy() override;

	// #102 : 삭제 대기로 들어갈 경우 한프레임 늦게 삭제되기 때문에 일부 액션(Ex:TimeScale)
	//        의 원상복구가 되지 않는 문제가 있어 추가함
	void Destroying() override; // #102

	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play() override;
	void Stop() override;

	bool IsAutoFinished() const override;

	bool PlayInternal(float InOffsetTimeSec) override; // #56

private:
	float TimeScale;

	FT4ActionEasingCurveBlender EasingCurveBlender;

#if WITH_EDITOR
	FT4TimeScaleAction ActionCached;
#endif
};
