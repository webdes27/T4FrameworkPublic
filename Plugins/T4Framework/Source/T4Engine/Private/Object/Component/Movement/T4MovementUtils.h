// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/RootMotionSource.h"

/**
  * see GameFramework/Character.h"
 */

class UPrimitiveComponent;
class UT4MovementComponent;

 /** MovementBaseUtility provides utilities for working with movement bases, for which we may need relative positioning info. */
namespace T4MovementUtil
{
	/** Determine whether MovementBase can possibly move. */
	bool IsDynamicBase(const UPrimitiveComponent* MovementBase);

	/** Determine if we should use relative positioning when based on a component (because it may move). */
	FORCEINLINE bool UseRelativeLocation(const UPrimitiveComponent* MovementBase)
	{
		return IsDynamicBase(MovementBase);
	}

	/** Ensure that BasedObjectTick ticks after NewBase */
	void AddTickDependency(FTickFunction& BasedObjectTick, UPrimitiveComponent* NewBase);

	/** Remove tick dependency of BasedObjectTick on OldBase */
	void RemoveTickDependency(FTickFunction& BasedObjectTick, UPrimitiveComponent* OldBase);

	/** Get the velocity of the given component, first checking the ComponentVelocity and falling back to the physics velocity if necessary. */
	FVector GetMovementBaseVelocity(const UPrimitiveComponent* MovementBase, const FName BoneName);

	/** Get the tangential velocity at WorldLocation for the given component. */
	FVector GetMovementBaseTangentialVelocity(
		const UPrimitiveComponent* MovementBase, 
		const FName BoneName, 
		const FVector& WorldLocation
	);

	/** Get the transforms for the given MovementBase, optionally at the location of a bone. Returns false if MovementBase is nullptr, or if BoneName is not a valid bone. */
	bool GetMovementBaseTransform(
		const UPrimitiveComponent* MovementBase, 
		const FName BoneName, 
		FVector& OutLocation, 
		FQuat& OutQuat
	);

	FString GetMovementName(EMovementMode MovementMode);
}
