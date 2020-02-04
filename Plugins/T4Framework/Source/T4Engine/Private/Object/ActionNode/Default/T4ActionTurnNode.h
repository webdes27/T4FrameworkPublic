// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/ActionNode/T4ActionNodeBase.h"
#include "Public/T4Engine.h"

/**
  *
 */
class FT4ActionNodeControl;
class FT4ActionTurnNode : public FT4ActionNodeBase
{
public:
	explicit FT4ActionTurnNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionTurnNode();

	static FT4ActionTurnNode* CreateNode(
		FT4ActionNodeControl* InControl,
		const FT4TurnAction& InAction,
		const FT4ActionParameters* InParameters // #54
	);

	const ET4ActionType GetType() const override { return ET4ActionType::Turn; }

protected:
	bool Create(const FT4ActionCommand* InAction) override;
	void Destroy() override;

	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play() override;
	void Stop() override;

	bool IsAutoFinished() const override;

	bool PlayInternal(float InOffsetTimeSec) override; // #56

private:
	bool SetRotationTargetObject();
	bool SetRotationTargetLocation();
	bool SetRotationTargetDirection();
	bool SetRotationTargetYawAngle(); // #40

	void SetLookAt(const FRotator& InRotation);

private:
	ET4TargetType TargetType;
	float TargetYawAngle; // #40
	float RotationYawRate; // #44

#if WITH_EDITOR
	FT4TurnAction ActionCached;
#endif
};
