// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ManualReactionNode.h"
#include "Public/T4Engine.h"

/**
  *
 */
class FT4ActionControl;
class UT4CharacterEntityAsset;
class FT4ManualResurrectNode : public FT4ManualReactionNode
{
public:
	explicit FT4ManualResurrectNode(FT4ManualControl* InControl);
	virtual ~FT4ManualResurrectNode();

	const ET4ActionType GetType() const override { return ET4ActionType::Resurrect; }

	bool Play(const FT4ResurrectAction& InAction);

	void StopAll() override;

protected:
	void Advance(const FT4UpdateTime& InUpdateTime) override;

private:

};
