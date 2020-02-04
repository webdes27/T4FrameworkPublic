// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Classes/Entity/T4MapEntityAsset.h"

#include "Serialization/CustomVersion.h"

#include "T4AssetInternal.h"

/**
  * #35
 */
const FGuid FT4MapEntityCustomVersion::GUID(0xFBF47AFA, 0x40734283, 0xB9B9E658, 0x11A42D42);

// Register the custom version with core
FCustomVersionRegistration GRegisterT4MapEntityCustomVersion(
	FT4MapEntityCustomVersion::GUID,
	FT4MapEntityCustomVersion::LatestVersion,
	TEXT("T4MapEntityVer")
);

UT4MapEntityAsset::UT4MapEntityAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4MapEntityAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(FT4MapEntityCustomVersion::GUID); // only changes version if not loading
	const int32 MapEntityVer = Ar.CustomVer(FT4MapEntityCustomVersion::GUID);
}

void UT4MapEntityAsset::PostLoad()
{
	Super::PostLoad();
}

void UT4MapEntityAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);
}