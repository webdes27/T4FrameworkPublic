// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4RehearsalEditorSettings.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4RehearsalEditorSettings"

/**
  *
 */
UT4RehearsalEditorSettings::UT4RehearsalEditorSettings(const FObjectInitializer& ObjectInitlaizer)
	: Super(ObjectInitlaizer)
{
	// Plugins/T4Framework/Config/DefaultT4Framework.ini
}

FName UT4RehearsalEditorSettings::GetCategoryName() const
{
	return TEXT("T4Framework");
}

FText UT4RehearsalEditorSettings::GetSectionText() const
{
	return NSLOCTEXT("T4RehearsalEditorSettings", "T4RehearsalEditorSettingsSectionName", "T4RehearsalEditor");
}

void UT4RehearsalEditorSettings::PostInitProperties()
{
	Super::PostInitProperties();
}

void UT4RehearsalEditorSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property != nullptr)
	{
		SettingsChangedDelegate.Broadcast(PropertyChangedEvent.Property->GetName(), this);
	}
}

UT4RehearsalEditorSettings::FT4OnRehearsalEdSettingsChanged& UT4RehearsalEditorSettings::OnSettingsChanged()
{
	return SettingsChangedDelegate;
}

UT4RehearsalEditorSettings::FT4OnRehearsalEdSettingsChanged UT4RehearsalEditorSettings::SettingsChangedDelegate;

#undef LOCTEXT_NAMESPACE