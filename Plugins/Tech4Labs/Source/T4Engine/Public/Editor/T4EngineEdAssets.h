// Copyright 2019 Tech4 Labs, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4EngineTypes.h"

#include "AssetTypeCategories.h"

/**
  * #39
 */
#if WITH_EDITOR

class UObject;
class UBlendSpaceBase;
class UAnimSequence;
class UT4AnimSetAsset;
class UT4ContiAsset;
class UT4EntityAsset;

T4ENGINE_API UObject* T4EngineEdAssetNew(
	UClass* InAssetClass,
	const FName InCallingContext,
	const FString& InAssetName,
	const FString& InPackagePath
);

T4ENGINE_API bool T4EngineEdAssetSave(
	UObject* InSaveObject,
	bool bInCheckDirty
);

T4ENGINE_API bool T4EngineEdAssetSaveThumbnailImage(
	UObject* InSaveObject,
	UTexture2D* InThumbnail
);

T4ENGINE_API bool T4EngineEdEntitySaveThumbnailCameraInfo(
	UT4EntityAsset* InEntityAsset,
	const FRotator& ThumbnailRotation,
	const FVector& ThumbnailLocation
);

T4ENGINE_API bool T4EngineEdAnimSetAnimSequenceAddOrUpdate(
	UT4AnimSetAsset* InOutAnimSetAsset,
	const FName& InAnimMontageName,
	const FName& InAnimSequenceName,
	TSoftObjectPtr<UAnimSequence>& InAnimSequence,
	FString& OutErrorMessage
);

T4ENGINE_API bool T4EngineEdAnimSetAnimSequenceRemove(
	UT4AnimSetAsset* InOutAnimSetAsset,
	const FName& InAnimMontageName,
	const FName& InAnimSequenceName,
	FString& OutErrorMessage
);

T4ENGINE_API bool T4EngineEdAnimSetBaseBlendSpaceAddOrUpdate(
	UT4AnimSetAsset* InOutAnimSetAsset,
	const FName& InBlendSpaceName,
	TSoftObjectPtr<UBlendSpaceBase>& InBlendSpace,
	FString& OutErrorMessage
);

T4ENGINE_API bool T4EngineEdAnimSetBaseBlendSpaceRemove(
	UT4AnimSetAsset* InOutAnimSetAsset,
	const FName& InBlendSpaceName,
	FString& OutErrorMessage
);

T4ENGINE_API bool T4EngineEdAnimSetAnimMontageBuild(
	UT4AnimSetAsset* InOutAnimSetAsset,
	const FName& InAnimMontageName,
	const FName& InObjectName
);

T4ENGINE_API bool T4EngineEdAnimSetBaseBlendSpaceBuild(
	UT4AnimSetAsset* InOutAnimSetAsset
);

T4ENGINE_API bool T4EngineEdAnimSetBuild(
	UT4AnimSetAsset* InOutAnimSetAsset,
	FString& OutErrorMessage
);

DECLARE_DELEGATE_TwoParams(FOnT4ContiAssetTypeAction, UT4ContiAsset*, const TSharedPtr<IToolkitHost>&);
DECLARE_DELEGATE_TwoParams(FOnT4EntityAssetTypeAction, UT4EntityAsset*, const TSharedPtr<IToolkitHost>&);

T4ENGINE_API FOnT4ContiAssetTypeAction& T4EngineEdOnContiAssetTypeActionGet();
T4ENGINE_API FOnT4EntityAssetTypeAction& T4EngineEdOnEntityAssetTypeActionGet();

T4ENGINE_API EAssetTypeCategories::Type T4EngineEdAssetCategoryGet(); // #42

#endif