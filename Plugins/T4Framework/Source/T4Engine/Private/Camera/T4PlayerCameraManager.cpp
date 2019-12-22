// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Camera/T4PlayerCameraManager.h"
#include "Classes/Camera/T4CameraModifier.h" // #101

#include "T4EngineInternal.h"

/**
  * https://docs.unrealengine.com/en-US/Gameplay/Framework/Camera	
 */
AT4PlayerCameraManager::AT4PlayerCameraManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultModifiers.Add(UT4CameraModifier::StaticClass()); // #100, #101
}

void AT4PlayerCameraManager::UpdateCamera(float InDeltaTime)
{
	Super::UpdateCamera(InDeltaTime);
}
