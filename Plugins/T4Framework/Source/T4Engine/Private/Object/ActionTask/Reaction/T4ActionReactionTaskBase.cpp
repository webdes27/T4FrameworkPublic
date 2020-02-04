// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionReactionTaskBase.h"

#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "Components/SkeletalMeshComponent.h"

#include "T4EngineInternal.h"

/**
  * #76
 */
FT4ActionReactionTaskBase::FT4ActionReactionTaskBase(FT4ActionTaskControl* InControl)
	: FT4ActionTaskBase(InControl)
{
}

FT4ActionReactionTaskBase::~FT4ActionReactionTaskBase()
{
}

void FT4ActionReactionTaskBase::Advance(const FT4UpdateTime& InUpdateTime)
{
	for (TArray<FT4ReactionPlayInfo>::TIterator It(PlayInfos); It; ++It)
	{
		FT4ReactionPlayInfo& PlayInfo = *It;
		if (PlayInfo.bPlayFailed)
		{
			PlayInfos.RemoveAt(It.GetIndex());
			continue;
		}
		bool bResult = AdvancePlayInfo(InUpdateTime, PlayInfo);
		if (!bResult)
		{
			PlayInfos.RemoveAt(It.GetIndex());
		}
	}
}

bool FT4ActionReactionTaskBase::AdvancePlayInfo(
	const FT4UpdateTime& InUpdateTime,
	FT4ReactionPlayInfo& InPlayInfo
)
{
	InPlayInfo.TimeSec += InUpdateTime.ScaledTimeSec;
	bool bAutoDestroy = (0.0f < InPlayInfo.Data.MaxPlayTimeSec) ? true : false;
	if (bAutoDestroy)
	{
		if (InPlayInfo.TimeSec >= InPlayInfo.Data.MaxPlayTimeSec)
		{
			check(nullptr != ManualControlRef);
			ManualControlRef->PhysicsEnd();
			return false;
		}
	}
	AdvancePhysicsStart(InUpdateTime, InPlayInfo);
	AdvancePhysicsStop(InUpdateTime, InPlayInfo);
	AdvanceAnimation(InUpdateTime, InPlayInfo);
	return true;
}

void FT4ActionReactionTaskBase::AdvancePhysicsStart(
	const FT4UpdateTime& InUpdateTime,
	FT4ReactionPlayInfo& InPlayInfo
)
{
	check(nullptr != ManualControlRef);
	const FT4EntityCharacterReactionData& ReactionData = InPlayInfo.Data;
	if (!ReactionData.bUsePhysicsStart)
	{
		return;
	}
	if (!InPlayInfo.bPhysicsStarted)
	{
		bool bResult = TryPhysicsStart(InPlayInfo);
		if (!bResult)
		{
			return;
		}
	}
	const FT4EntityCharacterReactionPhysicsStartData& PhysicsData = ReactionData.PhysicsStartData;
	const FT4EntityCharacterReactionPhysicsBlendData& PhysicsBlendData = ReactionData.PhysicsStartData.BlendData;
	if (0.0f >= ReactionData.MaxPlayTimeSec) // 플레이 시간이 없다면 Hit 처리는 의미가 없음으로 패스한다.
	{
		return;
	}
	
	const bool bBlendInUsed = (0.0f < PhysicsBlendData.BlendInTimeSec) ? true : false;
	const bool bBlendOutUsed = (0.0f < PhysicsBlendData.BlendOutTimeSec) ? true : false;
	if (!PhysicsData.bSimulateBodiesBelow || (!bBlendInUsed && !bBlendOutUsed))
	{
		return;
	}

	float ApplyBlendWeightAlpha = 1.0f;
	float PlayTimeSec = InPlayInfo.TimeSec - PhysicsData.DelayTimeSec;
	if (bBlendInUsed && PlayTimeSec < PhysicsBlendData.BlendInTimeSec)
	{
		ApplyBlendWeightAlpha = PlayTimeSec / PhysicsBlendData.BlendInTimeSec;
	}
	else if (bBlendOutUsed && PlayTimeSec >= (ReactionData.MaxPlayTimeSec - PhysicsBlendData.BlendOutTimeSec))
	{
		PlayTimeSec -= (ReactionData.MaxPlayTimeSec - PhysicsBlendData.BlendOutTimeSec);
		ApplyBlendWeightAlpha = 1.0f - (PlayTimeSec / PhysicsBlendData.BlendOutTimeSec);
	}
	const float ApplyWeight = ApplyBlendWeightAlpha * PhysicsBlendData.TargetWeight;

	USkeletalMeshComponent* SkeletonMeshComponent = ManualControlRef->GetSkeletonMeshComponent();
	check(nullptr != SkeletonMeshComponent);

	SkeletonMeshComponent->SetAllBodiesBelowPhysicsBlendWeight(
		PhysicsData.ImpulseMainActionPoint,
		ApplyWeight
	);
	if (PhysicsData.ImpulseSubActionPoint != NAME_None)
	{
		SkeletonMeshComponent->SetAllBodiesBelowPhysicsBlendWeight(
			PhysicsData.ImpulseSubActionPoint,
			ApplyWeight
		);
	}
}

void FT4ActionReactionTaskBase::AdvancePhysicsStop(
	const FT4UpdateTime& InUpdateTime,
	FT4ReactionPlayInfo& InPlayInfo
)
{
	const FT4EntityCharacterReactionData& ReactionData = InPlayInfo.Data;
	if (!ReactionData.bUsePhysicsStop)
	{
		return;
	}
	if (!InPlayInfo.bPhysicsStopped)
	{
		TryPhysicsStop(InPlayInfo);
	}
}

void FT4ActionReactionTaskBase::AdvanceAnimation(
	const FT4UpdateTime& InUpdateTime,
	FT4ReactionPlayInfo& InPlayInfo
)
{
	const FT4EntityCharacterReactionData& ReactionData = InPlayInfo.Data;
	if (!ReactionData.bUseAnimation)
	{
		return;
	}
	if (!InPlayInfo.bAnimationStarted)
	{
		TryAnimation(InPlayInfo);
	}
}

void FT4ActionReactionTaskBase::StopAll()
{
	PlayInfos.Empty();
}

bool FT4ActionReactionTaskBase::TryPhysicsStart(FT4ReactionPlayInfo& InPlayInfo)
{
	if (InPlayInfo.bPhysicsStarted)
	{
		return true;
	}
	const FT4EntityCharacterReactionData& ReactionData = InPlayInfo.Data;
	if (!ReactionData.bUsePhysicsStart)
	{
		return false;
	}
	const FT4EntityCharacterReactionPhysicsStartData& PhysicsData = ReactionData.PhysicsStartData;
	if (PhysicsData.DelayTimeSec > InPlayInfo.TimeSec)
	{
		return false;
	}

	check(nullptr != ManualControlRef);
	bool bResult = ManualControlRef->PhysicsStart(&PhysicsData, InPlayInfo.ShotDirection);
	if (!bResult)
	{
		InPlayInfo.bPlayFailed = true;
		return false;
	}

	InPlayInfo.bPhysicsStarted = true;
	return true;
}

bool FT4ActionReactionTaskBase::TryPhysicsStop(FT4ReactionPlayInfo& InPlayInfo)
{
	if (InPlayInfo.bPhysicsStopped)
	{
		return true;
	}
	const FT4EntityCharacterReactionData& ReactionData = InPlayInfo.Data;
	if (!ReactionData.bUsePhysicsStop)
	{
		return false;
	}
	const FT4EntityCharacterReactionPhysicsStopData& PhysicsData = ReactionData.PhysicsStopData;
	if (PhysicsData.DelayTimeSec > InPlayInfo.TimeSec)
	{
		return false;
	}

	check(nullptr != ManualControlRef);
	bool bResult = ManualControlRef->PhysicsStop(&PhysicsData);
	if (bResult)
	{

	}

	InPlayInfo.bPhysicsStopped = true;
	return true;
}

bool FT4ActionReactionTaskBase::TryAnimation(FT4ReactionPlayInfo& InPlayInfo)
{
	if (InPlayInfo.bAnimationStarted)
	{
		return true;
	}
	const FT4EntityCharacterReactionData& ReactionData = InPlayInfo.Data;
	if (!ReactionData.bUseAnimation)
	{
		return false;
	}

	const FT4EntityCharacterReactionAnimationData& AnimationData = ReactionData.AnimationData;
	if (AnimationData.DelayTimeSec > InPlayInfo.TimeSec)
	{
		return false;
	}

	AT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);

	IT4AnimControl* AnimControl = OwnerGameObject->GetAnimControl();
	check(nullptr != AnimControl);

	if (AnimationData.StartAnimSectionName != NAME_None)
	{
		float AnimDurationSec = 0.0f;
		if (nullptr != AnimControl)
		{
			AnimDurationSec = AnimControl->GetDurationSec(
				T4Const_OverlayAnimMontageName,
				AnimationData.StartAnimSectionName
			);
		}
		if (0.0f < AnimDurationSec)
		{
			FT4AnimParameters AnimParameters; // #38
			AnimParameters.AnimMontageName = T4Const_OverlayAnimMontageName;
			AnimParameters.SectionName = AnimationData.StartAnimSectionName;
			AnimParameters.PlayRate = 1.0f;
			AnimParameters.BlendInTimeSec = AnimationData.BlendInTimeSec;
			AnimParameters.BlendOutTimeSec = AnimationData.BlendOutTimeSec;
			AnimParameters.LoopCount = 1;
			check(nullptr != AnimControl);
			AnimControl->PlayAnimation(AnimParameters);
		}
	}

	InPlayInfo.bAnimationStarted = true;
	return true;
}

bool FT4ActionReactionTaskBase::GetDataInEntity(
	const UT4CharacterEntityAsset* InEntity,
	const FName InReactionName,
	bool bInTransientPlay,
	FT4EntityCharacterReactionData& OutReactionData
) // #76
{
	check(nullptr != InEntity);
#if WITH_EDITOR
	if (bInTransientPlay)
	{
		const FT4EntityCharacterEditorTransientData& EditorTransientData = InEntity->EditorTransientCharacterData;
		OutReactionData.ReactionType = EditorTransientData.TransientReactionType;
		OutReactionData.MaxPlayTimeSec = EditorTransientData.TransientReactionMaxPlayTimeSec;
		OutReactionData.bUsePhysicsStart = EditorTransientData.bTransientReactionPhysicsStartUsed;
		OutReactionData.PhysicsStartData = EditorTransientData.TransientReactionPhysicsStartData;
		OutReactionData.bUsePhysicsStop = EditorTransientData.bTransientReactionPhysicsStopUsed;
		OutReactionData.PhysicsStopData = EditorTransientData.TransientReactionPhysicsStopData;
		OutReactionData.bUseAnimation = EditorTransientData.bTransientReactionAnimationUsed;
		OutReactionData.AnimationData = EditorTransientData.TransientReactionAnimationData;
		return true;
	}
#endif
	const FT4EntityCharacterReactionSetData& ReactionSetData = InEntity->ReactionSetData;
	if (!ReactionSetData.ReactionMap.Contains(InReactionName))
	{
		return false;
	}
	OutReactionData = ReactionSetData.ReactionMap[InReactionName];
	return true;
}
