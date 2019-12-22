// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WorldNavigationSystem.h"

#include "World/T4GameWorld.h"

#include "Engine/World.h"
#include "NavigationSystem.h"

#include "T4EngineInternal.h"

/**
  * #87
 */
FT4WorldNavigationSystem::FT4WorldNavigationSystem(FT4GameWorld* InGameWorld)
	: GameWorldRef(InGameWorld)
{
}

FT4WorldNavigationSystem::~FT4WorldNavigationSystem()
{
}

void FT4WorldNavigationSystem::Reset()
{

}

bool FT4WorldNavigationSystem::ProjectPoint(
	const FVector& InGoal,
	const FVector& InExtent, // INVALID_NAVEXTENT, FVector::ZeroVector
	FVector& OutLocation
) // #31
{
	check(nullptr != GameWorldRef);
	UWorld* UnrealWorld = GameWorldRef->GetWorld();
	if (nullptr == UnrealWorld)
	{
		return false;
	}
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(UnrealWorld);
	if (nullptr == NavSystem)
	{
#if WITH_EDITOR
		// #31 에디터 모드라면...NullPlane 으로 처리해준다.
		OutLocation = InGoal;
		return true;
#else
		return false;
#endif
	}
	FNavLocation ProjectedLocation;
	if (!NavSystem->ProjectPointToNavigation(InGoal, ProjectedLocation, InExtent))
	{
		return false;
	}
	OutLocation = ProjectedLocation.Location;
	return true;
}

bool FT4WorldNavigationSystem::HasReached(
	const FVector& InStartLocation,
	const FVector& InEndLocation
) // #52
{
	return true; // TODO : #52 : Importent!!
}

bool FT4WorldNavigationSystem::GetRandomLocation(FVector& OutLocation) // #87
{
	check(nullptr != GameWorldRef);
	UWorld* UnrealWorld = GameWorldRef->GetWorld();
	if (nullptr == UnrealWorld)
	{
		return false;
	}
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(UnrealWorld);
	if (nullptr == NavSystem)
	{
#if WITH_EDITOR
		return true;
#else
		return false;
#endif
	}
	FNavLocation NewTargetPoint;
	if (!NavSystem->GetRandomPoint(NewTargetPoint))
	{
		return false;
	}
	OutLocation = NewTargetPoint.Location;
	return true;
}

bool FT4WorldNavigationSystem::GetRandomLocation(
	const FVector& InOrigin,
	float InMaxRadius,
	FVector& OutLocation
) // #31
{
	check(nullptr != GameWorldRef);
	UWorld* UnrealWorld = GameWorldRef->GetWorld();
	if (nullptr == UnrealWorld)
	{
		return false;
	}
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(UnrealWorld);
	if (nullptr == NavSystem)
	{
#if WITH_EDITOR
		// #31 에디터 모드라면...NullPlane 으로 처리해준다.
		OutLocation = InOrigin;
		OutLocation.X += FMath::RandRange(-InMaxRadius, InMaxRadius);
		OutLocation.Y += FMath::RandRange(-InMaxRadius, InMaxRadius);
		return true;
#else
		return false;
#endif
	}
	FNavLocation NewTargetPoint;
	if (!NavSystem->GetRandomPointInNavigableRadius(InOrigin, InMaxRadius, NewTargetPoint))
	{
		return false;
	}
	OutLocation = NewTargetPoint.Location;
	return true;
}
