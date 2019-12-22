// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionPlaybackController.h"
#include "Public/Playback/T4ActionPlaybackAPI.h"

#if !UE_BUILD_SHIPPING

#include "T4ActionPlaybackPlayer.h"
#include "T4ActionPlaybackRecorder.h"

#include "World/T4GameWorld.h"

#include "T4EngineInternal.h"

/**
  * #68
 */
FT4ActionPlaybackController::FT4ActionPlaybackController()
	: LayerType(ET4LayerType::Max)
	, bPlayRepeat(false)
	, bPlayerPossessed(true)
	, PlaybackPlayer(nullptr)
	, PlaybackRecorder(nullptr)
{
}

FT4ActionPlaybackController::~FT4ActionPlaybackController()
{
	check(nullptr == PlaybackPlayer);
	check(nullptr == PlaybackRecorder);
}

void FT4ActionPlaybackController::Reset()
{
	DoStopPlaying();
	DoStopRecording();
}

void FT4ActionPlaybackController::Process(float InDeltaTime)
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

void FT4ActionPlaybackController::ProcessOutsideWorld(float InDeltaTime)
{
	if (nullptr != PlaybackPlayer)
	{
		PlaybackPlayer->ProcessPlayOutsideWorld(InDeltaTime);
	}
}

IT4ActionPlaybackPlayer* FT4ActionPlaybackController::GetPlayer() const
{ 
	return static_cast<IT4ActionPlaybackPlayer*>(PlaybackPlayer);
}

IT4ActionPlaybackRecorder* FT4ActionPlaybackController::GetRecorder() const
{ 
	return static_cast<IT4ActionPlaybackRecorder*>(PlaybackRecorder);
}

bool FT4ActionPlaybackController::DoPlay(const FSoftObjectPath& InPlayPath)
{
	if (nullptr != PlaybackPlayer)
	{
		DoStopPlaying();
	}
	check(nullptr == PlaybackPlayer);
	PlaybackPlayer = new FT4ActionPlaybackPlayer(LayerType, bPlayRepeat, bPlayerPossessed);
	check(nullptr != PlaybackPlayer);
	bool bResult = PlaybackPlayer->DoPlay(InPlayPath);
	if (!bResult)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4ActionPlaybackController : Failed to playing. '%s' not found."),
			*(InPlayPath.ToString())
		);
		PlaybackPlayer->OnReset();
		delete PlaybackPlayer;
		PlaybackPlayer = nullptr;
		return false;
	}
	return true;
}

bool FT4ActionPlaybackController::DoPlay(
	const FString& InPlayAssetName,
	const FString& InFolderName
) // /T4Framework/Editor/ActionPlayback/<InFolderName>/<InPlayAssetName>.uasset
{
	if (nullptr != PlaybackPlayer)
	{
		DoStopPlaying();
	}
	check(nullptr == PlaybackPlayer);
	PlaybackPlayer = new FT4ActionPlaybackPlayer(LayerType, bPlayRepeat, bPlayerPossessed);
	check(nullptr != PlaybackPlayer);
	bool bResult = PlaybackPlayer->DoPlay(InPlayAssetName, InFolderName);
	if (!bResult)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4ActionPlaybackController : Failed to playing. '%s' not found."),
			*(InPlayAssetName)
		);
		PlaybackPlayer->OnReset();
		delete PlaybackPlayer;
		PlaybackPlayer = nullptr;
		return false;
	}
	return true;
}

void FT4ActionPlaybackController::DoStopPlaying()
{
	if (nullptr == PlaybackPlayer)
	{
		return;
	}
	PlaybackPlayer->OnStop();
	delete PlaybackPlayer;
	PlaybackPlayer = nullptr;
}

void FT4ActionPlaybackController::SetPlayRepeat(bool bEnable)
{
	bPlayRepeat = bEnable;
	if (nullptr == PlaybackPlayer)
	{
		return;
	}
	PlaybackPlayer->SetRepeat(bEnable);
}

void FT4ActionPlaybackController::SetPlayerPossessed(bool bPossess)
{
	bPlayerPossessed = bPossess;
	if (nullptr == PlaybackPlayer)
	{
		return;
	}
	PlaybackPlayer->SetPlayerPossessed(bPossess);
}

bool FT4ActionPlaybackController::DoRec(const FSoftObjectPath& InRecPath)
{
	if (nullptr != PlaybackRecorder)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4ActionPlaybackController : Failed to recording. Already recorded.")
		);
		return false;
	}
	PlaybackRecorder = new FT4ActionPlaybackRecorder(LayerType);
	check(nullptr != PlaybackRecorder);
	bool bResult = PlaybackRecorder->DoRec(InRecPath);
	if (!bResult)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4ActionPlaybackController : Failed to recording. '%s' not found."),
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
	GameWorld->DoPlaybackSnapshotSave();
	return true;
}

bool FT4ActionPlaybackController::DoRec(
	const FString& InRecAssetName,
	const FString& InFolderName
) // /T4Framework/Editor/ActionPlayback/<InFolderName>/<InRecAssetName>.uasset
{
	if (nullptr != PlaybackRecorder)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4ActionPlaybackController : Failed to recording. Already recorded.")
		);
		return false;
	}
	PlaybackRecorder = new FT4ActionPlaybackRecorder(LayerType);
	check(nullptr != PlaybackRecorder);
	bool bResult = PlaybackRecorder->DoRec(InRecAssetName, InFolderName);
	if (!bResult)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4ActionPlaybackController : Failed to recording. '%s' not found."),
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
	GameWorld->DoPlaybackSnapshotSave();
	return true;
}

void FT4ActionPlaybackController::DoStopRecording()
{
	if (nullptr == PlaybackRecorder)
	{
		return;
	}
	PlaybackRecorder->OnStop();
	delete PlaybackRecorder;
	PlaybackRecorder = nullptr;
}

FT4GameWorld* FT4ActionPlaybackController::GetGameWorld() const
{
	return static_cast<FT4GameWorld*>(T4EngineWorldGet(LayerType));
}

#endif