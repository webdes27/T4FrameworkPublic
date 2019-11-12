// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4EngineTypes.h"

/**
  * #92
 */
class UWorld;
class ADirectionalLight;
class ASkyLight;
class AAtmosphericFog;
class AExponentialHeightFog;
struct FT4MapDirectionalLightData;
struct FT4MapSkyLightData;
struct FT4MapAtmosphericFogData;
struct FT4MapExponentialHeightFogData;
namespace T4EngineUtility
{
	// #90, #92 : Environment, 툴에서의 호출을 위해 Utility 로 노출함!
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

#if WITH_EDITOR
	T4ENGINE_API bool GetDirectionalLightData(UWorld* InWorld, FT4MapDirectionalLightData* OutData);
	T4ENGINE_API bool GetSkyLightDataData(UWorld* InWorld, FT4MapSkyLightData* OutData);
	T4ENGINE_API bool GetAtmosphericFogData(UWorld* InWorld, FT4MapAtmosphericFogData* OutData);
	T4ENGINE_API bool GetExponentialHeightFogData(UWorld* InWorld, FT4MapExponentialHeightFogData* OutData);
#endif
}