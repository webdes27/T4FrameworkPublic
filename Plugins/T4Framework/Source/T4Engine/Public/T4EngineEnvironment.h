// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4EngineTypes.h"
#include "T4Asset/Public/Entity/T4EntityTypes.h" // #94
#include "T4Asset/Public/Action/T4ActionTypes.h" // #102
#include "T4Asset/Classes/World/T4EnvironmentAsset.h"
#include "FinalPostProcessSettings.h" // #98

/**
  * #92
 */
class UWorld;
class ADirectionalLight;
class ASkyLight;
class AAtmosphericFog;
class AExponentialHeightFog;
class UT4EnvironmentAsset; // #94
class AT4WorldZoneVolume;
namespace T4EngineEnvironment
{
	// #102
	T4ENGINE_API float Evaluate(
		ET4BuiltInEasing InBlendCurve,
		float InInterp // 0 ~ 1
	);

	// #90, #92 : World Editor 등의 툴에서의 호출을 위해 Utility 로 노출함!
	T4ENGINE_API AT4WorldZoneVolume* FindWorldZoneVolumeOnWorld(
		UWorld* InWorld,
		FName InWorldZoneName
	);

	T4ENGINE_API bool GetWorldZoneVolumesOnWorld(
		UWorld* InWorld,
		TArray<AT4WorldZoneVolume*>& OutWorldZoneVolumes
	);

	T4ENGINE_API AT4WorldZoneVolume* WorldSpawnWorldZoneVolume(
		UWorld* InWorld,
		const FVector& InLocation,
		const FRotator& InRotation,
		const FVector& InScale,
		ET4EntityZoneBrushType InBrushType,
		FName InZoneName,
		bool bInTransient
	); // #94

	T4ENGINE_API FName GetNextTimeTagName(FName InTimeName); // #93
	T4ENGINE_API FName GetPrevTimeTagName(FName InTimeName); // #93

	// #T4_ADD_TOD_TAG
	T4ENGINE_API bool ApplyDirectional(
		UWorld* InWorld,
		const FT4EnvDirectionalData* InData,
		ADirectionalLight* InCachedActor = nullptr // #92
	); // #93
	T4ENGINE_API bool ApplyDirectionalLight(
		UWorld* InWorld, 
		const FT4EnvDirectionalLightData* InData,
		ADirectionalLight* InCachedActor = nullptr // #92
	);
	T4ENGINE_API bool ApplyBPSkySphere(
		UWorld* InWorld,
		const FT4EnvBPSkySphereData* InData,
		AActor* InCachedActor = nullptr,
		ADirectionalLight* InCachedDirectionalLightActor = nullptr
	);// #97
	T4ENGINE_API bool ApplySkyLight(
		UWorld* InWorld, 
		const FT4EnvSkyLightData* InData,
		ASkyLight* InCachedActor = nullptr // #92
	);
	T4ENGINE_API bool ApplyAtmosphericFog(
		UWorld* InWorld, 
		const FT4EnvAtmosphericFogData* InData,
		AAtmosphericFog* InCachedActor = nullptr // #92
	);
	T4ENGINE_API bool ApplyExponentialHeightFog(
		UWorld* InWorld, 
		const FT4EnvExponentialHeightFogData* InData,
		AExponentialHeightFog* InCachedActor = nullptr // #92
	);
	T4ENGINE_API bool ApplyPostProcess(
		UWorld* InWorld,
		const FPostProcessSettings* InData,
		AT4WorldZoneVolume* InCachedVolume = nullptr // #98
	);


	T4ENGINE_API void BlendTimeTagData(
		const FT4EnvTimeTagData& InTimeTagData,
		float InWeight,
		FT4EnvTimeTagData& OutTimeTagData
	); // #93, #98 : BlendPostProcess 는 별도로 처리!!

	// #T4_ADD_TOD_TAG
	T4ENGINE_API void BlendDirectional(
		const FT4EnvDirectionalData* InData,
		float InWeight,
		FT4EnvDirectionalData& OutData // #93
	); // #93
	T4ENGINE_API void BlendDirectionalLight(
		const FT4EnvDirectionalLightData* InData,
		float InWeight,
		FT4EnvDirectionalLightData& OutData // #93
	);
	T4ENGINE_API void BlendBPSkySphere(
		const FT4EnvBPSkySphereData* InData,
		float InWeight,
		FT4EnvBPSkySphereData& OutData
	); // #97
	T4ENGINE_API void BlendSkyLight(
		const FT4EnvSkyLightData* InData,
		float InWeight,
		FT4EnvSkyLightData& OutData // #93
	);
	T4ENGINE_API void BlendAtmosphericFog(
		const FT4EnvAtmosphericFogData* InData,
		float InWeight,
		FT4EnvAtmosphericFogData& OutData // #93
	);
	T4ENGINE_API void BlendExponentialHeightFog(
		const FT4EnvExponentialHeightFogData* InData,
		float InWeight,
		FT4EnvExponentialHeightFogData& OutData // #93
	);
	T4ENGINE_API void BlendPostProcess(
		const FPostProcessSettings* InData,
		ET4BuiltInEasing InBlendCurve,
		float InWeight,
		FFinalPostProcessSettings& OutData // #98
	);

	// #T4_ADD_TOD_TAG
	// #97
	T4ENGINE_API ADirectionalLight* SpawnDirectionalLightActor(
		UWorld* InWorld, 
		EObjectFlags InObjectFlags,
		const FT4EnvDirectionalLightData* InData = nullptr
	);
	T4ENGINE_API AActor* SpawnBPSkySphereActor(
		UWorld* InWorld,
		EObjectFlags InObjectFlags,
		const FT4EnvBPSkySphereData* InData = nullptr
	);
	T4ENGINE_API ASkyLight* SpawnSkyLightActor(
		UWorld* InWorld, 
		EObjectFlags InObjectFlags,
		const FT4EnvSkyLightData* InData = nullptr
	);
	T4ENGINE_API AAtmosphericFog* SpawnAtmosphericFogActor(
		UWorld* InWorld, 
		EObjectFlags InObjectFlags,
		const FT4EnvAtmosphericFogData* InData = nullptr
	);
	T4ENGINE_API AExponentialHeightFog* SpawnExponentialHeightFogActor(
		UWorld* InWorld, 
		EObjectFlags InObjectFlags,
		const FT4EnvExponentialHeightFogData* InData = nullptr
	);
	// ~#97

	// #97
	T4ENGINE_API ADirectionalLight* FindDirectionalLightActor(UWorld* InWorld);
	T4ENGINE_API AActor* FindBPSkySphereActor(UWorld* InWorld); // #97
	T4ENGINE_API ASkyLight* FindSkyLightActor(UWorld* InWorld);
	T4ENGINE_API AAtmosphericFog* FindAtmosphericFogActor(UWorld* InWorld);
	T4ENGINE_API AExponentialHeightFog* FindExponentialHeightFogActor(UWorld* InWorld);
	T4ENGINE_API AT4WorldZoneVolume* FindGlobalWorldZoneVolume(UWorld* InWorld); // #98
	// #97

#if WITH_EDITOR
	// #T4_ADD_TOD_TAG
	T4ENGINE_API bool GetDirectionalData(UWorld* InWorld, FT4EnvDirectionalData* OutData); // #93
	T4ENGINE_API bool GetDirectionalLightData(UWorld* InWorld, FT4EnvDirectionalLightData* OutData);
	T4ENGINE_API bool GetBPSkySphereData(UWorld* InWorld, FT4EnvBPSkySphereData* OutData); // #97
	T4ENGINE_API bool GetSkyLightData(UWorld* InWorld, FT4EnvSkyLightData* OutData);
	T4ENGINE_API bool GetAtmosphericFogData(UWorld* InWorld, FT4EnvAtmosphericFogData* OutData);
	T4ENGINE_API bool GetExponentialHeightFogData(UWorld* InWorld, FT4EnvExponentialHeightFogData* OutData);
	T4ENGINE_API bool GetGlobalPostProcessData(UWorld* InWorld, FT4EnvPostProcessData* OutData); // #98
#endif
}