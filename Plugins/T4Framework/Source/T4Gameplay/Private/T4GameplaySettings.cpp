// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameplaySettings.h"

#include "T4GameplayInternal.h"

#define LOCTEXT_NAMESPACE "T4GameplaySettings"

/**
  * #43
 */
UT4GameplaySettings::UT4GameplaySettings()
{
	// Plugins/T4Framework/Config/DefaultT4Framework.ini
}

#if WITH_EDITOR
void UT4GameplaySettings::PostEditChangeProperty(
	FPropertyChangedEvent& PropertyChangedEvent
)
{
}
#endif

#undef LOCTEXT_NAMESPACE