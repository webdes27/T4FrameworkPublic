// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ManualDieNode.h"

#include "Object/Action/T4ActionControl.h"
#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ManualDieNode::FT4ManualDieNode(FT4ManualControl* InControl)
	: FT4ManualReactionNode(InControl)
{
}

FT4ManualDieNode::~FT4ManualDieNode()
{
}

void FT4ManualDieNode::Advance(const FT4UpdateTime& InUpdateTime)
{
	FT4ManualReactionNode::Advance(InUpdateTime);
}

bool FT4ManualDieNode::Play(const FT4DieAction& InAction)
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
