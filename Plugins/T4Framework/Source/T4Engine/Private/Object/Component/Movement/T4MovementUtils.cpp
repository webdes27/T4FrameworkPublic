// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4MovementUtils.h"
#include "Object/Component/T4MovementComponent.h"

#include "T4EngineInternal.h"

/**
  * see GameFramework/Character.h"
 */
namespace T4MovementUtil
{
	bool IsDynamicBase(const UPrimitiveComponent* MovementBase)
	{
		return (MovementBase && MovementBase->Mobility == EComponentMobility::Movable);
	}

	void AddTickDependency(FTickFunction& BasedObjectTick, UPrimitiveComponent* NewBase)
	{
		if (NewBase && T4MovementUtil::UseRelativeLocation(NewBase))
		{
			if (NewBase->PrimaryComponentTick.bCanEverTick)
			{
				BasedObjectTick.AddPrerequisite(NewBase, NewBase->PrimaryComponentTick);
			}

			AActor* NewBaseOwner = NewBase->GetOwner();
			if (NewBaseOwner)
			{
				if (NewBaseOwner->PrimaryActorTick.bCanEverTick)
				{
					BasedObjectTick.AddPrerequisite(NewBaseOwner, NewBaseOwner->PrimaryActorTick);
				}

				// @TODO: We need to find a more efficient way of finding all ticking components in an actor.
				for (UActorComponent* Component : NewBaseOwner->GetComponents())
				{
					if (Component && Component->PrimaryComponentTick.bCanEverTick)
					{
						BasedObjectTick.AddPrerequisite(Component, Component->PrimaryComponentTick);
					}
				}
			}
		}
	}

	void RemoveTickDependency(FTickFunction& BasedObjectTick, UPrimitiveComponent* OldBase)
	{
		if (OldBase && T4MovementUtil::UseRelativeLocation(OldBase))
		{
			BasedObjectTick.RemovePrerequisite(OldBase, OldBase->PrimaryComponentTick);
			AActor* OldBaseOwner = OldBase->GetOwner();
			if (OldBaseOwner)
			{
				BasedObjectTick.RemovePrerequisite(OldBaseOwner, OldBaseOwner->PrimaryActorTick);

				// @TODO: We need to find a more efficient way of finding all ticking components in an actor.
				for (UActorComponent* Component : OldBaseOwner->GetComponents())
				{
					if (Component && Component->PrimaryComponentTick.bCanEverTick)
					{
						BasedObjectTick.RemovePrerequisite(Component, Component->PrimaryComponentTick);
					}
				}
			}
		}
	}

	FVector GetMovementBaseVelocity(const UPrimitiveComponent* MovementBase, const FName BoneName)
	{
		FVector BaseVelocity = FVector::ZeroVector;
		if (T4MovementUtil::IsDynamicBase(MovementBase))
		{
			if (BoneName != NAME_None)
			{
				const FBodyInstance* BodyInstance = MovementBase->GetBodyInstance(BoneName);
				if (BodyInstance)
				{
					BaseVelocity = BodyInstance->GetUnrealWorldVelocity();
					return BaseVelocity;
				}
			}

			BaseVelocity = MovementBase->GetComponentVelocity();
			if (BaseVelocity.IsZero())
			{
				// Fall back to actor's Root component
				const AActor* Owner = MovementBase->GetOwner();
				if (Owner)
				{
					// Component might be moved manually (not by simulated physics or a movement component), see if the root component of the actor has a velocity.
					BaseVelocity = MovementBase->GetOwner()->GetVelocity();
				}
			}

			// Fall back to physics velocity.
			if (BaseVelocity.IsZero())
			{
				if (FBodyInstance* BaseBodyInstance = MovementBase->GetBodyInstance())
				{
					BaseVelocity = BaseBodyInstance->GetUnrealWorldVelocity();
				}
			}
		}

		return BaseVelocity;
	}

	FVector GetMovementBaseTangentialVelocity(
		const UPrimitiveComponent* MovementBase, 
		const FName BoneName, 
		const FVector& WorldLocation
	)
	{
		if (T4MovementUtil::IsDynamicBase(MovementBase))
		{
			if (const FBodyInstance* BodyInstance = MovementBase->GetBodyInstance(BoneName))
			{
				const FVector BaseAngVelInRad = BodyInstance->GetUnrealWorldAngularVelocityInRadians();
				if (!BaseAngVelInRad.IsNearlyZero())
				{
					FVector BaseLocation;
					FQuat BaseRotation;
					if (T4MovementUtil::GetMovementBaseTransform(MovementBase, BoneName, BaseLocation, BaseRotation))
					{
						const FVector RadialDistanceToBase = WorldLocation - BaseLocation;
						const FVector TangentialVel = BaseAngVelInRad ^ RadialDistanceToBase;
						return TangentialVel;
					}
				}
			}
		}

		return FVector::ZeroVector;
	}

	bool GetMovementBaseTransform(
		const UPrimitiveComponent* MovementBase, 
		const FName BoneName, 
		FVector& OutLocation, 
		FQuat& OutQuat
	)
	{
		if (MovementBase)
		{
			if (BoneName != NAME_None)
			{
				bool bFoundBone = false;
				if (MovementBase)
				{
					// Check if this socket or bone exists (DoesSocketExist checks for either, as does requesting the transform).
					if (MovementBase->DoesSocketExist(BoneName))
					{
						MovementBase->GetSocketWorldLocationAndRotation(BoneName, OutLocation, OutQuat);
						bFoundBone = true;
					}
					else
					{
						T4_LOG(Warning, TEXT("Invalid bone or socket '%s' for PrimitiveComponent base %s"), *BoneName.ToString(), *GetPathNameSafe(MovementBase));
					}
				}

				if (!bFoundBone)
				{
					OutLocation = MovementBase->GetComponentLocation();
					OutQuat = MovementBase->GetComponentQuat();
				}
				return bFoundBone;
			}

			// No bone supplied
			OutLocation = MovementBase->GetComponentLocation();
			OutQuat = MovementBase->GetComponentQuat();
			return true;
		}

		// nullptr MovementBase
		OutLocation = FVector::ZeroVector;
		OutQuat = FQuat::Identity;
		return false;
	}

	FString GetMovementName(EMovementMode MovementMode)
	{
		// Using character movement
		switch (MovementMode)
		{
			case MOVE_None:				return TEXT("NULL"); break;
			case MOVE_Walking:			return TEXT("Walking"); break;
			case MOVE_NavWalking:		return TEXT("NavWalking"); break;
			case MOVE_Falling:			return TEXT("Falling"); break;
			case MOVE_Swimming:			return TEXT("Swimming"); break;
			case MOVE_Flying:			return TEXT("Flying"); break;
			case MOVE_Custom:			return TEXT("Custom"); break;
			default:					break;
		}
		return TEXT("Unknown");
	}
}
