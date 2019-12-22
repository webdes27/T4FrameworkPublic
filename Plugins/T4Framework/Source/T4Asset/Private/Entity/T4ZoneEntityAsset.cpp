// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Entity/T4ZoneEntityAsset.h"

#include "Serialization/CustomVersion.h"

#include "T4AssetInternal.h"

/**
  * #94
 */
const FGuid FT4ZoneEntityCustomVersion::GUID(0xFBF47AFA, 0x40734283, 0xB0B9E669, 0x22B13E48);

// Register the custom version with core
FCustomVersionRegistration GRegisterT4ZoneEntityCustomVersion(
	FT4ZoneEntityCustomVersion::GUID,
	FT4ZoneEntityCustomVersion::LatestVersion,
	TEXT("T4ZoneEntityVer")
);

UT4ZoneEntityAsset::UT4ZoneEntityAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4ZoneEntityAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(FT4ZoneEntityCustomVersion::GUID); // only changes version if not loading
	const int32 PropEntityVer = Ar.CustomVer(FT4ZoneEntityCustomVersion::GUID);
}

void UT4ZoneEntityAsset::PostLoad()
{
	Super::PostLoad();
}

void UT4ZoneEntityAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);
}