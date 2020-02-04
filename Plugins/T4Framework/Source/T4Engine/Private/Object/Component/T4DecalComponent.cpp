// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4DecalComponent.h"

#include "T4EngineInternal.h"

/**
  * http://api.unrealengine.com/KOR/Resources/ContentExamples/Decals/index.html
 */
UT4DecalComponent::UT4DecalComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, PlayRate(1.0f)
{
}

void UT4DecalComponent::TickComponent(
	float DeltaTime,
	enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	const float ScaledDeltaTimeSec = DeltaTime * PlayRate; // #102
	Super::TickComponent(ScaledDeltaTimeSec, TickType, ThisTickFunction);
}

void UT4DecalComponent::BeginPlay()
{
	Super::BeginPlay();
}
