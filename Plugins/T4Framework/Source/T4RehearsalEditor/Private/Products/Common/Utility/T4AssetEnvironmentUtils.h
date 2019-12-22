// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
  * #94
 */
#if WITH_EDITOR

struct FT4EnvTimeTagData; // #90
class UT4EnvironmentAsset; // #90

namespace T4AssetUtil
{

	bool EnviromentTimeTagUpdate(
		UT4EnvironmentAsset* InEnvironmentAsset,
		FName InTimeTagName,
		const FT4EnvTimeTagData* InData,
		FString& OutErrorMessage
	); // #90, #95

	bool EnviromentTimeTagAdd(
		UT4EnvironmentAsset* InEnvironmentAsset,
		FName InTimeTagName,
		const FT4EnvTimeTagData* InData,
		FString& OutErrorMessage
	); // #90

	bool EnviromentTimeTagRemove(
		UT4EnvironmentAsset* InEnvironmentAsset,
		FName InTimeTagName,
		FString& OutErrorMessage
	); // #90

	bool EnviromentTimeTagCopyToClipboard(
		UT4EnvironmentAsset* InEnvironmentAsset,
		FName InTimeTagName,
		FString& OutJsonTimeTagDataString,
		FString& OutErrorMessage
	); // #104

	bool EnviromentTimeTagPastToClipboard(
		UT4EnvironmentAsset* InEnvironmentAsset,
		FName InTimeTagName,
		const FString& InJsonTimeTagDataString,
		FString& OutErrorMessage
	); // #104

	bool EnviromentAssetSave(
		UT4EnvironmentAsset* InEnvironmentAsset,
		FString& OutErrorMessage
	); // #90

}

#endif
