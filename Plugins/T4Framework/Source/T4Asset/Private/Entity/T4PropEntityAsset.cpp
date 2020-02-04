// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Classes/Entity/T4PropEntityAsset.h"

#include "Serialization/CustomVersion.h"

#include "T4AssetInternal.h"

/**
  * #35
 */
const FGuid FT4PropEntityCustomVersion::GUID(0xFBF47AFA, 0x40734283, 0xB9B9E658, 0x22B12E33);

// Register the custom version with core
FCustomVersionRegistration GRegisterT4PropEntityCustomVersion(
	FT4PropEntityCustomVersion::GUID,
	FT4PropEntityCustomVersion::LatestVersion,
	TEXT("T4PropEntityVer")
);

UT4PropEntityAsset::UT4PropEntityAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MeshType(ET4EntityPropMeshType::None)
{
}

void UT4PropEntityAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(FT4PropEntityCustomVersion::GUID); // only changes version if not loading
	const int32 PropEntityVer = Ar.CustomVer(FT4PropEntityCustomVersion::GUID);
}

void UT4PropEntityAsset::PostLoad()
{
	Super::PostLoad();
}

void UT4PropEntityAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);
}