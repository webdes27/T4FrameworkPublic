// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4BaseAnimInstance.h"

#include "Public/T4EngineDefinitions.h" // #39

#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h" // #39

#include "Animation/BlendSpace.h"
#include "Animation/AnimSequence.h"

#include "T4EngineInternal.h"

/**
  * https://docs.unrealengine.com/ko-kr/Engine/Animation
 */
UT4BaseAnimInstance::UT4BaseAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, AnimSetAsset(nullptr) // #39
	, bPaused(false) // #63
	, TimeScale(1.0f) // #102
#if WITH_EDITOR
	, EditorDynamicMontage(nullptr) // #71
#endif
{
	OnMontageEnded.AddDynamic(this, &UT4BaseAnimInstance::HandleOnAnimMontageEnded);
}

UT4BaseAnimInstance::~UT4BaseAnimInstance()
{
}

void UT4BaseAnimInstance::OnReset()
{
	// #38
	AnimMontages.Empty();
	BlendSpaces.Empty();
	Reset();
	bPaused = false; // #63
	TimeScale = 1.0f;// #102
#if WITH_EDITOR
	if (nullptr != EditorDynamicMontage) // #71
	{
		EditorDynamicMontage->RemoveFromRoot();
		EditorDynamicMontage = nullptr;
	}
#endif
	PlayAnimInstanceInfoMap.Empty(); // #102
}

void UT4BaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

bool UT4BaseAnimInstance::HandleNotify(const FAnimNotifyEvent& InAnimNotifyEvent)
{
	// #38
#if 0
	T4_LOG(
		Verbose,
		TEXT("AnimNotify EventName '%s'"),
		*(InAnimNotifyEvent.NotifyName.ToString())
	);
#endif
	if (T4Const_AnimNotifyFootStepName != InAnimNotifyEvent.NotifyName)
	{
		return false;
	}
	// #38 : hack!!!! Left/Right Trigger 를 설치하면 될 문제...
	FAnimNotifyEvent* AnimNotifyEvent = const_cast<FAnimNotifyEvent*>(&InAnimNotifyEvent);
	if (nullptr == AnimNotifyEvent)
	{
		return false;
	}
	const UAnimSequenceBase* LinkedSequence = AnimNotifyEvent->GetLinkedSequence();
	if (nullptr == LinkedSequence)
	{
		return false;
	}
	// 어셋은 왼발 => 오른발 순...
	const float CurrentTriggerTime = AnimNotifyEvent->GetTriggerTime();
	const float HalfSegmentLength = LinkedSequence->SequenceLength * 0.5f;
	if (CurrentTriggerTime < HalfSegmentLength)
	{
		OnAnimNotify.ExecuteIfBound(T4Const_AnimNotifyFootStepLeftName);
	}
	else
	{
		OnAnimNotify.ExecuteIfBound(T4Const_AnimNotifyFootStepRightName);
	}
	return true;
}

void UT4BaseAnimInstance::PreUpdateAnimation(float DeltaSeconds)
{
	Super::PreUpdateAnimation(DeltaSeconds);
}

bool UT4BaseAnimInstance::HasSection(
	const FName& InAnimMontageName,
	const FName& InSectionName
)
{
	if (!AnimMontages.Contains(InAnimMontageName))
	{
		return false;
	}
	UAnimMontage* SelectAnimMontage = AnimMontages[InAnimMontageName];
	check(nullptr != SelectAnimMontage);
	if (SelectAnimMontage->IsValidSectionName(InSectionName))
	{
		return true;
	}
	return false;
}

float UT4BaseAnimInstance::GetDurationSec(
	const FName& InAnimMontageName,
	const FName& InSectionName
)
{
	float SectionDurationSec = 0.0f;
	if (AnimMontages.Contains(InAnimMontageName))
	{
		UAnimMontage* SelectAnimMontage = AnimMontages[InAnimMontageName];
		check(nullptr != SelectAnimMontage);
		if (!GetSectionLength(SelectAnimMontage, InSectionName, SectionDurationSec))
		{
			return 0.0f;
		}
	}
	return SectionDurationSec;
}

bool UT4BaseAnimInstance::IsPlaying(const FName& InAnimMontageName) // #116
{
	if (AnimMontages.Contains(InAnimMontageName))
	{
		UAnimMontage* SelectAnimMontage = AnimMontages[InAnimMontageName];
		check(nullptr != SelectAnimMontage);
		if (Montage_IsPlaying(SelectAnimMontage))
		{
			return true;
		}
	}
	return false;
}

bool UT4BaseAnimInstance::IsPlaying(FT4AnimInstanceID InPlayInstanceID)
{
	FAnimMontageInstance* PlayMontangeInstance = GetMontageInstanceForID(InPlayInstanceID);
	if (nullptr == PlayMontangeInstance)
	{
		return false;
	}
	if (PlayMontangeInstance->IsPlaying())
	{
		return true;
	}
	return false;
}

bool UT4BaseAnimInstance::IsPlayingAndBlendOutStarted(FT4AnimInstanceID InPlayInstanceID)
{
	// #47 : elpased time 이 커서 MontageInstance 가 삭제될 수도 있기 때문에 true 를 리턴해주어야 한다.
	//       여기에 의존하고 있는 로직이 있을 수 있기 때문!
	FAnimMontageInstance* PlayMontangeInstance = GetMontageInstanceForID(InPlayInstanceID);
	if (nullptr == PlayMontangeInstance)
	{
		return true;
	}
	if (!PlayMontangeInstance->IsPlaying())
	{
		return true;
	}
	check(nullptr != PlayMontangeInstance->Montage);
	bool bResult = false;
	if (1.0f != PlayMontangeInstance->GetWeight() && 0.0f == PlayMontangeInstance->GetDesiredWeight())
	{
		// #47 : 명시적으로 BlendOut 시점(DesiredWeight == 0) 과 BlendedWeight 변경 시점(Weight != 1.0) 을 체크!!
		bResult = true;
	}
#if 0
	if (0.0f == PlayMontangeInstance->GetDesiredWeight())
	{
		T4_LOG(
			Verbose,
			TEXT("[bBlendOut:%s] IsPlayingAndBlendOutStarted : BlendTime (%.2f), BlendedValue (%.2f), DesiredValue (%.2f)"),
			(bResult) ? TEXT("True") : TEXT("False"),
			PlayMontangeInstance->GetBlendTime(),
			PlayMontangeInstance->GetWeight(),
			PlayMontangeInstance->GetDesiredWeight()
		);
	}
#endif
	return bResult;
}

bool UT4BaseAnimInstance::StopAnimation(
	const FName& InAnimMontageName,
	float InBlendOutTimeSec
)
{
	UAnimMontage* SelectAnimMontage = nullptr;
	if (AnimMontages.Contains(InAnimMontageName))
	{
		SelectAnimMontage = AnimMontages[InAnimMontageName];
	}
	else
	{
		T4_LOG(
			Error,
			TEXT("Unknown anim montage type '%s'"),
			*(InAnimMontageName.ToString())
		);
	}
	if (nullptr == SelectAnimMontage)
	{
		return false;
	}
	if (!Montage_IsPlaying(SelectAnimMontage))
	{
		return false;
	}
	Montage_Stop(InBlendOutTimeSec, SelectAnimMontage);
	return true;
}

bool UT4BaseAnimInstance::StopAnimation(
	FT4AnimInstanceID InPlayInstanceID,
	float InBlendOutTimeSec
)
{
	// #47
	FAnimMontageInstance* PlayMontangeInstance = GetMontageInstanceForID(InPlayInstanceID);
	if (nullptr == PlayMontangeInstance)
	{
		return true;
	}
	PlayMontangeInstance->Stop(
		FAlphaBlend(PlayMontangeInstance->Montage->BlendOut, InBlendOutTimeSec)
	);
	return true;
}

FT4AnimInstanceID UT4BaseAnimInstance::PlayAnimation(
	const FT4AnimParameters& InAnimParameters
)
{
	if (InAnimParameters.SectionName == NAME_None)
	{
		return INDEX_NONE;
	}
	UAnimMontage* SelectAnimMontage = nullptr;
	if (AnimMontages.Contains(InAnimParameters.AnimMontageName))
	{
		SelectAnimMontage = AnimMontages[InAnimParameters.AnimMontageName];
	}
	else
	{
		T4_LOG(
			Error,
			TEXT("Unknown Anim Montage Type '%s'"),
			*(InAnimParameters.AnimMontageName.ToString())
		);
	}
	if (nullptr == SelectAnimMontage)
	{
		return INDEX_NONE;
	}
	if (!SelectAnimMontage->IsValidSectionName(InAnimParameters.SectionName))
	{
		return INDEX_NONE;
	}
	if (Montage_IsPlaying(SelectAnimMontage))
	{
		Montage_Stop(InAnimParameters.BlendInTimeSec, SelectAnimMontage);
	}
	FT4AnimInstanceID MontageInstanceID = PlayAnimationInternal(
		SelectAnimMontage,
		InAnimParameters,
		*(InAnimParameters.AnimMontageName.ToString())
	);
	return MontageInstanceID;
}

#if WITH_EDITOR
bool UT4BaseAnimInstance::PlayAnimation(
	UAnimSequence* InPlayAnimSequence,
	float InPlayRate,
	float InBlendInTimeSec,
	float InBlendOutTimeSec
)
{
	// #39
	check(nullptr != InPlayAnimSequence);
	if (nullptr != EditorDynamicMontage) // #71
	{
		FAnimMontageInstance* NewMontageInstance = GetActiveInstanceForMontage(EditorDynamicMontage);
		if (nullptr != NewMontageInstance)
		{
			NewMontageInstance->Stop(FAlphaBlend(NewMontageInstance->Montage->BlendOut, InBlendOutTimeSec));
		}
		EditorDynamicMontage->RemoveFromRoot();
		EditorDynamicMontage = nullptr;
	}
	UAnimMontage* NewDynamicMontage = PlaySlotAnimationAsDynamicMontage(
		InPlayAnimSequence,
		FName(TEXT("SkillSlot")), // FAnimSlotGroup::DefaultSlotName, #48
		InBlendInTimeSec,
		InBlendOutTimeSec,
		InPlayRate, // #102 : 일단, 툴용이라 TimeScale 은 제외!
		1
	);
	if (nullptr != NewDynamicMontage) // #71
	{
		EditorDynamicMontage = NewDynamicMontage;
		EditorDynamicMontage->AddToRoot();
	}
	return (nullptr != NewDynamicMontage) ? true : false;
}
#endif

#if !UE_BUILD_SHIPPING
void UT4BaseAnimInstance::DebugPauseAnimation(
	FT4AnimInstanceID InPlayInstanceID,
	bool bPause
) // #54
{
	FAnimMontageInstance* PlayMontangeInstance = GetMontageInstanceForID(InPlayInstanceID);
	if (nullptr == PlayMontangeInstance)
	{
		return;
	}
	if (bPause)
	{
		PlayMontangeInstance->Pause();
	}
	else
	{
		PlayMontangeInstance->Play();
	}
}
#endif

void UT4BaseAnimInstance::SetPause(bool bInPause) // #63
{
	for (TMap<UAnimMontage*, FAnimMontageInstance*>::TConstIterator It(ActiveMontagesMap); It; ++It)
	{
		FAnimMontageInstance* ActiveMontageInstance = It.Value();
		check(nullptr != ActiveMontageInstance);
		if (bInPause)
		{
			ActiveMontageInstance->Pause();
		}
		else
		{
			ActiveMontageInstance->Play();
		}
	}
	bPaused = bInPause;
}

void UT4BaseAnimInstance::SetTimeScale(float InTimeScale) // #102
{
	for (TMap<UAnimMontage*, FAnimMontageInstance*>::TConstIterator It(ActiveMontagesMap); It; ++It)
	{
		FAnimMontageInstance* ActiveMontageInstance = It.Value();
		check(nullptr != ActiveMontageInstance);
		FT4AnimInstanceID AnimInstanceID = ActiveMontageInstance->GetInstanceID();
		if (PlayAnimInstanceInfoMap.Contains(AnimInstanceID))
		{
			const FT4PlayAnimInstanceInfo& AnimInstanceInfo = PlayAnimInstanceInfoMap[AnimInstanceID];
			const float FinalPlayRate = (0.0f < InTimeScale) ? AnimInstanceInfo.PlayRate * InTimeScale : 0.0f;
			ActiveMontageInstance->SetPlayRate(FinalPlayRate);
		}
	}
	TimeScale = InTimeScale;
}

void UT4BaseAnimInstance::SetAnimSetAsset(UT4AnimSetAsset* InAnimSetAsset)
{ 
	// #39, #38
	check(nullptr != InAnimSetAsset);
	AnimSetAsset = InAnimSetAsset; 
}

void UT4BaseAnimInstance::AddAnimMontage(const FName& InName, UAnimMontage* InAnimMontage)
{
	// #39, #38, #69
	check(nullptr != InAnimMontage);
	check(!AnimMontages.Contains(InName));
	AnimMontages.Add(InName, InAnimMontage);
}

void UT4BaseAnimInstance::AddBlendSpace(const FName& InName, UBlendSpaceBase* InBlendSpace)
{
	// #39, #38
	check(nullptr != InBlendSpace);
	check(!BlendSpaces.Contains(InName));
	BlendSpaces.Add(InName, InBlendSpace);
}

FT4AnimInstanceID UT4BaseAnimInstance::PlayAnimationInternal(
	UAnimMontage* InMontageToPlay, 
	const FT4AnimParameters& InAnimParameters,
	const TCHAR* InDebugString
)
{
	check(nullptr != InMontageToPlay);
	check(InAnimParameters.SectionName != NAME_None);
	InMontageToPlay->BlendIn.SetBlendTime(InAnimParameters.BlendInTimeSec);
	InMontageToPlay->BlendOut.SetBlendTime(InAnimParameters.BlendOutTimeSec);
	const float FinalPlayRate = InAnimParameters.PlayRate;
	const float PlayMontageLength = Montage_Play(
		InMontageToPlay,
		FinalPlayRate,
		EMontagePlayReturnType::MontageLength
	);
	bool bPlayedSuccessfully = (PlayMontageLength > 0.f);
	if (!bPlayedSuccessfully)
	{
		T4_LOG(
			Error,
			TEXT("%s : Failed to Play Anim Montage."),
			InDebugString
		);
		return INDEX_NONE;
	}
	FAnimMontageInstance* NewMontageInstance = GetActiveInstanceForMontage(InMontageToPlay);
	if (nullptr == NewMontageInstance)
	{
		T4_LOG(
			Error,
			TEXT("%s : Anim Montage Instance not found"),
			InDebugString
		);
		return INDEX_NONE;
	}
	FT4AnimInstanceID NewMontageInstanceID = NewMontageInstance->GetInstanceID();
	if (INDEX_NONE != NewMontageInstanceID)
	{
		bool const bEndOfSection = (NewMontageInstance->GetPlayRate() < 0.f);
		NewMontageInstance->JumpToSectionName(InAnimParameters.SectionName, bEndOfSection);
		if (0.0f < InAnimParameters.OffsetTimeSec) // #56
		{
			float CurrentPosition = NewMontageInstance->GetPosition();
			NewMontageInstance->SetPosition(CurrentPosition + InAnimParameters.OffsetTimeSec);
		}
	}

	if (!PlayAnimInstanceInfoMap.Contains(NewMontageInstanceID)) // #102
	{
		FT4PlayAnimInstanceInfo NewAnimInstanceInfo;
		NewAnimInstanceInfo.AnimInstanceID = NewMontageInstanceID;
		NewAnimInstanceInfo.PlayRate = InAnimParameters.PlayRate;
		PlayAnimInstanceInfoMap.Add(NewMontageInstanceID, NewAnimInstanceInfo);
	}
	else
	{
		FT4PlayAnimInstanceInfo& Info = PlayAnimInstanceInfoMap[NewMontageInstanceID];
		Info.PlayRate = InAnimParameters.PlayRate;
	}

	return NewMontageInstanceID;
}

bool UT4BaseAnimInstance::GetSectionLength(
	UAnimMontage* InMontage, 
	const FName& InSectionName, 
	float& OutLength
)
{
	if (nullptr == InMontage)
	{
		return false;
	}
	int32 SectionIndex = InMontage->GetSectionIndex(InSectionName);
	if (INDEX_NONE == SectionIndex)
	{
		return false;
	}
	OutLength = InMontage->GetSectionLength(SectionIndex);
	return true;
}

void UT4BaseAnimInstance::HandleOnAnimMontageEnded(UAnimMontage* InMontage, bool bInterrupted)
{
}