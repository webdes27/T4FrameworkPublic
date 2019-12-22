// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/Action/ActionNode/T4ActionNode.h"
#include "Public/T4Engine.h"

/**
  *
 */
class FT4ActionControl;
class FT4ActionSpecialMoveNode : public FT4ActionNode
{
public:
	explicit FT4ActionSpecialMoveNode(FT4ActionControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionSpecialMoveNode();

	static FT4ActionSpecialMoveNode* CreateNode(
		FT4ActionControl* InControl,
		const FT4SpecialMoveAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::SpecialMove; }

protected:
	bool Create(const FT4ActionStruct* InAction) override;
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
