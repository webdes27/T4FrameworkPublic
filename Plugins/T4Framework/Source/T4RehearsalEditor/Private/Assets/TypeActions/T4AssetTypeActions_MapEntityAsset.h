// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/IToolkitHost.h"
#include "AssetTypeActions_Base.h"

/**
  * #35
 */
class FT4AssetTypeActions_MapEntityAsset: public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions Implementation
	FText GetName() const override 
	{ 
		return NSLOCTEXT("T4AssetTypeActions", "T4AssetTypeActions_MapEntityAsset", "T4MapEntityAsset"); 
	}
	FColor GetTypeColor() const override;
	UClass* GetSupportedClass() const override;
	void OpenAssetEditor(
		const TArray<UObject*>& InObjects,
		TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()
	) override;
	uint32 GetCategories() override;
};
