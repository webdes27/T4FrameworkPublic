// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4Engine.h"

#if !UE_BUILD_SHIPPING

/**
  * #68
 */
class FT4GameWorld;
class FT4ActionPlaybackPlayer;
class FT4ActionPlaybackRecorder;
class FT4ActionPlaybackController : public IT4ActionPlaybackController
{
public:
	explicit FT4ActionPlaybackController();
	virtual ~FT4ActionPlaybackController();

	void SetLayerType(ET4LayerType InLayerType) { LayerType = InLayerType; } // #87

	// IT4ActionPlaybackController
	bool IsPlaying() const override { return (nullptr != PlaybackPlayer) ? true : false; }
	bool IsRecording() const override { return (nullptr != PlaybackRecorder) ? true : false; }

	IT4ActionPlaybackPlayer* GetPlayer() const override;
	IT4ActionPlaybackRecorder* GetRecorder() const override;

	bool DoPlay(const FSoftObjectPath& InPlayPath) override;
	bool DoPlay(const FString& InPlayAssetName, const FString& InFolderName) override; // /T4Framework/Editor/ActionPlayback/<InFolderName>/<InPlayAssetName>.uasset
	void DoStopPlaying() override;

	bool IsPlayRepeat() const override { return bPlayRepeat; }
	void SetPlayRepeat(bool bEnable) override;
	
	bool IsPlayerPossessed() const override { return bPlayerPossessed; }
	void SetPlayerPossessed(bool bPossess) override;

	bool DoRec(const FSoftObjectPath& InRecPath) override;
	bool DoRec(const FString& InRecAssetName, const FString& InFolderName) override; // /T4Framework/Editor/ActionPlayback/<InFolderName>/<InRecAssetName>.uasset
	void DoStopRecording() override;

public:
	void Reset();

	void Process(float InDeltaTime); // #87 : Input 또는 World Tick 에서 호출됨
	void ProcessOutsideWorld(float InDeltaTime); // #87 : World Tick 밖에서 처리해야 할 Action. 예) WorldTravel

private:
	FT4GameWorld* GetGameWorld() const;

private:
	ET4LayerType LayerType;
	bool bPlayRepeat;
	bool bPlayerPossessed;
	FT4ActionPlaybackPlayer* PlaybackPlayer;
	FT4ActionPlaybackRecorder* PlaybackRecorder;
};

#endif