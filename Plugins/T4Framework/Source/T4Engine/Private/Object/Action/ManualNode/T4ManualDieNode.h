// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ManualReactionNode.h"
#include "Public/T4Engine.h"
#include "Public/Action/T4ActionCodeStatus.h"

/**
  *
 */
class FT4ActionControl;
class UT4CharacterEntityAsset;
class FT4ManualDieNode : public FT4ManualReactionNode
{
public:
	explicit FT4ManualDieNode(FT4ManualControl* InControl);
	virtual ~FT4ManualDieNode();

	const ET4ActionType GetType() const override { return ET4ActionType::Die; }

	bool Play(const FT4DieAction& InAction);

protected:
	void Advance(const FT4UpdateTime& InUpdateTime) override;
};
