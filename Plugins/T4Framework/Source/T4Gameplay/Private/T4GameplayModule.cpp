// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameplayModule.h"
#include "T4GameplayLauncher.h"

#if WITH_EDITOR
#include "T4GameplaySettings.h" // #43
#include "ISettingsModule.h"
#endif

#include "T4GameplayInternal.h"

#define LOCTEXT_NAMESPACE "T4GameplayModule"

/**
  * #43
 */
class FT4GameplayModule : public IT4GameplayModule
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};

void FT4GameplayModule::StartupModule()
{
	GetGameplayLauncher().Initialize();

	// #43
#if WITH_EDITOR
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (nullptr != SettingsModule)
	{
		SettingsModule->RegisterSettings(
			"Editor", 
			"T4Framework", 
			"T4Gameplay",
			LOCTEXT("T4GameplaySettingsSection", "T4Gameplay"),
			LOCTEXT("T4GameplaySettingsDescription", "Configure T4Gameplay editing features."),
			GetMutableDefault<UT4GameplaySettings>()
		);
	}
#endif
}

void FT4GameplayModule::ShutdownModule()
{
	// #43
#if WITH_EDITOR
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (nullptr != SettingsModule)
	{
		SettingsModule->UnregisterSettings("Editor", "T4Framework", "T4Gameplay");
	}
#endif

	GetGameplayLauncher().Finalize();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FT4GameplayModule, T4Gameplay)
DEFINE_LOG_CATEGORY(LogT4Gameplay)
