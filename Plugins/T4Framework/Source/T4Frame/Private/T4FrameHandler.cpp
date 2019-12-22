// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4FrameHandler.h"

#include "Public/T4Frame.h"
#include "Classes/T4GameHUD.h" // #68

#include "Engine/World.h"
#include "Engine/Canvas.h"

#if WITH_EDITOR
#include "Editor.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"
#endif

#include "T4FrameInternal.h"

/**
  *
 */
FT4FrameHandler::FT4FrameHandler()
{
}

FT4FrameHandler::~FT4FrameHandler()
{
}

void FT4FrameHandler::Initialize()
{
	// #34
	OnWorldPreActorTickHandle = FWorldDelegates::OnWorldPreActorTick.AddRaw(
		this, 
		&FT4FrameHandler::HandleOnWorldPreActorTick
	);
	OnWorldPostActorTickHandle = FWorldDelegates::OnWorldPostActorTick.AddRaw(
		this, 
		&FT4FrameHandler::HandleOnWorldPostActorTick
	);

	// #68
	OnHUDPostRenderHandle = AHUD::OnHUDPostRender.AddRaw(
		this, 
		&FT4FrameHandler::HandleOnHUDPostRender
	);

#if WITH_EDITOR
	OnWorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddRaw(
		this, 
		&FT4FrameHandler::HandleOnWorldCleanup
	); // #29

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	OnMapChangedEventHandle = LevelEditorModule.OnMapChanged().AddRaw(
		this, 
		&FT4FrameHandler::HandleOnMapChangedEvent
	); // #17
#endif
}

void FT4FrameHandler::Finalize()
{
	if (OnWorldPreActorTickHandle.IsValid())
	{
		OnWorldPreActorTickHandle.Reset();
	}
	if (OnWorldPostActorTickHandle.IsValid())
	{
		OnWorldPostActorTickHandle.Reset();
	}
	if (OnHUDPostRenderHandle.IsValid())
	{
		OnHUDPostRenderHandle.Reset();
	}
#if WITH_EDITOR
	if (OnWorldCleanupHandle.IsValid())
	{
		OnWorldCleanupHandle.Reset();
	}
	if (OnMapChangedEventHandle.IsValid())
	{
		OnMapChangedEventHandle.Reset();
	}
#endif
}

void FT4FrameHandler::HandleOnWorldPreActorTick(
	UWorld* InWorld,
	ELevelTick InTickType,
	float InDeltaTime
)
{
	ET4LayerType LayerType = T4EngineLayer::Get(InWorld); // #12 : Support Multiple LayerType
	if (ET4LayerType::Max == LayerType)
	{
		return; // #61 : GameInstance 를 변경했을 경우에도 무조건 호출됨으로 T4용이 아니면 예외처리 해준다.
	}
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	if (nullptr != GameFrame)
	{
		GameFrame->OnProcessPre(InDeltaTime);
	}
}

void FT4FrameHandler::HandleOnWorldPostActorTick(
	UWorld* InWorld,
	ELevelTick InTickType,
	float InDeltaTime
)
{
	ET4LayerType LayerType = T4EngineLayer::Get(InWorld); // #12 : Support Multiple LayerType
	if (ET4LayerType::Max == LayerType)
	{
		return; // #61 : GameInstance 를 변경했을 경우에도 무조건 호출됨으로 T4용이 아니면 예외처리 해준다.
	}
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	if (nullptr != GameFrame)
	{
		GameFrame->OnProcessPost(InDeltaTime);
	}
}

void FT4FrameHandler::HandleOnHUDPostRender(AHUD* InHUD, UCanvas* InCanvas) // #68
{
	AT4GameHUD* GameHUD = Cast<AT4GameHUD>(InHUD);
	if (nullptr == GameHUD)
	{
		return;
	}
	const ET4LayerType GameHUDLayerType = GameHUD->GetLayerType();
	check(T4EngineLayer::IsClient(GameHUDLayerType)); // WARN : HUD 는 클라 레이어만 생성했다!!
	IT4GameFrame* GameFrame = T4FrameGet(GameHUDLayerType);
	if (nullptr != GameFrame && nullptr != GameFrame->GetViewport())
	{
		FT4HUDDrawInfo DrawInfo;
		GameFrame->OnDrawHUD(GameFrame->GetViewport(), InCanvas->Canvas, DrawInfo);
	}
}

#if WITH_EDITOR
void FT4FrameHandler::HandleOnWorldCleanup(
	UWorld* InWorld,
	bool bSessionEnded,
	bool bCleanupResources
) // #29
{
	ET4LayerType LayerType = T4EngineLayer::Get(InWorld); // #12 : Support Multiple LayerType
	if (ET4LayerType::Max == LayerType)
	{
		return; // #61 : GameInstance 를 변경했을 경우에도 무조건 호출됨으로 T4용이 아니면 예외처리 해준다.
	}
	if (T4EngineLayer::IsLevelEditor(LayerType)) // #30
	{
		IT4GameFrame* GameFrame = T4FrameGet(LayerType);
		if (nullptr != GameFrame)
		{
			GameFrame->OnReset();
		}
	}
}

void FT4FrameHandler::HandleOnMapChangedEvent(
	UWorld* InWorld,
	EMapChangeType InMapChangeType
) // #17
{
	ET4LayerType LayerType = T4EngineLayer::Get(InWorld); // #12 : Support Multiple LayerType
	if (ET4LayerType::Max == LayerType)
	{
		return; // #61 : GameInstance 를 변경했을 경우에도 무조건 호출됨으로 T4용이 아니면 예외처리 해준다.
	}
	if (!T4EngineLayer::IsLevelEditor(LayerType)) // #30
	{
		return;
	}
	switch (InMapChangeType)
	{
		case EMapChangeType::LoadMap:
		case EMapChangeType::NewMap:
			//OnStartPlay();
			break;

		/*
		// 에디터 종료시 호출이 안되어서 OnWorldCleanup 으로 대체!
		case EMapChangeType::TearDownWorld:
			check(nullptr != GameWorld);
			GameWorld->OnReset();
			break;
		*/

		case EMapChangeType::SaveMap:
			break;

		default:
			{
				UE_LOG(
					LogT4Frame,
					Error,
					TEXT("OnMapChangedEvent '%u' failed. no implementation."),
					uint32(InMapChangeType)
				);
			}
			break;
	}	
}
#endif

static FT4FrameHandler GT4FrameHandler;
FT4FrameHandler& GetFrameHandler()
{
	return GT4FrameHandler;
}