// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModifier_CameraShake.h"
#include "Engine/Scene.h" // #98
#include "T4CameraModifierShake.generated.h"

/**
  * #100
*/
UCLASS()
class T4ENGINE_API UT4CameraModifierShake : public UCameraModifier_CameraShake
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UCameraModifer Interface
	virtual bool ModifyCamera(float DeltaTime, struct FMinimalViewInfo& InOutPOV) override;
	//~ End UCameraModifer Interface
};
