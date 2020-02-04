// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/ActionNode/T4ActionNodeBase.h"
#include "Public/T4Engine.h"

/**
  * #56 : Conti Editor 에서 Invisible or Isolate 로 출력을 제어할 때 더미용으로 사용 (delay, duration 동작 보장)
 */
class FT4ActionNodeControl;
class FT4ActionDummyNode : public FT4ActionNodeBase
{
public:
	explicit FT4ActionDummyNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionDummyNode();

	static FT4ActionDummyNode* CreateNode(
		FT4ActionNodeControl* InControl,
		const FT4ActionCommand& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::Dummy; }

protected:
	bool Create(const FT4ActionCommand* InAction) override;
	void Destroy() override;

	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play() override;
	void Stop() override;

	bool IsAutoFinished() const override;

	bool PlayInternal(float InOffsetTimeSec) override { return true; } // #56

private:
	ET4ActionType SourceAction;
};
