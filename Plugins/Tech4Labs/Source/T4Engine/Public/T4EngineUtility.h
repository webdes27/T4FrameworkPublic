// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4EngineTypes.h"
#include "T4Asset/Classes/World/T4MapEnvironmentAsset.h"

/**
  * #92
 */
class UWorld;
class ADirectionalLight;
class ASkyLight;
class AAtmosphericFog;
class AExponentialHeightFog;
class AT4MapZoneVolume;
namespace T4EngineUtility
{
	// #90, #92 : World Editor 등의 툴에서의 호출을 위해 Utility 로 노출함!
	T4ENGINE_API AT4MapZoneVolume* FindMapZomeVolumeOnWorld(
		UWorld* InWorld,
		FName InMapZoneName
	);

	T4ENGINE_API bool GetMapZomeVolumesOnWorld(
		UWorld* InWorld,
		TArray<AT4MapZoneVolume*>& OutMapZoneVolumes
	);

	T4ENGINE_API FName GetNextTimeTagName(FName InTimeName); // #93
	T4ENGINE_API FName GetPrevTimeTagName(FName InTimeName); // #93

	// #T4_ADD_TOD_TAG
	T4ENGINE_API bool ApplyDirectional(
		UWorld* InWorld,
		const FT4MapDirectionalData* InData,
		ADirectionalLight* InCachedActor = nullptr // #92
	); // #93
	T4ENGINE_API bool ApplyDirectionalLight(
		UWorld* InWorld, 
		const FT4MapDirectionalLightData* InData,
		ADirectionalLight* InCachedActor = nullptr // #92
	);
	T4ENGINE_API bool ApplySkyLight(
		UWorld* InWorld, 
		const FT4MapSkyLightData* InData,
		ASkyLight* InCachedActor = nullptr // #92
	);
	T4ENGINE_API bool ApplyAtmosphericFog(
		UWorld* InWorld, 
		const FT4MapAtmosphericFogData* InData,
		AAtmosphericFog* InCachedActor = nullptr // #92
	);
	T4ENGINE_API bool ApplyExponentialHeightFog(
		UWorld* InWorld, 
		const FT4MapExponentialHeightFogData* InData,
		AExponentialHeightFog* InCachedActor = nullptr // #92
	);


	T4ENGINE_API void BlendTimeTagData(
		const FT4MapTimeTagData& InTimeTagData,
		float InWeight,
		FT4MapTimeTagData& OutTimeTagData
	); // #93

	// #T4_ADD_TOD_TAG
	T4ENGINE_API void BlendDirectional(
		const FT4MapDirectionalData* InData,
		float InWeight,
		FT4MapDirectionalData& OutData // #93
	); // #93
	T4ENGINE_API void BlendDirectionalLight(
		const FT4MapDirectionalLightData* InData,
		float InWeight,
		FT4MapDirectionalLightData& OutData // #93
	);
	T4ENGINE_API void BlendSkyLight(
		const FT4MapSkyLightData* InData,
		float InWeight,
		FT4MapSkyLightData& OutData // #93
	);
	T4ENGINE_API void BlendAtmosphericFog(
		const FT4MapAtmosphericFogData* InData,
		float InWeight,
		FT4MapAtmosphericFogData& OutData // #93
	);
	T4ENGINE_API void BlendExponentialHeightFog(
		const FT4MapExponentialHeightFogData* InData,
		float InWeight,
		FT4MapExponentialHeightFogData& OutData // #93
	);

#if WITH_EDITOR
	// #T4_ADD_TOD_TAG
	T4ENGINE_API bool GetDirectionalData(UWorld* InWorld, FT4MapDirectionalData* OutData); // #93
	T4ENGINE_API bool GetDirectionalLightData(UWorld* InWorld, FT4MapDirectionalLightData* OutData);
	T4ENGINE_API bool GetSkyLightDataData(UWorld* InWorld, FT4MapSkyLightData* OutData);
	T4ENGINE_API bool GetAtmosphericFogData(UWorld* InWorld, FT4MapAtmosphericFogData* OutData);
	T4ENGINE_API bool GetExponentialHeightFogData(UWorld* InWorld, FT4MapExponentialHeightFogData* OutData);
#endif
}