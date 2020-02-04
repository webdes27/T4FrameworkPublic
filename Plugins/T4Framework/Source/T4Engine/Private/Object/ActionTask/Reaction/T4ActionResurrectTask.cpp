// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionResurrectTask.h"

#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "Components/SkeletalMeshComponent.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionResurrectTask::FT4ActionResurrectTask(FT4ActionTaskControl* InControl)
	: FT4ActionReactionTaskBase(InControl)
{
}

FT4ActionResurrectTask::~FT4ActionResurrectTask()
{
}

void FT4ActionResurrectTask::Advance(const FT4UpdateTime& InUpdateTime)
{
	FT4ActionReactionTaskBase::Advance(InUpdateTime);
}

bool FT4ActionResurrectTask::Bind(const FT4ResurrectAction& InAction)
{
	bool bResult = Play(InAction); // #111
	return bResult;
}

bool FT4ActionResurrectTask::Play(const FT4ResurrectAction& InAction)
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
	AdvancePlayInfo(FT4UpdateTime::EmptyUpdateTime, NewPlayInfo);
	return true;
}

void FT4ActionResurrectTask::StopAll()
{
	for (TArray<FT4ReactionPlayInfo>::TIterator It(PlayInfos); It; ++It)
	{
		// 강제 종료되는데, Physics Stop 이 안불렸다면 강제로 호출해준다.
		FT4ReactionPlayInfo& PlayInfo = *It;
		if (PlayInfo.Data.bUsePhysicsStop && !PlayInfo.bPhysicsStopped)
		{
			check(nullptr != ManualControlRef);
			ManualControlRef->PhysicsEnd();
		}
	}
	FT4ActionReactionTaskBase::StopAll();
}