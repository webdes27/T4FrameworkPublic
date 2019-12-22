// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetTypeActions_AnimSetAsset.h"

#include "T4Asset/Public/T4AssetUtils.h" // #42, #62
#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #39
 */
 FColor FT4AssetTypeActions_AnimSetAsset::GetTypeColor() const
{
	 return FLinearColor(1, 1, 1).ToFColor(true);
}

void FT4AssetTypeActions_AnimSetAsset::OpenAssetEditor(
	const TArray<UObject*>& InObjects, 
	TSharedPtr<IToolkitHost> InEditWithinLevelEditor
)
{
	EToolkitMode::Type Mode = InEditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UT4AnimSetAsset* AnimSetAsset = Cast<UT4AnimSetAsset>(*ObjIt);
		if (nullptr != AnimSetAsset)
		{
			if (!AnimSetAsset->PreviewEntityAsset.IsNull())
			{
				UT4EntityAsset* PreviewEntityAsset = AnimSetAsset->PreviewEntityAsset.LoadSynchronous();
				if (nullptr != PreviewEntityAsset)
				{
					// #42
					GetOnT4EntityAssetTypeAction().ExecuteIfBound(
						PreviewEntityAsset,
						InEditWithinLevelEditor
					);
				}
			}
		}
	}
}

UClass* FT4AssetTypeActions_AnimSetAsset::GetSupportedClass() const
{
	return UT4AnimSetAsset::StaticClass();
}

uint32 FT4AssetTypeActions_AnimSetAsset::GetCategories()
{
	return GetT4AssetCategory();
}