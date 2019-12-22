// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
  * #94
 */
class UT4MapEntityAsset;
class FObjectThumbnail;
struct FT4LevelThumbnailData; // #104

namespace T4AssetUtil
{

	// #84
	const FT4LevelThumbnailData* MapEntityGetSubLevelThumbnail(
		UT4MapEntityAsset* InMapEntityAsset,
		const FName InLevelAssetName
	);

	bool MapEntityAddOrUpdateSubLevelThumbnail(
		UT4MapEntityAsset* InMapEntityAsset,
		const FName InLevelAssetName,
		FObjectThumbnail* InObjectThumbnail
	);
	// ~#84

}
