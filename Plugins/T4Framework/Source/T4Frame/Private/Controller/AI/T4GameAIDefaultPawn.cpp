// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Controller/AI/T4GameAIDefaultPawn.h"

#include "T4FrameInternal.h"

/**
  *
 */
AT4GameAIDefaultPawn::AT4GameAIDefaultPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, LayerType(ET4LayerType::Max)
{
	bReplicates = false; // #15
	bNetLoadOnClient = false; // #15
}

void AT4GameAIDefaultPawn::Tick(float InDeltaTime)
{
	Super::Tick(InDeltaTime);
}

bool AT4GameAIDefaultPawn::ShouldTickIfViewportsOnly() const
{
	return T4EngineLayer::IsLevelEditor(LayerType); // #17
}

void AT4GameAIDefaultPawn::EndPlay(const EEndPlayReason::Type InEndPlayReason)
{
	Super::EndPlay(InEndPlayReason);
}

void AT4GameAIDefaultPawn::BeginPlay()
{
	Super::BeginPlay();
	check(ET4LayerType::Max != LayerType);
	ET4LayerType TestLayerType = T4EngineLayer::Get(GetWorld());
	check(TestLayerType == LayerType);
}
