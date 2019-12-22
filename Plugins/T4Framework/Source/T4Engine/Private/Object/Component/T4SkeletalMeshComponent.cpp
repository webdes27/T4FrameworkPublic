// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4SkeletalMeshComponent.h"

#include "T4EngineInternal.h"

/**
  * https://docs.unrealengine.com/en-us/Engine/Content/Types/SkeletalMeshes
 */
UT4SkeletalMeshComponent::UT4SkeletalMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4SkeletalMeshComponent::TickComponent(
	float DeltaTime,
	enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UT4SkeletalMeshComponent::BeginPlay()
{
	Super::BeginPlay();
}
