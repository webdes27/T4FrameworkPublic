// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionStanceTask.h"

#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"
#include "Public/T4EngineAnimNotify.h" // #111

#include "T4EngineInternal.h"

static const float T4StanceEquipmentBlendTimeSec = 0.25f;
static const float T4StanceTransitionBlendTimeSec = 0.25f;

#define USES_DISABLE_EQUIPMENT_ANIM 0 // #111
#define USES_DISABLE_TRANSITION_ANIM 0 // #111

/**
  * #111
 */
FT4ActionStanceTask::FT4ActionStanceTask(FT4ActionTaskControl* InControl)
	: FT4ActionTaskBase(InControl)
	, bPendingChange(false)
	, PendingStanceName(NAME_None)
	, ChangeDelayTimeLeftSec(0.0f)
	, bPendingTransitionAnimation(false)
	, PendingTransitionTimeLeftSec(0.0f)
	, bPendingEquipAnimation(false)
	, PendingEquipmentTimeLeftSec(0.0f)
	, bPendingUnmountAnimNotifyFallback(false)
	, PendingUnmountAnimNotifyTimeLeftSec(0.0f)
	, PlayingAnimInstanceID(INDEX_NONE)
{
}

FT4ActionStanceTask::~FT4ActionStanceTask()
{
}

void FT4ActionStanceTask::Reset()
{
	bPendingChange = false;
	bPendingTransitionAnimation = false;
	bPendingEquipAnimation = false;
	bPendingUnmountAnimNotifyFallback = false;
	PendingStanceName = NAME_None;
	ChangeDelayTimeLeftSec = 0.0f;
}

void FT4ActionStanceTask::Advance(const FT4UpdateTime& InUpdateTime)
{
	if (bPendingTransitionAnimation) // #111
	{
		PendingTransitionTimeLeftSec -= InUpdateTime.ScaledTimeSec;
		if (0.0 >= PendingTransitionTimeLeftSec)
		{
			PlayTransitionAnimation(0.0f, 0.0f);
			bPendingTransitionAnimation = false;
		}
	}
	if (bPendingChange)
	{
		ChangeDelayTimeLeftSec -= InUpdateTime.ScaledTimeSec;
		if (0.0 >= ChangeDelayTimeLeftSec)
		{
			Change(PendingStanceName, true);
		}
	}
	if (bPendingEquipAnimation) // #111
	{
		PendingEquipmentTimeLeftSec -= InUpdateTime.ScaledTimeSec;
		if (0.0 >= PendingEquipmentTimeLeftSec)
		{
			PlayEquipAnimation();
			bPendingEquipAnimation = false;
		}
	}
	if (bPendingUnmountAnimNotifyFallback) // #111
	{
		PendingUnmountAnimNotifyTimeLeftSec -= InUpdateTime.ScaledTimeSec;
		if (0.0 >= PendingUnmountAnimNotifyTimeLeftSec)
		{
			// #111 : AnimNotify 로 장비 장착을 처리할 경우에 대한 예외 처리 (T4BaseEquipment::OnAnimNotify 참조)
			//		  Unmount AnimNotify 가 없을 경우에도 장비 장착을 할 수 있도록 조치
			AT4GameObject* OwnerObject = GetGameObject();
			check(nullptr != OwnerObject);
			const FName CurrentStanceName = OwnerObject->GetStanceName();
			SendAnimNotifyFallback(ET4EquipmentType::Unmount, CurrentStanceName);
			bPendingUnmountAnimNotifyFallback = false;
		}
	}
}

bool FT4ActionStanceTask::Bind(const FT4StanceAction& InAction)
{
	if (bPendingChange)
	{
		Flush(); // 동작중에 호출되면 강제 적용 후 진행
	}
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);
	if (!OwnerObject->IsLoaded())
	{
		Change(InAction.StanceName, false);  // 로딩전이면 즉시 적용해준다.
		return true;
	}
	IT4AnimControl* AnimControl = OwnerObject->GetAnimControl();
	check(nullptr != AnimControl);
	bool bValidTransitionAnimation = AnimControl->HasSection(T4Const_DefaultAnimMontageName, T4Const_DefaultSectionStanceTransitionName);
	if (!AnimControl->HasSection(T4Const_OverlayAnimMontageName, T4Const_OverlaySectionUnequipWeaponName))
	{
		if (!bValidTransitionAnimation)
		{
			Change(InAction.StanceName, true); // Stance 전환과 Transition 둘다 없으면 바로 업데이트
			return true;
		}
		ChangeDelayTimeLeftSec = T4StanceTransitionBlendTimeSec; // Transition 만 있다면 블랜딩만 하고 바로 Change
		PlayTransitionAnimation(0.0f, 0.0f);
	}
	else
	{
		ChangeDelayTimeLeftSec = AnimControl->GetDurationSec(
			T4Const_OverlayAnimMontageName,
			T4Const_OverlaySectionUnequipWeaponName
		);
		if (bValidTransitionAnimation)
		{
			// #111 : Stance 간 아이들이 상이하기 때문에 TransitionIdle 을 추가해 동작시킨다.
			//        즉, 스탠스 끝/시작 애니를 맞춰서 끊어지지 않도록 처리하는 것...
			bPendingTransitionAnimation = true;
			PendingTransitionTimeLeftSec = FMath::Max(ChangeDelayTimeLeftSec - T4StanceTransitionBlendTimeSec, 0.0f);
		}
		PlayUnequipAnimation();
	}
	if (0.0f < ChangeDelayTimeLeftSec)
	{
		// #111 : Stance AnimSet 을 Preloading 을 걸어준다.
		OwnerObject->PreloadStance(InAction.StanceName);
	}
	PendingStanceName = InAction.StanceName;
	bPendingChange = true;
	return true;
}

bool FT4ActionStanceTask::Change(FName InStanceName, bool bTryEquipAnimation)
{
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);
	{
		// #111 : AnimNotify 로 장비 탈착을 처리할 경우에 대한 예외 처리 (T4BaseEquipment::OnAnimNotify 참조)
		//        즉, Equip 처리가 모두 종료되고 Stance 전이전 강제로 AnimNotify 를 던져 무기를 Detach 하도록 처리해준다.
		//        참고로, 이 처리는 AnimNotify 가 없을 경우에 대한 처리도 포함된다.
		const FName PrevStanceName = OwnerObject->GetStanceName();
		SendAnimNotifyFallback(ET4EquipmentType::Mount, PrevStanceName);
	}
	if (InStanceName == NAME_None)
	{
		InStanceName = T4Const_DefaultStanceName;
	}
	OwnerObject->ChangeStance(InStanceName, false);
	bPendingChange = false;
	if (bTryEquipAnimation)
	{
		PlayTransitionAnimation(0.0f, T4StanceTransitionBlendTimeSec);
		bPendingEquipAnimation = true;
		PendingEquipmentTimeLeftSec = T4StanceTransitionBlendTimeSec;
	}
	else
	{
		// #111 : AnimNotify 로 장비 장착을 처리할 경우에 대한 예외 처리 (T4BaseEquipment::OnAnimNotify 참조)
		//		  Equip 처리를 하지 않을 경우 바로 아이템을 장착할 수 있도록 처리해준다.
		SendAnimNotifyFallback(ET4EquipmentType::Unmount, InStanceName);
	}
	return true;
}

bool FT4ActionStanceTask::PlayTransitionAnimation(
	float InBlendInTimeSec,
	float InBlendOutTimeSec
)
{
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);
	if (!OwnerObject->IsLoaded())
	{
		return false;
	}
	IT4AnimControl* AnimControl = OwnerObject->GetAnimControl();
	check(nullptr != AnimControl);
	if (!AnimControl->HasSection(T4Const_DefaultAnimMontageName, T4Const_DefaultSectionStanceTransitionName))
	{
		return false;
	}
#if !USES_DISABLE_TRANSITION_ANIM
	FT4AnimParameters PlayAnimParameters;
	PlayAnimParameters.AnimMontageName = T4Const_DefaultAnimMontageName;
	PlayAnimParameters.SectionName = T4Const_DefaultSectionStanceTransitionName;
	PlayAnimParameters.BlendInTimeSec = InBlendInTimeSec;
	PlayAnimParameters.BlendOutTimeSec = InBlendOutTimeSec;
	PlayingAnimInstanceID = AnimControl->PlayAnimation(PlayAnimParameters);
#endif
	return true;
}

bool FT4ActionStanceTask::PlayUnequipAnimation()
{
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);
	if (!OwnerObject->IsLoaded())
	{
		return false;
	}
	IT4AnimControl* AnimControl = OwnerObject->GetAnimControl();
	check(nullptr != AnimControl);
	if (!AnimControl->HasSection(T4Const_OverlayAnimMontageName, T4Const_OverlaySectionUnequipWeaponName))
	{
		return false;
	}
#if !USES_DISABLE_EQUIPMENT_ANIM
	FT4AnimParameters PlayAnimParameters;
	PlayAnimParameters.AnimMontageName = T4Const_OverlayAnimMontageName;
	PlayAnimParameters.SectionName = T4Const_OverlaySectionUnequipWeaponName;
	PlayAnimParameters.BlendInTimeSec = T4StanceEquipmentBlendTimeSec;
	PlayAnimParameters.BlendOutTimeSec = T4StanceTransitionBlendTimeSec;
	PlayingAnimInstanceID = OwnerObject->PlayAnimationAndBroadcast(PlayAnimParameters);
#endif
	return true;
}

bool FT4ActionStanceTask::PlayEquipAnimation()
{
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);
	const FName CurrentStanceName = OwnerObject->GetStanceName();
	if (!OwnerObject->IsLoaded())
	{
		// #111 : AnimNotify 로 장비 장착을 처리할 경우에 대한 예외 처리 (T4BaseEquipment::OnAnimNotify 참조)
		SendAnimNotifyFallback(ET4EquipmentType::Unmount, CurrentStanceName);
		return false;
	}
	IT4AnimControl* AnimControl = OwnerObject->GetAnimControl();
	check(nullptr != AnimControl);
	if (INDEX_NONE != PlayingAnimInstanceID) // Transition Anim
	{
		AnimControl->StopAnimation(PlayingAnimInstanceID, 0.1f);
		PlayingAnimInstanceID = INDEX_NONE;
	}
	if (!AnimControl->HasSection(T4Const_OverlayAnimMontageName, T4Const_OverlaySectionEquipWeaponName))
	{
		// #111 : AnimNotify 로 장비 장착을 처리할 경우에 대한 예외 처리 (T4BaseEquipment::OnAnimNotify 참조)
		SendAnimNotifyFallback(ET4EquipmentType::Unmount, CurrentStanceName);
		return false;
	}
	{
		// #111 : AnimNotify 로 장비 장착을 처리할 경우에 대한 예외 처리 (T4BaseEquipment::OnAnimNotify 참조)
		bPendingUnmountAnimNotifyFallback = true;
		PendingUnmountAnimNotifyTimeLeftSec = AnimControl->GetDurationSec(
			T4Const_OverlayAnimMontageName,
			T4Const_OverlaySectionEquipWeaponName
		);
	}
#if !USES_DISABLE_EQUIPMENT_ANIM
	FT4AnimParameters PlayAnimParameters;
	PlayAnimParameters.AnimMontageName = T4Const_OverlayAnimMontageName;
	PlayAnimParameters.SectionName = T4Const_OverlaySectionEquipWeaponName;
	PlayAnimParameters.BlendInTimeSec = T4StanceEquipmentBlendTimeSec;
	PlayAnimParameters.BlendOutTimeSec = T4StanceEquipmentBlendTimeSec;
	PlayingAnimInstanceID = OwnerObject->PlayAnimationAndBroadcast(PlayAnimParameters);
#endif
	return true;
}

void FT4ActionStanceTask::Flush() // #111 : 외부에서 강제로 즉시 적용할 경우 호출됨
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
	if (bPendingChange)
	{
		// #111 : 팬딩된 전환이 있다면 즉시 적용해준다.
		Change(PendingStanceName, false);
		Reset();
	}
	else if (bPendingEquipAnimation || bPendingUnmountAnimNotifyFallback) // #111
	{
		// #111 : Stance 는 전이되었으나 Equip 연출이 있을 경우 무기가 바로 보이도록 Unmount 전달
		AT4GameObject* OwnerObject = GetGameObject();
		check(nullptr != OwnerObject);
		const FName CurrentStanceName = OwnerObject->GetStanceName();
		SendAnimNotifyFallback(ET4EquipmentType::Unmount, CurrentStanceName);
		bPendingEquipAnimation = false;
		bPendingUnmountAnimNotifyFallback = false;
	}
}

void FT4ActionStanceTask::SendAnimNotifyFallback(
	ET4EquipmentType InEquipmentType,
	FName InStanceName
)
{
	// #111 : AnimNotify 가 없을 경우에 대한 처리. 
	//        즉, Equip/Unequip 애니메이션이 끝나면 AnimNotify 를 강제로 던진다. 타이밍이 맞는건 그 전에 보내졌을 것이다.
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);
	FT4AnimNotifyEquipment NewMessage;
	NewMessage.EquipmentType = InEquipmentType;
	NewMessage.SameStanceName = InStanceName;
#if WITH_EDITOR
	NewMessage.DebugSting = TEXT("AnimNotifyExpired");
#endif
	OwnerObject->OnAnimNotifyMessage(&NewMessage);
}