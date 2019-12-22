// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetHandle.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

#include "T4EngineInternal.h"

/**
  * http://api.unrealengine.com/KOR/Programming/Assets/AsyncLoading/
 */
FT4AssetHandle::FT4AssetHandle()
	: bAutoDestroy(false)
	, bLoadComplated(false)
	, bLoadFailed(false)
{
}

FT4AssetHandle::~FT4AssetHandle()
{
	Reset();
}

bool FT4AssetHandle::OnCreate(const FSoftObjectPath& InLoadPath, FT4LoadDelegate InLoadDelegate)
{
	LoadPath = InLoadPath;
	LoadDelegate = InLoadDelegate;
	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	StreamableHandle = StreamableManager.RequestAsyncLoad(
		InLoadPath, 
		FStreamableDelegate::CreateRaw(this, &FT4AssetHandle::OnLoadCompleate)
	);
	return true;
}

void FT4AssetHandle::OnDestroy()
{
	Reset();
	bAutoDestroy = true;
}

bool FT4AssetHandle::IsLoadFailed() const
{
	if (!StreamableHandle.IsValid())
	{
		return true;
	}
	if (StreamableHandle->WasCanceled())
	{
		return true;
	}
	if (1.0f > StreamableHandle->GetProgress())
	{
		return false; // 아직 로딩 전!
	}
	// Failed flag 가 명시적으로 없어서 체크, Is 류가 먼저 불리며 분기 처리를 하기 때문
	if (nullptr == StreamableHandle->GetLoadedAsset())
	{
		return true;
	}
	return false;
}

bool FT4AssetHandle::GetLoadProgress() const
{
	if (!StreamableHandle.IsValid())
	{
		return 0.0f;
	}
	return StreamableHandle->GetProgress();
}

UObject* FT4AssetHandle::GetLoadedAsset() const
{
	check(StreamableHandle.IsValid());
	UObject* LoadAsset = StreamableHandle->GetLoadedAsset();
	return LoadAsset;
}

void FT4AssetHandle::OnLoadCompleate()
{
	if (!StreamableHandle.IsValid())
	{
		bLoadFailed = true;
		return;
	}
	LoadedAsset = StreamableHandle->GetLoadedAsset();
	bLoadComplated = true;
	if (LoadDelegate.IsBound())
	{
		LoadDelegate.ExecuteIfBound(LoadedAsset.Get());
	}
}

void FT4AssetHandle::Reset()
{
	LoadedAsset.Reset();
	if (!StreamableHandle.IsValid())
	{
		return;
	}
	if (!bLoadComplated)
	{
		// WARN : Cancle 을 불러줘야 OnLoadCompleate delegate 가 불리지 않는다!
		StreamableHandle->CancelHandle();
	}
	StreamableHandle->ReleaseHandle();
	StreamableHandle = nullptr;
}
