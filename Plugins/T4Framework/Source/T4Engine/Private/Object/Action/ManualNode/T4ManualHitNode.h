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
class FT4ManualHitNode : public FT4ManualReactionNode
{
public:
	explicit FT4ManualHitNode(FT4ManualControl* InControl);
	virtual ~FT4ManualHitNode();

	const ET4ActionType GetType() const override { return ET4ActionType::Hit; }

	bool Play(const FT4HitAction& InAction);

protected:
	void Advance(const FT4UpdateTime& InUpdateTime) override;

private:

};
