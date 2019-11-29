// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "T4PlayerCameraManager.generated.h"

/**
  * https://docs.unrealengine.com/en-US/Gameplay/Framework/Camera
 */
UCLASS()
class T4ENGINE_API AT4PlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	void UpdateCamera(float InDeltaTime) override;
};
