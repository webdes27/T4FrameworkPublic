// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameFrame.h"

#include "T4Asset/Classes/Entity/T4MapEntityAsset.h" // #39

#include "T4Engine/Public/T4Engine.h"
#include "T4Engine/Public/Asset/T4AssetManager.h"
#include "T4Engine/Public/Action/T4ActionCodeWorld.h" // #87

#include "Engine/Engine.h"

#include "T4FrameInternal.h"

/**
  *
 */
FT4GameFrame::FT4GameFrame()
	: LayerType(ET4LayerType::Max)
	, bInitialized(false)
	, bBegunPlay(false)
	, GameWorld(nullptr)
	, GameplayInstance(nullptr) // #42
{
}

FT4GameFrame::~FT4GameFrame()
{
	check(nullptr == GameWorld);
	check(nullptr == GameplayInstance); // #42
}

bool FT4GameFrame::OnInitialize(
	const FT4WorldConstructionValues& InWorldConstructionValues // #87
)
{
	// #8
	T4AssetManagerGet()->Initialize();

	ET4WorldType CreateWorldType = ET4WorldType::Client;
	if (ET4FrameType::Frame_Server == GetType())
	{
		CreateWorldType = ET4WorldType::Server;
	}
	GameWorld = T4EngineWorldCreate(CreateWorldType, InWorldConstructionValues);
	check(nullptr != GameWorld);

	// #17, #87
	check(LayerType == ET4LayerType::Max);
	LayerType = GameWorld->GetLayerType();
	check(LayerType < ET4LayerType::Max);

	if (nullptr != GameplayInstance) // #42
	{
		GameplayInstance->OnInitialize(LayerType);
	}
	
	bInitialized = Initialize();
	if (!bInitialized)
	{
		UE_LOG(
			LogT4Frame,
			Display,
			TEXT("[SL:%u] FT4GameFrame : failed to Initialized."),
			uint32(LayerType)
		);

		T4AssetManagerGet()->Finalize(); // #8
		return false;
	}

	// #87 : 월드 이동 playback 지원
	FT4EngineDelegates::OnGameWorldTravelPre.AddRaw(this, &FT4GameFrame::HandleOnGameWorldTravelPre); // #87
	FT4EngineDelegates::OnGameWorldTravelPost.AddRaw(this, &FT4GameFrame::HandleOnGameWorldTravelPost); // #87

	return bInitialized;
}

void FT4GameFrame::OnFinalize()
{
	// #87 : 월드 이동 playback 지원
	FT4EngineDelegates::OnGameWorldTravelPre.RemoveAll(this); // #87
	FT4EngineDelegates::OnGameWorldTravelPost.RemoveAll(this); // #87

	UE_LOG(
		LogT4Frame,
		Display,
		TEXT("[SL:%u] FT4GameFrame::Finalize."),
		uint32(LayerType)
	);

	OnReset();

	Finalize();

	if (nullptr != GameplayInstance) // #42
	{
		GameplayInstance->OnFinalize();
		delete GameplayInstance;
		GameplayInstance = nullptr;
	}

	if (nullptr != GameWorld)
	{
		T4EngineWorldDestroy(GameWorld);
		GameWorld = nullptr;
	}

	T4AssetManagerGet()->Finalize(); // #8
}

UWorld* FT4GameFrame::GetWorld() const
{
	if (nullptr == GameWorld)
	{
		return nullptr;
	}
	return GameWorld->GetWorld();
}

IT4GameWorld* FT4GameFrame::GetGameWorld() const
{
	return GameWorld;
}

void FT4GameFrame::OnReset()
{
	check(nullptr != GameWorld);
	ResetPre();
	GameWorld->OnReset();
	bBegunPlay = false;
	if (nullptr != GameplayInstance)
	{
		GameplayInstance->OnReset(); // #42
	}
	ResetPost();
}

void FT4GameFrame::OnStartPlay()
{
	bBegunPlay = true;
	if (nullptr != GameplayInstance)
	{
		GameplayInstance->OnStartPlay(); // #42
	}
	StartPlay();
}

void FT4GameFrame::OnProcessPre(float InDeltaTime)
{
	if (!bInitialized || !bBegunPlay)
	{
		return;
	}
	check(nullptr != GameWorld);
	GameWorld->OnProcessPre(InDeltaTime);
	T4AssetManagerGet()->Process(InDeltaTime);
	ProcessPre(InDeltaTime);
	if (nullptr != GameplayInstance)
	{
		GameplayInstance->OnProcess(InDeltaTime); // #42
	}
}

void FT4GameFrame::OnProcessPost(float InDeltaTime)
{
	if (!bInitialized || !bBegunPlay)
	{
		return;
	}
	ProcessPost(InDeltaTime);
	check(nullptr != GameWorld);
	GameWorld->OnProcessPost(InDeltaTime);
}

void FT4GameFrame::OnDrawHUD(
	FViewport* InViewport, 
	FCanvas* InCanvas,
	FT4HUDDrawInfo& InOutDrawInfo
) // #68
{
	if (!bInitialized || !bBegunPlay)
	{
		return;
	}
	DrawHUD(InViewport, InCanvas, InOutDrawInfo);
}

bool FT4GameFrame::OnWorldTravel(const UT4MapEntityAsset* InMapEntityAsset) // #87
{
	if (nullptr != InMapEntityAsset)
	{
		if (ET4EntityType::Map != InMapEntityAsset->GetEntityType())
		{
			return false;
		}
		const FT4EntityMapData& MapData = InMapEntityAsset->MapData;
		if (MapData.LevelAsset.IsNull())
		{
			return false;
		}
	}
	// OnReset(); // #87 : 월드 이동 playback 지원 : FT4EngineDelegates::OnGameWorldTravelPre : 
	{
		FT4WorldTravelAction NewAction;
		NewAction.MapEntityOrLevelObjectPath = InMapEntityAsset;
#if WITH_EDITOR
		NewAction.bPreveiwScene = (nullptr == InMapEntityAsset) ? true : false;
#endif
		check(nullptr != GameWorld);
		GameWorld->DoExecuteAction(&NewAction); // #83, #87
	}
	// OnStartPlay(); // // #87 : 월드 이동 playback 지원 : FT4EngineDelegates::OnGameWorldTravelPost
	return true;
}

void FT4GameFrame::HandleOnGameWorldTravelPre(IT4GameWorld* InGameWorld) // #87 : 월드 이동 playback 지원
{
	if (nullptr == InGameWorld || GameWorld != InGameWorld)
	{
		return;
	}
	OnReset();
}

void FT4GameFrame::HandleOnGameWorldTravelPost(IT4GameWorld* InGameWorld) // #87 : 월드 이동 playback 지원
{
	if (nullptr == InGameWorld || GameWorld != InGameWorld)
	{
		return;
	}
	OnStartPlay();
}
