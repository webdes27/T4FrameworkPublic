// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionHitTask.h"

#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionHitTask::FT4ActionHitTask(FT4ActionTaskControl* InControl)
	: FT4ActionReactionTaskBase(InControl)
{
}

FT4ActionHitTask::~FT4ActionHitTask()
{
}

void FT4ActionHitTask::Advance(const FT4UpdateTime& InUpdateTime)
{
	FT4ActionReactionTaskBase::Advance(InUpdateTime);
}

bool FT4ActionHitTask::Bind(const FT4HitAction& InAction)
{
	bool bResult = Play(InAction); // #111
	return bResult;
}

bool FT4ActionHitTask::Play(const FT4HitAction& InAction)
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
