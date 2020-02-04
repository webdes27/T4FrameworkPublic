// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Classes/Replay/T4ActionReplayAsset.h"

#include "Serialization/CustomVersion.h"

#include "T4EngineInternal.h"

/**
  * #24
 */
const FGuid ET4ActionReplayVersion::GUID(0xFBF47AFA, 0x40734283, 0xB9B9E658, 0xFD914D35);

// Register the custom version with core
FCustomVersionRegistration GRegisterT4ActionReplayVersion(
	ET4ActionReplayVersion::GUID,
	ET4ActionReplayVersion::LatestVersion,
	TEXT("T4ActionReplayVer")
);

UT4ActionReplayAsset::UT4ActionReplayAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4ActionReplayAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(ET4ActionReplayVersion::GUID); // only changes version if not loading
	const int32 ContiVar = Ar.CustomVer(ET4ActionReplayVersion::GUID);
}
