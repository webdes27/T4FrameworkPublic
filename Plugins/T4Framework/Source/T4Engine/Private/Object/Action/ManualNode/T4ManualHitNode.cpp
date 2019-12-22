// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ManualHitNode.h"

#include "Object/Action/T4ActionControl.h"
#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ManualHitNode::FT4ManualHitNode(FT4ManualControl* InControl)
	: FT4ManualReactionNode(InControl)
{
}

FT4ManualHitNode::~FT4ManualHitNode()
{
}

void FT4ManualHitNode::Advance(const FT4UpdateTime& InUpdateTime)
{
	FT4ManualReactionNode::Advance(InUpdateTime);
}

bool FT4ManualHitNode::Play(const FT4HitAction& InAction)
{
	const UT4CharacterEntityAsset* CharacterEntityAsset = GetChracterEntityAsset();
	if (nullptr == CharacterEntityAsset)
	{
		return false;
	}
	FT4ReactionPlayInfo& NewPlayInfo = PlayInfos.AddDefaulted_GetRef();
	bool bResult = GetDataInEntity(
		CharacterEntityAsset,
		InAction.ReactionName,
		InAction.bTransientPlay,
		NewPlayInfo.Data
	);
	if (!bResult)
	{
		NewPlayInfo.bPlayFailed = true;
		return false;
	}
	NewPlayInfo.ShotDirection = InAction.ShotDirection;
	AdvancePlayInfo(FT4UpdateTime::EmptyUpdateTime, NewPlayInfo);
	return true;
}
