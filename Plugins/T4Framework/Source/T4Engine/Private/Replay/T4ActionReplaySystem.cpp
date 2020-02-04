// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionReplaySystem.h"
#include "Public/Replay/T4ActionReplayUtility.h"

#if !UE_BUILD_SHIPPING

#include "T4ActionReplayPlayer.h"
#include "T4ActionReplayRecorder.h"

#include "World/T4GameWorld.h"

#include "T4EngineInternal.h"

/**
  * #68
 */
FT4ActionReplaySystem::FT4ActionReplaySystem()
	: LayerType(ET4LayerType::Max)
	, bPlayRepeat(false)
	, bPlayerPossessed(true)
	, PlaybackPlayer(nullptr)
	, PlaybackRecorder(nullptr)
{
}

FT4ActionReplaySystem::~FT4ActionReplaySystem()
{
	check(nullptr == PlaybackPlayer);
	check(nullptr == PlaybackRecorder);
}

void FT4ActionReplaySystem::Reset()
{
	DoStopPlaying();
	DoStopRecording();
}

void FT4ActionReplaySystem::Process(float InDeltaTime)
{
	if (nullptr != PlaybackPlayer)
	{
		PlaybackPlayer->ProcessPlayInsideWorld(InDeltaTime);
	}
	if (nullptr != PlaybackRecorder)
	{
		PlaybackRecorder->ProcessRecording(InDeltaTime);
	}
}

void FT4ActionReplaySystem::ProcessOutsideWorld(float InDeltaTime)
{
	if (nullptr != PlaybackPlayer)
	{
		PlaybackPlayer->ProcessPlayOutsideWorld(InDeltaTime);
	}
}

IT4ActionReplayPlayer* FT4ActionReplaySystem::GetPlayer() const
{ 
	return static_cast<IT4ActionReplayPlayer*>(PlaybackPlayer);
}

IT4ActionReplayRecorder* FT4ActionReplaySystem::GetRecorder() const
{ 
	return static_cast<IT4ActionReplayRecorder*>(PlaybackRecorder);
}

bool FT4ActionReplaySystem::DoPlay(const FSoftObjectPath& InPlayPath)
{
	if (nullptr != PlaybackPlayer)
	{
		DoStopPlaying();
	}
	check(nullptr == PlaybackPlayer);
	PlaybackPlayer = new FT4ActionReplayPlayer(LayerType, bPlayRepeat, bPlayerPossessed);
	check(nullptr != PlaybackPlayer);
	bool bResult = PlaybackPlayer->DoPlay(InPlayPath);
	if (!bResult)
	{
		T4_LOG(
			Error,
			TEXT("Failed to playing. '%s' not found"),
			*(InPlayPath.ToString())
		);
		PlaybackPlayer->OnReset();
		delete PlaybackPlayer;
		PlaybackPlayer = nullptr;
		return false;
	}
	return true;
}

bool FT4ActionReplaySystem::DoPlay(
	const FString& InPlayAssetName,
	const FString& InFolderName
) // /T4Framework/Editor/ActionReplay/<InFolderName>/<InPlayAssetName>.uasset
{
	if (nullptr != PlaybackPlayer)
	{
		DoStopPlaying();
	}
	check(nullptr == PlaybackPlayer);
	PlaybackPlayer = new FT4ActionReplayPlayer(LayerType, bPlayRepeat, bPlayerPossessed);
	check(nullptr != PlaybackPlayer);
	bool bResult = PlaybackPlayer->DoPlay(InPlayAssetName, InFolderName);
	if (!bResult)
	{
		T4_LOG(
			Error,
			TEXT("Failed to playing. '%s' not found"),
			*(InPlayAssetName)
		);
		PlaybackPlayer->OnReset();
		delete PlaybackPlayer;
		PlaybackPlayer = nullptr;
		return false;
	}
	return true;
}

void FT4ActionReplaySystem::DoStopPlaying()
{
	if (nullptr == PlaybackPlayer)
	{
		return;
	}
	PlaybackPlayer->OnStop();
	delete PlaybackPlayer;
	PlaybackPlayer = nullptr;
}

void FT4ActionReplaySystem::SetPlayRepeat(bool bEnable)
{
	bPlayRepeat = bEnable;
	if (nullptr == PlaybackPlayer)
	{
		return;
	}
	PlaybackPlayer->SetRepeat(bEnable);
}

void FT4ActionReplaySystem::SetPlayerPossessed(bool bPossess)
{
	bPlayerPossessed = bPossess;
	if (nullptr == PlaybackPlayer)
	{
		return;
	}
	PlaybackPlayer->SetPlayerPossessed(bPossess);
}

bool FT4ActionReplaySystem::DoRec(const FSoftObjectPath& InRecPath)
{
	if (nullptr != PlaybackRecorder)
	{
		T4_LOG(
			Error,
			TEXT("Failed to recording. Already recorded")
		);
		return false;
	}
	PlaybackRecorder = new FT4ActionReplayRecorder(LayerType);
	check(nullptr != PlaybackRecorder);
	bool bResult = PlaybackRecorder->DoRec(InRecPath);
	if (!bResult)
	{
		T4_LOG(
			Error,
			TEXT("Failed to recording. '%s' not found"),
			*(InRecPath.ToString())
		);
		PlaybackRecorder->OnReset();
		delete PlaybackRecorder;
		PlaybackRecorder = nullptr;
		return false;
	}
	// #68 : 현 시점에 스폰된 오브젝트 정보를 기록해주어야 한다.
	FT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	GameWorld->SaveActionReplaySnapshot();
	return true;
}

bool FT4ActionReplaySystem::DoRec(
	const FString& InRecAssetName,
	const FString& InFolderName
) // /T4Framework/Editor/ActionReplay/<InFolderName>/<InRecAssetName>.uasset
{
	if (nullptr != PlaybackRecorder)
	{
		T4_LOG(
			Error,
			TEXT("Failed to recording. Already recorded")
		);
		return false;
	}
	PlaybackRecorder = new FT4ActionReplayRecorder(LayerType);
	check(nullptr != PlaybackRecorder);
	bool bResult = PlaybackRecorder->DoRec(InRecAssetName, InFolderName);
	if (!bResult)
	{
		T4_LOG(
			Error,
			TEXT("Failed to recording. '%s' not found"),
			*(InRecAssetName)
		);
		PlaybackRecorder->OnReset();
		delete PlaybackRecorder;
		PlaybackRecorder = nullptr;
		return false;
	}
	// #68 : 현 시점에 스폰된 오브젝트 정보를 기록해주어야 한다.
	FT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	GameWorld->SaveActionReplaySnapshot();
	return true;
}

void FT4ActionReplaySystem::DoStopRecording()
{
	if (nullptr == PlaybackRecorder)
	{
		return;
	}
	PlaybackRecorder->OnStop();
	delete PlaybackRecorder;
	PlaybackRecorder = nullptr;
}

FT4GameWorld* FT4ActionReplaySystem::GetGameWorld() const
{
	return static_cast<FT4GameWorld*>(T4EngineWorldGet(LayerType));
}

#endif