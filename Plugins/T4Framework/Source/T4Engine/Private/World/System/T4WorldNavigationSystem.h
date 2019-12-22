// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4Engine.h"

/**
  * #87
 */
class FT4GameWorld;
class FT4WorldNavigationSystem : public IT4WorldNavigationSystem
{
public:
	explicit FT4WorldNavigationSystem(FT4GameWorld* InGameWorld);
	virtual ~FT4WorldNavigationSystem();

	bool ProjectPoint(const FVector& InGoal, const FVector& InExtent, FVector& OutLocation) override; // #31 : INVALID_NAVEXTENT, FVector::ZeroVector

	bool HasReached(const FVector& InStartLocation, const FVector& InEndLocation) override; // #52

	bool GetRandomLocation(FVector& OutLocation) override; // #87
	bool GetRandomLocation(const FVector& InOrigin, float InMaxRadius, FVector& OutLocation) override; // #31

public:
	void Reset();

private:
	FT4GameWorld* GameWorldRef;
};
