// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

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
	, bOverlayAnimMontageAutoGen(true) // #69
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
}

void UT4AnimSetAsset::PostLoad()
{
	Super::PostLoad();

	const int32 AnimSetVar = GetLinkerCustomVersion(FT4AnimSetCustomVersion::GUID);

	if (AnimSetVar < FT4AnimSetCustomVersion::InitializeVer)
	{
	}
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
