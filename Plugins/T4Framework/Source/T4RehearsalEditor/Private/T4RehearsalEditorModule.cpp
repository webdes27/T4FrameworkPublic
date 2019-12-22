// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4RehearsalEditorModule.h"
#include "T4RehearsalEditorStyle.h"
#include "T4RehearsalEditorCommands.h"
#include "T4RehearsalEditorSettings.h"

#include "Products/EntityEditor/T4RehearsalEntityEditor.h" // #36
#include "Products/ContiEditor/T4RehearsalContiEditor.h"
#include "Products/WorldEditor/T4RehearsalWorldEditor.h" // #83

#include "Thumbnail/T4ThumbnailDefaultRenderer.h" // #91

// #T4_ADD_ENTITY_TAG
#include "Assets/TypeActions/T4AssetTypeActions_WorldAsset.h" // #83
#include "Assets/TypeActions/T4AssetTypeActions_EnvironmentAsset.h" // #90
#include "Assets/TypeActions/T4AssetTypeActions_ContiAsset.h" // #24
#include "Assets/TypeActions/T4AssetTypeActions_AnimSetAsset.h" // #39
#include "Assets/TypeActions/T4AssetTypeActions_MapEntityAsset.h" // #35
#include "Assets/TypeActions/T4AssetTypeActions_CharacterEntityAsset.h" // #35
#include "Assets/TypeActions/T4AssetTypeActions_PropEntityAsset.h" // #35
#include "Assets/TypeActions/T4AssetTypeActions_WeaponEntityAsset.h" // #35
#include "Assets/TypeActions/T4AssetTypeActions_CostumeEntityAsset.h" // #37
#include "Assets/TypeActions/T4AssetTypeActions_ZoneEntityAsset.h" // #94

#include "Products/Common/Helper/T4EditorObjectSelectionEditMode.h" // #94

#include "Products/Common/WorldMap/T4LevelCollectionCommands.h" // #83

#include "Products/ContiEditor/Sequencer/MovieScene/T4ContiActionChannel.h" // #56
#include "Products/ContiEditor/Sequencer/T4ContiActionTrackEditor.h"

#include "Products/LevelEditor/T4LevelEditorMode.h" // #17

#include "T4Asset/Public/T4AssetUtils.h" // #39, #62
#include "T4Engine/Public/T4EngineConstants.h" // #39

#include "GameMapsSettings.h" // #61
#include "Framework/MultiBox/MultiBoxBuilder.h" // #61

#include "ISettingsModule.h"
#include "SequencerChannelInterface.h" // #56
#include "ISequencerModule.h"
#include "EditorModeRegistry.h"
#include "LevelEditor.h"
#include "IAssetTools.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4RehearsalEditorModule"

static EAssetTypeCategories::Type GT4AssetCategory;
static IT4GameFrame* GEditorFramework = nullptr; // #17

/**
  *
 */
FT4RehearsalEditorModule::FT4RehearsalEditorModule()
	: SequencerSettings(nullptr)
{
}

FT4RehearsalEditorModule::~FT4RehearsalEditorModule()
{
}

void FT4RehearsalEditorModule::StartupModule()
{
	UT4RehearsalEditorSettings* RehearsalSettings = GetMutableDefault<UT4RehearsalEditorSettings>();
	check(nullptr != RehearsalSettings);

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	GT4AssetCategory = AssetTools.RegisterAdvancedAssetCategory(
		FName(TEXT("T4Framework")),
		LOCTEXT("T4RehearsalAssetCategoryName", "T4Framework")
	);

	{
		// #T4_ADD_ENTITY_TAG
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FT4AssetTypeActions_WorldAsset())); // #83
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FT4AssetTypeActions_EnvironmentAsset())); // #90
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FT4AssetTypeActions_ContiAsset()));
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FT4AssetTypeActions_AnimSetAsset())); // #39
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FT4AssetTypeActions_MapEntityAsset()));
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FT4AssetTypeActions_CharacterEntityAsset()));
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FT4AssetTypeActions_PropEntityAsset()));
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FT4AssetTypeActions_WeaponEntityAsset()));
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FT4AssetTypeActions_CostumeEntityAsset())); // #37
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FT4AssetTypeActions_ZoneEntityAsset())); // #94
	}

	// #17
	{
		FT4WorldConstructionValues WorldConstructionValues; // #87
		WorldConstructionValues.GameWorldType = ET4GameWorldType::LevelEditor; // #87
		WorldConstructionValues.WorldContextGameOrEditorOnly = &GEditor->GetEditorWorldContext();
		GEditorFramework = T4FrameCreate(ET4FrameType::Frame_Client, WorldConstructionValues);
	}

	MenuExtensibilityManager = MakeShareable(new FExtensibilityManager);
	ToolBarExtensibilityManager = MakeShareable(new FExtensibilityManager);

	FT4RehearsalEditorStyle::Initialize();
	FT4RehearsalEditorCommands::Register();
	FT4LevelCollectionCommands::Register(); // #83

	{
		ISequencerModule &SequencerModule = FModuleManager::LoadModuleChecked< ISequencerModule >("Sequencer");
		SequencerTrackEditorHandle = SequencerModule.RegisterTrackEditor(
			FOnCreateTrackEditor::CreateStatic(&FT4ContiActionTrackEditor::CreateTrackEditor)
		);
		SequencerModule.RegisterChannelInterface<FT4ContiActionChannel>(); // #56
	}

	{
		FEditorModeRegistry& EditorModeRegistry = FEditorModeRegistry::Get();

		EditorModeRegistry.RegisterMode<FT4LevelEditorMode>(
			FT4LevelEditorMode::EM_T4LevelEditorMode,
			LOCTEXT("T4RehearsalEditorModeName", "T4EditorMode"),
			FSlateIcon(
				FT4RehearsalEditorStyle::GetStyleSetName(),
				"T4RehearsalEditorStyle.LevelEditorMode_40x",
				"T4RehearsalEditorStyle.LevelEditorMode_16x"
			),
			true,
			1000
		); // #17

		EditorModeRegistry.RegisterMode<FT4EditorObjectSelectionEditMode>(
			FT4EditorObjectSelectionEditMode::EM_T4EditorObjectSelectionEditMode, 
			LOCTEXT("T4RehearsalEditorGameObjectSelectionEditModeName", "GameObjectSelection"), 
			FSlateIcon(), 
			false
		); // #94
	}

	// #61
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
		FLevelEditorModule::FLevelEditorMenuExtender ViewMenuExtender 
			= FLevelEditorModule::FLevelEditorMenuExtender::CreateRaw(this, &FT4RehearsalEditorModule::HandleOnLevelEditorPlayMenu);
		TArray<FLevelEditorModule::FLevelEditorMenuExtender>& LevelEditorPlayMenu = LevelEditorModule.GetAllLevelEditorToolbarPlayMenuExtenders();
		LevelEditorPlayMenu.Add(ViewMenuExtender);
		LevelEditorPlayExtenderHandle = LevelEditorPlayMenu.Last().GetHandle();
		CachedEngineGameInstanceClassName = GetDefault<UGameMapsSettings>()->GameInstanceClass;
		CachedEngineGlobalGameModeClassName = UGameMapsSettings::GetGlobalDefaultGameMode();
		if (RehearsalSettings->bT4GameplayEnabled)
		{
			GetMutableDefault<UGameMapsSettings>()->GameInstanceClass = RehearsalSettings->DefaultGameInstanceClass;
			UGameMapsSettings::SetGlobalDefaultGameMode(RehearsalSettings->DefaultGameModeClass.ToString());
		}
	}

	// #42
	{
		GetOnT4ContiAssetTypeAction().BindRaw(this, &FT4RehearsalEditorModule::HandleOnContiEditorLaunch);
		GetOnT4EntityAssetTypeAction().BindRaw(this, &FT4RehearsalEditorModule::HandleOnEntityEditorLaunch);
		GetOnT4WorldAssetTypeAction().BindRaw(this, &FT4RehearsalEditorModule::HandleOnWorldEditorLaunch); // #83
	}

	if (GIsEditor)
	{
		UT4ThumbnailDefaultRenderer::RegisterCustomRenderer();
	}

	{
		FT4EngineConstants* EngineConstants = T4EngineConstantsGet();
		check(nullptr != EngineConstants);

		// #39
		bool bResult = EngineConstants->LoadEditorConstants(
			ET4EngineConstantType::BlendSpace,
			RehearsalSettings->BlendSpaceNameTable
		);
		ensureMsgf(bResult, TEXT("FT4RehearsalEditorModule : Failed to load BlendSpaceNameTable. (DefaultT4Framework.ini)"));

		bResult = EngineConstants->LoadEditorConstants(
			ET4EngineConstantType::DefaultSection,
			RehearsalSettings->DefaultSectionNameTable
		);
		ensureMsgf(bResult, TEXT("FT4RehearsalEditorModule : Failed to load DefaultSectionNameTable. (DefaultT4Framework.ini)"));

		bResult = EngineConstants->LoadEditorConstants(
			ET4EngineConstantType::AdditiveSection,
			RehearsalSettings->AdditiveSectionNameTable
		);
		ensureMsgf(bResult, TEXT("FT4RehearsalEditorModule : Failed to load AdditiveSectionNameTable. (DefaultT4Framework.ini)"));

		bResult = EngineConstants->LoadEditorConstants(
			ET4EngineConstantType::SkillSection,
			RehearsalSettings->SkillSectionNameTable
		);
		ensureMsgf(bResult, TEXT("FT4RehearsalEditorModule : Failed to load SkillSectionNameTable. (DefaultT4Framework.ini)"));
		// ~#39

		bResult = EngineConstants->LoadEditorConstants(
			ET4EngineConstantType::ActionPoint,
			RehearsalSettings->ActionPointNameTable
		); // #57
		ensureMsgf(bResult, TEXT("FT4RehearsalEditorModule : Failed to load ActionPointNameTable. (DefaultT4Framework.ini)"));

		bResult = EngineConstants->LoadEditorConstants(
			ET4EngineConstantType::EquipPoint,
			RehearsalSettings->EquipPointNameTable
		); // #72
		ensureMsgf(bResult, TEXT("FT4RehearsalEditorModule : Failed to load EquipPointNameTable. (DefaultT4Framework.ini)"));

		bResult = EngineConstants->LoadEditorConstants(
			ET4EngineConstantType::CompositePart,
			RehearsalSettings->CompositePartNameTable
		); // #71
		ensureMsgf(bResult, TEXT("FT4RehearsalEditorModule : Failed to load CompositePartNameTable. (DefaultT4Framework.ini)"));

		bResult = EngineConstants->LoadEditorConstants(
			ET4EngineConstantType::LayerTag,
			RehearsalSettings->LayerTagNameTable
		); // #74
		ensureMsgf(bResult, TEXT("FT4RehearsalEditorModule : Failed to load LayerTagNameTable. (DefaultT4Framework.ini)"));

		bResult = EngineConstants->LoadEditorConstants(
			ET4EngineConstantType::Stance,
			RehearsalSettings->StanceNameTable
		); // #73
		ensureMsgf(bResult, TEXT("FT4RehearsalEditorModule : Failed to load StanceNameTable. (DefaultT4Framework.ini)"));

		bResult = EngineConstants->LoadEditorConstants(
			ET4EngineConstantType::Reaction,
			RehearsalSettings->ReactionNameTable
		); // #76
		ensureMsgf(bResult, TEXT("FT4RehearsalEditorModule : Failed to load ReactionNameTable. (DefaultT4Framework.ini)"));
	}
}

void FT4RehearsalEditorModule::ShutdownModule()
{
	// #17
	if (nullptr != GEditorFramework)
	{
		T4FrameDestroy(GEditorFramework);
		GEditorFramework = nullptr;
	}

	// #61
	{
		if (LevelEditorPlayExtenderHandle.IsValid())
		{
			FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
			TArray<FLevelEditorModule::FLevelEditorMenuExtender>& LevelEditorPlayMenu = LevelEditorModule.GetAllLevelEditorToolbarPlayMenuExtenders();
			LevelEditorPlayMenu.RemoveAll([=](const FLevelEditorModule::FLevelEditorMenuExtender& Extender) { return Extender.GetHandle() == LevelEditorPlayExtenderHandle; });
		}
	}

	// #17
	{
		FEditorModeRegistry& EditorModeRegistry = FEditorModeRegistry::Get();
		EditorModeRegistry.UnregisterMode(FT4LevelEditorMode::EM_T4LevelEditorMode);
		EditorModeRegistry.UnregisterMode(FT4EditorObjectSelectionEditMode::EM_T4EditorObjectSelectionEditMode); // #94
	}

	{
		ISequencerModule* SequencerModule = FModuleManager::GetModulePtr<ISequencerModule>("Sequencer");
		if (nullptr != SequencerModule)
		{
			SequencerModule->UnRegisterTrackEditor(SequencerTrackEditorHandle);
		}
	}

	FT4LevelCollectionCommands::Unregister(); // #83
	FT4RehearsalEditorCommands::Unregister();
	FT4RehearsalEditorStyle::Shutdown();

	MenuExtensibilityManager.Reset();
	ToolBarExtensibilityManager.Reset();

	if (UObjectInitialized() && GIsEditor)
	{
		UT4ThumbnailDefaultRenderer::UnregisterCustomRenderer();
	}

	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (auto CreatedAssetTypeAction : CreatedAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeAction.ToSharedRef());
		}
	}
	CreatedAssetTypeActions.Empty();
}

void FT4RehearsalEditorModule::HandleOnContiEditorLaunch(
	UT4ContiAsset* InContiAsset,
	const TSharedPtr<IToolkitHost>& InEditWithinLevelEditor
)
{
	EToolkitMode::Type Mode = InEditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	TSharedRef<FT4RehearsalContiEditor> NewRehearsalEditor(new FT4RehearsalContiEditor());
	NewRehearsalEditor->InitializeWithConti(Mode, InEditWithinLevelEditor, InContiAsset);
}

void FT4RehearsalEditorModule::HandleOnEntityEditorLaunch(
	UT4EntityAsset* InEntityAsset,
	const TSharedPtr<IToolkitHost>& InEditWithinLevelEditor
)
{
	EToolkitMode::Type Mode = InEditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	TSharedRef<FT4RehearsalEntityEditor> NewRehearsalEditor(new FT4RehearsalEntityEditor());
	NewRehearsalEditor->InitializeWithEntity(Mode, InEditWithinLevelEditor, InEntityAsset);
}

void FT4RehearsalEditorModule::HandleOnWorldEditorLaunch(
	UT4WorldAsset* InWorldAsset,
	const TSharedPtr<IToolkitHost>& InEditWithinLevelEditor
) // #83
{
	EToolkitMode::Type Mode = InEditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	TSharedRef<FT4RehearsalWorldEditor> NewRehearsalEditor(new FT4RehearsalWorldEditor());
	NewRehearsalEditor->InitializeWithWorld(Mode, InEditWithinLevelEditor, InWorldAsset);
}

// #61
TSharedRef<FExtender> FT4RehearsalEditorModule::HandleOnLevelEditorPlayMenu(
	const TSharedRef<FUICommandList> CommandList
) // #61
{
	TSharedRef<FExtender> Extender(new FExtender());

	Extender->AddMenuExtension(
		"LevelEditorPlayModes",
		EExtensionHook::Before,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FT4RehearsalEditorModule::CreateLevelEditorT4PlayOptionMenu)
	);

	return Extender;
}

void FT4RehearsalEditorModule::HandleOnToogleT4GameplayEnable()
{
	UT4RehearsalEditorSettings* RehearsalSettings = GetMutableDefault<UT4RehearsalEditorSettings>();
	check(nullptr != RehearsalSettings);
	RehearsalSettings->bT4GameplayEnabled = !RehearsalSettings->bT4GameplayEnabled;
	if (RehearsalSettings->bT4GameplayEnabled)
	{
		GetMutableDefault<UGameMapsSettings>()->GameInstanceClass = RehearsalSettings->DefaultGameInstanceClass;
		UGameMapsSettings::SetGlobalDefaultGameMode(RehearsalSettings->DefaultGameModeClass.ToString());
	}
	else
	{
		GetMutableDefault<UGameMapsSettings>()->GameInstanceClass = CachedEngineGameInstanceClassName;
		UGameMapsSettings::SetGlobalDefaultGameMode(CachedEngineGlobalGameModeClassName.ToString());
	}
}

bool FT4RehearsalEditorModule::HandleOnIsT4GameplayEnabled() const
{
	UT4RehearsalEditorSettings* RehearsalSettings = GetMutableDefault<UT4RehearsalEditorSettings>();
	check(nullptr != RehearsalSettings);
	return RehearsalSettings->bT4GameplayEnabled;
}

void FT4RehearsalEditorModule::CreateLevelEditorT4PlayOptionMenu(FMenuBuilder& InBuilder) // #61
{
	InBuilder.BeginSection("T4LevelEditorT4PlayOptions", LOCTEXT("T4LevelEditorT4PlayOptionsSection", "T4Framework Play Options"));
	{
		FUIAction T4LevelEditorPlayAction(
			FExecuteAction::CreateRaw(this, &FT4RehearsalEditorModule::HandleOnToogleT4GameplayEnable),
			FCanExecuteAction(),
			FIsActionChecked::CreateRaw(this, &FT4RehearsalEditorModule::HandleOnIsT4GameplayEnabled)
		);
		InBuilder.AddMenuEntry(
			LOCTEXT("T4LevelEditorT4GameplayMenu", "T4Gameplay Enabled"),
			LOCTEXT("T4LevelEditorT4GameplayMenu_Tooltip", "If Enabled, T4Framework Gameplay Enabled."),
			FSlateIcon(),
			T4LevelEditorPlayAction,
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
	}
	InBuilder.EndSection();
}
// ~#61

void FT4RehearsalEditorModule::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (SequencerSettings)
	{
		Collector.AddReferencedObject(SequencerSettings);
	}
}

void FT4RehearsalEditorModule::RegisterAssetTypeAction(
	IAssetTools& AssetTools,
	TSharedRef<IAssetTypeActions> Action
)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

EAssetTypeCategories::Type GetT4AssetCategory()
{
	return GT4AssetCategory;// #42
}

static FT4OnContiAssetTypeAction GOnT4ContiAssetTypeAction;
FT4OnContiAssetTypeAction& GetOnT4ContiAssetTypeAction()
{
	return GOnT4ContiAssetTypeAction;
}

static FT4OnEntityAssetTypeAction GOnT4EntityAssetTypeAction;
FT4OnEntityAssetTypeAction& GetOnT4EntityAssetTypeAction()
{
	return GOnT4EntityAssetTypeAction;
}

static FT4OnWorldAssetTypeAction GOnT4WorldAssetTypeAction;
FT4OnWorldAssetTypeAction& GetOnT4WorldAssetTypeAction()
{
	return GOnT4WorldAssetTypeAction;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FT4RehearsalEditorModule, T4RehearsalEditor);
DEFINE_LOG_CATEGORY(LogT4RehearsalEditor)