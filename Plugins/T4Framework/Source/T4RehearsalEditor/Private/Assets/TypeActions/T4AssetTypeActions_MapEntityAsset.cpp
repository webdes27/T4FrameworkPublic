// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetTypeActions_MapEntityAsset.h"

#include "T4Asset/Public/T4AssetUtils.h" // #42, #62
#include "T4Asset/Classes/Entity/T4MapEntityAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #35
 */
 FColor FT4AssetTypeActions_MapEntityAsset::GetTypeColor() const
{
	 return FLinearColor(1, 1, 1).ToFColor(true);
}

void FT4AssetTypeActions_MapEntityAsset::OpenAssetEditor(
	const TArray<UObject*>& InObjects, 
	TSharedPtr<IToolkitHost> InEditWithinLevelEditor
)
{
	EToolkitMode::Type Mode = InEditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UT4MapEntityAsset* MapEntityAsset = Cast<UT4MapEntityAsset>(*ObjIt);
		if (nullptr != MapEntityAsset)
		{
			// #42
			GetOnT4EntityAssetTypeAction().ExecuteIfBound(
				MapEntityAsset,
				InEditWithinLevelEditor
			);
		}
	}
}

UClass* FT4AssetTypeActions_MapEntityAsset::GetSupportedClass() const
{
	return UT4MapEntityAsset::StaticClass();
}

uint32 FT4AssetTypeActions_MapEntityAsset::GetCategories()
{
	return GetT4AssetCategory();
}