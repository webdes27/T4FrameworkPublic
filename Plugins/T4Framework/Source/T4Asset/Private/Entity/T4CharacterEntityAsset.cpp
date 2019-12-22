// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Entity/T4CharacterEntityAsset.h"

#include "Serialization/CustomVersion.h"

#include "T4AssetInternal.h"

/**
  * #35
 */
const FGuid FT4CharacterEntityCustomVersion::GUID(0xFBF47AFA, 0x40734283, 0xB9B9E658, 0x44A13D34);

// Register the custom version with core
FCustomVersionRegistration GRegisterT4CharacterEntityCustomVersion(
	FT4CharacterEntityCustomVersion::GUID,
	FT4CharacterEntityCustomVersion::LatestVersion,
	TEXT("T4CharacterEntityVer")
);

UT4CharacterEntityAsset::UT4CharacterEntityAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MeshType(ET4EntityCharacterMeshType::None)
{
}

void UT4CharacterEntityAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(FT4CharacterEntityCustomVersion::GUID); // only changes version if not loading
	const int32 CharacterEntityVer = Ar.CustomVer(FT4CharacterEntityCustomVersion::GUID);
}

void UT4CharacterEntityAsset::PostLoad()
{
	Super::PostLoad();
}

void UT4CharacterEntityAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);
}
