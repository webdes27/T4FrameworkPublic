// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/T4GameInstance.h"

#include "Public/T4Frame.h"

#include "T4FrameInternal.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Framework/GameFlow/index.html
 */
UT4GameInstance::UT4GameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, GameFrame(nullptr)
{
}

void UT4GameInstance::Init()
{
	Super::Init();
	// #15
	ET4FrameType CreateFrameworkType = ET4FrameType::Frame_Client;
	if (T4EngineLayer::IsServer(WorldContext))
	{
		CreateFrameworkType = ET4FrameType::Frame_Server;
	}
	FT4WorldConstructionValues WorldConstructionValues; // #87
	WorldConstructionValues.GameWorldType = ET4GameWorldType::Game; // #87 : 필요하다면 Editor 도 추가할 것!
	WorldConstructionValues.WorldContextGameOrEditorOnly = WorldContext;
	GameFrame = T4FrameCreate(CreateFrameworkType, WorldConstructionValues);
	check(nullptr != GameFrame);
}

void UT4GameInstance::Shutdown()
{
	if (nullptr != GameFrame)
	{
		T4FrameDestroy(GameFrame);
		GameFrame = nullptr;
	}
	Super::Shutdown();
}

#if WITH_EDITOR
FGameInstancePIEResult UT4GameInstance::StartPlayInEditorGameInstance(
	ULocalPlayer* LocalPlayer,
	const FGameInstancePIEParameters& Params
)
{
	// only editor used. game call to AT4GameMode::StartPlay()
	FGameInstancePIEResult PIEResult = Super::StartPlayInEditorGameInstance(LocalPlayer, Params);
	if (PIEResult.IsSuccess())
	{
		check(nullptr != GameFrame);
		GameFrame->OnStartPlay();
	}
	return PIEResult;
}
#endif