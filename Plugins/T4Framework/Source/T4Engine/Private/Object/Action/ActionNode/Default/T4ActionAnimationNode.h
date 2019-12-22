// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/Action/ActionNode/T4ActionNode.h"
#include "Public/T4Engine.h"

/**
  *
 */
class FT4ActionControl;
class FT4ActionAnimationNode : public FT4ActionNode
{
public:
	explicit FT4ActionAnimationNode(FT4ActionControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionAnimationNode();

	static FT4ActionAnimationNode* CreateNode(
		FT4ActionControl* InControl, 
		const FT4AnimationAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::Animation; }

protected:
	bool Create(const FT4ActionStruct* InAction) override;
	void Destroy() override;

	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play() override;
	void Stop() override;

	bool IsAutoFinished() const override;

	bool PlayInternal(float InOffsetTimeSec) override; // #56

private:
	FT4AnimParameters AnimParameters; // #38

	bool bAutoFinished; // #60
	float AutoDurationSec; // #54
	FT4AnimInstanceID PlayAnimInstanceID;

#if WITH_EDITOR
	FT4AnimationAction ActionCached;
#endif
};
