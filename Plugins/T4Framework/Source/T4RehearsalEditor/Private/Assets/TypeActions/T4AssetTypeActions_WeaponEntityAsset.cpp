// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetTypeActions_WeaponEntityAsset.h"

#include "T4Asset/Public/T4AssetUtils.h" // #42, #62
#include "T4Asset/Classes/Entity/T4WeaponEntityAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #35
 */
 FColor FT4AssetTypeActions_WeaponEntityAsset::GetTypeColor() const
{
	 return FLinearColor(1, 1, 1).ToFColor(true);
}

void FT4AssetTypeActions_WeaponEntityAsset::OpenAssetEditor(
	const TArray<UObject*>& InObjects, 
	TSharedPtr<IToolkitHost> InEditWithinLevelEditor
)
{
	EToolkitMode::Type Mode = InEditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UT4WeaponEntityAsset* ItemEntityAsset = Cast<UT4WeaponEntityAsset>(*ObjIt);
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

UClass* FT4AssetTypeActions_WeaponEntityAsset::GetSupportedClass() const
{
	return UT4WeaponEntityAsset::StaticClass();
}

uint32 FT4AssetTypeActions_WeaponEntityAsset::GetCategories()
{
	return GetT4AssetCategory();
}