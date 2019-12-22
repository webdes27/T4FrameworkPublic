// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4CapsuleComponent.h"

#include "T4EngineInternal.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Framework/Pawn/Character/index.html
 */
UT4CapsuleComponent::UT4CapsuleComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4CapsuleComponent::TickComponent(
	float DeltaTime,
	enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UT4CapsuleComponent::BeginPlay()
{
	Super::BeginPlay();
}
