// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetTypeActions_CharacterEntityAsset.h"

#include "T4Asset/Public/T4AssetUtils.h" // #42, #62
#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #35
 */
FColor FT4AssetTypeActions_CharacterEntityAsset::GetTypeColor() const
{
	return FLinearColor(1, 1, 1).ToFColor(true);
}

void FT4AssetTypeActions_CharacterEntityAsset::OpenAssetEditor(
	const TArray<UObject*>& InObjects, 
	TSharedPtr<IToolkitHost> InEditWithinLevelEditor
)
{
	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(*ObjIt);
		if (nullptr != CharacterEntityAsset)
		{
			// #42
			GetOnT4EntityAssetTypeAction().ExecuteIfBound(
				CharacterEntityAsset,
				InEditWithinLevelEditor
			);
		}
	}
}

UClass* FT4AssetTypeActions_CharacterEntityAsset::GetSupportedClass() const
{
	return UT4CharacterEntityAsset::StaticClass();
}

uint32 FT4AssetTypeActions_CharacterEntityAsset::GetCategories()
{
	return GetT4AssetCategory();
}