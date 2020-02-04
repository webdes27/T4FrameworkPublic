// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4WorldPreviewFloorMeshActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"

#include "T4EngineInternal.h"

/**
  * 
 */
AT4WorldPreviewFloorMeshActor::AT4WorldPreviewFloorMeshActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FloorMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("T4WorldPreviewFloorMeshActor"));

	FloorMeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	FloorMeshComponent->Mobility = EComponentMobility::Static;
	FloorMeshComponent->SetGenerateOverlapEvents(false);
	FloorMeshComponent->bUseDefaultCollision = true;

	RootComponent = FloorMeshComponent;
}
