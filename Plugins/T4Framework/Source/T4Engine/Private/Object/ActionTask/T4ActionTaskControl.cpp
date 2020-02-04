// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionTaskControl.h"
#include "T4ActionTaskBase.h"

#include "Object/T4GameObject.h"

#include "Public/Action/T4ActionParameters.h" // #28

#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h" // #76

#include "Components/SkeletalMeshComponent.h"

#include "T4EngineInternal.h"

/**
  * #76
 */
FT4ActionTaskControl::FT4ActionTaskControl()
	: bPhysicsStarted(false)
	, ActionDieTask(this)
	, ActionResurrectTask(this)
	, ActionHitTask(this)
	, ActionStanceTask(this) // #111
	, ActionSubStanceTask(this) // #111
{
}

FT4ActionTaskControl::~FT4ActionTaskControl()
{
	Reset();
}

void FT4ActionTaskControl::Reset()
{
	FT4ActionNodeControl::Reset();
}

void FT4ActionTaskControl::Set(AT4GameObject* InGameObject)
{
	FT4ActionNodeControl::Set(InGameObject);
}

void FT4ActionTaskControl::Advance(const FT4UpdateTime& InUpdateTime)
{
	ActionDieTask.OnAdvance(InUpdateTime);
	ActionResurrectTask.OnAdvance(InUpdateTime);
	ActionHitTask.OnAdvance(InUpdateTime);
	ActionStanceTask.OnAdvance(InUpdateTime); // #111
	ActionSubStanceTask.OnAdvance(InUpdateTime); // #111

	FT4ActionNodeControl::Advance(InUpdateTime);
}

bool FT4ActionTaskControl::IsPending(ET4ActionType InActionType) // #116
{
	switch (InActionType)
	{
		case ET4ActionType::Hit: // #76
			return ActionHitTask.IsPending();

		case ET4ActionType::Die: // #76
			return ActionDieTask.IsPending();

		case ET4ActionType::Resurrect: // #76
			return ActionResurrectTask.IsPending();

		case ET4ActionType::Stance: // #111
			return ActionStanceTask.IsPending();

		case ET4ActionType::SubStance: // #111
			return ActionSubStanceTask.IsPending();

		default:
			{
				T4_LOG(
					Error,
					TEXT("No implementation Action '%u'"),
					uint8(InActionType)
				);
			}
			break;
	};
	return false;
}

void FT4ActionTaskControl::Flush(ET4ActionType InActionType) // #111
{
	switch (InActionType)
	{
		case ET4ActionType::Hit: // #76
			ActionHitTask.Flush();
			break;

		case ET4ActionType::Die: // #76
			ActionDieTask.Flush();
			break;

		case ET4ActionType::Resurrect: // #76
			ActionResurrectTask.Flush();
			break;

		case ET4ActionType::Stance: // #111
			ActionStanceTask.Flush();
			break;

		case ET4ActionType::SubStance: // #111
			ActionSubStanceTask.Flush();
			break;

		default:
			{
				T4_LOG(
					Error,
					TEXT("No implementation Action '%u'"),
					uint8(InActionType)
				);
			}
			break;
	};
}

bool FT4ActionTaskControl::Play(const FT4ActionCommand* InAction)
{
	bool bResult = false;

	switch (InAction->ActionType)
	{
		case ET4ActionType::Hit: // #76
			{
				if (ActionDieTask.IsPlaying() || ActionResurrectTask.IsPlaying())
				{
					return false;
				}
				bResult = ActionHitTask.Bind(*(static_cast<const FT4HitAction*>(InAction)));
			}
			break;

		case ET4ActionType::Die: // #76
			{
				if (ActionHitTask.IsPlaying())
				{
					ActionHitTask.StopAll();
				}
				if (ActionResurrectTask.IsPlaying())
				{
					ActionResurrectTask.StopAll();
				}
				bResult = ActionDieTask.Bind(*(static_cast<const FT4DieAction*>(InAction)));
			}
			break;

		case ET4ActionType::Resurrect: // #76
			{
				if (ActionDieTask.IsPlaying())
				{
					ActionDieTask.StopAll();
				}
				bResult = ActionResurrectTask.Bind(*(static_cast<const FT4ResurrectAction*>(InAction)));
			}
			break;

		case ET4ActionType::Stance: // #111
			{
				bResult = ActionStanceTask.Bind(*(static_cast<const FT4StanceAction*>(InAction)));
			}
			break;

		case ET4ActionType::SubStance: // #111
			{
				bResult = ActionSubStanceTask.Bind(*(static_cast<const FT4SubStanceAction*>(InAction)));
			}
			break;

		default:
			{
				T4_LOG(
					Error,
					TEXT("No implementation Action '%s'"),
					*(InAction->ToString())
				);
			}
			break;
	};
	   
	return bResult;
}

bool FT4ActionTaskControl::PhysicsStart(
	const FT4EntityCharacterReactionPhysicsStartData* InPhysicsData,
	const FVector& InShotDirection
)
{
	check(nullptr != InPhysicsData);
	
	// #76 : 중첩 처리가 가능하다. 단, 분할 시뮬레에션 처리는 되지 않을 것이다. 우선순위가 낮다.

	// FPhysicsAssetEditorSharedData::EnableSimulation(bool bEnableSimulation)

	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);

	bool bResult = OwnerGameObject->StartPhysics(InPhysicsData->bSimulateBodiesBelow); // Compmnent Collision 제어!
	if (!bResult)
	{
		return false;
	}

	USkeletalMeshComponent* SkeletonMeshComponent = GetSkeletonMeshComponent();
	if (nullptr == SkeletonMeshComponent)
	{
		return false;
	}

	FName PhysicsImpulseSubActionPoint = InPhysicsData->ImpulseSubActionPoint;
	if (InPhysicsData->ImpulseMainActionPoint == PhysicsImpulseSubActionPoint)
	{
		PhysicsImpulseSubActionPoint = NAME_None;
	}

	FVector PhysicsImpulseVector;
	if (InShotDirection.IsNearlyZero())
	{
		PhysicsImpulseVector = -OwnerGameObject->GetFrontVector() * InPhysicsData->ImpulsePower;
	}
	else
	{
		PhysicsImpulseVector = InShotDirection;
		PhysicsImpulseVector.Normalize();
		PhysicsImpulseVector *= InPhysicsData->ImpulsePower;
	}

	if (!InPhysicsData->bSimulateBodiesBelow)
	{
		SkeletonMeshComponent->SetAllBodiesSimulatePhysics(true);
		SkeletonMeshComponent->SetSimulatePhysics(true);
	}
	else
	{
		SkeletonMeshComponent->SetAllBodiesBelowSimulatePhysics(InPhysicsData->ImpulseMainActionPoint, true);
		if (PhysicsImpulseSubActionPoint != NAME_None)
		{
			SkeletonMeshComponent->SetAllBodiesBelowSimulatePhysics(PhysicsImpulseSubActionPoint, true);
		}
	}

	SkeletonMeshComponent->WakeAllRigidBodies(); // Make it start simulating

	SkeletonMeshComponent->SetMassOverrideInKg(InPhysicsData->ImpulseMainActionPoint, InPhysicsData->MassOverrideInKg, true);
	SkeletonMeshComponent->SetCenterOfMass(InPhysicsData->CenterOfMass, InPhysicsData->ImpulseMainActionPoint);
	SkeletonMeshComponent->AddImpulse(PhysicsImpulseVector, InPhysicsData->ImpulseMainActionPoint);

	if (PhysicsImpulseSubActionPoint != NAME_None)
	{
		SkeletonMeshComponent->SetMassOverrideInKg(PhysicsImpulseSubActionPoint, InPhysicsData->MassOverrideInKg, true);
		SkeletonMeshComponent->SetCenterOfMass(InPhysicsData->CenterOfMass, PhysicsImpulseSubActionPoint);
		SkeletonMeshComponent->AddImpulse(PhysicsImpulseVector, PhysicsImpulseSubActionPoint);
	}

	const FT4EntityCharacterReactionPhysicsBlendData& BlendData = InPhysicsData->BlendData;
	if (!InPhysicsData->bSimulateBodiesBelow)
	{
		SkeletonMeshComponent->SetPhysicsBlendWeight(BlendData.TargetWeight);
	}
	else
	{
		const float ApplyWeight = (0.0f < InPhysicsData->BlendData.BlendInTimeSec) ? 0.0f : BlendData.TargetWeight;
		SkeletonMeshComponent->SetAllBodiesBelowPhysicsBlendWeight(
			InPhysicsData->ImpulseMainActionPoint,
			ApplyWeight
		);
		if (PhysicsImpulseSubActionPoint != NAME_None)
		{
			SkeletonMeshComponent->SetAllBodiesBelowPhysicsBlendWeight(
				PhysicsImpulseSubActionPoint,
				ApplyWeight
			);
		}
	}

	bPhysicsStarted = true;
	return true;
}

bool FT4ActionTaskControl::PhysicsEnd()
{
	if (!bPhysicsStarted)
	{
		// todo : 이미 동작중일 경우에 대한 처리...
		return true;
	}

	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);

	OwnerObject->StopPhysics(); // Compmnent Collision 제어!

	bPhysicsStarted = false;
	return true;
}

bool FT4ActionTaskControl::PhysicsStop(
	const FT4EntityCharacterReactionPhysicsStopData* InPhysicsData
)
{
	if (!bPhysicsStarted)
	{
		// todo : 이미 동작중일 경우에 대한 처리...
		return true;
	}

	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);

	OwnerObject->StopPhysics(); // Compmnent Collision 제어!

	bPhysicsStarted = false;
	return true;
}

USkeletalMeshComponent* FT4ActionTaskControl::GetSkeletonMeshComponent() const
{
	AT4GameObject* OwnerObject = GetGameObject();
	if (nullptr == OwnerObject)
	{
		return nullptr;
	}
	return OwnerObject->GetSkeletalMeshComponent();
}

#if WITH_EDITOR
void FT4ActionTaskControl::EditorRestoreReaction() // #76
{
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);

	if (ActionDieTask.IsPlaying())
	{
		ActionDieTask.StopAll();
	}
	if (ActionResurrectTask.IsPlaying())
	{
		ActionResurrectTask.StopAll();
	}
	if (ActionHitTask.IsPlaying())
	{
		ActionHitTask.StopAll();
	}

	PhysicsEnd();
}
#endif