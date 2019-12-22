// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/Asset/T4AssetManager.h"

/**
  * http://api.unrealengine.com/KOR/Programming/Assets/AsyncLoading/
 */
class FT4AssetHandle;
class FT4AssetManager : public IT4AssetManager
{
public:
	FT4AssetManager();
	~FT4AssetManager();

	bool Initialize() override;
	void Finalize() override;

	void Process(float InDeltaTime) override;

	IT4AssetHandle* RequestAsync(
		const FSoftObjectPath& InPath, 
		FT4LoadDelegate InLoadDelegate
	) override;

private:
	void Reset();

private:
	bool bInitialized;
	int32 InitCount;
	TArray<FT4AssetHandle*> Handles;
};
