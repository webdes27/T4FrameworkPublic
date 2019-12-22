// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetTypeActions_WorldAsset.h"

#include "T4Asset/Public/T4AssetUtils.h" // #42, #62
#include "T4Asset/Classes/World/T4WorldAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #83
 */
 FColor FT4AssetTypeActions_WorldAsset::GetTypeColor() const
{
	 return FLinearColor(1, 1, 1).ToFColor(true);
}

void FT4AssetTypeActions_WorldAsset::OpenAssetEditor(
	const TArray<UObject*>& InObjects, 
	TSharedPtr<IToolkitHost> InEditWithinLevelEditor
)
{
	EToolkitMode::Type Mode = InEditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UT4WorldAsset* WorldAsset = Cast<UT4WorldAsset>(*ObjIt);
		if (nullptr != WorldAsset)
		{
			// #42
			GetOnT4WorldAssetTypeAction().ExecuteIfBound(
				WorldAsset,
				InEditWithinLevelEditor
			);
		}
	}
}

UClass* FT4AssetTypeActions_WorldAsset::GetSupportedClass() const
{
	return UT4WorldAsset::StaticClass();
}

uint32 FT4AssetTypeActions_WorldAsset::GetCategories()
{
	return GetT4AssetCategory();
}