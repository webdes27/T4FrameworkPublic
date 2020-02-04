// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4MovementStructs.h"
#include "T4MovementUtils.h"
#include "Object/Component/T4MovementComponent.h"

#include "T4EngineInternal.h"

/**
  * see GameFramework/CharacterMovementComponent.h"
 */
bool FGroundMovementInfo::HasRelativeLocation() const
{
	return T4MovementUtil::UseRelativeLocation(MovementBase);
}

void FT4MovementComponentPostPhysicsTickFunction::ExecuteTick(
	float DeltaTime,
	enum ELevelTick TickType,
	ENamedThreads::Type CurrentThread,
	const FGraphEventRef& MyCompletionGraphEvent
)
{
	FActorComponentTickFunction::ExecuteTickHelper(Target, /*bTickInEditor=*/ false, DeltaTime, TickType, [this](float DilatedTime)
	{
		Target->PostPhysicsTickComponent(DilatedTime, *this);
	});
}

FString FT4MovementComponentPostPhysicsTickFunction::DiagnosticMessage()
{
	return Target->GetFullName() + TEXT("[UT4MovementComponent::PreClothTick]");
}

FName FT4MovementComponentPostPhysicsTickFunction::DiagnosticContext(bool bDetailed)
{
	if (bDetailed)
	{
		return FName(*FString::Printf(TEXT("SkeletalMeshComponentClothTick/%s"), *GetFullNameSafe(Target)));
	}

	return FName(TEXT("SkeletalMeshComponentClothTick"));
}

void FT4FindFloorResult::SetFromSweep(
	const FHitResult& InHit, 
	const float InSweepFloorDist,
	const bool bIsWalkableFloor
)
{
	bBlockingHit = InHit.IsValidBlockingHit();
	bWalkableFloor = bIsWalkableFloor;
	bLineTrace = false;
	FloorDist = InSweepFloorDist;
	LineDist = 0.f;
	HitResult = InHit;
}

void FT4FindFloorResult::SetFromLineTrace(
	const FHitResult& InHit, 
	const float InSweepFloorDist, 
	const float InLineDist, 
	const bool bIsWalkableFloor
)
{
	// We require a sweep that hit if we are going to use a line result.
	check(HitResult.bBlockingHit);
	if (HitResult.bBlockingHit && InHit.bBlockingHit)
	{
		// Override most of the sweep result with the line result, but save some values
		FHitResult OldHit(HitResult);
		HitResult = InHit;

		// Restore some of the old values. We want the new normals and hit actor, however.
		HitResult.Time = OldHit.Time;
		HitResult.ImpactPoint = OldHit.ImpactPoint;
		HitResult.Location = OldHit.Location;
		HitResult.TraceStart = OldHit.TraceStart;
		HitResult.TraceEnd = OldHit.TraceEnd;

		bLineTrace = true;
		FloorDist = InSweepFloorDist;
		LineDist = InLineDist;
		bWalkableFloor = bIsWalkableFloor;
	}
}