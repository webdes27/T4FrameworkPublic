// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4SphereComponent.h"

#include "T4EngineInternal.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Framework/Pawn/Character/index.html
 */
UT4SphereComponent::UT4SphereComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4SphereComponent::TickComponent(
	float DeltaTime,
	enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UT4SphereComponent::BeginPlay()
{
	Super::BeginPlay();
}
