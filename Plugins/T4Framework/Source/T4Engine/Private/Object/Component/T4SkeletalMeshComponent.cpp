// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4SkeletalMeshComponent.h"

#include "T4EngineInternal.h"

/**
  * https://docs.unrealengine.com/en-us/Engine/Content/Types/SkeletalMeshes
 */
UT4SkeletalMeshComponent::UT4SkeletalMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, LayerType(ET4LayerType::Max)
{
}

void UT4SkeletalMeshComponent::TickComponent(
	float InDeltaTime,
	enum ELevelTick InTickType,
	FActorComponentTickFunction* InThisTickFunction
)
{
	Super::TickComponent(InDeltaTime, InTickType, InThisTickFunction);
}

void UT4SkeletalMeshComponent::BeginPlay()
{
	Super::BeginPlay();
}
