// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ManualControl.h"

#include "Object/T4GameObject.h"

#include "ActionNode/T4ActionNode.h"
#include "Public/Action/T4ActionParameters.h" // #28

#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h" // #76

#include "Components/SkeletalMeshComponent.h"

#include "T4EngineInternal.h"

/**
  * #76
 */
FT4ManualControl::FT4ManualControl()
	: bPhysicsStarted(false)
	, ManualDieNode(this)
	, ManualResurrectNode(this)
	, ManualHitNode(this)
{
}

FT4ManualControl::~FT4ManualControl()
{
	Reset();
}

void FT4ManualControl::Reset()
{
	FT4ActionControl::Reset();
}

void FT4ManualControl::Set(AT4GameObject* InGameObject)
{
	FT4ActionControl::Set(InGameObject);
}

void FT4ManualControl::Advance(const FT4UpdateTime& InUpdateTime)
{
	ManualDieNode.OnAdvance(InUpdateTime);
	ManualResurrectNode.OnAdvance(InUpdateTime);
	ManualHitNode.OnAdvance(InUpdateTime);

	FT4ActionControl::Advance(InUpdateTime);
}

bool FT4ManualControl::Play(const FT4ActionStruct* InAction)
{
	bool bResult = false;

	switch (InAction->ActionType)
	{
		case ET4ActionType::Hit: // #76
			{
				if (ManualDieNode.IsPlaying() || ManualResurrectNode.IsPlaying())
				{
					return false;
				}
				bResult = ManualHitNode.Play(*(static_cast<const FT4HitAction*>(InAction)));
			}
			break;

		case ET4ActionType::Die: // #76
			{
				if (ManualHitNode.IsPlaying())
				{
					ManualHitNode.StopAll();
				}
				if (ManualResurrectNode.IsPlaying())
				{
					ManualResurrectNode.StopAll();
				}
				bResult = ManualDieNode.Play(*(static_cast<const FT4DieAction*>(InAction)));
			}
			break;

		case ET4ActionType::Resurrect: // #76
			{
				if (ManualDieNode.IsPlaying())
				{
					ManualDieNode.StopAll();
				}
				bResult = ManualResurrectNode.Play(*(static_cast<const FT4ResurrectAction*>(InAction)));
			}
			break;

		default:
			{
				UE_LOG(
					LogT4Engine,
					Error,
					TEXT("FT4ManualControl::Play '%s' failed. no implementation."),
					*(InAction->ToString())
				);
			}
			break;
	};
	   
	return bResult;
}

bool FT4ManualControl::StartPhysics(
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

bool FT4ManualControl::EndPhysics()
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

bool FT4ManualControl::StopPhysics(
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

USkeletalMeshComponent* FT4ManualControl::GetSkeletonMeshComponent() const
{
	AT4GameObject* OwnerObject = GetGameObject();
	if (nullptr == OwnerObject)
	{
		return nullptr;
	}
	return OwnerObject->GetSkeletalMeshComponent();
}

#if WITH_EDITOR
void FT4ManualControl::EditorRestoreReaction() // #76
{
	AT4GameObject* OwnerObject = GetGameObject();
	check(nullptr != OwnerObject);

	if (ManualDieNode.IsPlaying())
	{
		ManualDieNode.StopAll();
	}
	if (ManualResurrectNode.IsPlaying())
	{
		ManualResurrectNode.StopAll();
	}
	if (ManualHitNode.IsPlaying())
	{
		ManualHitNode.StopAll();
	}

	EndPhysics();
}
#endif