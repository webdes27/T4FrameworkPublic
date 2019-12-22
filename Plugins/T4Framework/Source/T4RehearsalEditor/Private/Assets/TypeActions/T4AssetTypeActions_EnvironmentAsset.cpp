// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetTypeActions_EnvironmentAsset.h"

#include "T4Asset/Public/T4AssetUtils.h" // #42, #62
#include "T4Asset/Classes/World/T4EnvironmentAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #90
 */
 FColor FT4AssetTypeActions_EnvironmentAsset::GetTypeColor() const
{
	 return FLinearColor(1, 1, 1).ToFColor(true);
}

void FT4AssetTypeActions_EnvironmentAsset::OpenAssetEditor(
	const TArray<UObject*>& InObjects, 
	TSharedPtr<IToolkitHost> InEditWithinLevelEditor
)
{
	EToolkitMode::Type Mode = InEditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UT4EnvironmentAsset* EnvironmentAsset = Cast<UT4EnvironmentAsset>(*ObjIt);
		if (nullptr != EnvironmentAsset)
		{
			if (!EnvironmentAsset->PreviewWorldAsset.IsNull())
			{
				// #42
				GetOnT4WorldAssetTypeAction().ExecuteIfBound(
					EnvironmentAsset->PreviewWorldAsset.LoadSynchronous(),
					InEditWithinLevelEditor
				);
			}
		}
	}
}

UClass* FT4AssetTypeActions_EnvironmentAsset::GetSupportedClass() const
{
	return UT4EnvironmentAsset::StaticClass();
}

uint32 FT4AssetTypeActions_EnvironmentAsset::GetCategories()
{
	return GetT4AssetCategory();
}