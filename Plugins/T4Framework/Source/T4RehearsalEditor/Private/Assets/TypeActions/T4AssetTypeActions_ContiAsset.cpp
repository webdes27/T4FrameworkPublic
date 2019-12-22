// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetTypeActions_ContiAsset.h"

#include "T4Asset/Public/T4AssetUtils.h" // #42, #62
#include "T4Asset/Classes/Conti/T4ContiAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #24
 */
 FColor FT4AssetTypeActions_ContiAsset::GetTypeColor() const
{
	 return FLinearColor(1, 1, 1).ToFColor(true);
}

void FT4AssetTypeActions_ContiAsset::OpenAssetEditor(
	const TArray<UObject*>& InObjects, 
	TSharedPtr<IToolkitHost> InEditWithinLevelEditor
)
{
	EToolkitMode::Type Mode = InEditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UT4ContiAsset* ContiAsset = Cast<UT4ContiAsset>(*ObjIt);
		if (nullptr != ContiAsset)
		{
			// #42
			GetOnT4ContiAssetTypeAction().ExecuteIfBound(
				ContiAsset,
				InEditWithinLevelEditor
			);
		}
	}
}

UClass* FT4AssetTypeActions_ContiAsset::GetSupportedClass() const
{
	return UT4ContiAsset::StaticClass();
}

uint32 FT4AssetTypeActions_ContiAsset::GetCategories()
{
	return GetT4AssetCategory();
}