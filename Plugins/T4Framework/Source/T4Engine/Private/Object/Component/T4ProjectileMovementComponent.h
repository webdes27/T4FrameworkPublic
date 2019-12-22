// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework\ProjectileMovementComponent.h"
#include "T4ProjectileMovementComponent.generated.h"

/**
  * https://docs.unrealengine.com/en-US/Programming/Tutorials/FirstPersonShooter/3/1/index.html
 */
UCLASS()
class UT4ProjectileMovementComponent : public UProjectileMovementComponent
{
	GENERATED_UCLASS_BODY()

public:
	void TickComponent(
		float DeltaTime,
		enum ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

protected:
	void BeginPlay() override;
};
