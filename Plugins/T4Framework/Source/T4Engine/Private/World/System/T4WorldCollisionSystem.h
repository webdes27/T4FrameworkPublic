// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4Engine.h"

/**
  * #87
 */
class FT4GameWorld;
class FT4WorldCollisionSystem : public IT4WorldCollisionSystem
{
public:
	explicit FT4WorldCollisionSystem(FT4GameWorld* InGameWorld);
	virtual ~FT4WorldCollisionSystem();

	void Reset();

	bool QueryLineTraceSingle(
		ET4CollisionChannel InCollisionChannel,
		const FVector& InStartLocation,
		const FVector& InEndLocation,
		const FCollisionQueryParams& InCollisionQueryParams,
		FT4HitSingleResult& OutHitResult
	) override;

	bool QueryLineTraceSingle(
		ET4CollisionChannel InCollisionChannel,
		const FVector& InStartLocation,
		const FVector& InStartDirection,
		const float InMaxDistance,
		const FCollisionQueryParams& InCollisionQueryParams,
		FT4HitSingleResult& OutHitResult
	) override;

private:
	FT4GameWorld* GameWorldRef;
};
