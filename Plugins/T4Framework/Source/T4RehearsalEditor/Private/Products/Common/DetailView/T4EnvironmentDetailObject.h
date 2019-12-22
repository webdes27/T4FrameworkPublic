// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Asset/Classes/World/T4EnvironmentAsset.h"
#include "T4EnvironmentDetailObject.generated.h"

/**
  * #90
 */
UCLASS()
class UT4EnvironmentDetailObject : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	
	DECLARE_MULTICAST_DELEGATE(FT4OnPropertiesChanged);
	FT4OnPropertiesChanged& OnPropertiesChanged() { return OnPropertiesChangedDelegate; }

public:
	bool CopyTo(FT4EnvTimeTagData& OutData);
	bool CopyFrom(const FT4EnvTimeTagData& InData);

	void SyncFrom(UWorld* InWorld);

	bool ApplyTo(UWorld* InWorld, FName InTimeTagName, FString& OutErrorMessage);
	void ApplyTo(UWorld* InSourceWorld, UWorld* InTargetWorld); // #93

public:
	// #T4_ADD_TOD_TAG
	UPROPERTY(EditAnywhere, Category = "TimeTag Details")
	FT4EnvDirectionalData DirectionalData; // #93

	UPROPERTY(EditAnywhere, Category = "TimeTag Details")
	FT4EnvDirectionalLightData DirectionalLightData; // #90

	UPROPERTY(EditAnywhere, Category = "TimeTag Details")
	FT4EnvBPSkySphereData BPSkySphereData; // #97

	UPROPERTY(EditAnywhere, Category = "TimeTag Details")
	FT4EnvSkyLightData SkyLightData; // #90

	UPROPERTY(EditAnywhere, Category = "TimeTag Details")
	FT4EnvAtmosphericFogData AtmosphericFogData; // #90

	UPROPERTY(EditAnywhere, Category = "TimeTag Details")
	FT4EnvExponentialHeightFogData ExponentialHeightFogData; // #90

	UPROPERTY(EditAnywhere, Category = "TimeTag Details")
	FT4EnvPostProcessData PostProcessData; // #98

private:
	FT4OnPropertiesChanged OnPropertiesChangedDelegate;
};
