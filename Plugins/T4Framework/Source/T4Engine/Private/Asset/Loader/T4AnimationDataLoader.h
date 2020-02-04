// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/Asset/T4AssetLoader.h"

/**
  *
 */
class FT4BaseAnimControl;
struct FT4EntityCharacterStanceData; // #73
class FT4AnimationLoader
{
public:
	explicit FT4AnimationLoader()
		: bLoadStart(false)
		, bBindComplated(false)
	{
	}
	virtual ~FT4AnimationLoader() {}

	virtual bool Process(FT4BaseAnimControl* InAnimControl)
	{ 
		return false; 
	}

	virtual void Load(
		const FT4EntityCharacterStanceData* InStanceData,
		bool bInSyncLoad,
		const TCHAR* InDebugString
	)
	{
		bLoadStart = true;
	}

	bool IsLoadStarted() const { return bLoadStart; }
	bool IsBinded() const{ return bBindComplated; }

	void SetBinded()
	{
		Reset();
		bBindComplated = true;
	}

protected:
	virtual void Reset() {}

protected:
	bool bLoadStart;
	bool bBindComplated;
};

// #39
class FT4CharacterAnimationDataLoader : public FT4AnimationLoader
{
public:
	explicit FT4CharacterAnimationDataLoader();
	~FT4CharacterAnimationDataLoader();

	bool Process(FT4BaseAnimControl* InAnimControl) override;

	void Load(
		const FT4EntityCharacterStanceData* InStanceData,
		bool bInSyncLoad,
		const TCHAR* InDebugString
	) override;

public:
	bool ProcessPre(); // #111

protected:
	void Reset() override;

private:
	FT4AnimSetAssetLoader AnimSetLoader;
	TMap<FName, FT4AnimMontageLoader*> AnimMontageLoaders; // #69
	TMap<FName, FT4BlendSpaceLoader*> BlendSpaceLoaders;
	bool bSyncLoad;
	bool bAnimSetLoaded;// #111
	bool bAnimMontageLoaded;// #111
	bool bBlendSpaceLoaded;
	FString DebugString;
	
};
