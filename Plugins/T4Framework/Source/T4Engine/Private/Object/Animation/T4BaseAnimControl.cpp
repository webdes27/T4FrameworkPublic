// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4BaseAnimControl.h"

#include "T4ItemAnimControl.h" // #107
#include "T4AnimalAnimControl.h" // #38
#include "T4HumanAnimControl.h" // #38

#include "AnimState/T4BaseAnimStates.h" // #47

#include "Object/Animation/AnimInstance/T4BaseAnimInstance.h"
#include "Object/Animation/AnimInstance/T4BaseAnimVariables.h"

#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4BaseAnimControl::FT4BaseAnimControl(AT4GameObject* InGameObject)
	: LayerType(ET4LayerType::Max)
	, bBegunPlay(false)
	, OwnerObjectPtr(InGameObject)
	, ActiveAnimStateName(NAME_None) // #47
	, NextAnimStateName(NAME_None) // #47
	, PendingAnimStateName(NAME_None) // #47
{
	check(nullptr != InGameObject);
	LayerType = InGameObject->GetLayerType();
	check(ET4LayerType::Max != LayerType);
}

FT4BaseAnimControl::~FT4BaseAnimControl()
{
	OnReset();
}

// #47
void FT4BaseAnimControl::OnAutoRegisterAnimStates()
{
	AutoRegisterAnimStates();
	IT4AnimState* ActiveAnimState = GetActiveAnimState();
	if (nullptr != ActiveAnimState)
	{
		ActiveAnimState->OnEnter();
	}
}

void FT4BaseAnimControl::AutoRegisterAnimStates()
{
	RegisterAnimState(T4Const_EmptyAnimStateName, new FT4EmptyAnimState());
	RegisterAnimState(T4Const_ErrorAnimStateName, new FT4ErrorAnimState());
	ActiveAnimStateName = T4Const_EmptyAnimStateName;
}

void FT4BaseAnimControl::RemoveAnimStates()
{
	for (TMap<FName, IT4AnimState*>::TIterator It(AnimStates); It; ++It)
	{
		delete It->Value;
	}
	AnimStates.Empty();
}

bool FT4BaseAnimControl::ChangeNextAnimState(const FName& InAnimStateName)
{
	if (!AnimStates.Contains(InAnimStateName))
	{
		return false;
	}
	NextAnimStateName = InAnimStateName;
	return true;
}

FT4StateAnimVariables* FT4BaseAnimControl::GetStateAnimVariables()
{
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	if (nullptr == AnimInstance)
	{
		return nullptr;
	}
	return AnimInstance->GetStateAnimVariables();
}

FT4MovementAnimVariables* FT4BaseAnimControl::GetMovementAnimVariables()
{
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	if (nullptr == AnimInstance)
	{
		return nullptr;
	}
	return AnimInstance->GetMovementAnimVariables();
}

const FName FT4BaseAnimControl::GetActiveAnimStateName() const
{
	return ActiveAnimStateName;
}

IT4AnimState* FT4BaseAnimControl::GetActiveAnimState()
{
	if (ActiveAnimStateName == NAME_None)
	{
		return nullptr;
	}
	if (!AnimStates.Contains(ActiveAnimStateName))
	{
		return nullptr;
	}
	return AnimStates[ActiveAnimStateName];
}

const IT4AnimState* FT4BaseAnimControl::GetActiveAnimState() const
{
	if (ActiveAnimStateName == NAME_None)
	{
		return nullptr;
	}
	if (!AnimStates.Contains(ActiveAnimStateName))
	{
		return nullptr;
	}
	return AnimStates[ActiveAnimStateName];
}

IT4AnimState* FT4BaseAnimControl::GetPendingAnimState()
{
	if (PendingAnimStateName == NAME_None)
	{
		return nullptr;
	}
	if (!AnimStates.Contains(PendingAnimStateName))
	{
		return nullptr;
	}
	return AnimStates[PendingAnimStateName];
}

const IT4AnimState* FT4BaseAnimControl::GetPendingAnimState() const
{
	if (PendingAnimStateName == NAME_None)
	{
		return nullptr;
	}
	if (!AnimStates.Contains(PendingAnimStateName))
	{
		return nullptr;
	}
	return AnimStates[PendingAnimStateName];
}

IT4AnimState* FT4BaseAnimControl::GetNextAnimState()
{
	if (NextAnimStateName == NAME_None)
	{
		return nullptr;
	}
	if (!AnimStates.Contains(NextAnimStateName))
	{
		return nullptr;
	}
	return AnimStates[NextAnimStateName];
}

bool FT4BaseAnimControl::TryChangeAnimState(
	const FName& InAnimStateName,
	bool bInCheckPriorityActiveState,
	bool bInCheckPriorityPendingActiveState
)
{
	if (!AnimStates.Contains(InAnimStateName))
	{
		T4_LOG(
			Verbose,
			TEXT("AnimState '%s' is  not found"),
			*(InAnimStateName.ToString())
		);
		return false;
	}
	if (bInCheckPriorityActiveState)
	{
		const IT4AnimState* ActiveAnimState = GetActiveAnimState();
		if (nullptr != ActiveAnimState)
		{
			const IT4AnimState* NewAnimState = AnimStates[InAnimStateName];
			check(nullptr != NewAnimState);
			const ET4AnimStatePriority& ActivePriority = ActiveAnimState->GetPriority();
			const ET4AnimStatePriority& NewPriority = NewAnimState->GetPriority();
			if (ActivePriority < NewPriority)
			{
				T4_LOG(
					Verbose,
					TEXT("Unable to Change AnimState '%s(%u)'. Higher Priority ActiveAnimState '%s(%u)'."),
					*(InAnimStateName.ToString()),
					uint8(NewPriority),
					*(ActiveAnimState->GetName().ToString()),
					uint8(ActivePriority)
				);
				return false;
			}
		}
	}
	if (bInCheckPriorityPendingActiveState)
	{
		const IT4AnimState* PendingAnimState = GetPendingAnimState();
		if (nullptr != PendingAnimState)
		{
			const IT4AnimState* NewAnimState = AnimStates[InAnimStateName];
			check(nullptr != NewAnimState);
			const ET4AnimStatePriority& PendingPriority = PendingAnimState->GetPriority();
			const ET4AnimStatePriority& NewPriority = NewAnimState->GetPriority();
			if (PendingPriority < NewPriority)
			{
				T4_LOG(
					Verbose,
					TEXT("Unable to Change AnimState '%s(%u)'. Higher Priority PendingAnimState '%s(%u)'."),
					*(InAnimStateName.ToString()),
					uint8(NewPriority),
					*(PendingAnimState->GetName().ToString()),
					uint8(PendingPriority)
				);
				return false;
			}
		}
	}
	PendingAnimStateName = InAnimStateName;
	return true;
}

void FT4BaseAnimControl::RegisterAnimState(
	const FName& InAnimStateName, 
	IT4AnimState* InAnimState
)
{
	check(nullptr != InAnimState);
	if (AnimStates.Contains(InAnimStateName))
	{
		T4_LOG(
			Warning,
			TEXT("Already Registered AnimState '%s'."),
			*(InAnimStateName.ToString())
		);
		return;
	}
	AnimStates.Add(InAnimStateName, InAnimState);
}

void FT4BaseAnimControl::UnregisterAnimState(
	const FName& InAnimStateName
)
{
	if (!AnimStates.Contains(InAnimStateName))
	{
		T4_LOG(
			Warning,
			TEXT("AnimState '%s' is  Not found"),
			*(InAnimStateName.ToString())
		);
		return;
	}
	IT4AnimState* RemoveAnimState = AnimStates[InAnimStateName];
	check(nullptr != RemoveAnimState);
	delete RemoveAnimState;
	AnimStates.Remove(InAnimStateName);
}
// ~#47

bool FT4BaseAnimControl::HasSection(
	const FName& InAnimMontageName,
	const FName& InSectionName
)
{
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	if (nullptr == AnimInstance)
	{
		return false;
	}
	return AnimInstance->HasSection(InAnimMontageName, InSectionName);
}

float FT4BaseAnimControl::GetDurationSec(
	const FName& InAnimMontageName,
	const FName& InSectionName
)
{
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	if (nullptr == AnimInstance)
	{
		return 0.0f;
	}
	return AnimInstance->GetDurationSec(InAnimMontageName, InSectionName);
}

bool FT4BaseAnimControl::IsPlayingAnimation(
	const FName& InAnimMontageName
) // #116
{
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	if (nullptr == AnimInstance)
	{
		return false;
	}
	return AnimInstance->IsPlaying(InAnimMontageName);
}

bool FT4BaseAnimControl::IsPlayingAnimation(
	FT4AnimInstanceID InPlayInstanceID
)
{
	if (INDEX_NONE == InPlayInstanceID)
	{
		return false;
	}
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	if (nullptr == AnimInstance)
	{
		return false;
	}
	return AnimInstance->IsPlaying(InPlayInstanceID);
}

bool FT4BaseAnimControl::IsPlayingAndBlendOutStarted(
	FT4AnimInstanceID InPlayInstanceID
)
{
	if (INDEX_NONE == InPlayInstanceID)
	{
		return false;
	}
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	if (nullptr == AnimInstance)
	{
		return false;
	}
	return AnimInstance->IsPlayingAndBlendOutStarted(InPlayInstanceID);
}

FT4AnimInstanceID FT4BaseAnimControl::PlayAnimation(
	const FT4AnimParameters& InAnimParameters
)
{
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	if (nullptr == AnimInstance)
	{
		return INDEX_NONE;
	}
	FT4AnimInstanceID ReturnInstanceID = AnimInstance->PlayAnimation(InAnimParameters);
	if (INDEX_NONE != ReturnInstanceID)
	{
		NotifyPlayAnimation(InAnimParameters); // #47
	}
	return ReturnInstanceID;
}

bool FT4BaseAnimControl::StopAnimation(
	const FName& InAnimMontageName, 
	float InBlendOutTimeSec
)
{
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	if (nullptr == AnimInstance)
	{
		return false;
	}
	return AnimInstance->StopAnimation(InAnimMontageName, InBlendOutTimeSec);
}

bool FT4BaseAnimControl::StopAnimation(
	FT4AnimInstanceID InPlayInstanceID, 
	float InBlendOutTimeSec
)
{
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	if (nullptr == AnimInstance)
	{
		return false;
	}
	return AnimInstance->StopAnimation(InPlayInstanceID, InBlendOutTimeSec);
}

#if !UE_BUILD_SHIPPING
void FT4BaseAnimControl::DebugPauseAnimation(
	FT4AnimInstanceID InPlayInstanceID, 
	bool bPause
) // #54
{
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	if (nullptr == AnimInstance)
	{
		return;
	}
	return AnimInstance->DebugPauseAnimation(InPlayInstanceID, bPause);
}
#endif

#if WITH_EDITOR
bool FT4BaseAnimControl::EditorPlayAnimation(
	UAnimSequence* InPlayAnimSequence,
	float InPlayRate,
	float InBlendInTimeSec,
	float InBlendOutTimeSec
)
{
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	if (nullptr == AnimInstance)
	{
		return false;
	}
	return AnimInstance->PlayAnimation(
		InPlayAnimSequence, 
		InPlayRate,
		InBlendInTimeSec,
		InBlendOutTimeSec
	);
}
#endif

FT4BaseAnimControl* FT4BaseAnimControl::CreateNewControl(
	AT4GameObject* InGameObject,
	ET4AnimInstance InAnimInstanceType
)
{
	switch (InAnimInstanceType)
	{
		case ET4AnimInstance::Human_Basic: // #38
			return new FT4HumanAnimControl(InGameObject);

		case ET4AnimInstance::Animal_Basic:
			return new FT4AnimalAnimControl(InGameObject);

		case ET4AnimInstance::Item_Basic: // #107
			return new FT4ItemAnimControl(InGameObject);

		default:
			{
				T4_LOG(
					Error,
					TEXT("[SL:%u] Unknown anim instance type '%u'"),
					uint32(InGameObject->GetLayerType()),
					uint32(InAnimInstanceType)
				);
			}
			break;
	}
	return nullptr;
}

void FT4BaseAnimControl::OnReset()
{
	// #38
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	if (nullptr != AnimInstance)
	{
		AnimInstance->OnReset();
	}
	Reset();
	// #47
	ActiveAnimStateName = NAME_None; 
	NextAnimStateName = NAME_None;
	PendingAnimStateName = NAME_None;
	RemoveAnimStates();
	OnAutoRegisterAnimStates();
	// ~#47
}

void FT4BaseAnimControl::OnAdvance(const FT4UpdateTime& InUpdateTime)
{
	if (!bBegunPlay)
	{
		return;
	}
	if (!IsLoaded())
	{
		return;
	}
	Advance(InUpdateTime);
	AdvanceAnimState(InUpdateTime); // #47
}

void FT4BaseAnimControl::ProcessPendingAnimState() // #47
{
	if (PendingAnimStateName == NAME_None)
	{
		return;
	}
	IT4AnimState* PendingAnimState = GetPendingAnimState();
	if (nullptr != PendingAnimState)
	{
		IT4AnimState* NextAnimState = GetNextAnimState();
		const ET4AnimStatePriority& PendingPriority = PendingAnimState->GetPriority();
		const ET4AnimStatePriority& NextPriority 
			= (nullptr != NextAnimState) ? NextAnimState->GetPriority() : ET4AnimStatePriority::AnimPriority_Low;
		if (PendingPriority > NextPriority)
		{
			T4_LOG(
				Verbose,
				TEXT("Unable to Change AnimState '%s(%u)'. Pending AnimState '%s(%u)'"),
				*(PendingAnimState->GetName().ToString()),
				uint8(PendingPriority),
				(nullptr != NextAnimState) ? *(NextAnimState->GetName().ToString()) : TEXT("No NextState"),
				uint8(NextPriority)
			);
			// WARN : 자동 전이될 NextAnimStateName 보다 priority 가 낮기 때문에 Pending State 를 취소한다.
			PendingAnimStateName = NAME_None;
			return;
		}
		if (nullptr != NextAnimState)
		{
			NextAnimStateName = NAME_None; // #47 : Pending AnimState 의 우선순위 높음으로 취소해준다.
		}
	}
	IT4AnimState* ActiveAnimState = GetActiveAnimState();
	check(nullptr != ActiveAnimState);
	check(nullptr != PendingAnimState);
	ActiveAnimState->OnLeave();
	ActiveAnimStateName = PendingAnimStateName;
	PendingAnimState->OnEnter();
	PendingAnimStateName = NAME_None;
}

void FT4BaseAnimControl::AdvanceAnimState(const FT4UpdateTime& InUpdateTime) // #47
{
	ProcessPendingAnimState();

	IT4AnimState* ActiveAnimState = GetActiveAnimState();
	if (nullptr != ActiveAnimState)
	{
		ActiveAnimState->OnUpdate(InUpdateTime);
		while (NextAnimStateName != NAME_None)
		{
			// #47 : AnimState 내에서 전이 될 경우 NextAnimStateName 이 없을때까지 재귀한다.
			ActiveAnimState->OnLeave();
			ActiveAnimStateName = NextAnimStateName;
			NextAnimStateName = NAME_None;
			ActiveAnimState = GetActiveAnimState();
			check(nullptr != ActiveAnimState);
			ActiveAnimState->OnEnter();
		}
	}
}

UT4BaseAnimInstance* FT4BaseAnimControl::GetAnimInstance()
{
	if (!OwnerObjectPtr.IsValid())
	{
		return nullptr;
	}
	return OwnerObjectPtr->GetAnimInstance();
}

bool FT4BaseAnimControl::IsLoaded() const
{
	if (!OwnerObjectPtr.IsValid())
	{
		return false;
	}
	return OwnerObjectPtr->IsLoaded();
}

bool FT4BaseAnimControl::QueryIKLineTrace(
	const FVector& InStartLocation,
	const FVector& InEndLocation,
	const FCollisionQueryParams& InCollisionQueryParams,
	FVector& OutLocation
)
{
	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);

	IT4WorldCollisionSystem* CollisionSystem = GameWorld->GetCollisionSystem();
	check(nullptr != CollisionSystem);

	FT4HitSingleResult HitResult;
	bool bResult = CollisionSystem->QueryLineTraceSingle(
		ET4CollisionChannel::CollisionVisibility,
		InStartLocation,
		InEndLocation,
		InCollisionQueryParams,
		HitResult
	);
	if (!bResult)
	{
		return false;
	}
	OutLocation = HitResult.ResultLocation;
	return true;
}

bool FT4BaseAnimControl::HasAnimState(const FName& InAnimStateName) const // #48
{
	if (!AnimStates.Contains(InAnimStateName))
	{
		return false;
	}
	return true;
}

IT4GameWorld* FT4BaseAnimControl::GetGameWorld() const
{
	return T4EngineWorldGet(LayerType);
}
