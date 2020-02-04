// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4AnimationDataLoader.h"

#include "Object/Animation/AnimInstance/T4BaseAnimInstance.h"
#include "Object/Animation/T4BaseAnimControl.h"

#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h"
#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h" // #39
#include "T4Asset/Public/Entity/T4Entity.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4CharacterAnimationDataLoader::FT4CharacterAnimationDataLoader()
	: bSyncLoad(false)
	, bAnimSetLoaded(false) // #111
	, bAnimMontageLoaded(false) // #111
	, bBlendSpaceLoaded(false)
{
}

FT4CharacterAnimationDataLoader::~FT4CharacterAnimationDataLoader()
{
	Reset();
}

// #39
void FT4CharacterAnimationDataLoader::Reset()
{
	AnimSetLoader.Reset();
	for (TMap<FName, FT4AnimMontageLoader*>::TIterator It(AnimMontageLoaders); It; ++It) // #69
	{
		delete It->Value;
	}
	AnimMontageLoaders.Empty();
	for (TMap<FName, FT4BlendSpaceLoader*>::TIterator It(BlendSpaceLoaders); It; ++It)
	{
		delete It->Value;
	}
	BlendSpaceLoaders.Empty();
	bAnimSetLoaded = false; // #111
	bAnimMontageLoaded = false; // #111
	bBlendSpaceLoaded = false;
}

bool FT4CharacterAnimationDataLoader::ProcessPre() // #111
{
	if (!bAnimSetLoaded)
	{
		if (!AnimSetLoader.IsLoadCompleted())
		{
			return false;
		}
		UT4AnimSetAsset* AnimSetAsset = AnimSetLoader.GetAnimSetAsset();
		check(nullptr != AnimSetAsset);
		{
			if (!AnimSetAsset->SkillAnimMontageAsset.IsNull()) // #69
			{
				FT4AnimMontageLoader* NewAnimMontageLoader = new FT4AnimMontageLoader;
				check(nullptr != NewAnimMontageLoader);
				NewAnimMontageLoader->Load(AnimSetAsset->SkillAnimMontageAsset.ToSoftObjectPath(), bSyncLoad, *DebugString);
				AnimMontageLoaders.Add(T4Const_SkillAnimMontageName, NewAnimMontageLoader);
			}
			if (!AnimSetAsset->OverlayAnimMontageAsset.IsNull()) // #69
			{
				FT4AnimMontageLoader* NewAnimMontageLoader = new FT4AnimMontageLoader;
				check(nullptr != NewAnimMontageLoader);
				NewAnimMontageLoader->Load(AnimSetAsset->OverlayAnimMontageAsset.ToSoftObjectPath(), bSyncLoad, *DebugString);
				AnimMontageLoaders.Add(T4Const_OverlayAnimMontageName, NewAnimMontageLoader);
			}
			if (!AnimSetAsset->DefaultAnimMontageAsset.IsNull()) // #69
			{
				FT4AnimMontageLoader* NewAnimMontageLoader = new FT4AnimMontageLoader;
				check(nullptr != NewAnimMontageLoader);
				NewAnimMontageLoader->Load(AnimSetAsset->DefaultAnimMontageAsset.ToSoftObjectPath(), bSyncLoad, *DebugString);
				AnimMontageLoaders.Add(T4Const_DefaultAnimMontageName, NewAnimMontageLoader);
			}
		}
		{
			for (TArray<FT4AnimSetBlendSpaceInfo>::TConstIterator It(AnimSetAsset->BlendSpaceArray); It; ++It)
			{
				const FT4AnimSetBlendSpaceInfo& BlendSpaceInfo = *It;
				FT4BlendSpaceLoader* NewBlendSpaceLoader = new FT4BlendSpaceLoader;
				check(nullptr != NewBlendSpaceLoader);
				NewBlendSpaceLoader->Load(BlendSpaceInfo.BlendSpaceAsset.ToSoftObjectPath(), bSyncLoad, *DebugString);
				BlendSpaceLoaders.Add(BlendSpaceInfo.Name, NewBlendSpaceLoader);
			}
		}
		bAnimSetLoaded = true;
	}
	if (!bAnimMontageLoaded)
	{
		for (TMap<FName, FT4AnimMontageLoader*>::TIterator It(AnimMontageLoaders); It; ++It) // #69
		{
			FT4AnimMontageLoader* AnimMontageLoader = It->Value;
			check(nullptr != AnimMontageLoader);
			if (!AnimMontageLoader->IsLoadCompleted())
			{
				return false;
			}
		}
		bAnimMontageLoaded = true;
	}
	if (!bBlendSpaceLoaded)
	{
		for (TMap<FName, FT4BlendSpaceLoader*>::TIterator It(BlendSpaceLoaders); It; ++It)
		{
			FT4BlendSpaceLoader* BlendSpaceLoader = It->Value;
			check(nullptr != BlendSpaceLoader);
			if (!BlendSpaceLoader->IsLoadCompleted())
			{
				return false;
			}
		}
		bBlendSpaceLoaded = true;
	}
	return true;
}

bool FT4CharacterAnimationDataLoader::Process(
	FT4BaseAnimControl* InAnimControl
)
{
	check(nullptr != InAnimControl);
	if (!ProcessPre())
	{
		return false;
	}
	UT4BaseAnimInstance* AnimInstance = InAnimControl->GetAnimInstance();
	check(nullptr != AnimInstance);
	AnimInstance->OnReset(); // #73 : AnimSet 로딩이 끝나면 Reset 후 일괄 Set 처리해준다.
	{
		check(!AnimSetLoader.IsBinded());
		check(AnimSetLoader.IsLoadCompleted());
		UT4AnimSetAsset* AnimSetAsset = AnimSetLoader.GetAnimSetAsset();
		check(nullptr != AnimSetAsset);
		InAnimControl->SetAnimSetAsset(AnimSetAsset);
		AnimSetLoader.SetBinded();
	}
	{
		for (TMap<FName, FT4BlendSpaceLoader*>::TIterator It(BlendSpaceLoaders); It; ++It)
		{
			FT4BlendSpaceLoader* BlendSpaceLoader = It->Value;
			check(nullptr != BlendSpaceLoader);
			check(!BlendSpaceLoader->IsBinded());
			check(BlendSpaceLoader->IsLoadCompleted());
			UBlendSpaceBase* BlendSpace = BlendSpaceLoader->GetBlendSpace();
			if (nullptr != BlendSpace)
			{
				AnimInstance->AddBlendSpace(It->Key, BlendSpace);
			}
			BlendSpaceLoader->SetBinded();
		}
	}
	{
		for (TMap<FName, FT4AnimMontageLoader*>::TIterator It(AnimMontageLoaders); It; ++It) // #69
		{
			FT4AnimMontageLoader* AnimMontageLoader = It->Value;
			check(nullptr != AnimMontageLoader);
			check(!AnimMontageLoader->IsBinded());
			check(AnimMontageLoader->IsLoadCompleted());
			UAnimMontage* AnimMontage = AnimMontageLoader->GetAnimMontage();
			if (nullptr != AnimMontage)
			{
				AnimInstance->AddAnimMontage(It->Key, AnimMontage);
			}
			AnimMontageLoader->SetBinded();
		}
	}
	return true;
}

void FT4CharacterAnimationDataLoader::Load(
	const FT4EntityCharacterStanceData* InStanceData,
	bool bInSyncLoad,
	const TCHAR* InDebugString
)
{
	check(nullptr != InStanceData);
	AnimSetLoader.Load(InStanceData->AnimSetAsset.ToString(), bInSyncLoad, InDebugString);
	bSyncLoad = bInSyncLoad;
	DebugString = InDebugString;
	bLoadStart = true;
	bAnimSetLoaded = false; // #111
	bAnimMontageLoaded = false; // #111
	bBlendSpaceLoaded = false;
}
