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
class FT4ActionResurrectTask : public FT4ActionReactionTaskBase
{
public:
	explicit FT4ActionResurrectTask(FT4ActionTaskControl* InControl);
	virtual ~FT4ActionResurrectTask();

	const ET4ActionType GetType() const override { return ET4ActionType::Resurrect; }

	bool Bind(const FT4ResurrectAction& InAction); // #111

	void StopAll() override;

protected:
	void Advance(const FT4UpdateTime& InUpdateTime) override;

	bool Play(const FT4ResurrectAction& InAction); // #111
}; 
