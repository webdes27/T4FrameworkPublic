// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ManualResurrectNode.h"

#include "Object/Action/T4ActionControl.h"
#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "Components/SkeletalMeshComponent.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ManualResurrectNode::FT4ManualResurrectNode(FT4ManualControl* InControl)
	: FT4ManualReactionNode(InControl)
{
}

FT4ManualResurrectNode::~FT4ManualResurrectNode()
{
}

void FT4ManualResurrectNode::Advance(const FT4UpdateTime& InUpdateTime)
{
	FT4ManualReactionNode::Advance(InUpdateTime);
}

bool FT4ManualResurrectNode::Play(const FT4ResurrectAction& InAction)
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

void FT4ManualResurrectNode::StopAll()
{
	for (TArray<FT4ReactionPlayInfo>::TIterator It(PlayInfos); It; ++It)
	{
		// 강제 종료되는데, Physics Stop 이 안불렸다면 강제로 호출해준다.
		FT4ReactionPlayInfo& PlayInfo = *It;
		if (PlayInfo.Data.bUsePhysicsStop && !PlayInfo.bPhysicsStopped)
		{
			check(nullptr != ManualControlRef);
			ManualControlRef->EndPhysics();
		}
	}
	FT4ManualReactionNode::StopAll();
}