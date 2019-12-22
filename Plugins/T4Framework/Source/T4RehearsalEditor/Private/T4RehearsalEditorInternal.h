// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "AssetTypeCategories.h"

/**
  *
 */
DECLARE_LOG_CATEGORY_EXTERN(LogT4RehearsalEditor, Log, All)

static const FName PersistentLevelName = TEXT("PersistentLevel"); // #91

EAssetTypeCategories::Type GetT4AssetCategory(); // #42

DECLARE_DELEGATE_TwoParams(FT4OnContiAssetTypeAction, class UT4ContiAsset*, const TSharedPtr<IToolkitHost>&);
DECLARE_DELEGATE_TwoParams(FT4OnEntityAssetTypeAction, class UT4EntityAsset*, const TSharedPtr<IToolkitHost>&);
DECLARE_DELEGATE_TwoParams(FT4OnWorldAssetTypeAction, class UT4WorldAsset*, const TSharedPtr<IToolkitHost>&); // #83

FT4OnContiAssetTypeAction& GetOnT4ContiAssetTypeAction();
FT4OnEntityAssetTypeAction& GetOnT4EntityAssetTypeAction();
FT4OnWorldAssetTypeAction& GetOnT4WorldAssetTypeAction(); // #83