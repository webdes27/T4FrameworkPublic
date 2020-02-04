// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Particles/ParticleSystemComponent.h"
#include "T4ParticleSystemComponent.generated.h"

/**
  * http://api.unrealengine.com/KOR/Engine/Rendering/ParticleSystems/
 */
UCLASS()
class UT4ParticleSystemComponent : public UParticleSystemComponent
{
	GENERATED_UCLASS_BODY()

public:
	void TickComponent(
		float DeltaTime,
		enum ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

public:
	void SetPlayRate(float InPlayRate) { PlayRate = InPlayRate; } // #102

protected:
	void BeginPlay() override;

private:
	float PlayRate; // #102
};
