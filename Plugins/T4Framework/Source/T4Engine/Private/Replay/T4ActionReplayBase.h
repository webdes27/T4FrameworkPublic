// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Classes/Replay/T4ActionReplayAsset.h"

#if !UE_BUILD_SHIPPING

/**
  * #68
 */
class FT4GameWorld;
class IT4GameObject;
class FT4ActionReplayBase
{
public:
	explicit FT4ActionReplayBase(ET4LayerType InLayerType);
	virtual ~FT4ActionReplayBase();

	void OnReset();

	void OnStop();

protected:
	virtual void Reset() {}
	virtual void Stop() {}

#if 0 // TODO : Saved 폴더에 저장해야 할 시점에 처리. 저장까지는 문제없는데, 로드에서 문제가 있음
	// #68 : Saved/T4Replay/<InRecAssetName>.dat
	bool DoLoad(const FString& InLoadFileName);
	bool OnSave(const FString& InSaveFileName);
#endif

	void ResetAsset();

	bool NewAsset();
	bool LoadAsset(const FSoftObjectPath& InLoadPath);
	bool SaveAsset(const FString& InAssetName, const FString& InPackagePath);

	FT4ActionReplayData& GetReplayData();

	FT4GameWorld* GetGameWorld() const;
	IT4GameObject* FindGameObject(const FT4ObjectID& InObjectID) const;

private:
	UT4ActionReplayAsset* CreateAsset(
		const FString& InAssetName,
		const FString& InPackagePath
	);

protected:
	ET4LayerType LayerType;
	UT4ActionReplayAsset* ActionReplayAsset;
};

#endif