// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Entity/T4ItemEntityAsset.h"

#include "T4AssetInternal.h"

/**
  * #24
 */
UT4ItemEntityAsset::UT4ItemEntityAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4ItemEntityAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
}

void UT4ItemEntityAsset::PostLoad()
{
	Super::PostLoad();
}

void UT4ItemEntityAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);
}
