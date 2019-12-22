// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4StaticMeshComponent.h"

#include "T4EngineInternal.h"

/**
  * https://docs.unrealengine.com/en-us/Engine/Content/Types/StaticMeshes
 */
UT4StaticMeshComponent::UT4StaticMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4StaticMeshComponent::TickComponent(
	float DeltaTime,
	enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UT4StaticMeshComponent::BeginPlay()
{
	Super::BeginPlay();
}
