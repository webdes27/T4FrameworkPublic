// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4SkinnedMeshComponent.h"

#include "T4EngineInternal.h"

/**
  * https://docs.unrealengine.com/en-us/Engine/Content/Types/SkeletalMeshes
 */
UT4SkinnedMeshComponent::UT4SkinnedMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4SkinnedMeshComponent::TickComponent(
	float DeltaTime,
	enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UT4SkinnedMeshComponent::BeginPlay()
{
	Super::BeginPlay();
}
