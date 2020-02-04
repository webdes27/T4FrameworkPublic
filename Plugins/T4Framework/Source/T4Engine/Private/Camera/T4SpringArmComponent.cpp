// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Classes/Camera/T4SpringArmComponent.h"

#include "T4EngineInternal.h"

/**
  * https://docs.unrealengine.com/en-us/Engine/Content/Types/StaticMeshes
 */
UT4SpringArmComponent::UT4SpringArmComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, BackupSocketOffset(FVector::ZeroVector) // #58
	, BlendWeight(0.0f) // #58
	, BlendSocketOffset(FVector::ZeroVector) // #58
{
}

void UT4SpringArmComponent::TickComponent(
	float DeltaTime,
	enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	if (0.0f < BlendWeight) // #58
	{
		SocketOffset = FMath::Lerp(BackupSocketOffset, BlendSocketOffset, BlendWeight);
	}
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UT4SpringArmComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UT4SpringArmComponent::UpdateBlendSocketOffset(
	const FVector& InBlendSocketOffset,
	float InBlendWeight
) // #58
{
	BlendWeight = InBlendWeight;
	if (0.0f >= InBlendWeight)
	{
		SocketOffset = BackupSocketOffset;
		return;
	}
	BlendSocketOffset = InBlendSocketOffset;
}