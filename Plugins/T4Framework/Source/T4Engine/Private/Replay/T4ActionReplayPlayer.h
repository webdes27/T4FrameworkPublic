// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionReplayBase.h"
#include "Public/T4Engine.h"

#if !UE_BUILD_SHIPPING

/**
  * #68
 */
class FT4ActionReplayPlayer : public FT4ActionReplayBase, public IT4ActionReplayPlayer
{
public:
	explicit FT4ActionReplayPlayer(ET4LayerType InLayerType, bool bInRepeat, bool bInPlayerPossessed);
	virtual ~FT4ActionReplayPlayer();

	bool IsPaused() const override { return bPaused; }
	void SetPause(bool bPause) override;

	const TCHAR* GetPlayAssetName() const override { return *PlayAssetName; } // #104
	const TCHAR* GetPlayFile() const override { return *PlayFullPath; }

	float GetPlayTimeSec() const override { return FMath::Clamp(PlayTimeSec, 0.0f, MaxPlayTimeSec); }
	float GetMaxPlayTimeSec() const override { return MaxPlayTimeSec; }

public:
	bool DoPlay(const FSoftObjectPath& InPlayPath);
	bool DoPlay(const FString& InPlayAssetName, const FString& InFolderName);

	bool IsPlaying() const { return bPlaying; }

	void SetRepeat(bool bEnable) { bRepeat = bEnable; }
	void SetPlayerPossessed(bool bPossess);

	void ProcessPlayInsideWorld(float InDeltaTime); // #68, #87 : World Tick Pre 에서 호출됨. 일부 Action 은 ProcessOutsideWorld 에서 처리됨!
	void ProcessPlayOutsideWorld(float InDeltaTime); // #87 : World Tick 밖에서 처리해야 할 Action. 예) WorldTravel

protected:
	void Reset() override;
	void Stop() override;

	void CheckRepeat();

private:
	bool Play(const FSoftObjectPath& InPlayPath);

private:
	bool bPlaying;
	bool bPaused;
	bool bRepeat;
	bool bPlayerPossessed;

	FString PlayAssetName;
	FString PlayFullPath;

	uint32 PlayStartItemIndex;

	float PlayTimeSec;
	float MaxPlayTimeSec;

	FT4ObjectID OriginalPlayerObjectID;
	FT4ObjectID PlaybackPlayerObjectID;

	bool bWorldTravelPending; // #87 : 월드 로딩이 끝나면 진입하도록 처리...
	bool bWorldTraveling; // #87 : 월드 로딩이 끝나면 진입하도록 처리...
	int32 WorldTravelPlaybackItemIndex; // #87 : ProcessOutsideWorld 에서 처리됨
	FVector WorldTravelStartLocation; // #87
};

#endif