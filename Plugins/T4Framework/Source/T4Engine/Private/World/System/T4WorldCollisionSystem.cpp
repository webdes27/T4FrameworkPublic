// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4WorldCollisionSystem.h"

#include "World/T4GameWorld.h"

#include "Engine/World.h"

#include "T4EngineInternal.h"

/**
  * #87
 */
FT4WorldCollisionSystem::FT4WorldCollisionSystem(FT4GameWorld* InGameWorld)
	: GameWorldRef(InGameWorld)
{
}

FT4WorldCollisionSystem::~FT4WorldCollisionSystem()
{
}

void FT4WorldCollisionSystem::Reset()
{

}

bool FT4WorldCollisionSystem::QueryLineTraceSingle(
	ET4CollisionChannel InCollisionChannel,
	const FVector& InStartLocation,
	const FVector& InEndLocation,
	const FCollisionQueryParams& InCollisionQueryParams,
	FT4HitSingleResult& OutHitResult
)
{
	check(nullptr != GameWorldRef);
	UWorld* UnrealWorld = GameWorldRef->GetWorld();
	if (nullptr == UnrealWorld)
	{
		return false;
	}
	ECollisionChannel QueryCollisionChannel = ECollisionChannel::ECC_MAX;
	switch (InCollisionChannel)
	{
		case ET4CollisionChannel::WorldStatic:
			QueryCollisionChannel = ECollisionChannel::ECC_WorldStatic;
			break;

		case ET4CollisionChannel::WorldDynamic: // #112
			//QueryCollisionChannel = ECollisionChannel::ECC_WorldDynamic;
			QueryCollisionChannel = ECollisionChannel::ECC_Visibility;
			break;

		case ET4CollisionChannel::SpawnObject:
			QueryCollisionChannel = ECollisionChannel::ECC_Pawn;
			break;

		case ET4CollisionChannel::CollisionVisibility:
			QueryCollisionChannel = ECollisionChannel::ECC_Visibility;
			break;

		default:
			return false;
	};

	FHitResult HitResult;
	bool bSuccess = UnrealWorld->LineTraceSingleByChannel(
		HitResult,
		InStartLocation,
		InEndLocation,
		QueryCollisionChannel,
		InCollisionQueryParams
	);
	if (!bSuccess)
	{
		return false;
	}
	OutHitResult.QueryCollisionChannel = QueryCollisionChannel;
	switch (InCollisionChannel)
	{
		case ET4CollisionChannel::WorldStatic:
		case ET4CollisionChannel::CollisionVisibility:
			{
				OutHitResult.ResultLocation = HitResult.Location;
				if (HitResult.Actor.IsValid())
				{
					OutHitResult.ResultObject = T4EnginetInternal::TryCastGameObject(HitResult.Actor.Get()); // #112
				}
			}
			break;

		case ET4CollisionChannel::WorldDynamic: // #112
		case ET4CollisionChannel::SpawnObject:
			{
				if (!HitResult.Actor.IsValid())
				{
					return false;
				}
				OutHitResult.ResultObject = T4EnginetInternal::TryCastGameObject(HitResult.Actor.Get());
				if (nullptr == OutHitResult.ResultObject)
				{
					return false;
				}
			}
			break;
	};
	OutHitResult.ResultImpactNormal = HitResult.ImpactNormal; // #112
	OutHitResult.ResultLocation = HitResult.Location;
	OutHitResult.ResultHitBone = HitResult.BoneName; // #112 : Only Skeletal Mesh
	return true;
}

bool FT4WorldCollisionSystem::QueryLineTraceSingle(
	ET4CollisionChannel InCollisionChannel,
	const FVector& InStartLocation,
	const FVector& InStartDirection,
	const float InMaxDistance,
	const FCollisionQueryParams& InCollisionQueryParams,
	FT4HitSingleResult& OutHitResult
)
{
	bool bResult = QueryLineTraceSingle(
		InCollisionChannel,
		InStartLocation,
		InStartLocation + (InMaxDistance * InStartDirection),
		InCollisionQueryParams,
		OutHitResult
	);
	return bResult;
}
