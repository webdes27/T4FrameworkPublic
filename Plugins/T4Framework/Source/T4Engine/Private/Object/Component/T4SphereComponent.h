// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "T4SphereComponent.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Framework/Pawn/Character/index.html
 */
UCLASS()
class UT4SphereComponent : public USphereComponent
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
