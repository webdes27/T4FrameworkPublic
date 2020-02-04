// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Classes/Entity/T4CostumeEntityAsset.h"

#include "Serialization/CustomVersion.h"

#include "T4AssetInternal.h"

/**
  * #35
 */
const FGuid FT4CostumeEntityCustomVersion::GUID(0xFBF47AFA, 0x40734283, 0xB9B9E658, 0x33A22D43);

// Register the custom version with core
FCustomVersionRegistration GRegisterT4CosttumeEntityCustomVersion(
	FT4CostumeEntityCustomVersion::GUID,
	FT4CostumeEntityCustomVersion::LatestVersion,
	TEXT("T4CostumeEntityVer")
);

UT4CostumeEntityAsset::UT4CostumeEntityAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4CostumeEntityAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(FT4CostumeEntityCustomVersion::GUID); // only changes version if not loading
	const int32 ItemEntityVer = Ar.CustomVer(FT4CostumeEntityCustomVersion::GUID);
}

void UT4CostumeEntityAsset::PostLoad()
{
	Super::PostLoad();
}

void UT4CostumeEntityAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);
#if WITH_EDITOR
	if (!MeshData.SkeletonAsset.IsNull()) // #71
	{
		OutTags.Add(
			FAssetRegistryTag(
				TEXT("SkeletonAsset"),
				MeshData.SkeletonAsset.ToString(),
				FAssetRegistryTag::TT_Hidden
			)
		);
	}
	if (MeshData.CompositePartName != NAME_None) // #72
	{
		OutTags.Add(
			FAssetRegistryTag(
				TEXT("CompositePartName"),
				MeshData.CompositePartName.ToString(),
				FAssetRegistryTag::TT_Hidden
			)
		);
	}
#endif
}