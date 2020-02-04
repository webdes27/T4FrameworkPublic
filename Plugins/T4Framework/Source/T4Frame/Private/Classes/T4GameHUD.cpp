// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Classes/T4GameHUD.h"

#include "Public/T4Frame.h"

#include "Kismet/GameplayStatics.h"

#include "T4FrameInternal.h"

/**
  * https://docs.unrealengine.com/ko/Gameplay/Framework/UIAndHUD/index.html
 */
AT4GameHUD::AT4GameHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, LayerType(ET4LayerType::Max)
{
}

void AT4GameHUD::BeginPlay()
{
	Super::BeginPlay();

	check(ET4LayerType::Max == LayerType);
	LayerType = T4EngineLayer::Get(GetWorld()); // #12 : Support Multiple LayerType
	check(ET4LayerType::Max != LayerType);
}