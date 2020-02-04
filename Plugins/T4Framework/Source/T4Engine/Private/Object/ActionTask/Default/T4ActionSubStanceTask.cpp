// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionSubStanceTask.h"

#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "T4EngineInternal.h"

/**
  * #111
 */
FT4ActionSubStanceTask::FT4ActionSubStanceTask(FT4ActionTaskControl* InControl)
	: FT4ActionTaskBase(InControl)
	, bPending(false)
	, PendingSubStanceName(NAME_None)
	, DelayTimeLeftSec(0.0f)
	, PlayingAnimInstanceID(INDEX_NONE)
	, AnimInstanceIDClearTimeLeftSec(0.0f)
{
}

FT4ActionSubStanceTask::~FT4ActionSubStanceTask()
{
}

void FT4ActionSubStanceTask::Reset()
{
	bPending = false;
	PendingSubStanceName = NAME_None;
	DelayTimeLeftSec = 0.0f;
}

void FT4ActionSubStanceTask::Advance(const FT4UpdateTime& InUpdateTime)
{
	if (INDEX_NONE != PlayingAnimInstanceID)
	{
		AnimInstanceIDClearTimeLeftSec -= InUpdateTime.ScaledTimeSec;
		if (0.0 >= AnimInstanceIDClearTimeLeftSec)
		{
			PlayingAnimInstanceID = INDEX_NONE; // 애니메이션 플레이가 종료되었음으로 자동 Reset
		}
	}
	if (bPending)
	{
		DelayTimeLeftSec -= InUpdateTime.ScaledTimeSec;
		if (0.0 >= DelayTimeLeftSec)
		{
			Change(PendingSubStanceName);
			Reset();
		}
	}
}

bool FT4ActionSubStanceTask::Bind(const FT4SubStanceAction& InAction)
{
	if (bPending)
	{
		Flush(); // 동작중에 호출되면 강제 적용 후 진행
	}
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);
	if (!OwnerObject->IsLoaded())
	{
		Change(InAction.SubStanceName);  // 로딩전이면 즉시 적용해준다.
		return true;
	}
	const FName CurrentSubStanceName = OwnerObject->GetSubStanceName();
	if (InAction.SubStanceName == CurrentSubStanceName)
	{
		Change(InAction.SubStanceName);  // SubStance 가 같다면 바로 업데이트. (TODO : Skip)
		return true;
	}
	FName PlaySectionName = NAME_None;
	if (T4Const_DefaultSubStanceName == CurrentSubStanceName) // Default to Combat
	{
		PlaySectionName = T4Const_OverlaySectionSubStanceDefaultToCombatName;
	}
	else // Combat to Default
	{
		PlaySectionName = T4Const_OverlaySectionSubStanceCombatToDefaultName;
	}
	IT4AnimControl* AnimControl = OwnerObject->GetAnimControl();
	check(nullptr != AnimControl);
	if (!AnimControl->HasSection(T4Const_OverlayAnimMontageName, PlaySectionName))
	{
		Change(InAction.SubStanceName); // SubStance 전환 애니가 없으면 바로 업데이트
		return true;
	}
	AnimInstanceIDClearTimeLeftSec = AnimControl->GetDurationSec(
		T4Const_OverlayAnimMontageName, 
		PlaySectionName
	);
	DelayTimeLeftSec = AnimInstanceIDClearTimeLeftSec * 0.5f; // 전환 애니가 있다면 50% 지점에서 SubStance 교체
	{
		FT4AnimParameters PlayAnimParameters;
		PlayAnimParameters.AnimMontageName = T4Const_OverlayAnimMontageName;
		PlayAnimParameters.SectionName = PlaySectionName;
		PlayingAnimInstanceID = OwnerObject->PlayAnimationAndBroadcast(PlayAnimParameters);
	}
	PendingSubStanceName = InAction.SubStanceName;
	bPending = true;
	return true;
}

bool FT4ActionSubStanceTask::Change(FName InSubStanceName)
{
	if (InSubStanceName == NAME_None)
	{
		InSubStanceName = T4Const_DefaultSubStanceName;
	}
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);
	bool bResult = OwnerObject->ChangeSubStance(InSubStanceName, false);
	return bResult;
}

void FT4ActionSubStanceTask::Flush()  // #111 : 외부에서 강제로 즉시 적용할 경우 호출됨
{
	if (INDEX_NONE != PlayingAnimInstanceID)
	{
		// #113 플레이중인 애니가 있다면 즉시 정지 시킨다.
		AT4GameObject* OwnerObject = GetGameObject();
		check(nullptr != OwnerObject);
		IT4AnimControl* AnimControl = OwnerObject->GetAnimControl();
		check(nullptr != AnimControl);
		AnimControl->StopAnimation(PlayingAnimInstanceID, 0.1f);
		PlayingAnimInstanceID = INDEX_NONE;
	}
	if (bPending)
	{
		// #111 : 팬딩된 전환이 있다면 즉시 적용해준다.
		Change(PendingSubStanceName);
		Reset();
	}
}