// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/Asset/T4AssetManager.h"

/**
  * http://api.unrealengine.com/KOR/Programming/Assets/AsyncLoading/
 */
struct FStreamableHandle;
class FT4AssetHandle : public IT4AssetHandle
{
public:
	explicit FT4AssetHandle();
	~FT4AssetHandle();

	void OnDestroy() override;

	bool IsLoadFailed() const override;
	bool IsLoadCompleted() const override { return bLoadComplated; }
	
	bool GetLoadProgress() const override;

	UObject* GetLoadedAsset() const override;

	const FSoftObjectPath& GetObjectPath() const { return LoadPath; }

public:
	bool OnCreate(const FSoftObjectPath& InLoadPath, FT4LoadDelegate InLoadDelegate);

	bool IsAutoDestroy() const { return bAutoDestroy; }

protected:
	void OnLoadCompleate();

private:
	void Reset();

private:
	bool bAutoDestroy;
	bool bLoadComplated;
	bool bLoadFailed;

	FSoftObjectPath LoadPath;
	FT4LoadDelegate LoadDelegate;

	TWeakObjectPtr<UObject> LoadedAsset;
	TSharedPtr<FStreamableHandle> StreamableHandle;
};
