// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Public/T4EngineEnvironment.h"

#include "Classes/World/T4WorldZoneVolume.h" // #92
#include "Public/T4EngineDefinitions.h" // #93
#include "Public/T4EngineStructs.h" // #93

#include "Engine/TextureCube.h"
#include "Components/ExponentialHeightFogComponent.h" // #90
#include "Engine/ExponentialHeightFog.h" // #90
#include "Atmosphere/AtmosphericFogComponent.h" // #90
#include "Atmosphere/AtmosphericFog.h" // #90
#include "Components/SkyLightComponent.h" // #90
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h" // #90
#include "Engine/DirectionalLight.h" // #90
#include "LightPropagationVolumeSettings.h" // #115

#include "Kismet/GameplayStatics.h"
#include "Misc/OutputDeviceNull.h" // #97

#include "Engine/Scene.h" // #98

#if WITH_EDITOR
#include "Engine/PostProcessVolume.h" // #98
#include "Model.h" // #92
#include "Components/BrushComponent.h" // #92
#include "Engine/BrushBuilder.h" // #92
#include "Builders/CylinderBuilder.h" // #94
#include "Builders/CubeBuilder.h" // #92
#include "Engine/Polys.h"// #92
#include "BSPOps.h" // #92
#endif

#include "EngineUtils.h"

#include "T4EngineInternal.h"

// #97
static const FName BPSkySpherePropertyDirectionalLightActorName(TEXT("Directional light actor"));
static const FName BPSkySpherePropertyColorsDeterminedBySunPositionName(TEXT("Colors Determined By Sun Position"));
static const FName BPSkySpherePropertySunHeightName(TEXT("Sun Height"));

static const FName BPSkySpherePropertySunBrightnessName(TEXT("Sun Brightness")); // #98
static const FName BPSkySpherePropertyCloudSpeedName(TEXT("Cloud Speed")); // #98
static const FName BPSkySpherePropertyCloudOpacityName(TEXT("Cloud Opacity")); // #98
static const FName BPSkySpherePropertyStarsBrightnessName(TEXT("Stars Brightness")); // #98
static const FName BPSkySpherePropertyZenithColorName(TEXT("Zenith Color")); // #98
static const FName BPSkySpherePropertyHorizonColorName(TEXT("Horizon Color")); // #98
static const FName BPSkySpherePropertyCloudColorName(TEXT("Cloud Color")); // #98
static const FName BPSkySpherePropertyOverallColorName(TEXT("Overall Color")); // #98

static const FName BPSkySphereFunctionRefreshMaterialName(TEXT("RefreshMaterial"));
static const FName BPSkySphereFunctionUpdateSunDirectionName(TEXT("UpdateSunDirection"));
// ~#97

/**
  * #92
 */
FT4UpdateTime FT4UpdateTime::EmptyUpdateTime = { 0.0f, 0.0f , 1.0f, false }; // #102

namespace T4EngineEnvironment
{

	// #102 : refer MovieSceneEasingCurves.cpp
	namespace Impl
	{
		float SinOut(float InTime)
		{
			return FMath::Sin(.5f * PI * InTime);
		}
		float SinIn(float InTime)
		{
			return 1.f - SinOut(1.f - InTime);
		}
		float PowIn(float InTime, float Power)
		{
			return FMath::Pow(InTime, Power);
		}
		float PowOut(float InTime, float Power)
		{
			return 1.f - FMath::Pow(1.f - InTime, Power);
		}
		float ExpIn(float InTime)
		{
			return FMath::Pow(2, 10 * (InTime - 1.f));
		}
		float ExpOut(float InTime)
		{
			return 1.f - ExpIn(1.f - InTime);
		}
		float CircIn(float InTime)
		{
			return 1.f - FMath::Sqrt(1 - InTime * InTime);
		}
		float CircOut(float InTime)
		{
			return 1.f - CircIn(1.f - InTime);
		}
	}

	float Evaluate(
		ET4BuiltInEasing InBlendCurve,
		float InInterp
	)
	{
		const float InTime = InInterp * 2.f;
		const float OutTime = (InInterp - .5f) * 2.f;

		switch (InBlendCurve)
		{
			case ET4BuiltInEasing::SinIn: 		return Impl::SinIn(InInterp);
			case ET4BuiltInEasing::SinOut: 		return Impl::SinOut(InInterp);
			case ET4BuiltInEasing::SinInOut: 	return InTime < 1.f ? .5f * Impl::SinIn(InTime) : .5f + .5f * Impl::SinOut(OutTime);

			case ET4BuiltInEasing::QuadIn: 		return Impl::PowIn(InInterp, 2);
			case ET4BuiltInEasing::QuadOut: 	return Impl::PowOut(InInterp, 2);
			case ET4BuiltInEasing::QuadInOut: 	return InTime < 1.f ? .5f * Impl::PowIn(InTime, 2) : .5f + .5f * Impl::PowOut(OutTime, 2);

			case ET4BuiltInEasing::CubicIn: 	return Impl::PowIn(InInterp, 3);
			case ET4BuiltInEasing::CubicOut: 	return Impl::PowOut(InInterp, 3);
			case ET4BuiltInEasing::CubicInOut: 	return InTime < 1.f ? .5f * Impl::PowIn(InTime, 3) : .5f + .5f * Impl::PowOut(OutTime, 3);

			case ET4BuiltInEasing::QuartIn: 	return Impl::PowIn(InInterp, 4);
			case ET4BuiltInEasing::QuartOut: 	return Impl::PowOut(InInterp, 4);
			case ET4BuiltInEasing::QuartInOut: 	return InTime < 1.f ? .5f * Impl::PowIn(InTime, 4) : .5f + .5f * Impl::PowOut(OutTime, 4);

			case ET4BuiltInEasing::QuintIn: 	return Impl::PowIn(InInterp, 5);
			case ET4BuiltInEasing::QuintOut: 	return Impl::PowOut(InInterp, 5);
			case ET4BuiltInEasing::QuintInOut: 	return InTime < 1.f ? .5f * Impl::PowIn(InTime, 5) : .5f + .5f * Impl::PowOut(OutTime, 5);

			case ET4BuiltInEasing::ExpoIn: 		return Impl::ExpIn(InInterp);
			case ET4BuiltInEasing::ExpoOut: 	return Impl::ExpOut(InInterp);
			case ET4BuiltInEasing::ExpoInOut: 	return InTime < 1.f ? .5f * Impl::ExpIn(InTime) : .5f + .5f * Impl::ExpOut(OutTime);

			case ET4BuiltInEasing::CircIn: 		return Impl::CircIn(InInterp);
			case ET4BuiltInEasing::CircOut: 	return Impl::CircOut(InInterp);
			case ET4BuiltInEasing::CircInOut: 	return InTime < 1.f ? .5f * Impl::CircIn(InTime) : .5f + .5f * Impl::CircOut(OutTime);

			case ET4BuiltInEasing::Linear:
			default:
				return InInterp;
		}
	}
	// ~#102

	UClass* GetBPSkySphereClass() // #97
	{
		const FSoftObjectPath SkySpherePath = TEXT("/Engine/EngineSky/BP_Sky_Sphere.BP_Sky_Sphere_C"); // #97
#if 1
		return LoadClass<UObject>(nullptr, *SkySpherePath.ToString());
#else
		Cast<UClass>(SkySpherePath.TryLoad());
#endif
	}

	UObjectProperty* FindBlueprintObjectProperty(AActor* InBPActor, FName InPropertyName, UObject** OutValue) // #97
	{
		UObjectProperty* FoundProperty = FindField<UObjectProperty>(InBPActor->GetClass(), InPropertyName);
		if (nullptr != FoundProperty && nullptr != OutValue)
		{
			*OutValue = FoundProperty->GetPropertyValue_InContainer(InBPActor);
		}
		return FoundProperty;
	}

	UBoolProperty* FindBlueprintBoolProperty(AActor* InBPActor, FName InPropertyName, bool* OutValue) // #97
	{
		UBoolProperty* FoundProperty = FindField<UBoolProperty>(InBPActor->GetClass(), InPropertyName);
		if (nullptr != FoundProperty && nullptr != OutValue)
		{
			*OutValue = FoundProperty->GetPropertyValue_InContainer(InBPActor);
		}
		return FoundProperty;
	}

	UFloatProperty* FindBlueprintFloatProperty(AActor* InBPActor, FName InPropertyName, float* OutValue) // #97
	{
		UFloatProperty* FoundProperty = FindField<UFloatProperty>(InBPActor->GetClass(), InPropertyName);
		if (nullptr != FoundProperty && nullptr != OutValue)
		{
			*OutValue = FoundProperty->GetPropertyValue_InContainer(InBPActor);
		}
		return FoundProperty;
	}

	UStructProperty* FindBlueprintColorProperty(AActor* InBPActor, FName InPropertyName, FColor* OutValue) // #97
	{
		UStructProperty* FoundProperty = FindField<UStructProperty>(InBPActor->GetClass(), InPropertyName);
		if (nullptr != FoundProperty && nullptr != OutValue)
		{
			*OutValue = *FoundProperty->ContainerPtrToValuePtr<FColor>(InBPActor);
		}
		return FoundProperty;
	}

	UFunction* FindBlueprintFunction(AActor* InBPActor, FName InFunctionName) // #97
	{
		UFunction* FoundFunction = FindField<UFunction>(InBPActor->GetClass(), InFunctionName);
		return FoundFunction;
	}

	AT4WorldZoneVolume* FindWorldZoneVolumeOnWorld(
		UWorld* InWorld,
		FName InWorldZoneName
	)
	{
		check(nullptr != InWorld);
		for (TActorIterator<AT4WorldZoneVolume> It(InWorld); It; ++It)
		{
			AT4WorldZoneVolume* ZoneVolume = *It;
			check(nullptr != ZoneVolume);
			if (ZoneVolume->ZoneName == InWorldZoneName)
			{
				return ZoneVolume;
			}
		}
		return nullptr;
	}

	bool GetWorldZoneVolumesOnWorld(
		UWorld* InWorld,
		TArray<AT4WorldZoneVolume*>& OutWorldZoneVolumes
	)
	{
		check(nullptr != InWorld);
		for (TActorIterator<AT4WorldZoneVolume> It(InWorld); It; ++It)
		{
			AT4WorldZoneVolume* ZoneVolume = *It;
			check(nullptr != ZoneVolume);
			OutWorldZoneVolumes.Add(ZoneVolume);
		}
		return (0 < OutWorldZoneVolumes.Num()) ? true : false;
	}

	AT4WorldZoneVolume* WorldSpawnWorldZoneVolume(
		UWorld* InWorld,
		const FVector& InLocation,
		const FRotator& InRotation,
		const FVector& InScale,
		ET4EntityZoneBrushType InBrushType,
		FName InZoneName,
		bool bInTransient
	) // #94
	{
		if (!bInTransient)
		{
			InWorld->MarkPackageDirty();
		}

		FActorSpawnParameters SpawnInfo;
		SpawnInfo.ObjectFlags |= (bInTransient) ? RF_Transient : RF_NoFlags;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnInfo.bDeferConstruction = true; // deley construction
		AT4WorldZoneVolume* NewWorldZoneVolume = InWorld->SpawnActor<AT4WorldZoneVolume>(
			AT4WorldZoneVolume::StaticClass(),
			InLocation,
			InRotation,
			SpawnInfo
		);
		if (nullptr == NewWorldZoneVolume)
		{
			return nullptr;
		}
		
		NewWorldZoneVolume->ZoneName = InZoneName; // #115 : Spawn Load 시 IsGlobalZone 유효 지원

#if WITH_EDITOR
		if (ET4EntityZoneBrushType::None != InBrushType)
		{
			// #92 : refer CreateBrushForVolumeActor
			NewWorldZoneVolume->PolyFlags = 0;
			NewWorldZoneVolume->Brush = NewObject<UModel>(NewWorldZoneVolume, NAME_None, RF_Transactional);
			NewWorldZoneVolume->Brush->Initialize(nullptr, true);
			NewWorldZoneVolume->Brush->Polys = NewObject<UPolys>(NewWorldZoneVolume->Brush, NAME_None, RF_Transactional);
			NewWorldZoneVolume->GetBrushComponent()->Brush = NewWorldZoneVolume->Brush;
			NewWorldZoneVolume->GetBrushComponent()->SetMobility(EComponentMobility::Movable);

			if (ET4EntityZoneBrushType::Cube == InBrushType)
			{
				UCubeBuilder* BrushBuilder = NewObject<UCubeBuilder>();
				check(nullptr != BrushBuilder);
				NewWorldZoneVolume->BrushBuilder = DuplicateObject<UBrushBuilder>(BrushBuilder, NewWorldZoneVolume);
				BrushBuilder->Build(InWorld, NewWorldZoneVolume);
			}
			else if (ET4EntityZoneBrushType::Cylinder == InBrushType)
			{
				UCylinderBuilder* BrushBuilder = NewObject<UCylinderBuilder>();
				check(nullptr != BrushBuilder);
				NewWorldZoneVolume->BrushBuilder = DuplicateObject<UBrushBuilder>(BrushBuilder, NewWorldZoneVolume);
				BrushBuilder->Build(InWorld, NewWorldZoneVolume);
			}
			else
			{
				check(false);
			}

			FBSPOps::csgPrepMovingBrush(NewWorldZoneVolume);

			// Set the texture on all polys to NULL.  This stops invisible textures
			// dependencies from being formed on volumes.
			if (NewWorldZoneVolume->Brush)
			{
				for (int32 poly = 0; poly < NewWorldZoneVolume->Brush->Polys->Element.Num(); ++poly)
				{
					FPoly* Poly = &(NewWorldZoneVolume->Brush->Polys->Element[poly]);
					Poly->Material = nullptr;
				}
			}
		}
#endif

		const FTransform SpawnTransform(InRotation, InLocation, InScale);
		UGameplayStatics::FinishSpawningActor(NewWorldZoneVolume, SpawnTransform); // bDeferConstruction = true; // deley construction

		return NewWorldZoneVolume;
	}

	FName GetNextTimeTagName(FName InTimeName) // #93
	{
		if (T4Const_WorldTimeTagDayName == InTimeName)
		{
			return T4Const_WorldTimeTagNoonName; // #97 : 정오, 한밤 추가!
		}
		if (T4Const_WorldTimeTagNoonName == InTimeName) // #97 : 정오, 한밤 추가!
		{
			return T4Const_WorldTimeTagSunsetName;
		}
		else if (T4Const_WorldTimeTagSunsetName == InTimeName)
		{
			return T4Const_WorldTimeTagNightName;
		}
		else if (T4Const_WorldTimeTagNightName == InTimeName)
		{
			return T4Const_WorldTimeTagMidnightName; // #97 : 정오, 한밤 추가!
		}
		else if (T4Const_WorldTimeTagMidnightName == InTimeName) // #97 : 정오, 한밤 추가!
		{
			return T4Const_WorldTimeTagSunriseName;
		}
		else if (T4Const_WorldTimeTagSunriseName == InTimeName)
		{
			return T4Const_WorldTimeTagDayName;
		}
		return T4Const_WorldTimeTagFallbackName;
	}

	FName GetPrevTimeTagName(FName InTimeName) // #93
	{
		if (T4Const_WorldTimeTagDayName == InTimeName)
		{
			return T4Const_WorldTimeTagSunriseName;
		}
		else if (T4Const_WorldTimeTagNoonName == InTimeName) // #97 : 정오, 한밤 추가!
		{
			return T4Const_WorldTimeTagDayName;
		}
		else if (T4Const_WorldTimeTagSunsetName == InTimeName)
		{
			return T4Const_WorldTimeTagNoonName; // #97 : 정오, 한밤 추가!
		}
		else if (T4Const_WorldTimeTagNightName == InTimeName)
		{
			return T4Const_WorldTimeTagSunsetName;
		}
		else if (T4Const_WorldTimeTagMidnightName == InTimeName) // #97 : 정오, 한밤 추가!
		{
			return T4Const_WorldTimeTagNightName; // #97 : 정오, 한밤 추가!
		}
		else if (T4Const_WorldTimeTagSunriseName == InTimeName)
		{
			return T4Const_WorldTimeTagMidnightName; // #97 : 정오, 한밤 추가!
		}
		return T4Const_WorldTimeTagFallbackName;
	}

	// #T4_ADD_TOD_TAG

	bool ApplyDirectional(
		UWorld* InWorld,
		const FT4EnvDirectionalData* InData,
		ADirectionalLight* InCachedActor // #92
	) // #93
	{
		if (nullptr == InCachedActor)
		{
			InCachedActor = FindDirectionalLightActor(InWorld);
		}
		if (nullptr == InCachedActor)
		{
			return false;
		}
		UDirectionalLightComponent* Component = Cast<UDirectionalLightComponent>(InCachedActor->GetLightComponent());
		if (nullptr != Component)
		{
			const FRotator ActorRotation = InCachedActor->GetActorRotation();
			if (ActorRotation != InData->Rotation)
			{
				// WARN : PIE 에서는 Actor Mobility 가 Moveable 이 아니면 조정이 안된다! 리소스를 수정할 것!
				InCachedActor->SetActorRotation(InData->Rotation);
			}
			return true;
		}
		return false;
	}

	bool ApplyDirectionalLight(
		UWorld* InWorld,
		const FT4EnvDirectionalLightData* InData,
		ADirectionalLight* InCachedActor // #92
	)
	{
		if (nullptr == InCachedActor)
		{
			InCachedActor = FindDirectionalLightActor(InWorld);
		}
		if (nullptr == InCachedActor)
		{
			return false;
		}
		UDirectionalLightComponent* Component = Cast<UDirectionalLightComponent>(InCachedActor->GetLightComponent());
		if (nullptr != Component)
		{
			bool bStateDirty = false;
			if (Component->Intensity != InData->Intensity)
			{
				Component->Intensity = InData->Intensity;
				bStateDirty = true;
			}
			if (Component->LightColor != InData->LightColor)
			{
				Component->LightColor = InData->LightColor;
				bStateDirty = true;
			}
			if (bStateDirty)
			{
				Component->MarkRenderStateDirty();
			}
			return true;
		}
		return false;
	}

	bool ApplyBPSkySphere(
		UWorld* InWorld,
		const FT4EnvBPSkySphereData* InData,
		AActor* InCachedActor,
		ADirectionalLight* InCachedDirectionalLightActor
	) // #97
	{
		if (nullptr == InCachedActor)
		{
			InCachedActor = FindBPSkySphereActor(InWorld);
		}
		if (nullptr == InCachedActor)
		{
			return false;
		}
		if (nullptr == InCachedDirectionalLightActor)
		{
			InCachedDirectionalLightActor = FindDirectionalLightActor(InWorld);
		}
		if (nullptr == InCachedDirectionalLightActor)
		{
			return false;
		}
		bool bStateDirty = false;
		{

			UObject* CurrentValue = nullptr;
			UObjectProperty* FoundProperty = FindBlueprintObjectProperty(InCachedActor, BPSkySpherePropertyDirectionalLightActorName, &CurrentValue);
			if (nullptr != FoundProperty)
			{
				if (InCachedDirectionalLightActor != CurrentValue)
				{
					FoundProperty->SetObjectPropertyValue_InContainer(InCachedActor, InCachedDirectionalLightActor);
					bStateDirty = true;
				}
			}
		}
		{
			float CurrentValue = 0.0f; // #98
			UFloatProperty* FoundProperty = FindBlueprintFloatProperty(InCachedActor, BPSkySpherePropertySunBrightnessName, &CurrentValue);
			if (nullptr != FoundProperty)
			{
				if (InData->SunBrightness != CurrentValue)
				{
					FoundProperty->SetPropertyValue_InContainer(InCachedActor, InData->SunBrightness);
					bStateDirty = true;
				}
			}
		}
		{
			float CurrentValue = 0.0f; // #98
			UFloatProperty* FoundProperty = FindBlueprintFloatProperty(InCachedActor, BPSkySpherePropertyCloudSpeedName, &CurrentValue);
			if (nullptr != FoundProperty)
			{
				if (InData->CloudSpeed != CurrentValue)
				{
					FoundProperty->SetPropertyValue_InContainer(InCachedActor, InData->CloudSpeed);
					bStateDirty = true;
				}
			}
		}
		{
			float CurrentValue = 0.0f; // #98
			UFloatProperty* FoundProperty = FindBlueprintFloatProperty(InCachedActor, BPSkySpherePropertyCloudOpacityName, &CurrentValue);
			if (nullptr != FoundProperty)
			{
				if (InData->CloudOpacity != CurrentValue)
				{
					FoundProperty->SetPropertyValue_InContainer(InCachedActor, InData->CloudOpacity);
					bStateDirty = true;
				}
			}
		}
		{
			float CurrentValue = 0.0f; // #98
			UFloatProperty* FoundProperty = FindBlueprintFloatProperty(InCachedActor, BPSkySpherePropertyStarsBrightnessName, &CurrentValue);
			if (nullptr != FoundProperty)
			{
				if (InData->StarsBrightness != CurrentValue)
				{
					FoundProperty->SetPropertyValue_InContainer(InCachedActor, InData->StarsBrightness);
					bStateDirty = true;
				}
			}
		}
		{
			bool CurrentValue = false;
			UBoolProperty* FoundProperty = FindBlueprintBoolProperty(InCachedActor, BPSkySpherePropertyColorsDeterminedBySunPositionName, &CurrentValue);
			if (nullptr != FoundProperty)
			{
				if (InData->bColorsDeterminedBySunPosition != CurrentValue)
				{
					FoundProperty->SetPropertyValue_InContainer(InCachedActor, InData->bColorsDeterminedBySunPosition);
					bStateDirty = true;
				}
			}
		}
		if (InData->bColorsDeterminedBySunPosition) // #98
		{
			{
				FColor CurrentValue; 
				UStructProperty* FoundProperty = FindBlueprintColorProperty(InCachedActor, BPSkySpherePropertyZenithColorName, &CurrentValue);
				if (nullptr != FoundProperty)
				{
					if (InData->ZenithColor != CurrentValue)
					{
						*FoundProperty->ContainerPtrToValuePtr<FColor>(InCachedActor) = InData->ZenithColor;
						bStateDirty = true;
					}
				}
			}
			{
				FColor CurrentValue;
				UStructProperty* FoundProperty = FindBlueprintColorProperty(InCachedActor, BPSkySpherePropertyHorizonColorName, &CurrentValue);
				if (nullptr != FoundProperty)
				{
					if (InData->HorizonColor != CurrentValue)
					{
						*FoundProperty->ContainerPtrToValuePtr<FColor>(InCachedActor) = InData->HorizonColor;
						bStateDirty = true;
					}
				}
			}
			{
				FColor CurrentValue;
				UStructProperty* FoundProperty = FindBlueprintColorProperty(InCachedActor, BPSkySpherePropertyCloudColorName, &CurrentValue);
				if (nullptr != FoundProperty)
				{
					if (InData->CloudColor != CurrentValue)
					{
						*FoundProperty->ContainerPtrToValuePtr<FColor>(InCachedActor) = InData->CloudColor;
						bStateDirty = true;
					}
				}
			}
			{
				FColor CurrentValue;
				UStructProperty* FoundProperty = FindBlueprintColorProperty(InCachedActor, BPSkySpherePropertyOverallColorName, &CurrentValue);
				if (nullptr != FoundProperty)
				{
					if (InData->OverallColor != CurrentValue)
					{
						*FoundProperty->ContainerPtrToValuePtr<FColor>(InCachedActor) = InData->OverallColor;
						bStateDirty = true;
					}
				}
			}
		}
		{
			FOutputDeviceNull Ar;
			if (InData->bColorsDeterminedBySunPosition || bStateDirty)
			{
				// #97 : BP 를 통한 업데이트를 위해 UFunction 을 직업 호출해 업데이트 되도록 처리한다.
				UFunction* FoundFunction = FindBlueprintFunction(InCachedActor, BPSkySphereFunctionRefreshMaterialName);
				if (nullptr != FoundFunction)
				{
					InCachedActor->CallFunctionByNameWithArguments(*(BPSkySphereFunctionRefreshMaterialName.ToString()), Ar, nullptr, true);
				}
			}
			UFunction* FoundFunction = FindBlueprintFunction(InCachedActor, BPSkySphereFunctionUpdateSunDirectionName);
			if (nullptr != FoundFunction)
			{
				InCachedActor->CallFunctionByNameWithArguments(*(BPSkySphereFunctionUpdateSunDirectionName.ToString()), Ar, nullptr, true);
			}
		}
		return true;
	}

	bool ApplySkyLight(
		UWorld* InWorld,
		const FT4EnvSkyLightData* InData,
		ASkyLight* InCachedActor
	)
	{
		if (nullptr == InCachedActor)
		{
			InCachedActor = FindSkyLightActor(InWorld);
		}
		if (nullptr == InCachedActor)
		{
			return false;
		}
		USkyLightComponent* Component = Cast<USkyLightComponent>(InCachedActor->GetLightComponent());
		if (nullptr != Component)
		{
			bool bStateDirty = false;
			if (Component->SourceType != InData->SourceType) // #97
			{
				Component->SourceType = InData->SourceType;
				bStateDirty = true;
			}
			UTextureCube* TextureCuve = (InData->CubemapPtr.IsNull()) ? nullptr : InData->CubemapPtr.LoadSynchronous();
			if (Component->Cubemap != TextureCuve)
			{
				Component->Cubemap = TextureCuve;
				bStateDirty = true;
			}
			if (Component->CubemapResolution != InData->CubemapResolution)
			{
				Component->CubemapResolution = InData->CubemapResolution;
				bStateDirty = true;
			}
			if (Component->Intensity != InData->Intensity)
			{
				Component->Intensity = InData->Intensity;
				bStateDirty = true;
			}
			if (Component->LightColor != InData->LightColor)
			{
				Component->LightColor = InData->LightColor;
				bStateDirty = true;
			}
			if (bStateDirty)
			{
				Component->MarkRenderStateDirty();
			}
			return true;
		}
		return false;
	}

	bool ApplyAtmosphericFog(
		UWorld* InWorld,
		const FT4EnvAtmosphericFogData* InData,
		AAtmosphericFog* InCachedActor
	)
	{
		if (nullptr == InCachedActor)
		{
			InCachedActor = FindAtmosphericFogActor(InWorld);
		}
		if (nullptr == InCachedActor)
		{
			return false;
		}
		UAtmosphericFogComponent* Component = Cast<UAtmosphericFogComponent>(InCachedActor->GetAtmosphericFogComponent());
		if (nullptr != Component)
		{
			bool bStateDirty = false;
			if (Component->SunMultiplier != InData->SunMultiplier)
			{
				Component->SunMultiplier = InData->SunMultiplier;
				bStateDirty = true;
			}
			if (Component->FogMultiplier != InData->FogMultiplier)
			{
				Component->FogMultiplier = InData->FogMultiplier;
				bStateDirty = true;
			}
			if (Component->DensityMultiplier != InData->DensityMultiplier)
			{
				Component->DensityMultiplier = InData->DensityMultiplier;
				bStateDirty = true;
			}
			if (Component->DensityOffset != InData->DensityOffset)
			{
				Component->DensityOffset = InData->DensityOffset;
				bStateDirty = true;
			}
			if (bStateDirty)
			{
				Component->MarkRenderStateDirty();
			}
			return true;
		}
		return false;
	}

	bool ApplyExponentialHeightFog(
		UWorld* InWorld,
		const FT4EnvExponentialHeightFogData* InData,
		AExponentialHeightFog* InCachedActor
	)
	{
		if (nullptr == InCachedActor)
		{
			InCachedActor = FindExponentialHeightFogActor(InWorld);
		}
		if (nullptr == InCachedActor)
		{
			return false;
		}
		UExponentialHeightFogComponent* Component = Cast<UExponentialHeightFogComponent>(InCachedActor->GetComponent());
		if (nullptr != Component)
		{
			bool bStateDirty = false;
			if (Component->FogDensity != InData->FogDensity)
			{
				Component->FogDensity = InData->FogDensity;
				bStateDirty = true;
			}
			if (Component->FogHeightFalloff != InData->FogHeightFalloff)
			{
				Component->FogHeightFalloff = InData->FogHeightFalloff;
				bStateDirty = true;
			}
			if (Component->SecondFogData.FogDensity != InData->SecondFogData.FogDensity ||
				Component->SecondFogData.FogHeightFalloff != InData->SecondFogData.FogHeightFalloff ||
				Component->SecondFogData.FogHeightOffset != InData->SecondFogData.FogHeightOffset)
			{
				Component->SecondFogData = InData->SecondFogData;
				bStateDirty = true;
			}
			if (Component->FogInscatteringColor != InData->FogInscatteringColor)
			{
				Component->FogInscatteringColor = InData->FogInscatteringColor;
				bStateDirty = true;
			}
			if (bStateDirty)
			{
				Component->MarkRenderStateDirty();
			}
			return true;
		}
		return false;
	}

	bool ApplyPostProcess(
		UWorld* InWorld,
		const FPostProcessSettings* InData,
		AT4WorldZoneVolume* InCachedVolume // #98
	)
	{
		if (nullptr == InCachedVolume)
		{
			InCachedVolume = FindGlobalWorldZoneVolume(InWorld);
		}
		if (nullptr == InCachedVolume)
		{
			return false;
		}
		check(InCachedVolume->IsGlobalZone());
		if (nullptr == InCachedVolume->PostProcessingSettings)
		{
			return false;
		}
		*InCachedVolume->PostProcessingSettings = *InData;
		return true;
	}

	// #93
	void BlendTimeTagData(
		const FT4EnvTimeTagData& InTimeTagData,
		float InWeight,
		FT4EnvTimeTagData& OutTimeTagData
	) // #93
	{
		// #T4_ADD_TOD_TAG
		BlendDirectional(&InTimeTagData.DirectionalData, InWeight, OutTimeTagData.DirectionalData);
		BlendDirectionalLight(&InTimeTagData.DirectionalLightData, InWeight, OutTimeTagData.DirectionalLightData);
		BlendBPSkySphere(&InTimeTagData.BPSkySphereData, InWeight, OutTimeTagData.BPSkySphereData); // #97
		BlendSkyLight(&InTimeTagData.SkyLightData, InWeight, OutTimeTagData.SkyLightData);
		BlendAtmosphericFog(&InTimeTagData.AtmosphericFogData, InWeight, OutTimeTagData.AtmosphericFogData);
		BlendExponentialHeightFog(&InTimeTagData.ExponentialHeightFogData, InWeight, OutTimeTagData.ExponentialHeightFogData);
		// #98 : BlendPostProcess 는 별도로 처리!!
	}

	// #T4_ADD_TOD_TAG
	void BlendDirectional(
		const FT4EnvDirectionalData* InData,
		float InWeight,
		FT4EnvDirectionalData& OutData // #93
	) // #93
	{
		if (!InData->bEnabled)
		{
			return;
		}
		if (1.0f == InWeight)
		{
			OutData = *InData;
			return;
		}
		//float SourcePitch = OutData.Rotation.Pitch;
		OutData.Rotation = FMath::Lerp(OutData.Rotation, InData->Rotation, InWeight);
#if 0
		T4_LOG(
			Display, 
			TEXT("%.2f = (%.2f ~ %.2f) %.2f"),
			OutData.Rotation.Pitch,
			SourcePitch,
			InData->Rotation.Pitch,
			InWeight
		);
#endif
	}

	void BlendDirectionalLight(
		const FT4EnvDirectionalLightData* InData,
		float InWeight,
		FT4EnvDirectionalLightData& OutData // #93
	)
	{
		if (!InData->bEnabled)
		{
			return;
		}
		if (1.0f == InWeight)
		{
			OutData = *InData;
			return;
		}
		OutData.Intensity = FMath::Lerp(OutData.Intensity, InData->Intensity, InWeight);
		OutData.LightColor.R = FMath::Lerp(OutData.LightColor.R, InData->LightColor.R, InWeight);
		OutData.LightColor.G = FMath::Lerp(OutData.LightColor.G, InData->LightColor.G, InWeight);
		OutData.LightColor.B = FMath::Lerp(OutData.LightColor.B, InData->LightColor.B, InWeight);
		OutData.LightColor.A = FMath::Lerp(OutData.LightColor.A, InData->LightColor.A, InWeight);
	}

	void BlendBPSkySphere(
		const FT4EnvBPSkySphereData* InData,
		float InWeight,
		FT4EnvBPSkySphereData& OutData
	) // #97
	{
		if (!InData->bEnabled)
		{
			return;
		}
		if (1.0f == InWeight)
		{
			OutData = *InData;
			return;
		}
		OutData.SunBrightness = FMath::Lerp(OutData.SunBrightness, InData->SunBrightness, InWeight); // #98
		OutData.CloudSpeed = FMath::Lerp(OutData.CloudSpeed, InData->CloudSpeed, InWeight); // #98
		OutData.CloudOpacity = FMath::Lerp(OutData.CloudOpacity, InData->CloudOpacity, InWeight); // #98
		OutData.StarsBrightness = FMath::Lerp(OutData.StarsBrightness, InData->StarsBrightness, InWeight); // #98

		OutData.bColorsDeterminedBySunPosition = InData->bColorsDeterminedBySunPosition; 
		if (OutData.bColorsDeterminedBySunPosition) // #98
		{
			OutData.ZenithColor.R = FMath::Lerp(OutData.ZenithColor.R, InData->ZenithColor.R, InWeight);
			OutData.ZenithColor.G = FMath::Lerp(OutData.ZenithColor.G, InData->ZenithColor.G, InWeight);
			OutData.ZenithColor.B = FMath::Lerp(OutData.ZenithColor.B, InData->ZenithColor.B, InWeight);
			OutData.ZenithColor.A = FMath::Lerp(OutData.ZenithColor.A, InData->ZenithColor.A, InWeight);

			OutData.HorizonColor.R = FMath::Lerp(OutData.HorizonColor.R, InData->HorizonColor.R, InWeight);
			OutData.HorizonColor.G = FMath::Lerp(OutData.HorizonColor.G, InData->HorizonColor.G, InWeight);
			OutData.HorizonColor.B = FMath::Lerp(OutData.HorizonColor.B, InData->HorizonColor.B, InWeight);
			OutData.HorizonColor.A = FMath::Lerp(OutData.HorizonColor.A, InData->HorizonColor.A, InWeight);

			OutData.CloudColor.R = FMath::Lerp(OutData.CloudColor.R, InData->CloudColor.R, InWeight);
			OutData.CloudColor.G = FMath::Lerp(OutData.CloudColor.G, InData->CloudColor.G, InWeight);
			OutData.CloudColor.B = FMath::Lerp(OutData.CloudColor.B, InData->CloudColor.B, InWeight);
			OutData.CloudColor.A = FMath::Lerp(OutData.CloudColor.A, InData->CloudColor.A, InWeight);

			OutData.OverallColor.R = FMath::Lerp(OutData.OverallColor.R, InData->OverallColor.R, InWeight);
			OutData.OverallColor.G = FMath::Lerp(OutData.OverallColor.G, InData->OverallColor.G, InWeight);
			OutData.OverallColor.B = FMath::Lerp(OutData.OverallColor.B, InData->OverallColor.B, InWeight);
			OutData.OverallColor.A = FMath::Lerp(OutData.OverallColor.A, InData->OverallColor.A, InWeight);
		}
	}

	void BlendSkyLight(
		const FT4EnvSkyLightData* InData,
		float InWeight,
		FT4EnvSkyLightData& OutData // #93
	)
	{
		if (!InData->bEnabled)
		{
			return;
		}
		if (1.0f == InWeight)
		{
			OutData = *InData;
			return;
		}
		OutData.SourceType = InData->SourceType; // TODO : CubeMap Blending ? // #97
		OutData.CubemapPtr = InData->CubemapPtr; // TODO : CubeMap Blending
		OutData.CubemapResolution = FMath::Lerp(OutData.CubemapResolution, InData->CubemapResolution, InWeight);
		OutData.Intensity = FMath::Lerp(OutData.Intensity, InData->Intensity, InWeight);
		OutData.LightColor.R = FMath::Lerp(OutData.LightColor.R, InData->LightColor.R, InWeight);
		OutData.LightColor.G = FMath::Lerp(OutData.LightColor.G, InData->LightColor.G, InWeight);
		OutData.LightColor.B = FMath::Lerp(OutData.LightColor.B, InData->LightColor.B, InWeight);
		OutData.LightColor.A = FMath::Lerp(OutData.LightColor.A, InData->LightColor.A, InWeight);
	}

	void BlendAtmosphericFog(
		const FT4EnvAtmosphericFogData* InData,
		float InWeight,
		FT4EnvAtmosphericFogData& OutData // #93
	)
	{
		if (!InData->bEnabled)
		{
			return;
		}
		if (1.0f == InWeight)
		{
			OutData = *InData;
			return;
		}
		OutData.SunMultiplier = FMath::Lerp(OutData.SunMultiplier, InData->SunMultiplier, InWeight);
		OutData.FogMultiplier = FMath::Lerp(OutData.FogMultiplier, InData->FogMultiplier, InWeight);
		OutData.DensityMultiplier = FMath::Lerp(OutData.DensityMultiplier, InData->DensityMultiplier, InWeight);
		OutData.DensityOffset = FMath::Lerp(OutData.DensityOffset, InData->DensityOffset, InWeight);
	}

	void BlendExponentialHeightFog(
		const FT4EnvExponentialHeightFogData* InData,
		float InWeight,
		FT4EnvExponentialHeightFogData& OutData // #93
	)
	{
		if (!InData->bEnabled)
		{
			return;
		}
		if (1.0f == InWeight)
		{
			OutData = *InData;
			return;
		}
		OutData.FogDensity = FMath::Lerp(OutData.FogDensity, InData->FogDensity, InWeight);
		OutData.FogHeightFalloff = FMath::Lerp(OutData.FogHeightFalloff, InData->FogHeightFalloff, InWeight);
		{
			FExponentialHeightFogData& SecondFogData = OutData.SecondFogData;
			SecondFogData.FogDensity = FMath::Lerp(SecondFogData.FogDensity, InData->SecondFogData.FogDensity, InWeight);
			SecondFogData.FogHeightFalloff = FMath::Lerp(SecondFogData.FogHeightFalloff, InData->SecondFogData.FogHeightFalloff, InWeight);
			SecondFogData.FogHeightOffset = FMath::Lerp(SecondFogData.FogHeightOffset, InData->SecondFogData.FogHeightOffset, InWeight);
		}
		OutData.FogInscatteringColor = FMath::Lerp(OutData.FogInscatteringColor, InData->FogInscatteringColor, InWeight);
	}

	void BlendPostProcess(
		const FPostProcessSettings* InData,
		float Weight,
		FFinalPostProcessSettings& OutData
	) // #98 : PostProcess 는 별도로 처리한다.
	{
		check(nullptr != InData);

		if (1.0f == Weight)
		{
			FPostProcessSettings* ConvertSettings = static_cast<FPostProcessSettings*>(&OutData);
			*ConvertSettings = *InData;
			return;
		}

		// WARN : 엔진 업데이트마다 확인 필요!
		// void FSceneView::OverridePostProcessSettings(const FPostProcessSettings & Src, float Weight)

		#define LERP_PP(NAME) if(Src.bOverride_ ## NAME)	Dest . NAME = FMath::Lerp(Dest . NAME, Src . NAME, Weight);
		#define IF_PP(NAME) if(Src.bOverride_ ## NAME && Src . NAME)

		{
			// #4.24

			const FPostProcessSettings& Src = *InData;

			{
				FFinalPostProcessSettings& Dest = OutData;

				// The following code needs to be adjusted when settings in FPostProcessSettings change.
				LERP_PP(WhiteTemp);
				LERP_PP(WhiteTint);

				LERP_PP(ColorSaturation);
				LERP_PP(ColorContrast);
				LERP_PP(ColorGamma);
				LERP_PP(ColorGain);
				LERP_PP(ColorOffset);

				LERP_PP(ColorSaturationShadows);
				LERP_PP(ColorContrastShadows);
				LERP_PP(ColorGammaShadows);
				LERP_PP(ColorGainShadows);
				LERP_PP(ColorOffsetShadows);

				LERP_PP(ColorSaturationMidtones);
				LERP_PP(ColorContrastMidtones);
				LERP_PP(ColorGammaMidtones);
				LERP_PP(ColorGainMidtones);
				LERP_PP(ColorOffsetMidtones);

				LERP_PP(ColorSaturationHighlights);
				LERP_PP(ColorContrastHighlights);
				LERP_PP(ColorGammaHighlights);
				LERP_PP(ColorGainHighlights);
				LERP_PP(ColorOffsetHighlights);

				LERP_PP(ColorCorrectionShadowsMax);
				LERP_PP(ColorCorrectionHighlightsMin);

				LERP_PP(BlueCorrection);
				LERP_PP(ExpandGamut);

				LERP_PP(FilmWhitePoint);
				LERP_PP(FilmSaturation);
				LERP_PP(FilmChannelMixerRed);
				LERP_PP(FilmChannelMixerGreen);
				LERP_PP(FilmChannelMixerBlue);
				LERP_PP(FilmContrast);
				LERP_PP(FilmDynamicRange);
				LERP_PP(FilmHealAmount);
				LERP_PP(FilmToeAmount);
				LERP_PP(FilmShadowTint);
				LERP_PP(FilmShadowTintBlend);
				LERP_PP(FilmShadowTintAmount);

				LERP_PP(FilmSlope);
				LERP_PP(FilmToe);
				LERP_PP(FilmShoulder);
				LERP_PP(FilmBlackClip);
				LERP_PP(FilmWhiteClip);

				LERP_PP(SceneColorTint);
				LERP_PP(SceneFringeIntensity);
				LERP_PP(ChromaticAberrationStartOffset);
				LERP_PP(BloomIntensity);
				LERP_PP(BloomThreshold);
				LERP_PP(Bloom1Tint);
				LERP_PP(BloomSizeScale);
				LERP_PP(Bloom1Size);
				LERP_PP(Bloom2Tint);
				LERP_PP(Bloom2Size);
				LERP_PP(Bloom3Tint);
				LERP_PP(Bloom3Size);
				LERP_PP(Bloom4Tint);
				LERP_PP(Bloom4Size);
				LERP_PP(Bloom5Tint);
				LERP_PP(Bloom5Size);
				LERP_PP(Bloom6Tint);
				LERP_PP(Bloom6Size);
				LERP_PP(BloomDirtMaskIntensity);
				LERP_PP(BloomDirtMaskTint);
				LERP_PP(BloomConvolutionSize);
				LERP_PP(BloomConvolutionCenterUV);
				LERP_PP(BloomConvolutionPreFilterMin);
				LERP_PP(BloomConvolutionPreFilterMax);
				LERP_PP(BloomConvolutionPreFilterMult);
				LERP_PP(AmbientCubemapIntensity);
				LERP_PP(AmbientCubemapTint);
				LERP_PP(CameraShutterSpeed);
				LERP_PP(CameraISO);
				LERP_PP(AutoExposureLowPercent);
				LERP_PP(AutoExposureHighPercent);
				LERP_PP(AutoExposureMinBrightness);
				LERP_PP(AutoExposureMaxBrightness);
				LERP_PP(AutoExposureCalibrationConstant);
				LERP_PP(AutoExposureSpeedUp);
				LERP_PP(AutoExposureSpeedDown);
				LERP_PP(AutoExposureBias);
				LERP_PP(HistogramLogMin);
				LERP_PP(HistogramLogMax);
				LERP_PP(LensFlareIntensity);
				LERP_PP(LensFlareTint);
				LERP_PP(LensFlareBokehSize);
				LERP_PP(LensFlareThreshold);
				LERP_PP(VignetteIntensity);
				LERP_PP(GrainIntensity);
				LERP_PP(GrainJitter);
				LERP_PP(AmbientOcclusionIntensity);
				LERP_PP(AmbientOcclusionStaticFraction);
				LERP_PP(AmbientOcclusionRadius);
				LERP_PP(AmbientOcclusionFadeDistance);
				LERP_PP(AmbientOcclusionFadeRadius);
				LERP_PP(AmbientOcclusionDistance_DEPRECATED);
				LERP_PP(AmbientOcclusionPower);
				LERP_PP(AmbientOcclusionBias);
				LERP_PP(AmbientOcclusionQuality);
				LERP_PP(AmbientOcclusionMipBlend);
				LERP_PP(AmbientOcclusionMipScale);
				LERP_PP(AmbientOcclusionMipThreshold);
				LERP_PP(IndirectLightingColor);
				LERP_PP(IndirectLightingIntensity);
				LERP_PP(DepthOfFieldFocalDistance);
				LERP_PP(DepthOfFieldFstop);
				LERP_PP(DepthOfFieldMinFstop);
				LERP_PP(DepthOfFieldSensorWidth);
				LERP_PP(DepthOfFieldDepthBlurRadius);
				LERP_PP(DepthOfFieldDepthBlurAmount);
				LERP_PP(DepthOfFieldFocalRegion);
				LERP_PP(DepthOfFieldNearTransitionRegion);
				LERP_PP(DepthOfFieldFarTransitionRegion);
				LERP_PP(DepthOfFieldScale);
				LERP_PP(DepthOfFieldNearBlurSize);
				LERP_PP(DepthOfFieldFarBlurSize);
				LERP_PP(DepthOfFieldOcclusion);
				LERP_PP(DepthOfFieldSkyFocusDistance);
				LERP_PP(DepthOfFieldVignetteSize);
				LERP_PP(MotionBlurAmount);
				LERP_PP(MotionBlurMax);
				LERP_PP(MotionBlurPerObjectSize);
				LERP_PP(ScreenPercentage);
				LERP_PP(ScreenSpaceReflectionQuality);
				LERP_PP(ScreenSpaceReflectionIntensity);
				LERP_PP(ScreenSpaceReflectionMaxRoughness);

				// Ray Tracing
				if (Src.bOverride_ReflectionsType)
				{
					Dest.ReflectionsType = Src.ReflectionsType;
				}

				if (Src.bOverride_RayTracingReflectionsMaxRoughness)
				{
					Dest.RayTracingReflectionsMaxRoughness = Src.RayTracingReflectionsMaxRoughness;
				}

				if (Src.bOverride_RayTracingReflectionsMaxBounces)
				{
					Dest.RayTracingReflectionsMaxBounces = Src.RayTracingReflectionsMaxBounces;
				}

				if (Src.bOverride_RayTracingReflectionsSamplesPerPixel)
				{
					Dest.RayTracingReflectionsSamplesPerPixel = Src.RayTracingReflectionsSamplesPerPixel;
				}

				if (Src.bOverride_RayTracingReflectionsShadows)
				{
					Dest.RayTracingReflectionsShadows = Src.RayTracingReflectionsShadows;
				}

				if (Src.bOverride_RayTracingReflectionsTranslucency)
				{
					Dest.RayTracingReflectionsTranslucency = Src.RayTracingReflectionsTranslucency;
				}

				if (Src.bOverride_TranslucencyType)
				{
					Dest.TranslucencyType = Src.TranslucencyType;
				}

				if (Src.bOverride_RayTracingTranslucencyMaxRoughness)
				{
					Dest.RayTracingTranslucencyMaxRoughness = Src.RayTracingTranslucencyMaxRoughness;
				}

				if (Src.bOverride_RayTracingTranslucencyRefractionRays)
				{
					Dest.RayTracingTranslucencyRefractionRays = Src.RayTracingTranslucencyRefractionRays;
				}

				if (Src.bOverride_RayTracingTranslucencySamplesPerPixel)
				{
					Dest.RayTracingTranslucencySamplesPerPixel = Src.RayTracingTranslucencySamplesPerPixel;
				}

				if (Src.bOverride_RayTracingTranslucencyShadows)
				{
					Dest.RayTracingTranslucencyShadows = Src.RayTracingTranslucencyShadows;
				}

				if (Src.bOverride_RayTracingTranslucencyRefraction)
				{
					Dest.RayTracingTranslucencyRefraction = Src.RayTracingTranslucencyRefraction;
				}

				if (Src.bOverride_RayTracingGI)
				{
					Dest.RayTracingGIType = Src.RayTracingGIType;
				}

				if (Src.bOverride_RayTracingGIMaxBounces)
				{
					Dest.RayTracingGIMaxBounces = Src.RayTracingGIMaxBounces;
				}

				if (Src.bOverride_RayTracingGISamplesPerPixel)
				{
					Dest.RayTracingGISamplesPerPixel = Src.RayTracingGISamplesPerPixel;
				}

				if (Src.bOverride_RayTracingAO)
				{
					Dest.RayTracingAO = Src.RayTracingAO;
				}

				if (Src.bOverride_RayTracingAOSamplesPerPixel)
				{
					Dest.RayTracingAOSamplesPerPixel = Src.RayTracingAOSamplesPerPixel;
				}

				if (Src.bOverride_PathTracingMaxBounces)
				{
					Dest.PathTracingMaxBounces = Src.PathTracingMaxBounces;
				}

				if (Src.bOverride_PathTracingSamplesPerPixel)
				{
					Dest.PathTracingSamplesPerPixel = Src.PathTracingSamplesPerPixel;
				}


				if (Src.bOverride_DepthOfFieldBladeCount)
				{
					Dest.DepthOfFieldBladeCount = Src.DepthOfFieldBladeCount;
				}

				// cubemaps are getting blended Overlayly - in contrast to other properties, maybe we should make that consistent
				if (Src.AmbientCubemap && Src.bOverride_AmbientCubemapIntensity)
				{
					FFinalPostProcessSettings::FCubemapEntry Entry;

					Entry.AmbientCubemapTintMulScaleValue = FLinearColor(1, 1, 1, 1) * Src.AmbientCubemapIntensity;

					if (Src.bOverride_AmbientCubemapTint)
					{
						Entry.AmbientCubemapTintMulScaleValue *= Src.AmbientCubemapTint;
					}

					Entry.AmbientCubemap = Src.AmbientCubemap;
					Dest.UpdateEntry(Entry, Weight);
				}

				IF_PP(ColorGradingLUT)
				{
					float ColorGradingIntensity = FMath::Clamp(Src.ColorGradingIntensity, 0.0f, 1.0f);
					Dest.LerpTo(Src.ColorGradingLUT, ColorGradingIntensity * Weight);
				}

				// actual texture cannot be blended but the intensity can be blended
				IF_PP(BloomDirtMask)
				{
					Dest.BloomDirtMask = Src.BloomDirtMask;
				}

				IF_PP(BloomMethod)
				{
					Dest.BloomMethod = Src.BloomMethod;
				}

				// actual texture cannot be blended but the intensity can be blended
				IF_PP(BloomConvolutionTexture)
				{
					Dest.BloomConvolutionTexture = Src.BloomConvolutionTexture;
				}

				// A continuous blending of this value would result trashing the pre-convolved bloom kernel cache.
				IF_PP(BloomConvolutionBufferScale)
				{
					Dest.BloomConvolutionBufferScale = Src.BloomConvolutionBufferScale;
				}

				// Curve assets can not be blended.
				IF_PP(AutoExposureBiasCurve)
				{
					Dest.AutoExposureBiasCurve = Src.AutoExposureBiasCurve;
				}

				// actual texture cannot be blended but the intensity can be blended
				IF_PP(LensFlareBokehShape)
				{
					Dest.LensFlareBokehShape = Src.LensFlareBokehShape;
				}

				if (Src.bOverride_LensFlareTints)
				{
					for (uint32 i = 0; i < 8; ++i)
					{
						Dest.LensFlareTints[i] = FMath::Lerp(Dest.LensFlareTints[i], Src.LensFlareTints[i], Weight);
					}
				}

				if (Src.bOverride_MobileHQGaussian)
				{
					Dest.bMobileHQGaussian = Src.bMobileHQGaussian;
				}

				if (Src.bOverride_AutoExposureMethod)
				{
					Dest.AutoExposureMethod = Src.AutoExposureMethod;
				}

				if (Src.bOverride_AmbientOcclusionRadiusInWS)
				{
					Dest.AmbientOcclusionRadiusInWS = Src.AmbientOcclusionRadiusInWS;
				}

				if (Src.bOverride_MotionBlurTargetFPS)
				{
					Dest.MotionBlurTargetFPS = Src.MotionBlurTargetFPS;
				}
			}

			// will be deprecated soon, use the new asset LightPropagationVolumeBlendable instead
			{
				//FLightPropagationVolumeSettings& Dest = FinalPostProcessSettings.BlendableManager.GetSingleFinalData<FLightPropagationVolumeSettings>();
				FLightPropagationVolumeSettings& Dest = OutData.BlendableManager.GetSingleFinalData<FLightPropagationVolumeSettings>();

				LERP_PP(LPVIntensity);
				LERP_PP(LPVSecondaryOcclusionIntensity);
				LERP_PP(LPVSecondaryBounceIntensity);
				LERP_PP(LPVVplInjectionBias);
				LERP_PP(LPVGeometryVolumeBias);
				LERP_PP(LPVEmissiveInjectionIntensity);
				LERP_PP(LPVDirectionalOcclusionIntensity);
				LERP_PP(LPVDirectionalOcclusionRadius);
				LERP_PP(LPVDiffuseOcclusionExponent);
				LERP_PP(LPVSpecularOcclusionExponent);
				LERP_PP(LPVDiffuseOcclusionIntensity);
				LERP_PP(LPVSpecularOcclusionIntensity);
				LERP_PP(LPVFadeRange);
				LERP_PP(LPVDirectionalOcclusionFadeRange);

				if (Src.bOverride_LPVSize)
				{
					Dest.LPVSize = Src.LPVSize;
				}
			}

#if 0 // TODO : 현재 WeightedBlendables (PP용 Material) 은 Global(Final) 만 지원 ( WorldEnvinronmentControl 참고)
			// #115
			// Blendable objects
			{
				uint32 Count = Src.WeightedBlendables.Array.Num();

				for(uint32 i = 0; i < Count; ++i)
				{
					UObject* Object = Src.WeightedBlendables.Array[i].Object;

					if(!Object || !Object->IsValidLowLevel())
					{
						continue;
					}

					IBlendableInterface* BlendableInterface = Cast<IBlendableInterface>(Object);

					if(!BlendableInterface)
					{
						continue;
					}

					float LocalWeight = FMath::Min(1.0f, Src.WeightedBlendables.Array[i].Weight) * Weight;
					if(LocalWeight > 0.0f)
					{
						BlendableInterface->OverrideBlendableSettings(*this, LocalWeight);
					}
				}
			}
#endif
		}
	}


	// #97
	ADirectionalLight* SpawnDirectionalLightActor(
		UWorld* InWorld,
		EObjectFlags InObjectFlags,
		const FT4EnvDirectionalLightData* InData
	)
	{
		check(nullptr != InWorld);
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.ObjectFlags |= InObjectFlags;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnInfo.bDeferConstruction = false;
		ADirectionalLight* SpawnedActor = InWorld->SpawnActor<ADirectionalLight>(
			ADirectionalLight::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnInfo
		);
		if (nullptr == SpawnedActor)
		{
			return nullptr;
		}
		SpawnedActor->SetMobility(EComponentMobility::Movable);
		if (nullptr != InData)
		{
			ApplyDirectionalLight(InWorld, InData, SpawnedActor);
		}
		return SpawnedActor;
	}
	
	AActor* SpawnBPSkySphereActor(
		UWorld* InWorld,
		EObjectFlags InObjectFlags,
		const FT4EnvBPSkySphereData* InData
	)
	{
		check(nullptr != InWorld);
		UClass* BPSkySphereClass = GetBPSkySphereClass();
		if (nullptr == BPSkySphereClass)
		{
			return nullptr;
		}
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.ObjectFlags |= InObjectFlags;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnInfo.bDeferConstruction = false;
		AActor* SpawnedActor = InWorld->SpawnActor<AActor>(
			BPSkySphereClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnInfo
		);
		if (nullptr == SpawnedActor)
		{
			return nullptr;
		}
		{
			SpawnedActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
		}
		if (nullptr != InData)
		{
			ApplyBPSkySphere(InWorld, InData, nullptr, nullptr);
		}
		return SpawnedActor;
	}

	ASkyLight* SpawnSkyLightActor(
		UWorld* InWorld,
		EObjectFlags InObjectFlags,
		const FT4EnvSkyLightData* InData
	)
	{
		check(nullptr != InWorld);
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.ObjectFlags |= InObjectFlags;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnInfo.bDeferConstruction = false;
		ASkyLight* SpawnedActor = InWorld->SpawnActor<ASkyLight>(
			ASkyLight::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnInfo
		);
		if (nullptr == SpawnedActor)
		{
			return nullptr;
		}
		if (nullptr != InData)
		{
			ApplySkyLight(InWorld, InData, SpawnedActor);
		}
		return SpawnedActor;
	}
	
	AAtmosphericFog* SpawnAtmosphericFogActor(
		UWorld* InWorld,
		EObjectFlags InObjectFlags,
		const FT4EnvAtmosphericFogData* InData
	)
	{
		check(nullptr != InWorld);
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.ObjectFlags |= InObjectFlags;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnInfo.bDeferConstruction = false;
		AAtmosphericFog* SpawnedActor = InWorld->SpawnActor<AAtmosphericFog>(
			AAtmosphericFog::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnInfo
		);
		if (nullptr == SpawnedActor)
		{
			return nullptr;
		}
		if (nullptr != InData)
		{
			ApplyAtmosphericFog(InWorld, InData, SpawnedActor);
		}
		return SpawnedActor;
	}
	
	AExponentialHeightFog* SpawnExponentialHeightFogActor(
		UWorld* InWorld,
		EObjectFlags InObjectFlags,
		const FT4EnvExponentialHeightFogData* InData
	)
	{
		check(nullptr != InWorld);
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.ObjectFlags |= InObjectFlags;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnInfo.bDeferConstruction = false;
		AExponentialHeightFog* SpawnedActor = InWorld->SpawnActor<AExponentialHeightFog>(
			AExponentialHeightFog::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnInfo
		);
		if (nullptr == SpawnedActor)
		{
			return nullptr;
		}
		if (nullptr != InData)
		{
			ApplyExponentialHeightFog(InWorld, InData, SpawnedActor);
		}
		return SpawnedActor;
	}
	// ~#97

	// #97
	ADirectionalLight* FindDirectionalLightActor(UWorld* InWorld)
	{
		for (TActorIterator<ADirectionalLight> It(InWorld); It; ++It)
		{
			return *It;
		}
		return nullptr;
	}

	AActor* FindBPSkySphereActor(UWorld* InWorld)
	{
		UClass* BPSkySphereClass = GetBPSkySphereClass();
		if (nullptr == BPSkySphereClass)
		{
			return nullptr;
		}
		for (TActorIterator<AActor> It(InWorld, BPSkySphereClass); It; ++It)
		{
			return *It;
		}
		return nullptr;
	}

	ASkyLight* FindSkyLightActor(UWorld* InWorld)
	{
		for (TActorIterator<ASkyLight> It(InWorld); It; ++It)
		{
			return *It;
		}
		return nullptr;
	}

	AAtmosphericFog* FindAtmosphericFogActor(UWorld* InWorld)
	{
		for (TActorIterator<AAtmosphericFog> It(InWorld); It; ++It)
		{
			return *It;
		}
		return nullptr;
	}

	AExponentialHeightFog* FindExponentialHeightFogActor(UWorld* InWorld)
	{
		for (TActorIterator<AExponentialHeightFog> It(InWorld); It; ++It)
		{
			return *It;
		}
		return nullptr;
	}

	AT4WorldZoneVolume* FindGlobalWorldZoneVolume(UWorld* InWorld)
	{
		return FindWorldZoneVolumeOnWorld(InWorld, T4Const_GlobalWorldZoneName);
	}
	// #97

#if WITH_EDITOR
	// #T4_ADD_TOD_TAG

	bool GetDirectionalData(
		UWorld* InWorld,
		FT4EnvDirectionalData* OutData
	) // #93
	{
		bool bHasData = false;
		for (TActorIterator<ADirectionalLight> It(InWorld); It; ++It)
		{
			ADirectionalLight* Actor = *It;
			check(nullptr != Actor);
			if (bHasData)
			{
				T4_LOG(
					Warning,
					TEXT("Already exists. skipped")
				);
				break;
			}
			UDirectionalLightComponent* Component = Cast<UDirectionalLightComponent>(Actor->GetLightComponent());
			if (nullptr != Component)
			{
				OutData->Rotation = Actor->GetActorRotation();
			}
			bHasData = true;
		}
		return bHasData;
	}

	bool GetDirectionalLightData(
		UWorld* InWorld,
		FT4EnvDirectionalLightData* OutData
	)
	{
		bool bHasData = false;
		for (TActorIterator<ADirectionalLight> It(InWorld); It; ++It)
		{	
			ADirectionalLight* Actor = *It;
			check(nullptr != Actor);
			if (bHasData)
			{
				T4_LOG(
					Warning,
					TEXT("Already exists. skipped")
				);
				break;
			}
			UDirectionalLightComponent* Component = Cast<UDirectionalLightComponent>(Actor->GetLightComponent());
			if (nullptr != Component)
			{
				OutData->Intensity = Component->Intensity;
				OutData->LightColor = Component->LightColor;
			}
			bHasData = true;
		}
		return bHasData;
	}

	bool GetBPSkySphereData(
		UWorld* InWorld,
		FT4EnvBPSkySphereData* OutData
	)
	{
		bool bHasData = false;
		UClass* BPSkySphereClass = GetBPSkySphereClass();
		if (nullptr == BPSkySphereClass)
		{
			return false;
		}
		for (TActorIterator<AActor> It(InWorld, BPSkySphereClass); It; ++It)
		{
			AActor* Actor = *It;
			check(nullptr != Actor);
			if (bHasData)
			{
				T4_LOG(
					Warning,
					TEXT("Already exists. skipped")
				);
				break;
			}
			{
				float CurrentValue = 0.0f; // #98
				UFloatProperty* FoundProperty = FindBlueprintFloatProperty(Actor, BPSkySpherePropertySunBrightnessName, &CurrentValue);
				if (nullptr != FoundProperty)
				{
					OutData->SunBrightness = CurrentValue;
				}
			}
			{
				float CurrentValue = 0.0f; // #98
				UFloatProperty* FoundProperty = FindBlueprintFloatProperty(Actor, BPSkySpherePropertyCloudSpeedName, &CurrentValue);
				if (nullptr != FoundProperty)
				{
					OutData->CloudSpeed = CurrentValue;
				}
			}
			{
				float CurrentValue = 0.0f; // #98
				UFloatProperty* FoundProperty = FindBlueprintFloatProperty(Actor, BPSkySpherePropertyCloudOpacityName, &CurrentValue);
				if (nullptr != FoundProperty)
				{
					OutData->CloudOpacity = CurrentValue;
				}
			}
			{
				float CurrentValue = 0.0f; // #98
				UFloatProperty* FoundProperty = FindBlueprintFloatProperty(Actor, BPSkySpherePropertyStarsBrightnessName, &CurrentValue);
				if (nullptr != FoundProperty)
				{
					OutData->StarsBrightness = CurrentValue;
				}
			}
			{
				bool CurrentValue = false;
				UBoolProperty* FoundProperty = FindBlueprintBoolProperty(Actor, BPSkySpherePropertyColorsDeterminedBySunPositionName, &CurrentValue);
				if (nullptr != FoundProperty)
				{
					OutData->bColorsDeterminedBySunPosition = CurrentValue;
				}
			}
			{
				FColor CurrentValue;  // #98 
				UStructProperty* FoundProperty = FindBlueprintColorProperty(Actor, BPSkySpherePropertyZenithColorName, &CurrentValue);
				if (nullptr != FoundProperty)
				{
					OutData->ZenithColor = CurrentValue;
				}
			}
			{
				FColor CurrentValue;  // #98
				UStructProperty* FoundProperty = FindBlueprintColorProperty(Actor, BPSkySpherePropertyHorizonColorName, &CurrentValue);
				if (nullptr != FoundProperty)
				{
					OutData->HorizonColor = CurrentValue;
				}
			}
			{
				FColor CurrentValue;  // #98
				UStructProperty* FoundProperty = FindBlueprintColorProperty(Actor, BPSkySpherePropertyCloudColorName, &CurrentValue);
				if (nullptr != FoundProperty)
				{
					OutData->CloudColor = CurrentValue;
				}
			}
			{
				FColor CurrentValue;  // #98
				UStructProperty* FoundProperty = FindBlueprintColorProperty(Actor, BPSkySpherePropertyOverallColorName, &CurrentValue);
				if (nullptr != FoundProperty)
				{
					OutData->OverallColor = CurrentValue;
				}
			}
			bHasData = true;
		}
		return bHasData;
	}

	bool GetSkyLightData(
		UWorld* InWorld,
		FT4EnvSkyLightData* OutData
	)
	{
		bool bHasData = false;
		for (TActorIterator<ASkyLight> It(InWorld); It; ++It)
		{
			ASkyLight* Actor = *It;
			check(nullptr != Actor);
			if (bHasData)
			{
				T4_LOG(
					Warning,
					TEXT("Already exists. skipped")
				);
				break;
			}
			USkyLightComponent* Component = Cast<USkyLightComponent>(Actor->GetLightComponent());
			if (nullptr != Component)
			{
				OutData->SourceType = Component->SourceType; // #97
				OutData->CubemapPtr = Component->Cubemap;
				OutData->CubemapResolution = Component->CubemapResolution;
				OutData->Intensity = Component->Intensity;
				OutData->LightColor = Component->LightColor;
			}
			bHasData = true;
		}
		return bHasData;
	}

	bool GetAtmosphericFogData(
		UWorld* InWorld,
		FT4EnvAtmosphericFogData* OutData
	)
	{
		bool bHasData = false;
		for (TActorIterator<AAtmosphericFog> It(InWorld); It; ++It)
		{
			AAtmosphericFog* Actor = *It;
			check(nullptr != Actor);
			if (bHasData)
			{
				T4_LOG(
					Warning,
					TEXT("Already exists. skipped")
				);
				break;
			}
			UAtmosphericFogComponent* Component = Cast<UAtmosphericFogComponent>(Actor->GetAtmosphericFogComponent());
			if (nullptr != Component)
			{
				OutData->SunMultiplier = Component->SunMultiplier;
				OutData->FogMultiplier = Component->FogMultiplier;
				OutData->DensityMultiplier = Component->DensityMultiplier;
				OutData->DensityOffset = Component->DensityOffset;
			}
			bHasData = true;
		}
		return bHasData;
	}

	bool GetExponentialHeightFogData(
		UWorld* InWorld,
		FT4EnvExponentialHeightFogData* OutData
	)
	{
		bool bHasData = false;
		for (TActorIterator<AExponentialHeightFog> It(InWorld); It; ++It)
		{
			AExponentialHeightFog* Actor = *It;
			check(nullptr != Actor);
			if (bHasData)
			{
				T4_LOG(
					Warning,
					TEXT("Already exists. skipped")
				);
				break;
			}
			UExponentialHeightFogComponent* Component = Cast<UExponentialHeightFogComponent>(Actor->GetComponent());
			if (nullptr != Component)
			{
				OutData->FogDensity = Component->FogDensity;
				OutData->FogHeightFalloff = Component->FogHeightFalloff;
				OutData->SecondFogData = Component->SecondFogData;
				OutData->FogInscatteringColor = Component->FogInscatteringColor;
			}
			bHasData = true;
		}
		return bHasData;
	}

	bool GetGlobalPostProcessData(
		UWorld* InWorld,
		FT4EnvPostProcessData* OutData
	) // #98
	{
		bool bHasData = false;
		for (TActorIterator<APostProcessVolume> It(InWorld); It; ++It)
		{
			APostProcessVolume* Actor = *It;
			check(nullptr != Actor);
			if (!Actor->bEnabled || !Actor->bUnbound)
			{
				continue;
			}
			OutData->Settings = Actor->Settings;
			bHasData = true;
		}
		return bHasData;
	}

#endif

}