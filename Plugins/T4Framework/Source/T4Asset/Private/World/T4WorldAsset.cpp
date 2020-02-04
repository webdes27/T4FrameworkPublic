// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Classes/World/T4WorldAsset.h"

#include "Serialization/CustomVersion.h"

#include "T4AssetInternal.h"

/**
  * #83
 */
const FGuid FT4WorldCustomVersion::GUID(0xFBF47AFA, 0x40734283, 0xB9B9E421, 0xFEA14E44);

// Register the custom version with core
FCustomVersionRegistration GRegisterT4WorldCustomVersion(
	FT4WorldCustomVersion::GUID,
	FT4WorldCustomVersion::LatestVersion,
	TEXT("T4WorldVer")
);

UT4WorldAsset::UT4WorldAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
#if WITH_EDITORONLY_DATA
	, ThumbnailImage(nullptr)
#endif
{
}

void UT4WorldAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(FT4WorldCustomVersion::GUID); // only changes version if not loading
	const int32 WorldVar = Ar.CustomVer(FT4WorldCustomVersion::GUID);

#if 0
	if (Ar.IsLoading())
	{
		PreviewEntityAsset = TestSettings.EntityAsset;
	}
#endif
}

void UT4WorldAsset::PostLoad()
{
	Super::PostLoad();
}

#if WITH_EDITOR
void UT4WorldAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
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

void UT4WorldAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);
#if WITH_EDITOR

#endif
}
