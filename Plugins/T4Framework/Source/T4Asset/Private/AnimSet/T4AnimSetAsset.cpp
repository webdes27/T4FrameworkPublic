// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/AnimSet/T4AnimSetAsset.h"

#include "Serialization/CustomVersion.h"

#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"

#include "T4AssetInternal.h"

/**
  * #39
 */
const FGuid FT4AnimSetCustomVersion::GUID(0xFBF47AFA, 0x40734283, 0xB9B9E658, 0xFEB22491);

// Register the custom version with core
FCustomVersionRegistration GRegisterT4AnimSetCustomVersion(
	FT4AnimSetCustomVersion::GUID,
	FT4AnimSetCustomVersion::LatestVersion,
	TEXT("T4AnimSetVer")
);

UT4AnimSetAsset::UT4AnimSetAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bSkillAnimMontageAutoGen(true) // #69
	, bAdditiveAnimMontageAutoGen(true) // #69
	, bDefaultAnimMontageAutoGen(true) // #69
#if WITH_EDITORONLY_DATA
	, ThumbnailImage(nullptr)
#endif
{
}

void UT4AnimSetAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(FT4AnimSetCustomVersion::GUID); // only changes version if not loading
	const int32 ContiVar = Ar.CustomVer(FT4AnimSetCustomVersion::GUID);

#if 0 // #69 : 마이그레이션 과정에서 중복 추가가되어 복구 처리
	if (Ar.IsLoading())
	{
		TArray<FT4AnimSequenceInfo> SwapSequenceInfos;
		for (TArray<FT4AnimSequenceInfo>::TIterator It(SkillAnimSequenceArray); It; ++It)
		{
			FT4AnimSequenceInfo& Info = *It;
			if (nullptr == SwapSequenceInfos.FindByKey(Info.Name))
			{
				SwapSequenceInfos.Add(Info);
			}
		}
		SkillAnimSequenceArray.Empty();
		SkillAnimSequenceArray = SwapSequenceInfos;
		SwapSequenceInfos.Empty();

		for (TArray<FT4AnimSequenceInfo>::TIterator It(AdditiveAnimSequenceArray); It; ++It)
		{
			FT4AnimSequenceInfo& Info = *It;
			if (nullptr == SwapSequenceInfos.FindByKey(Info.Name))
			{
				SwapSequenceInfos.Add(Info);
			}
		}
		AdditiveAnimSequenceArray.Empty();
		AdditiveAnimSequenceArray = SwapSequenceInfos;
		SwapSequenceInfos.Empty();

		for (TArray<FT4AnimSequenceInfo>::TIterator It(DefaultAnimSequenceArray); It; ++It)
		{
			FT4AnimSequenceInfo& Info = *It;
			if (nullptr == SwapSequenceInfos.FindByKey(Info.Name))
			{
				SwapSequenceInfos.Add(Info);
			}
		}
		DefaultAnimSequenceArray.Empty();
		DefaultAnimSequenceArray = SwapSequenceInfos;
		SwapSequenceInfos.Empty();

		TArray<FT4BlendSpaceInfo> SwapBlendSpaceInfos;
		for (TArray<FT4BlendSpaceInfo>::TIterator It(BlendSpaceArray); It; ++It)
		{
			FT4BlendSpaceInfo& Info = *It;
			if (nullptr == SwapBlendSpaceInfos.FindByKey(Info.Name))
			{
				SwapBlendSpaceInfos.Add(Info);
			}
		}
		BlendSpaceArray.Empty();
		BlendSpaceArray = SwapBlendSpaceInfos;
		SwapBlendSpaceInfos.Empty();

		MarkPackageDirty();
	}
#endif
}

void UT4AnimSetAsset::PostLoad()
{
	Super::PostLoad();
}

#if WITH_EDITOR
void UT4AnimSetAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (nullptr == PropertyChangedEvent.Property) // #77
	{
		return;
	}
	if (PropertyChangedEvent.Property->HasAnyPropertyFlags(CPF_Transient))
	{
		return; // #71 : Transient Property 는 Changed 이벤트를 보내지 않도록 조치
	}
	OnPropertiesChanged().Broadcast();
}
#endif

void UT4AnimSetAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);
#if WITH_EDITOR
	if (!SkeletonAsset.IsNull()) // #73
	{
		OutTags.Add(
			FAssetRegistryTag(
				TEXT("SkeletonAsset"),
				SkeletonAsset.ToString(),
				FAssetRegistryTag::TT_Hidden
			)
		);
	}
#endif
}
