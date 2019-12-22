// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Public/T4EngineSettings.h" // #40

#include "T4EngineInternal.h"

#define LOCTEXT_NAMESPACE "T4Engine"

/**
  * #40
 */
UT4EngineSettings::UT4EngineSettings(const FObjectInitializer& ObjectInitlaizer)
	: Super(ObjectInitlaizer)
{
	// Plugins/T4Framework/Config/DefaultT4Framework.ini
}

FName UT4EngineSettings ::GetCategoryName() const
{
	return TEXT("T4Framework");
}

#if WITH_EDITOR
FText UT4EngineSettings ::GetSectionText() const
{
	return NSLOCTEXT("T4EngineSettings", "T4EngineSettingsSection", "T4Engine");
}
#endif

#if WITH_EDITOR
void UT4EngineSettings ::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property != nullptr)
	{
		SettingsChangedDelegate.Broadcast(PropertyChangedEvent.Property->GetName(), this);
	}
}

UT4EngineSettings::FT4OnEngineSettingsChanged& UT4EngineSettings::OnSettingsChanged()
{
	return SettingsChangedDelegate;
}

UT4EngineSettings::FT4OnEngineSettingsChanged UT4EngineSettings::SettingsChangedDelegate;
#endif

#undef LOCTEXT_NAMESPACE