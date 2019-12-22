// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WorldEnvironmentControl.h"

#include "T4WorldController.h" // #93
#include "World/Container/T4WorldContainer.h" // #94
#include "World/T4GameWorld.h" // #94

#include "Object/Component/T4EnvironmentZoneComponent.h" // #99

#include "Classes/World/T4MapZoneVolume.h" // #92

#include "Public/T4EngineUtility.h" // #92
#include "Public/T4EngineDefinitions.h" // #93

#include "Engine/Scene.h" // #98
#include "Engine/ExponentialHeightFog.h" // #90
#include "Atmosphere/AtmosphericFog.h" // #90
#include "Engine/SkyLight.h"
#include "Engine/DirectionalLight.h" // #90

#include "T4EngineInternal.h"

/**
  * #92
 */
FT4WorldEnvironmentControl::FT4WorldEnvironmentControl(FT4WorldController* InWorldController)
	: WorldControllerRef(InWorldController)
	, bPaused(false) // #92 : Map Environemnt Update 제어 옵션 처리
{
}

FT4WorldEnvironmentControl::~FT4WorldEnvironmentControl()
{
}

void FT4WorldEnvironmentControl::Reset()
{
	DirectionalLightPtr.Reset(); // #92
	BPSkySpherePtr.Reset();
	SkyLightPtr.Reset();
	AtmosphericFogPtr.Reset();
	ExponentialHeightFogPtr.Reset();
	GlobalMapZoneVolumePtr.Reset();
}

void FT4WorldEnvironmentControl::Process(float InDeltaTime)
{
	if (bPaused) 
	{
		return; // #92 : Map Environemnt Update 제어 옵션 처리
	}

	check(nullptr != WorldControllerRef);
	FT4GameWorld* GameWorld = WorldControllerRef->GetGameWorldImpl();
	check(nullptr != GameWorld);

	FVector UpdateLocation = FVector::ZeroVector;
	if (GameWorld->HasPlayerObject())
	{
		// Player 가 있다면 플레이어 위치 기준으로
		UpdateLocation = GameWorld->GetPlayerObject()->GetRootLocation();
	}
	else
	{
		UpdateLocation = GameWorld->GetCameraLocation();
	}

	FT4WorldContainer* WorldContainer = GameWorld->GetContainerImpl();
	check(nullptr != WorldContainer);

	TArray<FT4EnvironmentZoneInfo> ActiveEnvironmentZoneInfos;
	
	TArray<AT4MapZoneVolume*> StaticMapZoneVolumes;
	bool bValidStaticZone = WorldContainer->GetStaticMapZoneVolumes(StaticMapZoneVolumes);
	for (AT4MapZoneVolume* MapZoneVolume : StaticMapZoneVolumes)
	{
		check(nullptr != MapZoneVolume);
		if (MapZoneVolume->EnvironmentAsset.IsNull())
		{
			continue;
		}

		float DistanceToPoint = 0.0f;

		MapZoneVolume->Update(InDeltaTime);
		MapZoneVolume->EncompassesPoint(UpdateLocation, 0.0f, &DistanceToPoint);

		if (MapZoneVolume->IsEntered())
		{
			if (0.0f < DistanceToPoint)
			{
				MapZoneVolume->Leave();
			}
		}
		else
		{
			if (0.0f >= DistanceToPoint)
			{
				MapZoneVolume->Enter();
			}
		}

		if (0.0f < MapZoneVolume->GetBlendWeight())
		{
			FT4EnvironmentZoneInfo& NewZoneInfo = ActiveEnvironmentZoneInfos.AddDefaulted_GetRef();
			NewZoneInfo.ZoneName = MapZoneVolume->ZoneName;
			NewZoneInfo.ZoneType = MapZoneVolume->ZoneType;
			NewZoneInfo.BlendPriority = MapZoneVolume->GetBlendPriority();
			NewZoneInfo.LayerBlendWeight = MapZoneVolume->GetBlendWeight();
			NewZoneInfo.EnvironmentAsset = MapZoneVolume->EnvironmentAsset.LoadSynchronous();
		}
	}

	TArray<UT4EnvironmentZoneComponent*> EnvironmentZoneComponents; // #99
	WorldContainer->GetEnvironmentZoneComponentsInSpawnObject(EnvironmentZoneComponents); // #99
	for (UT4EnvironmentZoneComponent* EnvironmentZoneComponent : EnvironmentZoneComponents) // #94
	{
		check(nullptr != EnvironmentZoneComponent);
		if (EnvironmentZoneComponent->TestEncompassesPoint(UpdateLocation))
		{
			FT4EnvironmentZoneInfo& NewZoneInfo = ActiveEnvironmentZoneInfos.AddDefaulted_GetRef();
			EnvironmentZoneComponent->GetEnvironmentZoneInfo(NewZoneInfo);
		}
	}

	if (0 >= ActiveEnvironmentZoneInfos.Num())
	{
		return;
	}

	{
		// #93 : 활성화 된 Zone 을 BlendPriority 순으로 소팅한다. 낮을 수록 우선순위가 높다. (Global = -1)
		ActiveEnvironmentZoneInfos.Sort([](const FT4EnvironmentZoneInfo& A, const FT4EnvironmentZoneInfo& B)
		{
			return A.BlendPriority < B.BlendPriority;
		});
	}

	FT4WorldTimeControl* WorldTimeControl = WorldControllerRef->GetWorldTimeControl();
	check(nullptr != WorldTimeControl);

	const FName SourceTimeTagName = WorldTimeControl->GetTimeTagName(); // #93
	const FName TargetTimeTagName = WorldTimeControl->GetNextTimeTagName(); // #93
	const float LocalBlendWeight = WorldTimeControl->GetRatioForNameRange(); // #93 : 같은 Zone 내 BlendRatio

	bool bSetGlobal = false; // #94 : Blend Priority 로 소팅했기 때문에 첫번째가 무조건 Global 로 가정한다.
	for (const FT4EnvironmentZoneInfo& ZoneInfo : ActiveEnvironmentZoneInfos)
	{
		if (nullptr == ZoneInfo.EnvironmentAsset)
		{
			continue; // 이후 Async Loading 처리가 될 수 있음으로 없다면 pass 해준다.
		}
		if (!bSetGlobal)
		{
			bool bResult = SetGlobal(
				ZoneInfo.EnvironmentAsset,
				SourceTimeTagName, 
				TargetTimeTagName, 
				LocalBlendWeight
			);
			if (!bResult)
			{
				UE_LOG(
					LogT4Engine,
					Warning,
					TEXT("ProcessEnvironment : SetGlobalZone failed.")
				);
				return;
			}
			bSetGlobal = true;
		}
		else
		{
			check(bSetGlobal);
			bool bResult = BlendLocalZone(
				ZoneInfo.EnvironmentAsset,
				ZoneInfo.LayerBlendWeight,
				SourceTimeTagName, 
				TargetTimeTagName, 
				LocalBlendWeight
			);
			if (!bResult)
			{
				UE_LOG(
					LogT4Engine,
					Warning,
					TEXT("ProcessEnvironment : BlendLocalZone failed.")
				);
			}
		}
	}

	UWorld* UnrealWorld = WorldControllerRef->GetWorld();
	check(nullptr != UnrealWorld);

	Apply(UnrealWorld);
}

bool FT4WorldEnvironmentControl::SetGlobal(
	const UT4EnvironmentAsset* InEnvironmentAsset,
	const FName InSourceTimeName, // #93
	const FName InTargetTimeName, // #93
	const float InLocalBlendWeight // #93
)
{
	check(nullptr != InEnvironmentAsset);
	FinalEnvrionmentData.PostProcessSettings.SetBaseValues();
	bool bResult = BlendEnvironmentDataData(
		InEnvironmentAsset,
		InSourceTimeName,
		InTargetTimeName,
		InLocalBlendWeight,
		FinalEnvrionmentData
	);
	return bResult;
}

bool FT4WorldEnvironmentControl::BlendLocalZone(
	const UT4EnvironmentAsset* InEnvironmentAsset,
	const float InLayerBlendWeight, // #94
	const FName InSourceTimeName, // #93
	const FName InTargetTimeName, // #93
	const float InLocalBlendWeight // #93
)
{
	check(nullptr != InEnvironmentAsset);
	FFinalEnvironmentData LocalEnvironmentData; // #93 : Local 용 TOD data 에 저장하고, 다시 최종 TOD Data 와 Blending 한다.
	LocalEnvironmentData.PostProcessSettings.SetBaseValues();
	bool bResult = BlendEnvironmentDataData(
		InEnvironmentAsset,
		InSourceTimeName,
		InTargetTimeName,
		InLocalBlendWeight,
		LocalEnvironmentData
	);
	if (!bResult)
	{
		return false;
	}
	T4EngineUtility::BlendTimeTagData(
		LocalEnvironmentData.TimeTagData, 
		InLayerBlendWeight, 
		FinalEnvrionmentData.TimeTagData
	); // write to FinalTimeTagData

	if (LocalEnvironmentData.TimeTagData.PostProcessData.bEnabled)
	{
		T4EngineUtility::BlendPostProcess(
			static_cast<const FPostProcessSettings*>(&LocalEnvironmentData.PostProcessSettings),
			InLayerBlendWeight,
			FinalEnvrionmentData.PostProcessSettings
		); // #98 : PP 는 덮어씌우면 안된다. 무조건 블랜딩 로직을 타도록...
	}
	return true;
}

bool FT4WorldEnvironmentControl::BlendEnvironmentDataData(
	const UT4EnvironmentAsset* InEnvironmentAsset,
	const FName InSourceTimeName,
	const FName InTargetTimeName,
	const float InLocalBlendWeight, 
	FFinalEnvironmentData& OutEnvironmentData
) // #93
{
	check(nullptr != InEnvironmentAsset);
	if (0 >= InEnvironmentAsset->TimeTagSetData.TimeTagMap.Num())
	{
		return false;
	}
	if (T4WorldEivronmentTimeTagNameOfFallback == InSourceTimeName)
	{
		const FT4EnvTimeTagData* MapTimeTagData = GetTimeTagData(InEnvironmentAsset, InSourceTimeName);
		if (nullptr == MapTimeTagData)
		{
			// #93 :: fallback 이 없으면 등록된 것중 첫번째 것으로...(자동 fallback?)
			MapTimeTagData = &InEnvironmentAsset->TimeTagSetData.TimeTagMap.CreateConstIterator().Value();
			if (nullptr == MapTimeTagData)
			{
				return false;
			}
		}
		OutEnvironmentData.TimeTagData = *MapTimeTagData;
		if (MapTimeTagData->PostProcessData.bEnabled)
		{
			T4EngineUtility::BlendPostProcess(
				&MapTimeTagData->PostProcessData.Settings,
				1.0f,
				OutEnvironmentData.PostProcessSettings
			); // #98 : PP 는 덮어씌우면 안된다. 무조건 블랜딩 로직을 타도록...
		}
	}
	else
	{
		const FT4EnvTimeTagData* SourceMapTimeTagData = GetTimeTagData(InEnvironmentAsset, InSourceTimeName);
		if (nullptr == SourceMapTimeTagData)
		{
			// #97 : 현재 시간이 없다면 NextTime 으로 시도!
			SourceMapTimeTagData = GetTimeTagData(InEnvironmentAsset, T4EngineUtility::GetNextTimeTagName(InSourceTimeName));
			if (nullptr == SourceMapTimeTagData)
			{
				// #93 : 현재 시간이 없다면 Fallback 으로 시도!
				SourceMapTimeTagData = GetTimeTagData(InEnvironmentAsset, T4WorldEivronmentTimeTagNameOfFallback);
				if (nullptr == SourceMapTimeTagData)
				{
					// #94 : Fallback 이 없다면!! 첫번째 것으로 
					SourceMapTimeTagData = &InEnvironmentAsset->TimeTagSetData.TimeTagMap.CreateConstIterator().Value();
				}
			}
		}
		OutEnvironmentData.TimeTagData = *SourceMapTimeTagData; // copy Source
		if (SourceMapTimeTagData->PostProcessData.bEnabled)
		{
			T4EngineUtility::BlendPostProcess(
				&SourceMapTimeTagData->PostProcessData.Settings,
				1.0f,
				OutEnvironmentData.PostProcessSettings
			); // #98 : PP 는 덮어씌우면 안된다. 무조건 블랜딩 로직을 타도록...
		}
		if (InTargetTimeName == NAME_None)
		{
			return true; // Source 만 취하고 리턴
		}
		const FT4EnvTimeTagData* TargetMapTimeTagData = GetTimeTagData(InEnvironmentAsset, InTargetTimeName);
		if (nullptr == TargetMapTimeTagData)
		{
			return true; // Source 만 취하고 리턴
		}
		if (0.0f < InLocalBlendWeight)
		{
			const float BlendWeight = InLocalBlendWeight;
			T4EngineUtility::BlendTimeTagData(*TargetMapTimeTagData, BlendWeight, OutEnvironmentData.TimeTagData); // Target
			if (TargetMapTimeTagData->PostProcessData.bEnabled)
			{
				T4EngineUtility::BlendPostProcess(
					&TargetMapTimeTagData->PostProcessData.Settings,
					BlendWeight,
					OutEnvironmentData.PostProcessSettings
				); // #98 : PP 는 덮어씌우면 안된다. 무조건 블랜딩 로직을 타도록...
			}
		}
	}
	return true;
}

const FT4EnvTimeTagData* FT4WorldEnvironmentControl::GetTimeTagData(
	const UT4EnvironmentAsset* InEnvironmentAsset,
	const FName InTimeName
)
{
	check(nullptr != InEnvironmentAsset);
	const FT4EnvTimeTagSetData& TimeTagSetData = InEnvironmentAsset->TimeTagSetData;
	if (!TimeTagSetData.TimeTagMap.Contains(InTimeName))
	{
		return nullptr;
	}
	const FT4EnvTimeTagData& MapTimeTagData = TimeTagSetData.TimeTagMap[InTimeName];
	return &MapTimeTagData;
}

void FT4WorldEnvironmentControl::Apply(UWorld* InWorld)
{
	// #T4_ADD_TOD_TAG
	{
		if (!DirectionalLightPtr.IsValid())
		{
			DirectionalLightPtr = T4EngineUtility::FindDirectionalLightActor(InWorld);
		}
		T4EngineUtility::ApplyDirectional(
			InWorld,
			&FinalEnvrionmentData.TimeTagData.DirectionalData,
			DirectionalLightPtr.Get()
		); // #93
	}
	{
		if (!DirectionalLightPtr.IsValid())
		{
			DirectionalLightPtr = T4EngineUtility::FindDirectionalLightActor(InWorld);
		}
		T4EngineUtility::ApplyDirectionalLight(
			InWorld,
			&FinalEnvrionmentData.TimeTagData.DirectionalLightData,
			DirectionalLightPtr.Get()
		);
	}
	{
		// #97
		if (!BPSkySpherePtr.IsValid())
		{
			BPSkySpherePtr = T4EngineUtility::FindBPSkySphereActor(InWorld);
		}
		T4EngineUtility::ApplyBPSkySphere(
			InWorld,
			&FinalEnvrionmentData.TimeTagData.BPSkySphereData,
			BPSkySpherePtr.Get(),
			DirectionalLightPtr.Get() // #97
		);
	}
	{
		if (!SkyLightPtr.IsValid())
		{
			SkyLightPtr = T4EngineUtility::FindSkyLightActor(InWorld);
		}
		T4EngineUtility::ApplySkyLight(
			InWorld,
			&FinalEnvrionmentData.TimeTagData.SkyLightData,
			SkyLightPtr.Get()
		);
	}
	{
		if (!AtmosphericFogPtr.IsValid())
		{
			AtmosphericFogPtr = T4EngineUtility::FindAtmosphericFogActor(InWorld);
		}
		T4EngineUtility::ApplyAtmosphericFog(
			InWorld,
			&FinalEnvrionmentData.TimeTagData.AtmosphericFogData,
			AtmosphericFogPtr.Get()
		);
	}
	{
		if (!ExponentialHeightFogPtr.IsValid())
		{
			ExponentialHeightFogPtr = T4EngineUtility::FindExponentialHeightFogActor(InWorld);
		}
		T4EngineUtility::ApplyExponentialHeightFog(
			InWorld,
			&FinalEnvrionmentData.TimeTagData.ExponentialHeightFogData,
			ExponentialHeightFogPtr.Get()
		);
	}
	{
		// #98
		if (!GlobalMapZoneVolumePtr.IsValid())
		{
			GlobalMapZoneVolumePtr = T4EngineUtility::FindGlobalMapZoneVolume(InWorld);
		}
		T4EngineUtility::ApplyPostProcess(
			InWorld,
			static_cast<const FPostProcessSettings*>(&FinalEnvrionmentData.PostProcessSettings),
			GlobalMapZoneVolumePtr.Get()
		);
	}
}
