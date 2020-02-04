// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/ActionTask/T4ActionTaskBase.h"
#include "Public/Action/T4ActionStatusCommands.h"

/**
  * #111
 */
class FT4ActionStanceTask : public FT4ActionTaskBase
{
public:
	explicit FT4ActionStanceTask(FT4ActionTaskControl* InControl);
	virtual ~FT4ActionStanceTask();

	const ET4ActionType GetType() const override { return ET4ActionType::Stance; }

	bool IsPending() const override // #116
	{ 
		return bPendingChange || bPendingTransitionAnimation || bPendingUnmountAnimNotifyFallback; 
	} 
	void Flush() override; // #111 : 외부에서 강제로 즉시 적용할 경우 호출됨

	void Reset();

	bool Bind(const FT4StanceAction& InAction);

protected:
	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Change(FName InStanceName, bool bTryEquipAnimation);

	bool PlayTransitionAnimation(float InBlendInTimeSec, float InBlendOutTimeSec);
	bool PlayEquipAnimation();
	bool PlayUnequipAnimation();

	void SendAnimNotifyFallback(ET4EquipmentType InEquipmentType, FName InStanceName); // #111 : AnimNotify 가 없을 경우에 대한 처리

private:
	bool bPendingChange;
	FName PendingStanceName;
	float ChangeDelayTimeLeftSec;

	bool bPendingTransitionAnimation; // #111
	float PendingTransitionTimeLeftSec; // #111

	bool bPendingEquipAnimation; // #111
	float PendingEquipmentTimeLeftSec; // #111

	bool bPendingUnmountAnimNotifyFallback; // #111
	float PendingUnmountAnimNotifyTimeLeftSec; // #111

	FT4AnimInstanceID PlayingAnimInstanceID;
};
