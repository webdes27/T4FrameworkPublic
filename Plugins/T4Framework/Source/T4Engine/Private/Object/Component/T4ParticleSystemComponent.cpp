// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ParticleSystemComponent.h"

#include "T4EngineInternal.h"

/**
  * http://api.unrealengine.com/KOR/Engine/Rendering/ParticleSystems/
 */
UT4ParticleSystemComponent::UT4ParticleSystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, PlayRate(1.0f)
{
}

void UT4ParticleSystemComponent::TickComponent(
	float DeltaTime,
	enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	const float ScaledDeltaTimeSec = DeltaTime * PlayRate; // #102
	Super::TickComponent(ScaledDeltaTimeSec, TickType, ThisTickFunction);
}

void UT4ParticleSystemComponent::BeginPlay()
{
	Super::BeginPlay();
}
