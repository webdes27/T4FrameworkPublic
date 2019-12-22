// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetTypeActions_PropEntityAsset.h"

#include "T4Asset/Public/T4AssetUtils.h" // #42, #62
#include "T4Asset/Classes/Entity/T4PropEntityAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #35
 */
 FColor FT4AssetTypeActions_PropEntityAsset::GetTypeColor() const
{
	 return FLinearColor(1, 1, 1).ToFColor(true);
}

void FT4AssetTypeActions_PropEntityAsset::OpenAssetEditor(
	const TArray<UObject*>& InObjects, 
	TSharedPtr<IToolkitHost> InEditWithinLevelEditor
)
{
	EToolkitMode::Type Mode = InEditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UT4PropEntityAsset* PropEntityAsset = Cast<UT4PropEntityAsset>(*ObjIt);
		if (nullptr != PropEntityAsset)
		{
			// #42
			GetOnT4EntityAssetTypeAction().ExecuteIfBound(
				PropEntityAsset,
				InEditWithinLevelEditor
			);
		}
	}
}

UClass* FT4AssetTypeActions_PropEntityAsset::GetSupportedClass() const
{
	return UT4PropEntityAsset::StaticClass();
}

uint32 FT4AssetTypeActions_PropEntityAsset::GetCategories()
{
	return GetT4AssetCategory();
}