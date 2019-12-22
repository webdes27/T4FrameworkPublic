// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Playback/T4ActionPlaybackAsset.h"

#include "Serialization/CustomVersion.h"

#include "T4EngineInternal.h"

/**
  * #24
 */
const FGuid ET4ActionPlaybackVersion::GUID(0xFBF47AFA, 0x40734283, 0xB9B9E658, 0xFD914D35);

// Register the custom version with core
FCustomVersionRegistration GRegisterT4ActionPlaybackVersion(
	ET4ActionPlaybackVersion::GUID,
	ET4ActionPlaybackVersion::LatestVersion,
	TEXT("T4ActionPlaybackVer")
);

UT4ActionPlaybackAsset::UT4ActionPlaybackAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4ActionPlaybackAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(ET4ActionPlaybackVersion::GUID); // only changes version if not loading
	const int32 ContiVar = Ar.CustomVer(ET4ActionPlaybackVersion::GUID);
}
