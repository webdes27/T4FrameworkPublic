// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Classes/World/T4EnvironmentAsset.h"

#include "Serialization/CustomVersion.h"

#include "T4AssetInternal.h"

/**
  * #90
 */
const FGuid FT4EnvironmentCustomVersion::GUID(0xFBF47AFA, 0x40734283, 0xB9B8E522, 0xFEA34E26);

// Register the custom version with core
FCustomVersionRegistration GRegisterT4WorldEnvironmentCustomVersion(
	FT4EnvironmentCustomVersion::GUID,
	FT4EnvironmentCustomVersion::LatestVersion,
	TEXT("T4WorldEnvironmentVer")
);

UT4EnvironmentAsset::UT4EnvironmentAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4EnvironmentAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(FT4EnvironmentCustomVersion::GUID); // only changes version if not loading
	const int32 WorldEnvironmentVar = Ar.CustomVer(FT4EnvironmentCustomVersion::GUID);

#if 0
	// #93
	if (Ar.IsLoading())
	{
		for (TMap<FName, FT4EnvTimeTagData>::TIterator It(TimeOfDaySetData.TimeOfDayMap); It; ++It) // #69
		{
			TimeTagSetData.TimeTagMap.Add(It->Key, It->Value);
		}
	}
#endif
}

void UT4EnvironmentAsset::PostLoad()
{
	Super::PostLoad();
}

#if WITH_EDITOR
void UT4EnvironmentAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
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

void UT4EnvironmentAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);
#if WITH_EDITOR

#endif
}
