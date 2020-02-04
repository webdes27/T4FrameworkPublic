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
class FT4ActionDieTask : public FT4ActionReactionTaskBase
{
public:
	explicit FT4ActionDieTask(FT4ActionTaskControl* InControl);
	virtual ~FT4ActionDieTask();

	const ET4ActionType GetType() const override { return ET4ActionType::Die; }

	bool Bind(const FT4DieAction& InAction); // #111

protected:
	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play(const FT4DieAction& InAction); // #111
};
