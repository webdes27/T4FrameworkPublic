// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "T4Engine/Public/T4EngineTypes.h"
#include "T4GameHUD.generated.h"

/**
  * https://docs.unrealengine.com/ko/Gameplay/Framework/UIAndHUD/index.html
 */

UCLASS()
class T4FRAME_API AT4GameHUD : public AHUD
{
	GENERATED_UCLASS_BODY()

public:
	ET4LayerType GetLayerType() const { return LayerType; }

protected:
	void BeginPlay() override;

protected:
	ET4LayerType LayerType;
};
