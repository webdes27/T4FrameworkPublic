// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/ActionNode/T4ActionNodeBase.h"
#include "Public/T4Engine.h"

/**
  *
 */
class FT4ActionNodeControl;
class FT4ActionSpecialMoveNode : public FT4ActionNodeBase
{
public:
	explicit FT4ActionSpecialMoveNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionSpecialMoveNode();

	static FT4ActionSpecialMoveNode* CreateNode(
		FT4ActionNodeControl* InControl,
		const FT4SpecialMoveAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::SpecialMove; }

protected:
	bool Create(const FT4ActionCommand* InAction) override;
	void Destroy() override;

	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play() override;
	void Stop() override;

	bool IsAutoFinished() const override;

	bool PlayInternal(float InOffsetTimeSec) override { return true; } // #56

private:
#if WITH_EDITOR
	FT4SpecialMoveAction ActionCached;
#endif
};
