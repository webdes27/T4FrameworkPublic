// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
  * #84
 */
#if WITH_EDITOR


class UT4WorldAsset; // #84
class AT4MapZoneVolume; // #92

namespace T4AssetUtil
{

	bool WorldAddNewMapZoneVolume(
		UT4WorldAsset* InWorldAsset,
		const FName InMapZoneName,
		UWorld* InWorld
	); // #92

	bool WorldSelectMapZoneVolumeByName(
		UT4WorldAsset* InWorldAsset,
		const AT4MapZoneVolume* InMapZoneVolume
	); // #92

	bool WorldUpdateMapZoneVolume(
		UT4WorldAsset* InWorldAsset,
		AT4MapZoneVolume* OutMapZoneVolume
	); // #92

}

#endif
