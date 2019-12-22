// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WorldPreviewGameInstance.h"
#include "T4WorldPreviewGameMode.h"

#include "T4EngineInternal.h"

/**
  * #79
 */
UT4WorldPreviewGameInstance::UT4WorldPreviewGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bLevelStreamingEnabled(true) // #86
{
}

AGameModeBase* UT4WorldPreviewGameInstance::CreateGameModeForURL(FURL InURL)
{
	return Super::CreateGameModeForURL(InURL);
}

TSubclassOf<AGameModeBase> UT4WorldPreviewGameInstance::OverrideGameModeClass(
	TSubclassOf<AGameModeBase> GameModeClass,
	const FString& MapName,
	const FString& Options,
	const FString& Portal
) const
{
	return AT4WorldPreviewGameMode::StaticClass();
}

void UT4WorldPreviewGameInstance::SetWorldContext(FWorldContext* InWorldContext)
{
	WorldContext = InWorldContext;
}

void UT4WorldPreviewGameInstance::SetLocalPlayer(ULocalPlayer* InLocalPlayer) // #86
{
	check(nullptr != InLocalPlayer);
	check(0 == GetNumLocalPlayers());
	AddLocalPlayer(InLocalPlayer, 1);
	bLevelStreamingEnabled = true;
}

void UT4WorldPreviewGameInstance::SetLevelStreaming(bool bEnable) // #86
{
	// #86 : Rehearsal GameInstance 는 기본적으로 LocalPlayer 를 제공하지 않으나,
	//       SimulationMode 일 경우에만 노출함. 즉, 노출이 되어야 UpdateStreamingState 가 업데이트 되기 때문
	if (bLevelStreamingEnabled == bEnable)
	{
		if (bLevelStreamingEnabled)
		{
			check(1 == LocalPlayers.Num());
		}
		return;
	}
	bLevelStreamingEnabled = bEnable;
	if (bLevelStreamingEnabled)
	{
		LocalPlayers.Empty();
		LocalPlayers = BackupLocalPlayer;
		BackupLocalPlayer.Empty();
	}
	else
	{
		BackupLocalPlayer.Empty();
		BackupLocalPlayer = LocalPlayers;
		LocalPlayers.Empty();
	}
}