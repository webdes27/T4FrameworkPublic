// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4WorldContextPreviewScene.h"

#if WITH_EDITOR

#include "Preview/T4WorldPreviewFloorMeshActor.h"

#include "Classes/World/T4WorldZoneVolume.h" // #94
#include "Public/T4EngineEnvironment.h" // #94
#include "Public/T4EngineSettings.h" // #40

#include "T4Asset/Classes/Entity/T4MapEntityAsset.h" // #87

#include "Components/StaticMeshComponent.h"
#include "Components/BrushComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/LineBatchComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SphereReflectionCaptureComponent.h"

#include "Engine/SkyLight.h"
#include "Engine/DirectionalLight.h" // #90
#include "Engine/ExponentialHeightFog.h" // #90
#include "Atmosphere/AtmosphericFog.h" // #90
#include "AudioDevice.h"

#include "NavMesh/NavMeshBoundsVolume.h"
#include "AI/NavigationSystemBase.h"
#include "AI/AISystemBase.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"

#include "T4EngineInternal.h"

static const float DefaultFloorMeshScaleXY = 8.0f;
static const float DefaultNavBoundCM = 20000.0f; // 200M

/**
  * #87
 */
FT4WorldContextPreviewScene::FT4WorldContextPreviewScene(
	FT4WorldController* InWorldController,
	bool bInThumbnailMode
)
	: PreviewWorld(nullptr)
	, WorldControllerRef(InWorldController)
	, bThumbnailMode(bInThumbnailMode)
	, bRealWorldUsed(false)
{
	EObjectFlags NewObjectFlags = RF_Transactional;
	PreviewWorld = NewObject<UWorld>(GetTransientPackage(), NAME_None, NewObjectFlags);
	PreviewWorld->WorldType = EWorldType::GamePreview;

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(PreviewWorld->WorldType);
	WorldContext.SetCurrentWorld(PreviewWorld);

	PreviewWorld->InitializeNewWorld(
		UWorld::InitializationValues()
		.AllowAudioPlayback(false)
		.CreatePhysicsScene(true)
		.RequiresHitProxies((bThumbnailMode) ? false : true)
		.CreateNavigation(false)
		.CreateAISystem(false)
		.ShouldSimulatePhysics(true)
		.SetTransactional(true)
	);

	PreviewWorld->InitializeActorsForPlay(FURL());

	// since advance preview scenes are used in conjunction with material/mesh editors we should match their feature level with the global one
	if (GIsEditor && GEditor != nullptr)
	{
		PreviewWorld->ChangeFeatureLevel(GEditor->DefaultWorldFeatureLevel);
	}

	CreateEnvironment(); // #94
	CreateNavigationSystem(); // #34

	if (!bThumbnailMode)
	{
		PreviewWorld->CreateAISystem();
		PreviewWorld->GetAISystem()->InitializeActorsForPlay(false);
	}
	else
	{
		SetThumbnailMode(true);
	}
}

FT4WorldContextPreviewScene::~FT4WorldContextPreviewScene()
{
	// Stop any audio components playing in this scene
	if (GEngine)
	{
		if (nullptr != PreviewWorld)
		{
			if (FAudioDevice* AudioDevice = PreviewWorld->GetAudioDevice())
			{
				AudioDevice->Flush(PreviewWorld, false);
			}
		}
	}

	PreviewWorld->CleanupWorld();
	GEngine->DestroyWorldContext(PreviewWorld);

	PreviewWorld->DestroyWorld(false);
	PreviewWorld = nullptr;
}

void FT4WorldContextPreviewScene::Reset()
{
	if (NavMeshBoundsVolumePtr.IsValid())
	{
		NavMeshBoundsVolumePtr->RemoveFromRoot();
		NavMeshBoundsVolumePtr.Reset();
	}
	if (FloorMeshActorPtr.IsValid())
	{
		FloorMeshActorPtr->RemoveFromRoot();
		FloorMeshActorPtr.Reset();
	}
	// #T4_ADD_TOD_TAG
	// #94
	if (DirectionalLightActorPtr.IsValid())
	{
		DirectionalLightActorPtr->RemoveFromRoot();
		DirectionalLightActorPtr.Reset();
	}
	if (BPSkySphereActorPtr.IsValid()) // #97
	{
		BPSkySphereActorPtr->RemoveFromRoot();
		BPSkySphereActorPtr.Reset();
	}
	if (SkyLightActorPtr.IsValid())
	{
		SkyLightActorPtr->RemoveFromRoot();
		SkyLightActorPtr.Reset();
	}
	if (AtmosphericFogActorPtr.IsValid())
	{
		AtmosphericFogActorPtr->RemoveFromRoot();
		AtmosphericFogActorPtr.Reset();
	}
	if (ExponentialHeightFogActorPtr.IsValid())
	{
		ExponentialHeightFogActorPtr->RemoveFromRoot();
		ExponentialHeightFogActorPtr.Reset();
	}
	if (GlobalWorldZoneVolumePtr.IsValid()) // #98
	{
		GlobalWorldZoneVolumePtr->RemoveFromRoot();
		GlobalWorldZoneVolumePtr.Reset();
	}
	// ~#94
	bRealWorldUsed = true;
}

void FT4WorldContextPreviewScene::ProcessPre(float InDeltaTime) // #34 : OnWorldPreActorTick
{
	if (bThumbnailMode)
	{
		// 다른 Preview 창이 열리면 기본 값으로 돌려서 다시 옵션을 적용해준다.
		if (FloorMeshActorPtr->FloorMeshComponent->IsVisible())
		{
			SetThumbnailMode(bThumbnailMode);
		}
	}
}

void FT4WorldContextPreviewScene::ProcessPost(float InDeltaTime) // #34 : OnWorldPostActorTick
{

}

FWorldContext* FT4WorldContextPreviewScene::GetOwnerWorldContext() const // #87 : WorldContext 소유권이 있다.
{
	return GEngine->GetWorldContextFromWorld(PreviewWorld);
}

bool FT4WorldContextPreviewScene::WorldTravel(
	const FSoftObjectPath& InAssetPath, 
	const FVector& InStartLocation
)
{
	UWorld* UnrealWorld = WorldControllerRef->GetWorld();
	if (nullptr == UnrealWorld)
	{
		return false;
	}
	const FString LongPackageName = InAssetPath.GetLongPackageName();
	return true;
}

void FT4WorldContextPreviewScene::CreateEnvironment() // #94
{
	// #94 : PreviewWorld 의 Environment 속성을 제거하고 신규 Actor 로 스폰해준다. EnvironmentAsset TOD 적용
	//       아래 컨포넌는 모두 FAdvancedPreviewScene 에서 스폰한 것이라 삭제를 해주어야 함!
	// #T4_ADD_TOD_TAG
	{
		DirectionalLightActorPtr = T4EngineEnvironment::SpawnDirectionalLightActor(PreviewWorld, RF_Transient);
		check(DirectionalLightActorPtr.IsValid());
		DirectionalLightActorPtr->AddToRoot();
	}
	{
		BPSkySphereActorPtr = T4EngineEnvironment::SpawnBPSkySphereActor(PreviewWorld, RF_Transient);
		check(BPSkySphereActorPtr.IsValid());
		BPSkySphereActorPtr->AddToRoot();
	}
	{
		SkyLightActorPtr = T4EngineEnvironment::SpawnSkyLightActor(PreviewWorld, RF_Transient);
		check(SkyLightActorPtr.IsValid());
		SkyLightActorPtr->AddToRoot();
	}
	{
		AtmosphericFogActorPtr = T4EngineEnvironment::SpawnAtmosphericFogActor(PreviewWorld, RF_Transient);
		check(AtmosphericFogActorPtr.IsValid());
		AtmosphericFogActorPtr->AddToRoot();
	}
	{
		ExponentialHeightFogActorPtr = T4EngineEnvironment::SpawnExponentialHeightFogActor(PreviewWorld, RF_Transient);
		check(ExponentialHeightFogActorPtr.IsValid());
		ExponentialHeightFogActorPtr->AddToRoot();
	}
	{
		GlobalWorldZoneVolumePtr = T4EngineEnvironment::WorldSpawnWorldZoneVolume(
			PreviewWorld,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			FVector(1000.0f),
			ET4EntityZoneBrushType::Cube,
			T4Const_GlobalWorldZoneName,
			true
		);
		check(GlobalWorldZoneVolumePtr.IsValid());
		GlobalWorldZoneVolumePtr->AddToRoot();
		UT4EngineSettings* EngineSettings = GetMutableDefault<UT4EngineSettings>();
		check(nullptr != EngineSettings);
		if (bThumbnailMode)
		{
			GlobalWorldZoneVolumePtr->EnvironmentAsset = EngineSettings->ThumbnailSceneEnvironmentAssetPath; // #97
		}
		else
		{
			GlobalWorldZoneVolumePtr->EnvironmentAsset = EngineSettings->PreviewSceneEnvironmentAssetPath;
		}
		GlobalWorldZoneVolumePtr->BlendPriority = -1;
		GlobalWorldZoneVolumePtr->BlendInTimeSec = 1.0f;
		GlobalWorldZoneVolumePtr->BlendOutTimeSec = 1.0f;
		GlobalWorldZoneVolumePtr->DebugColor = FColor::White;
		if (nullptr != GlobalWorldZoneVolumePtr->GetBrushComponent())
		{
			GlobalWorldZoneVolumePtr->GetBrushComponent()->SetMobility(EComponentMobility::Movable);
		}
		//GlobalWorldZoneVolumePtr->Modify();
	}
}

void FT4WorldContextPreviewScene::CreateNavigationSystem()
{
	// #34 : NavMeshBoundVolume Actor 를 스폰한다.
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save AI controllers into a map
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnInfo.bDeferConstruction = false;
		NavMeshBoundsVolumePtr = PreviewWorld->SpawnActor<ANavMeshBoundsVolume>(
			ANavMeshBoundsVolume::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnInfo
		);
		check(NavMeshBoundsVolumePtr.IsValid());
		NavMeshBoundsVolumePtr->AddToRoot();
	}
	// #34 : Navigation 생성을 위해 Floor Actor 를 생성한다.
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save AI controllers into a map
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnInfo.bDeferConstruction = true;
		FloorMeshActorPtr = PreviewWorld->SpawnActor<AT4WorldPreviewFloorMeshActor>(
			AT4WorldPreviewFloorMeshActor::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnInfo
		);
		check(FloorMeshActorPtr.IsValid());

		UStaticMesh* FloorMesh = LoadObject<UStaticMesh>(
			nullptr,
			TEXT("/Engine/EditorMeshes/AssetViewer/Floor_Mesh.Floor_Mesh"),
			nullptr,
			LOAD_None,
			nullptr
		);
		check(FloorMesh);
		FloorMeshActorPtr->FloorMeshComponent->SetStaticMesh(FloorMesh);
		FloorMeshActorPtr->SetActorScale3D(FVector(DefaultFloorMeshScaleXY, DefaultFloorMeshScaleXY, 1.0f));
		FloorMeshActorPtr->SetActorLocation(FVector(0.0f, 0.0f, -1.5f)); // Grid Z-Fighting
		FloorMeshActorPtr->AddToRoot();

		const FTransform SpawnTransform(FRotator::ZeroRotator, FVector::ZeroVector);
		UGameplayStatics::FinishSpawningActor(FloorMeshActorPtr.Get(), SpawnTransform);
	}
	// #34 : NavMesh 를 World 에 바인딩 한다.
	{
		FNavigationSystem::AddNavigationSystemToWorld(
			*PreviewWorld,
			FNavigationSystemRunMode::EditorMode,
			nullptr,
			true /*bInitializeForWorld=*/
		);
		PreviewWorld->GetNavigationSystem()->OnInitializeActors();
	}
	// #34 : NavMesh Bound Update
	{
		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(PreviewWorld);
		if (nullptr != NavSystem)
		{
			UBrushComponent* Brush = NavMeshBoundsVolumePtr->GetBrushComponent();
			if (nullptr != Brush)
			{
				Brush->Bounds = FBox(-FVector(DefaultNavBoundCM), FVector(DefaultNavBoundCM));
			}
			NavSystem->OnNavigationBoundsUpdated(NavMeshBoundsVolumePtr.Get());
		}
	}
}

void FT4WorldContextPreviewScene::SetThumbnailMode(bool bInShowWindowMode)
{
	bThumbnailMode = bInShowWindowMode;
	if (FloorMeshActorPtr.IsValid())
	{
		FloorMeshActorPtr->FloorMeshComponent->SetVisibility(!bThumbnailMode);
	}
}

#endif