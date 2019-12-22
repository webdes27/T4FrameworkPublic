// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Asset/Classes/World/T4EnvironmentAsset.h"
#include "Public/T4Engine.h"
#include "FinalPostProcessSettings.h" // #98

/**
  * #92
 */
class UWorld;
class UT4EnvironmentAsset;
class AActor;
class ADirectionalLight;
class ASkyLight;
class AAtmosphericFog;
class AExponentialHeightFog;
class AT4MapZoneVolume; // #98
class FT4WorldController; // #93

struct FFinalEnvironmentData
{
	FT4EnvTimeTagData TimeTagData; // #90
	FFinalPostProcessSettings PostProcessSettings; // #98 : PostProcess 는 별도로 처리한다.
};

class FT4WorldEnvironmentControl
{
public:
	explicit FT4WorldEnvironmentControl(FT4WorldController* InWorldController);
	virtual ~FT4WorldEnvironmentControl();

	void Reset();

	void Process(float InDeltaTime);

	void SetPause(bool bInPause) { bPaused = bInPause; }

protected:
	bool SetGlobal(
		const UT4EnvironmentAsset* InEnvironmentAsset,
		const FName InSourceTimeName, // #93
		const FName InTargetTimeName, // #93
		const float InLocalBlendWeight // #93
	);
	bool BlendLocalZone(
		const UT4EnvironmentAsset* InEnvironmentAsset,
		const float InLayerBlendWeight, // #94
		const FName InSourceTimeName, // #93
		const FName InTargetTimeName, // #93
		const float InLocalBlendWeight // #93
	);

private:
	bool BlendEnvironmentDataData(
		const UT4EnvironmentAsset* InEnvironmentAsset,
		const FName InSourceTimeName,
		const FName InTargetTimeName,
		const float InTimeLocalRatio,
		FFinalEnvironmentData& OutEnvironmentData
	); // #93

	const FT4EnvTimeTagData* GetTimeTagData(
		const UT4EnvironmentAsset* InEnvironmentAsset,
		const FName InTimeName
	);

	void Apply(UWorld* InWorld);

private:
	FT4WorldController* WorldControllerRef;

	bool bPaused; // #92 : Map Environemnt Update 제어 옵션 처리

	FFinalEnvironmentData FinalEnvrionmentData; // #98

// #T4_ADD_TOD_TAG
	TWeakObjectPtr<ADirectionalLight> DirectionalLightPtr; // #92
	TWeakObjectPtr<AActor> BPSkySpherePtr;
	TWeakObjectPtr<ASkyLight> SkyLightPtr;
	TWeakObjectPtr<AAtmosphericFog> AtmosphericFogPtr;
	TWeakObjectPtr<AExponentialHeightFog> ExponentialHeightFogPtr;
	TWeakObjectPtr<AT4MapZoneVolume> GlobalMapZoneVolumePtr;
};
