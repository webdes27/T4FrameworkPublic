// Copyright 2019 SoonBo Noh. All Rights Reserved.

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
}

bool FT4CharacterAnimationDataLoader::Process(
	FT4BaseAnimControl* InAnimControl
)
{
	check(nullptr != InAnimControl);

	UT4BaseAnimInstance* AnimInstance = InAnimControl->GetAnimInstance();
	check(nullptr != AnimInstance);

	if (!AnimSetLoader.IsBinded())
	{
		if (AnimSetLoader.IsLoadCompleted())
		{
			UT4AnimSetAsset* AnimSetAsset = AnimSetLoader.GetAnimSetAsset();
			check(nullptr != AnimSetAsset);
			{
				if (!AnimSetAsset->SkillAnimMontageAsset.IsNull()) // #69
				{
					FT4AnimMontageLoader* NewAnimMontageLoader = new FT4AnimMontageLoader;
					check(nullptr != NewAnimMontageLoader);
					NewAnimMontageLoader->Load(AnimSetAsset->SkillAnimMontageAsset.ToSoftObjectPath(), bSyncLoad, *DebugString);
					AnimMontageLoaders.Add(T4AnimSetAnimMontageSkillName, NewAnimMontageLoader);
				}
				if (!AnimSetAsset->AdditiveAnimMontageAsset.IsNull()) // #69
				{
					FT4AnimMontageLoader* NewAnimMontageLoader = new FT4AnimMontageLoader;
					check(nullptr != NewAnimMontageLoader);
					NewAnimMontageLoader->Load(AnimSetAsset->AdditiveAnimMontageAsset.ToSoftObjectPath(), bSyncLoad, *DebugString);
					AnimMontageLoaders.Add(T4AnimSetAnimMontageAdditiveName, NewAnimMontageLoader);
				}
				if (!AnimSetAsset->DefaultAnimMontageAsset.IsNull()) // #69
				{
					FT4AnimMontageLoader* NewAnimMontageLoader = new FT4AnimMontageLoader;
					check(nullptr != NewAnimMontageLoader);
					NewAnimMontageLoader->Load(AnimSetAsset->DefaultAnimMontageAsset.ToSoftObjectPath(), bSyncLoad, *DebugString);
					AnimMontageLoaders.Add(T4AnimSetAnimMontageDefaultName, NewAnimMontageLoader);
				}
			}
			{
				for (TArray<FT4BlendSpaceInfo>::TConstIterator It(AnimSetAsset->BlendSpaceArray); It; ++It)
				{
					const FT4BlendSpaceInfo& BlendSpaceInfo = *It;
					FT4BlendSpaceLoader* NewBlendSpaceLoader = new FT4BlendSpaceLoader;
					check(nullptr != NewBlendSpaceLoader);
					NewBlendSpaceLoader->Load(BlendSpaceInfo.BlendSpaceAsset.ToSoftObjectPath(), bSyncLoad, *DebugString);
					BlendSpaceLoaders.Add(BlendSpaceInfo.Name, NewBlendSpaceLoader);
				}
			}
			InAnimControl->SetAnimSetAsset(AnimSetAsset);
			AnimSetLoader.SetBinded();
		}
		else
		{
			return false;
		}
	}

	if (!bBlendSpaceLoaded)
	{
		// #73 : BlendSpace 로딩이 모두 끝난 후 기존 AnimInstance 를 Clear 하도록 한다.
		for (TMap<FName, FT4BlendSpaceLoader*>::TIterator It(BlendSpaceLoaders); It; ++It)
		{
			FT4BlendSpaceLoader* BlendSpaceLoader = It->Value;
			check(nullptr != BlendSpaceLoader);
			if (!BlendSpaceLoader->IsBinded())
			{
				if (!BlendSpaceLoader->IsLoadCompleted())
				{
					return false;
				}
			}
		}

		AnimInstance->OnReset(); // #73 : AnimSet 로딩이 끝난 후 AnimInstance 를 Reset 해준다. (임시처리)

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
		bBlendSpaceLoaded = true;
	}

	bool bAnimMontangeLoaded = true;
	for (TMap<FName, FT4AnimMontageLoader*>::TIterator It(AnimMontageLoaders); It; ++It) // #69
	{
		FT4AnimMontageLoader* AnimMontageLoader = It->Value;
		check(nullptr != AnimMontageLoader);
		if (!AnimMontageLoader->IsBinded())
		{
			if (AnimMontageLoader->IsLoadCompleted())
			{
				UAnimMontage* AnimMontage = AnimMontageLoader->GetAnimMontage();
				if (nullptr != AnimMontage)
				{
					AnimInstance->AddAnimMontage(It->Key, AnimMontage);
				}
				AnimMontageLoader->SetBinded();
			}
			else
			{
				bAnimMontangeLoaded = false;
			}
		}
	}
	return bAnimMontangeLoaded;
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
}
