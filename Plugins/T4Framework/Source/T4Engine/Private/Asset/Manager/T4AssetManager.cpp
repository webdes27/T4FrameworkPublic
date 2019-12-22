// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetManager.h"
#include "T4AssetHandle.h"

#include "T4EngineInternal.h"

/**
  * http://api.unrealengine.com/KOR/Programming/Assets/AsyncLoading/
 */
FT4AssetManager::FT4AssetManager()
	: bInitialized(false)
	, InitCount(0)
{
}

FT4AssetManager::~FT4AssetManager()
{
}

bool FT4AssetManager::Initialize()
{
	InitCount++;
	if (bInitialized)
	{
		return true;
	}
	check(nullptr != GEngine);
	check(nullptr != GEngine->AssetManager);
	check(0 == Handles.Num());
	bInitialized = true;
	return true;
}

void FT4AssetManager::Finalize()
{
	if (!bInitialized)
	{
		return;
	}
	InitCount--;
	if (0 < InitCount)
	{
		return;
	}
	Reset();
	bInitialized = false;
	check(0 == Handles.Num());
}

void FT4AssetManager::Process(float InDeltaTime)
{
	if (!bInitialized)
	{
		return;
	}
	for (auto Iter = Handles.CreateIterator(); Iter; ++Iter)
	{
		FT4AssetHandle* Handle = *Iter;
		if (Handle->IsAutoDestroy())
		{
			Iter.RemoveCurrent();
			delete Handle;
		}
	}
}

void FT4AssetManager::Reset()
{
	for (FT4AssetHandle* Handle : Handles)
	{
		delete Handle;
	}
	Handles.Empty();
}

IT4AssetHandle* FT4AssetManager::RequestAsync(
	const FSoftObjectPath& InPath,
	FT4LoadDelegate InLoadDelegate
)
{
	FT4AssetHandle* NewHandle = new FT4AssetHandle;
	NewHandle->OnCreate(InPath, InLoadDelegate);
	Handles.Add(NewHandle);
	return NewHandle;
}

static FT4AssetManager GT4AssetLoader;
IT4AssetManager* T4AssetManagerGet()
{
	return &GT4AssetLoader;
}