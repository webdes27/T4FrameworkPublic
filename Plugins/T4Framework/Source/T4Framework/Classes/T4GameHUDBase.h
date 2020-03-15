// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "T4Engine/Public/T4EngineTypes.h"
#include "T4GameHUDBase.generated.h"

/**
  * https://docs.unrealengine.com/ko/Gameplay/Framework/UIAndHUD/index.html
 */
class UTexture2D;
UCLASS()
class T4FRAMEWORK_API AT4GameHUDBase : public AHUD
{
	GENERATED_UCLASS_BODY()

public:
	ET4LayerType GetLayerType() const { return LayerType; }

	virtual void DrawHUD() override; // #121 : Crosshair

	void SetCrosshairTexture(UTexture2D* InTexture) { CrosshairTexture = InTexture; } // #121
	void SetCrosshairTextureScale(float InScale) { Scale = InScale; }

protected:
	void BeginPlay() override;

protected:
	ET4LayerType LayerType;
	
	UPROPERTY(EditDefaultsOnly)
	UTexture2D* CrosshairTexture; // #121

	float Scale;
};
