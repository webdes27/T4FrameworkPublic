// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ProjectileMovementComponent.h"

#include "T4EngineInternal.h"

/**
  * https://docs.unrealengine.com/en-US/Programming/Tutorials/FirstPersonShooter/3/1/index.html
 */
UT4ProjectileMovementComponent::UT4ProjectileMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4ProjectileMovementComponent::TickComponent(
	float DeltaTime,
	enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UT4ProjectileMovementComponent::BeginPlay()
{
	Super::BeginPlay();
}
