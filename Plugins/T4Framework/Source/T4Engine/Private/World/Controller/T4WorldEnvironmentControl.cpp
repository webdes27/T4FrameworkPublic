// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4WorldEnvironmentControl.h"

#include "T4WorldController.h" // #93
#include "World/Container/T4WorldContainer.h" // #94
#include "World/T4GameWorld.h" // #94

#include "Object/Component/T4EnvironmentZoneComponent.h" // #99

#include "Classes/World/T4WorldZoneVolume.h" // #92

#include "Public/T4EngineConstants.h" // #115
#include "Public/T4EngineEnvironment.h" // #92
#include "Public/T4EngineDefinitions.h" // #93

#include "Components/BrushComponent.h" //  #115

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
	if (TransientGlobalWorldZoneVolumePtr.IsValid()) // #115 : World 에 Global Zone 이 없을 경우 강제로 생성 (PP 효과)
	{
		TransientGlobalWorldZoneVolumePtr->RemoveFromRoot();
		TransientGlobalWorldZoneVolumePtr.Reset();
	}
	DirectionalLightPtr.Reset(); // #92
	BPSkySpherePtr.Reset();
	SkyLightPtr.Reset();
	AtmosphericFogPtr.Reset();
	ExponentialHeightFogPtr.Reset();
	GlobalWorldZoneVolumePtr.Reset();
}

void FT4WorldEnvironmentControl::StartPlay() // #115
{
	FT4GameWorld* GameWorld = WorldControllerRef->GetGameWorldImpl();
	check(nullptr != GameWorld);
	const ET4WorldType GameWorldType = GameWorld->GetType();
	if (ET4WorldType::Server == GameWorldType)
	{
		return; // #115 : 서버는 Env 를 사용할 필요가 없다. (현재는 비쥬얼 요소 Only)
	}
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
	const ET4WorldType GameWorldType = GameWorld->GetType(); // #115 : 서버는 Env 를 사용할 필요가 없다. (현재는 비쥬얼 요소 Only)
	const bool bClientRenderable = GameWorld->IsClientRenderable(); // #115 : 아직 그릴 준비가 안되어있다. (PC가 스폰되지 않음)
	if (ET4WorldType::Client != GameWorldType || !bClientRenderable) 
	{
		return;
	}

	UWorld* UnrealWorld = WorldControllerRef->GetWorld();
	check(nullptr != UnrealWorld);

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
	
	AT4WorldZoneVolume* GlobalWorldZoneVolume = nullptr; // #115

	TArray<AT4WorldZoneVolume*> StaticWorldZoneVolumes;
	bool bValidStaticZone = WorldContainer->GetStaticWorldZoneVolumes(StaticWorldZoneVolumes);
	for (AT4WorldZoneVolume* WorldZoneVolume : StaticWorldZoneVolumes)
	{
		check(nullptr != WorldZoneVolume);
		if (nullptr == GlobalWorldZoneVolume && WorldZoneVolume->IsGlobalZone())
		{
			GlobalWorldZoneVolume = WorldZoneVolume; // #115
		}

		if (WorldZoneVolume->EnvironmentAsset.IsNull())
		{
			continue;
		}

		float DistanceToPoint = 0.0f;

		WorldZoneVolume->Update(InDeltaTime);
		WorldZoneVolume->EncompassesPoint(UpdateLocation, 0.0f, &DistanceToPoint);

		if (WorldZoneVolume->IsEntered())
		{
			if (0.0f < DistanceToPoint)
			{
				WorldZoneVolume->Leave();
			}
		}
		else
		{
			if (0.0f >= DistanceToPoint)
			{
				WorldZoneVolume->Enter();
			}
		}

		if (0.0f < WorldZoneVolume->GetBlendWeight())
		{
			FT4EnvironmentZoneInfo& NewZoneInfo = ActiveEnvironmentZoneInfos.AddDefaulted_GetRef();
			NewZoneInfo.ZoneName = WorldZoneVolume->ZoneName;
			NewZoneInfo.ZoneType = WorldZoneVolume->ZoneType;
			NewZoneInfo.BlendPriority = WorldZoneVolume->GetBlendPriority();
			NewZoneInfo.LayerBlendWeight = WorldZoneVolume->GetBlendWeight();
			NewZoneInfo.EnvironmentAsset = WorldZoneVolume->EnvironmentAsset.LoadSynchronous();
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
		if (nullptr != GlobalWorldZoneVolume)
		{
			// #115 : Global MapVolume 을 설치하지 않았을 경우 PP 효과 대응을 위한 조치
			if (!GlobalWorldZoneVolumePtr.IsValid())
			{
				GlobalWorldZoneVolumePtr = T4EngineEnvironment::FindGlobalWorldZoneVolume(UnrealWorld);
			}
			T4EngineEnvironment::ApplyPostProcess(
				UnrealWorld,
				static_cast<const FPostProcessSettings*>(&FinalEnvrionmentData.PostProcessSettings),
				GlobalWorldZoneVolumePtr.Get()
			);
		}
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
			SetDefaultFinalPostProcessSettings(); // #115
			bool bResult = SetGlobal(
				ZoneInfo.EnvironmentAsset,
				SourceTimeTagName, 
				TargetTimeTagName, 
				LocalBlendWeight
			);
			if (!bResult)
			{
				T4_LOG(
					Warning,
					TEXT("SetGlobalZone failed")
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
				T4_LOG(
					Warning,
					TEXT("BlendLocalZone failed")
				);
			}
		}
	}

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
	T4EngineEnvironment::BlendTimeTagData(
		LocalEnvironmentData.TimeTagData, 
		InLayerBlendWeight, 
		FinalEnvrionmentData.TimeTagData
	); // write to FinalTimeTagData

	if (LocalEnvironmentData.TimeTagData.PostProcessData.bEnabled)
	{
		T4EngineEnvironment::BlendPostProcess(
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
	if (T4Const_WorldTimeTagFallbackName == InSourceTimeName)
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
			T4EngineEnvironment::BlendPostProcess(
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
			SourceMapTimeTagData = GetTimeTagData(InEnvironmentAsset, T4EngineEnvironment::GetNextTimeTagName(InSourceTimeName));
			if (nullptr == SourceMapTimeTagData)
			{
				// #93 : 현재 시간이 없다면 Fallback 으로 시도!
				SourceMapTimeTagData = GetTimeTagData(InEnvironmentAsset, T4Const_WorldTimeTagFallbackName);
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
			T4EngineEnvironment::BlendPostProcess(
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
			T4EngineEnvironment::BlendTimeTagData(*TargetMapTimeTagData, BlendWeight, OutEnvironmentData.TimeTagData); // Target
			if (TargetMapTimeTagData->PostProcessData.bEnabled)
			{
				T4EngineEnvironment::BlendPostProcess(
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
			DirectionalLightPtr = T4EngineEnvironment::FindDirectionalLightActor(InWorld);
		}
		T4EngineEnvironment::ApplyDirectional(
			InWorld,
			&FinalEnvrionmentData.TimeTagData.DirectionalData,
			DirectionalLightPtr.Get()
		); // #93
	}
	{
		if (!DirectionalLightPtr.IsValid())
		{
			DirectionalLightPtr = T4EngineEnvironment::FindDirectionalLightActor(InWorld);
		}
		T4EngineEnvironment::ApplyDirectionalLight(
			InWorld,
			&FinalEnvrionmentData.TimeTagData.DirectionalLightData,
			DirectionalLightPtr.Get()
		);
	}
	{
		// #97
		if (!BPSkySpherePtr.IsValid())
		{
			BPSkySpherePtr = T4EngineEnvironment::FindBPSkySphereActor(InWorld);
		}
		T4EngineEnvironment::ApplyBPSkySphere(
			InWorld,
			&FinalEnvrionmentData.TimeTagData.BPSkySphereData,
			BPSkySpherePtr.Get(),
			DirectionalLightPtr.Get() // #97
		);
	}
	{
		if (!SkyLightPtr.IsValid())
		{
			SkyLightPtr = T4EngineEnvironment::FindSkyLightActor(InWorld);
		}
		T4EngineEnvironment::ApplySkyLight(
			InWorld,
			&FinalEnvrionmentData.TimeTagData.SkyLightData,
			SkyLightPtr.Get()
		);
	}
	{
		if (!AtmosphericFogPtr.IsValid())
		{
			AtmosphericFogPtr = T4EngineEnvironment::FindAtmosphericFogActor(InWorld);
		}
		T4EngineEnvironment::ApplyAtmosphericFog(
			InWorld,
			&FinalEnvrionmentData.TimeTagData.AtmosphericFogData,
			AtmosphericFogPtr.Get()
		);
	}
	{
		if (!ExponentialHeightFogPtr.IsValid())
		{
			ExponentialHeightFogPtr = T4EngineEnvironment::FindExponentialHeightFogActor(InWorld);
		}
		T4EngineEnvironment::ApplyExponentialHeightFog(
			InWorld,
			&FinalEnvrionmentData.TimeTagData.ExponentialHeightFogData,
			ExponentialHeightFogPtr.Get()
		);
	}
	{
		// #98
		if (!GlobalWorldZoneVolumePtr.IsValid())
		{
			GlobalWorldZoneVolumePtr = T4EngineEnvironment::FindGlobalWorldZoneVolume(InWorld);
		}
		T4EngineEnvironment::ApplyPostProcess(
			InWorld,
			static_cast<const FPostProcessSettings*>(&FinalEnvrionmentData.PostProcessSettings),
			GlobalWorldZoneVolumePtr.Get()
		);
	}
}

void FT4WorldEnvironmentControl::SetDefaultFinalPostProcessSettings() // #115
{
	FinalEnvrionmentData.PostProcessSettings.SetBaseValues();
	{
		// #115 : Global 한 PP Material 을 모든 EnvrionmentAsset 에 추가하는 것은 번거로운 일임으로
		//        SceneView 로 보내는 Final Data 에만 값을 추가해준다.
		FWeightedBlendables& WeightedBlendables = FinalEnvrionmentData.PostProcessSettings.WeightedBlendables;
		UObject* MaterialPostProcessOutlinerObject = T4EngineConstant::GetMaterialPostProcessOutlinerObject();
		if (nullptr != MaterialPostProcessOutlinerObject)
		{
			FWeightedBlendable& WeightBlendable = WeightedBlendables.Array.AddDefaulted_GetRef();
			WeightBlendable.Weight = 1.0f;
			WeightBlendable.Object = MaterialPostProcessOutlinerObject;
		}
	}
}

void FT4WorldEnvironmentControl::TryCheckAndSpawnGlobalWorldZoneVolume() // #115
{
	// #115 : Global WorldZoneVolume 이 있는지 확인하고 없다면 강제로 생성해준다.
	//        Global WorldZoneVolume 에 기본 PP 효과(Outline) Material 설정이 추가된다.
	// #115 : World 는 무조건 Global WorldZoneVolume 추가하도록 강제한다면 이 처리는 필요가 없다.
	//        현재는 정책적으로 정해지지 않았기 때문에 툴과 클라는 결과가 같도록 유지하기 위해 Fallback Spawn 처리를 해준다.
	FT4GameWorld* GameWorld = WorldControllerRef->GetGameWorldImpl();
	check(nullptr != GameWorld);
	FT4WorldContainer* WorldContainer = GameWorld->GetContainerImpl();
	check(nullptr != WorldContainer);
	TArray<AT4WorldZoneVolume*> StaticWorldZoneVolumes;
	bool bValidStaticZone = WorldContainer->GetStaticWorldZoneVolumes(StaticWorldZoneVolumes);
	for (AT4WorldZoneVolume* WorldZoneVolume : StaticWorldZoneVolumes)
	{
		if (WorldZoneVolume->IsGlobalZone())
		{
			return;
		}
	}
	TransientGlobalWorldZoneVolumePtr = T4EngineEnvironment::WorldSpawnWorldZoneVolume(
		GameWorld->GetWorld(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		FVector(1000.0f),
		ET4EntityZoneBrushType::None, // Global 이라 Brush 를 생성하지 않는다. (사실, Editor 가 아니면 생성 코드도 타지 않음)
		T4Const_GlobalWorldZoneName,
		true
	);
	check(TransientGlobalWorldZoneVolumePtr.IsValid());
	TransientGlobalWorldZoneVolumePtr->AddToRoot();
	TransientGlobalWorldZoneVolumePtr->EnvironmentAsset = nullptr; // #97
	TransientGlobalWorldZoneVolumePtr->BlendPriority = -1;
	TransientGlobalWorldZoneVolumePtr->BlendInTimeSec = 1.0f;
	TransientGlobalWorldZoneVolumePtr->BlendOutTimeSec = 1.0f;
	SetDefaultFinalPostProcessSettings(); // #115
}
