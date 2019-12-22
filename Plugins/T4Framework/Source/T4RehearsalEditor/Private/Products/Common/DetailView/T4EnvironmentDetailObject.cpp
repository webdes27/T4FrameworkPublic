// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4EnvironmentDetailObject.h"

#include "T4Asset/Classes/World/T4EnvironmentAsset.h" // #93
#include "T4Engine/Classes/World/T4MapZoneVolume.h" // #93
#include "T4Engine/Public/T4EngineDefinitions.h" // #93
#include "T4Engine/Public/T4EngineUtility.h" // #92

#include "T4RehearsalEditorInternal.h"

/**
  * #90
 */
UT4EnvironmentDetailObject::UT4EnvironmentDetailObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4EnvironmentDetailObject::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (nullptr == PropertyChangedEvent.Property) // #77
	{
		return;
	}
	if (PropertyChangedEvent.Property->HasAnyPropertyFlags(CPF_Transient))
	{
		return; // #71 : Transient Property 는 Changed 이벤트를 보내지 않도록 조치
	}
	OnPropertiesChanged().Broadcast();
}

bool UT4EnvironmentDetailObject::CopyTo(FT4EnvTimeTagData& OutData)
{
	// #T4_ADD_TOD_TAG
	OutData.DirectionalData = DirectionalData; // #93
	OutData.DirectionalLightData = DirectionalLightData;
	OutData.BPSkySphereData = BPSkySphereData; // #97
	OutData.SkyLightData = SkyLightData;
	OutData.AtmosphericFogData = AtmosphericFogData;
	OutData.ExponentialHeightFogData = ExponentialHeightFogData;
	OutData.PostProcessData = PostProcessData; // #98
	return true;
}

bool UT4EnvironmentDetailObject::CopyFrom(const FT4EnvTimeTagData& InData)
{
	// #T4_ADD_TOD_TAG
	DirectionalData = InData.DirectionalData; // #93
	DirectionalLightData = InData.DirectionalLightData;
	BPSkySphereData = InData.BPSkySphereData; // #97
	SkyLightData = InData.SkyLightData;
	AtmosphericFogData = InData.AtmosphericFogData;
	ExponentialHeightFogData = InData.ExponentialHeightFogData;
	PostProcessData = InData.PostProcessData; // #98
	return true;
}

void UT4EnvironmentDetailObject::SyncFrom(UWorld* InWorld)
{
	// #T4_ADD_TOD_TAG
	T4EngineUtility::GetDirectionalData(InWorld, &DirectionalData); // #93
	T4EngineUtility::GetDirectionalLightData(InWorld, &DirectionalLightData);
	T4EngineUtility::GetBPSkySphereData(InWorld, &BPSkySphereData);  // #97
	T4EngineUtility::GetSkyLightData(InWorld, &SkyLightData);
	T4EngineUtility::GetAtmosphericFogData(InWorld, &AtmosphericFogData);
	T4EngineUtility::GetExponentialHeightFogData(InWorld, &ExponentialHeightFogData);
	T4EngineUtility::GetGlobalPostProcessData(InWorld, &PostProcessData); // #98
}

bool UT4EnvironmentDetailObject::ApplyTo(
	UWorld* InWorld,
	FName InTimeTagName,
	FString& OutErrorMessage // #93
)
{
	// #93 : Enable 옵션이 꺼져있으면 GlobalZone 의 설정을 사용하기 위한 처리이다. 툴에서만 사용한다!
	AT4MapZoneVolume* GlobalMapZone = T4EngineUtility::FindMapZomeVolumeOnWorld(
		InWorld,
		T4WorldGlobalMapZoneName
	);
	if (nullptr == GlobalMapZone)
	{
		OutErrorMessage = TEXT("Global Zone is Not found");
		return false;
	}
	if (GlobalMapZone->EnvironmentAsset.IsNull())
	{
		OutErrorMessage = TEXT("EnvironmentAsset is NULL in Global Zone");
		return false;
	}
	UT4EnvironmentAsset* EnvironmentAsset = GlobalMapZone->EnvironmentAsset.LoadSynchronous();
	check(nullptr != EnvironmentAsset);
	const FT4EnvTimeTagSetData& TimeTagSetData = EnvironmentAsset->TimeTagSetData;
	if (0 >= TimeTagSetData.TimeTagMap.Num())
	{
		OutErrorMessage = TEXT("TimeTagData is empty in EnvironmentAsset");
		return false;
	}
	FName TimeTagSelected = InTimeTagName;
	if (!TimeTagSetData.TimeTagMap.Contains(TimeTagSelected))
	{
		if (!TimeTagSetData.TimeTagMap.Contains(T4WorldEivronmentTimeTagNameOfFallback))
		{
			TimeTagSelected = TimeTagSetData.TimeTagMap.CreateConstIterator().Key();
		}
		else
		{
			TimeTagSelected = T4WorldEivronmentTimeTagNameOfFallback;
		}
	}

	check(TimeTagSelected != NAME_None);
	const FT4EnvTimeTagData& DefaultTimeTagData = TimeTagSetData.TimeTagMap[TimeTagSelected];

	// #T4_ADD_TOD_TAG
	if (!DirectionalData.bEnabled) // #93
	{
		T4EngineUtility::ApplyDirectional(InWorld, &DefaultTimeTagData.DirectionalData);
	}
	else
	{
		T4EngineUtility::ApplyDirectional(InWorld, &DirectionalData);
	}
	if (!DirectionalLightData.bEnabled)
	{
		T4EngineUtility::ApplyDirectionalLight(InWorld, &DefaultTimeTagData.DirectionalLightData);
	}
	else
	{
		T4EngineUtility::ApplyDirectionalLight(InWorld, &DirectionalLightData);
	}
	if (!BPSkySphereData.bEnabled) // #97
	{
		T4EngineUtility::ApplyBPSkySphere(InWorld, &DefaultTimeTagData.BPSkySphereData);
	}
	else
	{
		T4EngineUtility::ApplyBPSkySphere(InWorld, &BPSkySphereData);
	}
	if (!SkyLightData.bEnabled)
	{
		T4EngineUtility::ApplySkyLight(InWorld, &DefaultTimeTagData.SkyLightData);
	}
	else
	{
		T4EngineUtility::ApplySkyLight(InWorld, &SkyLightData);
	}
	if (!AtmosphericFogData.bEnabled)
	{
		T4EngineUtility::ApplyAtmosphericFog(InWorld, &DefaultTimeTagData.AtmosphericFogData);
	}
	else
	{
		T4EngineUtility::ApplyAtmosphericFog(InWorld, &AtmosphericFogData);
	}
	if (!ExponentialHeightFogData.bEnabled)
	{
		T4EngineUtility::ApplyExponentialHeightFog(InWorld, &DefaultTimeTagData.ExponentialHeightFogData);
	}
	else
	{
		T4EngineUtility::ApplyExponentialHeightFog(InWorld, &ExponentialHeightFogData);
	}
	if (!PostProcessData.bEnabled) // #98
	{
		T4EngineUtility::ApplyPostProcess(InWorld, &DefaultTimeTagData.PostProcessData.Settings);
	}
	else
	{
		T4EngineUtility::ApplyPostProcess(InWorld, &PostProcessData.Settings);
	}
	return true;
}

void UT4EnvironmentDetailObject::ApplyTo(
	UWorld* InSourceWorld,
	UWorld* InTargetWorld
) // #93
{
	// Preview 월드의 설정을 Editor 월드로 세팅해준다.
	// 현재는 미니맵을 찍기 위한 용도인데, 다른 용도로 얼마든지 활용할 수 있을 듯함

	FT4EnvTimeTagData TempTimeTagData;

	// #T4_ADD_TOD_TAG
	if (T4EngineUtility::GetDirectionalData(InSourceWorld, &TempTimeTagData.DirectionalData))
	{
		T4EngineUtility::ApplyDirectional(InTargetWorld, &TempTimeTagData.DirectionalData); // #93
	}
	if (T4EngineUtility::GetDirectionalLightData(InSourceWorld, &TempTimeTagData.DirectionalLightData))
	{
		T4EngineUtility::ApplyDirectionalLight(InTargetWorld, &TempTimeTagData.DirectionalLightData);
	}
	if (T4EngineUtility::GetBPSkySphereData(InSourceWorld, &TempTimeTagData.BPSkySphereData)) // #97
	{
		T4EngineUtility::ApplyBPSkySphere(InTargetWorld, &TempTimeTagData.BPSkySphereData); // #97
	}
	if (T4EngineUtility::GetSkyLightData(InSourceWorld, &TempTimeTagData.SkyLightData))
	{
		T4EngineUtility::ApplySkyLight(InTargetWorld, &TempTimeTagData.SkyLightData);
	}
	if (T4EngineUtility::GetAtmosphericFogData(InSourceWorld, &TempTimeTagData.AtmosphericFogData))
	{
		T4EngineUtility::ApplyAtmosphericFog(InTargetWorld, &TempTimeTagData.AtmosphericFogData);
	}
	if (T4EngineUtility::GetExponentialHeightFogData(InSourceWorld, &TempTimeTagData.ExponentialHeightFogData))
	{
		T4EngineUtility::ApplyExponentialHeightFog(InTargetWorld, &TempTimeTagData.ExponentialHeightFogData);
	}
	if (T4EngineUtility::GetGlobalPostProcessData(InSourceWorld, &TempTimeTagData.PostProcessData)) // #98
	{
		T4EngineUtility::ApplyPostProcess(InTargetWorld, &TempTimeTagData.PostProcessData.Settings); // #98
	}
}