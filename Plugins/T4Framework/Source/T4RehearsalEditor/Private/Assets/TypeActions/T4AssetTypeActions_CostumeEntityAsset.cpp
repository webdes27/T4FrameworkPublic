// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetTypeActions_CostumeEntityAsset.h"

#include "T4Asset/Public/T4AssetUtils.h" // #42, #62
#include "T4Asset/Classes/Entity/T4CostumeEntityAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #35
 */
FColor FT4AssetTypeActions_CostumeEntityAsset::GetTypeColor() const
{
	return FLinearColor(1, 1, 1).ToFColor(true);
}

void FT4AssetTypeActions_CostumeEntityAsset::OpenAssetEditor(
	const TArray<UObject*>& InObjects, 
	TSharedPtr<IToolkitHost> InEditWithinLevelEditor
)
{
	EToolkitMode::Type Mode = InEditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UT4CostumeEntityAsset* ItemEntityAsset = Cast<UT4CostumeEntityAsset>(*ObjIt);
		if (nullptr != ItemEntityAsset)
		{
			// #42
			GetOnT4EntityAssetTypeAction().ExecuteIfBound(
				ItemEntityAsset,
				InEditWithinLevelEditor
			);
		}
	}
}

UClass* FT4AssetTypeActions_CostumeEntityAsset::GetSupportedClass() const
{
	return UT4CostumeEntityAsset::StaticClass();
}

uint32 FT4AssetTypeActions_CostumeEntityAsset::GetCategories()
{
	return GetT4AssetCategory();
}