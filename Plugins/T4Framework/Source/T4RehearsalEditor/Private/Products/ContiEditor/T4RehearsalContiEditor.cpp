// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4RehearsalContiEditor.h"

#include "Products/T4RehearsalEditorUtils.h"

#include "T4RehearsalEditorCommands.h"
#include "T4RehearsalEditorModule.h"

#include "Products/ContiEditor/ViewModel/T4ContiViewModel.h"
#include "Products/Common/ViewModel/T4PreviewEntityViewModel.h"

#include "Products/Common/Widgets/ST4ContiBrowserWidget.h" // #30

#include "Products/ContiEditor/DetailView/ST4EditorPlaySettingWidget.h" // #60
#include "Products/ContiEditor/DetailView/ST4ContiObjectWidget.h"

#include "Products/Common/Viewport/T4RehearsalViewport.h"
#include "T4RehearsalEditorStyle.h"

#include "T4Asset/Public/T4AssetUtils.h" // #39, #62
#include "T4Asset/Classes/Entity/T4EntityAsset.h"
#include "T4Asset/Classes/Conti/T4ContiAsset.h"
#include "T4Engine/Public/T4Engine.h" // #58

#include "Widgets/Docking/SDockTab.h"

#include "Subsystems/AssetEditorSubsystem.h"
#include "Toolkits/AssetEditorManager.h"
#include "Modules/ModuleManager.h"
#include "ISequencerModule.h"
#include "PropertyEditorModule.h"

#include "T4RehearsalEditorInternal.h"

const FName FT4RehearsalContiEditor::AppIdentifier = FName(TEXT("T4RehearsaleContiEditorApp"));
const FName FT4RehearsalContiEditor::PreviewTabId = FName(TEXT("PreviewViewport"));
const FName FT4RehearsalContiEditor::MainViewportTabId = FName(TEXT("Viewport"));
const FName FT4RehearsalContiEditor::DetailTabId = FName(TEXT("Detail"));
const FName FT4RehearsalContiEditor::EditorPlaySettingsTabId = FName(TEXT("EditorPlaySettings")); // #60
const FName FT4RehearsalContiEditor::SequencerTabId = FName(TEXT("Timeline"));
const FName FT4RehearsalContiEditor::ContiBrowserTabId = FName(TEXT("ContiBrowser")); // #30

#define LOCTEXT_NAMESPACE "T4RehearsalContiEditor"

/**
  * 
 */
FT4RehearsalContiEditor::FT4RehearsalContiEditor()
{
}

FT4RehearsalContiEditor::~FT4RehearsalContiEditor()
{
	if (MainViewportPtr.IsValid())
	{
		MainViewportPtr->OnCleanup(); // #97 : 안전하게 Viewport 먼저 삭제한다. World 는 ViewportClient 와 완전히 분리되었다.
		MainViewportPtr.Reset();
	}
	if (PreviewViewportPtr.IsValid())
	{
		PreviewViewportPtr->OnCleanup(); // #97 : 안전하게 Viewport 먼저 삭제한다. World 는 ViewportClient 와 완전히 분리되었다.
		PreviewViewportPtr.Reset();
	}
	ContiBrowserPtr.Reset();
	if (PreviewViewModelPtr.IsValid())
	{
		PreviewViewModelPtr->OnCleanup();
		PreviewViewModelPtr.Reset();
	}
	if (MainViewModelPtr.IsValid())
	{
		MainViewModelPtr->OnCleanup();
		MainViewModelPtr.Reset();
	}
	ContiAssetPtr.Reset();
}

void FT4RehearsalContiEditor::InitializeWithConti(
	const EToolkitMode::Type InMode,
	const TSharedPtr<IToolkitHost>& InInitToolkitHost,
	UT4ContiAsset* InContiAsset
)
{
	ContiAssetPtr = InContiAsset;
	ContiAssetPtr->SetFlags(RF_Transactional); // Undo, Redo

	const bool bIsUpdatable = false;
	const bool bIsLockable = false;
	const bool bAllowFavorites = true;

	{
		static uint32 GContiViewModelInstanceCount = 0; // #65

		FT4ContiViewModelOptions ViewModelOptions;
		ViewModelOptions.InstanceKey = ++GContiViewModelInstanceCount; // #65
		ViewModelOptions.ContiAsset = ContiAssetPtr.Get();
		ViewModelOptions.ContiEditor = this;
		ViewModelOptions.OnGetSequencerAddMenuContent.BindSP(this, &FT4RehearsalContiEditor::GetSequencerAddMenuContent);
		MainViewModelPtr = MakeShareable(new FT4ContiViewModel(ViewModelOptions));

		// Editor
		//   ViewportClient : Camera or Input
		//     Viewport : Redering state
		//       PreviewScene : Rnedering View

		MainViewportPtr = SNew(ST4RehearsalViewport)
			.ViewModel(MainViewModelPtr.Get())
			.OnThumbnailCaptured(this, &FT4RehearsalEditor::HandleOnThumbnailCaptured)
			.OnHotKeyJumpToPlay(this, &FT4RehearsalContiEditor::HandleOnHotKeyJumpToPlay)  // #99 : Keys::Up
			.OnHotKeyJumpToEnd(this, &FT4RehearsalContiEditor::HandleOnHotKeyJumpToEnd) // #99 : Keys::Up + CTRL
			.OnHotKeyTogglePlay(this, &FT4RehearsalContiEditor::HandleOnHotKeyTogglePlay) // #99 : Keys::Down
			.OnSimulationButtonClicked(ST4RehearsalViewport::FT4OnOverlayButtonCallback::CreateRaw(this, &FT4RehearsalContiEditor::HandleOnToggleSimulation)); // #102
	}

	{
		FT4PreviewEntityViewModelOptions ViewModelOptions;

		PreviewViewModelPtr = MakeShareable(new FT4PreviewEntityViewModel(ViewModelOptions));
		PreviewViewportPtr = SNew(ST4RehearsalViewport)
			.ViewModel(PreviewViewModelPtr.Get())
			.OnThumbnailCaptured(this, &FT4RehearsalEditor::HandleOnThumbnailCaptured);
	}

	{
		// #30
		ContiBrowserPtr = SNew(ST4ContiBrowserWidget)
			.OnSelectAsset(ST4ContiBrowserWidget::FT4OnSelectAsset::CreateRaw(this, &FT4RehearsalContiEditor::HandleSelectContiAsset))
			.OnDoubleClicked(ST4ContiBrowserWidget::FT4OnDoubleClicked::CreateRaw(this, &FT4RehearsalContiEditor::HandleOpenContiAsset))
			.FilterEntityAsset(nullptr); // #71
	}

	const TSharedRef<FTabManager::FLayout> EditorDefaultLayout = FTabManager::NewLayout("T4RehearsalConti_Layout_v6")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)->SetHideTabWell(true)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetSizeCoefficient(0.9f)
				->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewSplitter()
					->SetSizeCoefficient(0.7f)
					->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewSplitter()
						->SetSizeCoefficient(0.8f)
						->SetOrientation(Orient_Horizontal)
						->Split
						(
							FTabManager::NewSplitter()
							->SetSizeCoefficient(0.3f)
							->SetOrientation(Orient_Vertical)
							->Split
							(
								FTabManager::NewStack()
								->SetSizeCoefficient(0.5f)
								->AddTab(PreviewTabId, ETabState::OpenedTab)->SetHideTabWell(true)
							)
							->Split
							(
								FTabManager::NewStack()
								->SetSizeCoefficient(0.5f)
								->AddTab(ContiBrowserTabId, ETabState::OpenedTab)->SetHideTabWell(true) // #30
							)
						)
						->Split
						(
							FTabManager::NewStack()
							->SetSizeCoefficient(0.7f)
							->AddTab(MainViewportTabId, ETabState::OpenedTab)->SetHideTabWell(true)
						)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.2f)
						->AddTab(SequencerTabId, ETabState::OpenedTab)->SetHideTabWell(true)
					)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.3f)
					->AddTab(DetailTabId, ETabState::OpenedTab)
					->AddTab(EditorPlaySettingsTabId, ETabState::OpenedTab) // #60
				)
			)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor(
		InMode, 
		InInitToolkitHost,
		AppIdentifier, 
		EditorDefaultLayout, 
		bCreateDefaultStandaloneMenu, 
		bCreateDefaultToolbar, 
		InContiAsset
	);

	SetupCommands();

	ExtendMenu();
	ExtendToolbar();
	RegenerateMenusAndToolbars();

	// #79
	{
		MainViewModelPtr->OnStartPlay(MainViewportPtr->GetViewportClient());
		PreviewViewModelPtr->OnStartPlay(PreviewViewportPtr->GetViewportClient());
	}
}

void FT4RehearsalContiEditor::SetupCommands()
{
	TSharedRef<FUICommandList> CommandList = GetToolkitCommands();
	CommandList->MapAction(
		FT4RehearsalEditorCommands::Get().SaveThumbnailImage,
		FExecuteAction::CreateSP(this, &FT4RehearsalContiEditor::HandleOnSaveThumbnailImage)
	);
	CommandList->MapAction(
		FT4RehearsalEditorCommands::Get().MirrorToPIE,
		FExecuteAction::CreateSP(this, &FT4RehearsalContiEditor::HandleOnMirrorToPIE)
	); // #59

	CommandList->MapAction(
		FT4RehearsalEditorCommands::Get().GoPreviewScene,
		FExecuteAction::CreateRaw(this, &FT4RehearsalContiEditor::HandleOnGoPreviewScene)
	); // #87

	CommonSetupCommands(CommandList); // #68
}

void FT4RehearsalContiEditor::ExtendMenu()
{

}

void FT4RehearsalContiEditor::ExtendToolbar()
{
	struct Local
	{
		static void FillToolbar(FToolBarBuilder& InToolbarBuilder, FT4RehearsalContiEditor* InContiEditor)
		{
			InToolbarBuilder.BeginSection("ContiEditorToolBar");
			{
				InToolbarBuilder.AddToolBarButton(
					FT4RehearsalEditorCommands::Get().SaveThumbnailImage, 
					NAME_None,
					LOCTEXT("T4ContiEditorGenerateThumbnail", "Thumbnail"),
					LOCTEXT("T4ContiEditorGenerateThumbnail_Tooltip", "Generate a thumbnail image."),
					FSlateIcon(FEditorStyle::GetStyleSetName(), "Cascade.SaveThumbnailImage")
				);

				InToolbarBuilder.AddSeparator();

				// #59
				InToolbarBuilder.AddToolBarButton(
					FT4RehearsalEditorCommands::Get().MirrorToPIE,
					NAME_None,
					LOCTEXT("T4ContiEditorMirrorToPIE", "MirrorToPIE"),
					LOCTEXT("T4ContiEditorMirrorToPIE_Tooltip", "Viewport Mirroring to PIE Viewport"),
					FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.ContiEditorMirrorToPIE_40x")
				);

				InToolbarBuilder.AddSeparator();

				InContiEditor->CommonExtendToolbar(InToolbarBuilder); // #68

				InToolbarBuilder.AddToolBarButton(
					FT4RehearsalEditorCommands::Get().GoPreviewScene,
					NAME_None,
					LOCTEXT("T4EntityEditorGoPreviewScene", "PreviewScene"),
					LOCTEXT("T4EntityEditorGoPreviewScene_Tooltip", "Go to the PreviewScene"),
					FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.ToolbarGoPreviewScene_40x")
				); // #87
			}
			InToolbarBuilder.EndSection();
		}
	};

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateStatic(&Local::FillToolbar, this)
	);

	AddToolbarExtender(ToolbarExtender);

	FT4RehearsalEditorModule& RehearsalEditorModule = FModuleManager::LoadModuleChecked<FT4RehearsalEditorModule>("T4RehearsalEditor");
	AddToolbarExtender(
		RehearsalEditorModule.GetToolBarExtensibilityManager()->GetAllExtenders(
			GetToolkitCommands(), GetEditingObjects()
		)
	);
}

void FT4RehearsalContiEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = TabManager->AddLocalWorkspaceMenuCategory(
		LOCTEXT("T4ContiEditorTabSpawnereMenu", "T4RehearsalConti Editor")
	);
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	TabManager->RegisterTabSpawner(PreviewTabId, FOnSpawnTab::CreateSP(this, &FT4RehearsalContiEditor::SpawnTab_Preview))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.TabThumbnail_16px"));

	TabManager->RegisterTabSpawner(MainViewportTabId, FOnSpawnTab::CreateSP(this, &FT4RehearsalContiEditor::SpawnTab_Viewport))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.TabMainViewport_16px"));

	TabManager->RegisterTabSpawner(DetailTabId, FOnSpawnTab::CreateSP(this, &FT4RehearsalContiEditor::SpawnTab_Detail))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.TabDetail_16px"));

	TabManager->RegisterTabSpawner(EditorPlaySettingsTabId, FOnSpawnTab::CreateSP(this, &FT4RehearsalContiEditor::SpawnTab_EditorPlaySettings))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.TabEditorPlaySettings_16px"));

	TabManager->RegisterTabSpawner(SequencerTabId, FOnSpawnTab::CreateSP(this, &FT4RehearsalContiEditor::SpawnTab_Sequencer))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.TabTimelie_16px"));

	TabManager->RegisterTabSpawner(ContiBrowserTabId, FOnSpawnTab::CreateSP(this, &FT4RehearsalContiEditor::SpawnTab_ContiBrowser))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.TabContiBrowser_16px"));
}

void FT4RehearsalContiEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(PreviewTabId);
	InTabManager->UnregisterTabSpawner(MainViewportTabId);
	InTabManager->UnregisterTabSpawner(DetailTabId);
	InTabManager->UnregisterTabSpawner(EditorPlaySettingsTabId);
	InTabManager->UnregisterTabSpawner(SequencerTabId);
	InTabManager->UnregisterTabSpawner(ContiBrowserTabId);
}

FName FT4RehearsalContiEditor::GetToolkitFName() const
{
	return FName("T4RehearsalConti Editor");
}

FText FT4RehearsalContiEditor::GetBaseToolkitName() const
{
	return LOCTEXT("T4ContiEditorToolKitName", "T4RehearsalConti Editor");
}

FString FT4RehearsalContiEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("T4ContiEditorWorldCentricTabPrefix", "T4ContiAsset ").ToString();
}

FLinearColor FT4RehearsalContiEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.0f, 0.0f, 0.2f, 0.5f);
}

ET4LayerType FT4RehearsalContiEditor::GetLayerType() const // #68
{
	if (!MainViewModelPtr.IsValid())
	{
		return ET4LayerType::Max;
	}
	return MainViewModelPtr->GetLayerType();
}

IT4RehearsalViewModel* FT4RehearsalContiEditor::GetMainViewModelInterface() const // #104
{
	if (!MainViewModelPtr.IsValid())
	{
		return nullptr;
	}
	return MainViewModelPtr.Get();
}

bool FT4RehearsalContiEditor::HasActionPlaybackController() const // #68
{
	if (!MainViewModelPtr.IsValid())
	{
		return false;
	}
	return MainViewModelPtr->HasActionPlaybackController();
}

UT4EditorActionPlaybackController* FT4RehearsalContiEditor::GetActionPlaybackController() const // #68
{
	if (!MainViewModelPtr.IsValid())
	{
		return nullptr;
	}
	return MainViewModelPtr->GetActionPlaybackController();
}

void FT4RehearsalContiEditor::SetPreviewViewTarget(UT4EntityAsset* EntityAsset) // #60
{
	if (PreviewViewModelPtr.IsValid())
	{
		PreviewViewModelPtr->OnChangeViewTarget(EntityAsset, 5.0f);
	}
}

bool FT4RehearsalContiEditor::CanSaveAsset() const 
{
	return MainViewModelPtr->CanSave();
}

void FT4RehearsalContiEditor::SaveAsset_Execute()
{
	if (!ContiAssetPtr.IsValid())
	{
		return;
	}
	MainViewModelPtr->DoSave();
}

bool FT4RehearsalContiEditor::CanSaveAssetAs() const
{
	return false;
}

void FT4RehearsalContiEditor::SaveAssetAs_Execute()
{
}

void FT4RehearsalContiEditor::HandleOnSaveThumbnailImage()
{
	if (MainViewportPtr.IsValid())
	{
		MainViewportPtr->CreateThumbnail(Cast<UObject>(ContiAssetPtr.Get()));
	}
}

void FT4RehearsalContiEditor::HandleOnMirrorToPIE() // #59
{
	if (MainViewModelPtr.IsValid())
	{
		MainViewModelPtr->ToggleMirrorToPIE();
	}
}

void FT4RehearsalContiEditor::HandleSelectContiAsset(UObject* InAsset)
{
	if (PreviewViewModelPtr.IsValid())
	{
		UT4ContiAsset* ContiAssetAsset = Cast<UT4ContiAsset>(InAsset);
		if (nullptr != ContiAssetAsset)
		{
			if (ContiAssetAsset->PreviewEntityAsset.IsValid())
			{
				PreviewViewModelPtr->OnChangeViewTarget(
					ContiAssetAsset->PreviewEntityAsset.Get(),
					ContiAssetAsset->TotalPlayTimeSec
				);
			}
			if (ContiAssetAsset->TestSettings.WeaponNameID != NAME_None)
			{
				PreviewViewModelPtr->ServerEquipWeapon(ContiAssetPtr->TestSettings.WeaponNameID, false); // #60
			}
			PreviewViewModelPtr->ClientPlayConti(ContiAssetAsset, nullptr);
		}
	}
}

void FT4RehearsalContiEditor::HandleOpenContiAsset(UObject* InAsset)
{
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(InAsset);
}

void FT4RehearsalContiEditor::HandleOnGoPreviewScene() // #87, #85
{
	if (!MainViewModelPtr.IsValid())
	{
		return;
	}
	IT4GameWorld* GameWorld = MainViewModelPtr->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return;
	}
	if (GameWorld->GetController()->IsPreviewScene())
	{
		return;
	}
	MainViewModelPtr->ClientWorldTravel(nullptr);
}

void FT4RehearsalContiEditor::HandleOnHotKeyJumpToPlay() // #99 : Keys::Up
{
	if (!MainViewModelPtr.IsValid())
	{
		return;
	}
	MainViewModelPtr->JumpToPlayAction();
}

void FT4RehearsalContiEditor::HandleOnHotKeyJumpToEnd() // #99 : Keys::Up + CTRL
{
	if (!MainViewModelPtr.IsValid())
	{
		return;
	}
	MainViewModelPtr->JumpToEndAction();
}

void FT4RehearsalContiEditor::HandleOnHotKeyTogglePlay() // #99 : Keys::Down
{
	if (!MainViewModelPtr.IsValid())
	{
		return;
	}
	MainViewModelPtr->TogglePlayAction();
}

void FT4RehearsalContiEditor::HandleOnToggleSimulation() // #102
{
	if (!MainViewModelPtr.IsValid())
	{
		return;
	}
	MainViewModelPtr->ToggleSimulation();
}

void FT4RehearsalContiEditor::GetSequencerAddMenuContent(
	FMenuBuilder& MenuBuilder, 
	TSharedRef<ISequencer> Sequencer
)
{
#if 0
	MenuBuilder.AddSubMenu(
		LOCTEXT("T4ContiEditorContiAsset", "T4Action Conti..."),
		LOCTEXT("T4ContiEditorContiAsset_Tooltip", "Add an existing contis..."),
		FNewMenuDelegate::CreateLambda([&](FMenuBuilder& InMenuBuilder)
	{
		InMenuBuilder.AddWidget(CreateAddContiMenuContent(), FText());
	}));
#endif
}

TSharedRef<SWidget> FT4RehearsalContiEditor::CreateAddContiMenuContent()
{
	/*
	FAssetPickerConfig AssetPickerConfig;
	{
		AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &FNiagaraSystemToolkit::EmitterAssetSelected);
		AssetPickerConfig.bAllowNullSelection = false;
		AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
		AssetPickerConfig.Filter.ClassNames.Add(UNiagaraEmitter::StaticClass()->GetFName());
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	return SNew(SBox)
		.WidthOverride(300.0f)
		.HeightOverride(300.f)
		[
			ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
		];
		*/
	return SNew(SBox);
}

TSharedRef<SDockTab> FT4RehearsalContiEditor::SpawnTab_Preview(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == PreviewTabId);
	return SNew(SDockTab)
		[
			PreviewViewportPtr.ToSharedRef()
		];
}

TSharedRef<SDockTab> FT4RehearsalContiEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == MainViewportTabId);
	return SNew(SDockTab)
		[
			MainViewportPtr.ToSharedRef()
		];
}

TSharedRef<SDockTab> FT4RehearsalContiEditor::SpawnTab_Detail(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == DetailTabId);
	TSharedRef<SDockTab> SpawnedTab =
		SNew(SDockTab)
		[
			SNew(ST4ContiObjectWidget, MainViewModelPtr)
		];

	return SpawnedTab;
}

TSharedRef<SDockTab> FT4RehearsalContiEditor::SpawnTab_EditorPlaySettings(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == EditorPlaySettingsTabId);
	TSharedRef<SDockTab> SpawnedTab =
		SNew(SDockTab)
		[
			SNew(ST4EditorPlaySettingWidget, MainViewModelPtr)
		];

	return SpawnedTab;
}

TSharedRef<SDockTab> FT4RehearsalContiEditor::SpawnTab_Sequencer(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId().TabType == SequencerTabId);

	TSharedRef<SDockTab> SpawnedTab =
		SNew(SDockTab)
		[
			MainViewModelPtr->GetSequencer()->GetSequencerWidget()
		];

	return SpawnedTab;
}

TSharedRef<SDockTab> FT4RehearsalContiEditor::SpawnTab_ContiBrowser(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == ContiBrowserTabId);
	return SNew(SDockTab)
		[
			ContiBrowserPtr.ToSharedRef()
		];
}

#undef LOCTEXT_NAMESPACE