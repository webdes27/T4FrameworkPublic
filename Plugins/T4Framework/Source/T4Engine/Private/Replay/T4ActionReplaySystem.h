// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4Engine.h"

#if !UE_BUILD_SHIPPING

/**
  * #68
 */
class FT4GameWorld;
class FT4ActionReplayPlayer;
class FT4ActionReplayRecorder;
class FT4ActionReplaySystem : public IT4ActionReplaySystem
{
public:
	explicit FT4ActionReplaySystem();
	virtual ~FT4ActionReplaySystem();

	void SetLayerType(ET4LayerType InLayerType) { LayerType = InLayerType; } // #87

	// IT4ActionReplaySystem
	bool IsPlaying() const override { return (nullptr != PlaybackPlayer) ? true : false; }
	bool IsRecording() const override { return (nullptr != PlaybackRecorder) ? true : false; }

	IT4ActionReplayPlayer* GetPlayer() const override;
	IT4ActionReplayRecorder* GetRecorder() const override;

	bool DoPlay(const FSoftObjectPath& InPlayPath) override;
	bool DoPlay(const FString& InPlayAssetName, const FString& InFolderName) override; // /T4Framework/Editor/ActionReplay/<InFolderName>/<InPlayAssetName>.uasset
	void DoStopPlaying() override;

	bool IsPlayRepeat() const override { return bPlayRepeat; }
	void SetPlayRepeat(bool bEnable) override;
	
	bool IsPlayerPossessed() const override { return bPlayerPossessed; }
	void SetPlayerPossessed(bool bPossess) override;

	bool DoRec(const FSoftObjectPath& InRecPath) override;
	bool DoRec(const FString& InRecAssetName, const FString& InFolderName) override; // /T4Framework/Editor/ActionReplay/<InFolderName>/<InRecAssetName>.uasset
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
	FT4ActionReplayPlayer* PlaybackPlayer;
	FT4ActionReplayRecorder* PlaybackRecorder;
};

#endif