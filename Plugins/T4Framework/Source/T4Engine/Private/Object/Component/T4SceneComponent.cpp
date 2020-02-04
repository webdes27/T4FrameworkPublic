// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4SceneComponent.h"

#include "T4EngineInternal.h"

/**
  *
 */
UT4SceneComponent::UT4SceneComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4SceneComponent::TickComponent(
	float DeltaTime,
	enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UT4SceneComponent::BeginPlay()
{
	Super::BeginPlay();
}
