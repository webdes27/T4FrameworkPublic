// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionReactionTaskBase.h"
#include "Public/Action/T4ActionStatusCommands.h"

/**
  *
 */
class FT4ActionNodeControl;
class UT4CharacterEntityAsset;
class FT4ActionHitTask : public FT4ActionReactionTaskBase
{
public:
	explicit FT4ActionHitTask(FT4ActionTaskControl* InControl);
	virtual ~FT4ActionHitTask();

	const ET4ActionType GetType() const override { return ET4ActionType::Hit; }

	bool Bind(const FT4HitAction& InAction); // #111

protected:
	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play(const FT4HitAction& InAction); // #111
};
