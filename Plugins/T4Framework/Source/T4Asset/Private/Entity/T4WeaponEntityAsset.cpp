// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Entity/T4WeaponEntityAsset.h"

#include "Serialization/CustomVersion.h"

#include "T4AssetInternal.h"

/**
  * #35
 */
const FGuid FT4WeaponEntityCustomVersion::GUID(0xFBF47AFA, 0x40734283, 0xB9B9E658, 0x43A32D32);

// Register the custom version with core
FCustomVersionRegistration GRegisterT4WeaponEntityCustomVersion(
	FT4WeaponEntityCustomVersion::GUID,
	FT4WeaponEntityCustomVersion::LatestVersion,
	TEXT("T4WeaponEntityVer")
);

UT4WeaponEntityAsset::UT4WeaponEntityAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4WeaponEntityAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(FT4WeaponEntityCustomVersion::GUID); // only changes version if not loading
	const int32 ItemEntityVer = Ar.CustomVer(FT4WeaponEntityCustomVersion::GUID);
}

void UT4WeaponEntityAsset::PostLoad()
{
	Super::PostLoad();
}

void UT4WeaponEntityAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);
}