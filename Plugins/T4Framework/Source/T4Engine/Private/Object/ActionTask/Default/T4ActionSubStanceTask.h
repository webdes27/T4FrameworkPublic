// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/ActionTask/T4ActionTaskBase.h"
#include "Public/Action/T4ActionStatusCommands.h"

/**
  * #111
 */
class FT4ActionSubStanceTask : public FT4ActionTaskBase
{
public:
	explicit FT4ActionSubStanceTask(FT4ActionTaskControl* InControl);
	virtual ~FT4ActionSubStanceTask();

	const ET4ActionType GetType() const override { return ET4ActionType::SubStance; }

	void Flush() override; // #111 : 외부에서 강제로 즉시 적용할 경우 호출됨

	void Reset();

	bool Bind(const FT4SubStanceAction& InAction);
	
protected:
	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Change(FName InSubStanceName);

private:
	bool bPending;
	FName PendingSubStanceName;
	float DelayTimeLeftSec;
	FT4AnimInstanceID PlayingAnimInstanceID;
	float AnimInstanceIDClearTimeLeftSec;
};
