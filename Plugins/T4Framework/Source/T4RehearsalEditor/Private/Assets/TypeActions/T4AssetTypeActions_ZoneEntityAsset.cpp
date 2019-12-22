// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetTypeActions_ZoneEntityAsset.h"

#include "T4Asset/Public/T4AssetUtils.h" // #42, #62
#include "T4Asset/Classes/Entity/T4ZoneEntityAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #94
 */
 FColor FT4AssetTypeActions_ZoneEntityAsset::GetTypeColor() const
{
	 return FLinearColor(1, 1, 1).ToFColor(true);
}

void FT4AssetTypeActions_ZoneEntityAsset::OpenAssetEditor(
	const TArray<UObject*>& InObjects, 
	TSharedPtr<IToolkitHost> InEditWithinLevelEditor
)
{
	EToolkitMode::Type Mode = InEditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UT4ZoneEntityAsset* ZoneEntityAsset = Cast<UT4ZoneEntityAsset>(*ObjIt);
		if (nullptr != ZoneEntityAsset)
		{
			// #42
			GetOnT4EntityAssetTypeAction().ExecuteIfBound(
				ZoneEntityAsset,
				InEditWithinLevelEditor
			);
		}
	}
}

UClass* FT4AssetTypeActions_ZoneEntityAsset::GetSupportedClass() const
{
	return UT4ZoneEntityAsset::StaticClass();
}

uint32 FT4AssetTypeActions_ZoneEntityAsset::GetCategories()
{
	return GetT4AssetCategory();
}