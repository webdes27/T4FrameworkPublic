// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4RehearsalEntityEditor.h"

#include "T4RehearsalEditorCommands.h"
#include "T4RehearsalEditorModule.h"

#include "Products/EntityEditor/ViewModel/T4EntityViewModel.h"
#include "Products/Common/ViewModel/T4PreviewEntityViewModel.h"

#include "Products/Common/Widgets/ST4EntityBrowserWidget.h" // #36
#include "Products/Common/Widgets/ST4ContiBrowserWidget.h" // #71

#include "Products/EntityEditor/DetailView/ST4EntityObjectWidget.h"
#include "Products/EntityEditor/DetailView/ST4AnimSetObjectsWidget.h" // #39

#include "Products/Common/Viewport/T4RehearsalViewport.h"
#include "Products/Common/Viewport/T4RehearsalViewportClient.h" // #94
#include "T4RehearsalEditorStyle.h"

#include "T4Asset/Public/T4AssetUtils.h" // #39, #62
#include "T4Asset/Classes/Entity/T4EntityAsset.h"
#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h" // #72

#include "Widgets/Docking/SDockTab.h"

#include "Subsystems/AssetEditorSubsystem.h"
#include "Toolkits/AssetEditorManager.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"

#include "T4RehearsalEditorInternal.h"

const FName FT4RehearsalEntityEditor::AppIdentifier = FName(TEXT("T4RehearsaleEntityEditorApp"));
const FName FT4RehearsalEntityEditor::PreviewTabId = FName(TEXT("PreviewViewport"));
const FName FT4RehearsalEntityEditor::EntityBrowserTabId = FName(TEXT("EntityBrowser")); // #36
const FName FT4RehearsalEntityEditor::MainViewportTabId = FName(TEXT("Viewport"));
const FName FT4RehearsalEntityEditor::EntityDetailTabId = FName(TEXT("EntityDetail"));
const FName FT4RehearsalEntityEditor::AnimSetDetailTabId = FName(TEXT("AnimSetDetail")); // #39
const FName FT4RehearsalEntityEditor::ContiBrowserTabId = FName(TEXT("ContiBrowser")); // #71

#define LOCTEXT_NAMESPACE "T4RehearsalEntityEditor"

/**
  *
 */
FT4RehearsalEntityEditor::FT4RehearsalEntityEditor()
{
}

FT4RehearsalEntityEditor::~FT4RehearsalEntityEditor()
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
	EntityBrowserPtr.Reset();
	ContiBrowserPtr.Reset(); // #71
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
}

void FT4RehearsalEntityEditor::InitializeWithEntity(
	const EToolkitMode::Type InMode,
	const TSharedPtr<IToolkitHost>& InInitToolkitHost,
	UT4EntityAsset* InEntityAsset
)
{
	EntityAssetPtr = InEntityAsset;
	EntityAssetPtr->SetFlags(RF_Transactional); // Undo, Redo

	const ET4EntityType EntityType = EntityAssetPtr->GetEntityType();

	const bool bIsUpdatable = false;
	const bool bIsLockable = false;
	const bool bAllowFavorites = true;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	const FDetailsViewArgs DetailsViewArgs(bIsUpdatable, bIsLockable, bAllowFavorites, FDetailsViewArgs::ObjectsUseNameArea, false);

	{
		FT4EntityViewModelOptions ViewModelOptions;
		ViewModelOptions.EntityAsset = InEntityAsset;
		ViewModelOptions.EntityEditor = this;
		MainViewModelPtr = MakeShareable(new FT4EntityViewModel(ViewModelOptions));

		// Editor
		//   ViewportClient : Camera or Input
		//     Viewport : Redering state
		//       PreviewScene : Rnedering View

		MainViewportPtr = SNew(ST4RehearsalViewport)
			.ViewModel(MainViewModelPtr.Get())
			.OnThumbnailCaptured(this, &FT4RehearsalEditor::HandleOnThumbnailCaptured);
	}

	{
		FT4PreviewEntityViewModelOptions ViewModelOptions;
		ViewModelOptions.EntityAsset = EntityAssetPtr.Get();

		PreviewViewModelPtr = MakeShareable(new FT4PreviewEntityViewModel(ViewModelOptions));
		PreviewViewportPtr = SNew(ST4RehearsalViewport)
			.ViewModel(PreviewViewModelPtr.Get())
			.OnThumbnailCaptured(this, &FT4RehearsalEditor::HandleOnThumbnailCaptured);
	}

	{
		// #72
		ST4EntityBrowserWidget::FT4OnSubMenuSelected EquipItemSubMeshDelegate = nullptr;
		ST4EntityBrowserWidget::FT4OnSubMenuSelected ExchangeItemSubMeshDelegate = nullptr;
		if (ET4EntityType::Character == EntityType)
		{
			UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(EntityAssetPtr.Get());
			check(nullptr != CharacterEntityAsset);
			// 현재는 Character Entity 타입만 Equip Item 메뉴 노출
			EquipItemSubMeshDelegate = ST4EntityBrowserWidget::FT4OnSubMenuSelected::CreateRaw(
				this,
				&FT4RehearsalEntityEditor::HandleOnEquipItem
			);
			if (ET4EntityCharacterMeshType::Composite == CharacterEntityAsset->MeshType) // Mesh 타입이 Composite 일 경우만 Exchange 메뉴 노출
			{
				ExchangeItemSubMeshDelegate = ST4EntityBrowserWidget::FT4OnSubMenuSelected::CreateRaw(
					this,
					&FT4RehearsalEntityEditor::HandleOnExchangeItem
				);
			}
		}
		EntityBrowserPtr = SNew(ST4EntityBrowserWidget)
			.OnSelectAsset(ST4EntityBrowserWidget::FT4OnSelectAsset::CreateRaw(this, &FT4RehearsalEntityEditor::HandleSelectAsset))
			.OnSpawnAsset(ST4EntityBrowserWidget::FT4OnSpawnAsset::CreateRaw(this, &FT4RehearsalEntityEditor::HandleSpawnAsset))
			.OnOpenAsset(ST4EntityBrowserWidget::FT4OnOpenAsset::CreateRaw(this, &FT4RehearsalEntityEditor::HandleOpenAsset))
			.OnSaveAsset(ST4EntityBrowserWidget::FT4OnSaveAsset::CreateRaw(this, &FT4RehearsalEntityEditor::HandleSaveAsset))
			.OnUpdateThumbnail(ST4EntityBrowserWidget::FT4OnUpdateThumbnail::CreateRaw(this, &FT4RehearsalEntityEditor::HandleOnUpdateThumbnail))
			.OnSavePreviewCameraInfo(ST4EntityBrowserWidget::FT4OnSavePreviewCameraInfo::CreateRaw(this, &FT4RehearsalEntityEditor::HandleOnSavePreviewCameraInfo))
			.OnDoubleClicked(ST4EntityBrowserWidget::FT4OnDoubleClicked::CreateRaw(this, &FT4RehearsalEntityEditor::HandleOnDoubleClickedEntityAsset))
			.OnEquipItemSubMenuSelected(EquipItemSubMeshDelegate) // #72
			.OnExchangeItemSubMenuSelected(ExchangeItemSubMeshDelegate); // #72
	}

	{
		// #71
		ContiBrowserPtr = SNew(ST4ContiBrowserWidget)
			.OnSelectAsset(ST4ContiBrowserWidget::FT4OnSelectAsset::CreateRaw(this, &FT4RehearsalEntityEditor::HandleSelectContiAsset))
			.OnDoubleClicked(ST4EntityBrowserWidget::FT4OnDoubleClicked::CreateRaw(this, &FT4RehearsalEntityEditor::HandleOpenContiAsset))
			.FilterEntityAsset(EntityAssetPtr.Get());
	}

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;

	if (ET4EntityType::Character == EntityType)
	{
		const TSharedRef<FTabManager::FLayout> EditorDefaultLayout = FTabManager::NewLayout("T4RehearsalEntity_Layout_v7")
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
						->SetSizeCoefficient(0.75f)
						->SetOrientation(Orient_Horizontal)
						->Split
						(
							FTabManager::NewSplitter()
							->SetSizeCoefficient(0.25f)
							->SetOrientation(Orient_Vertical)
							->Split
							(
								FTabManager::NewStack()
								->SetSizeCoefficient(0.3f)
								->AddTab(PreviewTabId, ETabState::OpenedTab)->SetHideTabWell(true)
							)
							->Split
							(
								FTabManager::NewStack()
								->SetSizeCoefficient(0.5f)
								->AddTab(EntityBrowserTabId, ETabState::OpenedTab)->SetHideTabWell(true)
							)
							->Split
							(
								FTabManager::NewStack()
								->SetSizeCoefficient(0.2f)
								->AddTab(ContiBrowserTabId, ETabState::OpenedTab)->SetHideTabWell(true)
							)
						)
						->Split
						(
							FTabManager::NewStack()
							->SetSizeCoefficient(0.75f)
							->AddTab(MainViewportTabId, ETabState::OpenedTab)->SetHideTabWell(true)
						)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.25f)
						->AddTab(EntityDetailTabId, ETabState::OpenedTab)->SetHideTabWell(false)
						->AddTab(AnimSetDetailTabId, ETabState::OpenedTab)->SetHideTabWell(false) // #39
					)
				)
			);

		FAssetEditorToolkit::InitAssetEditor(
			InMode,
			InInitToolkitHost,
			AppIdentifier,
			EditorDefaultLayout,
			bCreateDefaultStandaloneMenu,
			bCreateDefaultToolbar,
			EntityAssetPtr.Get()
		);
	}
	else
	{
		const TSharedRef<FTabManager::FLayout> EditorWithoutAnimSetLayout = FTabManager::NewLayout("T4RehearsalEntity_NonPlayable_Layout_v1")
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
						->SetSizeCoefficient(0.75f)
						->SetOrientation(Orient_Horizontal)
						->Split
						(
							FTabManager::NewSplitter()
							->SetSizeCoefficient(0.25f)
							->SetOrientation(Orient_Vertical)
							->Split
							(
								FTabManager::NewStack()
								->SetSizeCoefficient(0.3f)
								->AddTab(PreviewTabId, ETabState::OpenedTab)->SetHideTabWell(true)
							)
							->Split
							(
								FTabManager::NewStack()
								->SetSizeCoefficient(0.7f)
								->AddTab(EntityBrowserTabId, ETabState::OpenedTab)->SetHideTabWell(true)
							)
						)
						->Split
						(
							FTabManager::NewStack()
							->SetSizeCoefficient(0.75f)
							->AddTab(MainViewportTabId, ETabState::OpenedTab)->SetHideTabWell(true)
						)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.25f)
						->AddTab(EntityDetailTabId, ETabState::OpenedTab)->SetHideTabWell(true)
					)
				)
			);

		FAssetEditorToolkit::InitAssetEditor(
			InMode,
			InInitToolkitHost,
			AppIdentifier,
			EditorWithoutAnimSetLayout,
			bCreateDefaultStandaloneMenu,
			bCreateDefaultToolbar,
			EntityAssetPtr.Get()
		);
	}

	SetupCommands();

	ExtendMenu();
	ExtendToolbar();
	RegenerateMenusAndToolbars();

	// #79
	{
		FT4RehearsalViewportClient* MainViewportClient = MainViewportPtr->GetViewportClient();
		if (nullptr != MainViewportClient) // #94
		{
			MainViewportClient->SetToolkitHost(GetToolkitHost());
			MainViewModelPtr->OnStartPlay(MainViewportClient);
		}
		PreviewViewModelPtr->OnStartPlay(PreviewViewportPtr->GetViewportClient());
	}
}

void FT4RehearsalEntityEditor::SetupCommands()
{
	TSharedRef<FUICommandList> CommandList = GetToolkitCommands();

	if (EntityAssetPtr.IsValid() && ET4EntityType::Map == EntityAssetPtr->GetEntityType()) // #79
	{
		CommandList->MapAction(
			FT4RehearsalEditorCommands::Get().SaveThumbnailImage,
			FExecuteAction::CreateSP(this, &FT4RehearsalEntityEditor::HandleOnSaveThumbnailImage)
		);
	}

	CommandList->MapAction(
		FT4RehearsalEditorCommands::Get().ReloadSpawnObject,
		FExecuteAction::CreateSP(this, &FT4RehearsalEntityEditor::HandleOnReload)
	);

	CommandList->MapAction(
		FT4RehearsalEditorCommands::Get().GoPreviewScene,
		FExecuteAction::CreateRaw(this, &FT4RehearsalEntityEditor::HandleOnGoPreviewScene)
	); // #87

	CommonSetupCommands(CommandList); // #68
}

void FT4RehearsalEntityEditor::ExtendMenu()
{

}

void FT4RehearsalEntityEditor::ExtendToolbar()
{
	struct Local
	{
		static void FillToolbar(
			FToolBarBuilder& InToolbarBuilder, 
			FT4RehearsalEntityEditor* InEntityEditor,
			bool bShowThumbnailButton // #79
		)
		{
			InToolbarBuilder.BeginSection("EntityEditorToolBar");
			{
				if (bShowThumbnailButton) // #79
				{
					InToolbarBuilder.AddToolBarButton(
						FT4RehearsalEditorCommands::Get().SaveThumbnailImage,
						NAME_None,
						LOCTEXT("T4EntityEditorGenerateThumbnail", "Thumbnail"),
						LOCTEXT("T4EntityEditorGenerateThumbnail_Tooltip", "Generate a thumbnail image."),
						FSlateIcon(FEditorStyle::GetStyleSetName(), "Cascade.SaveThumbnailImage")
					);

					InToolbarBuilder.AddSeparator();
				}

				InToolbarBuilder.AddToolBarButton(
					FT4RehearsalEditorCommands::Get().ReloadSpawnObject,
					NAME_None,
					LOCTEXT("T4EntityEditorReloadEntity", "Reload"),
					LOCTEXT("T4EntityEditorReloadEntity_Tooltip", "Reload Entity"),
					FSlateIcon(FEditorStyle::GetStyleSetName(), "Persona.ReimportAnimation"));

				InToolbarBuilder.AddSeparator();

				InEntityEditor->CommonExtendToolbar(InToolbarBuilder); // #68

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
		FToolBarExtensionDelegate::CreateStatic(
			&Local::FillToolbar, 
			this, 
			(ET4EntityType::Map == EntityAssetPtr->GetEntityType()) ? true : false
		)
	);

	AddToolbarExtender(ToolbarExtender);

	FT4RehearsalEditorModule& RehearsalEditorModule = FModuleManager::LoadModuleChecked<FT4RehearsalEditorModule>("T4RehearsalEditor");
	AddToolbarExtender(
		RehearsalEditorModule.GetToolBarExtensibilityManager()->GetAllExtenders(
			GetToolkitCommands(), GetEditingObjects()
		)
	);
}

void FT4RehearsalEntityEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = TabManager->AddLocalWorkspaceMenuCategory(
		LOCTEXT("T4EntityEditorTabSpawnersMenu", "T4RehearsalEntity Editor")
	);
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	TabManager->RegisterTabSpawner(PreviewTabId, FOnSpawnTab::CreateSP(this, &FT4RehearsalEntityEditor::SpawnTab_Preview))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.TabThumbnail_16px"));

	TabManager->RegisterTabSpawner(EntityBrowserTabId, FOnSpawnTab::CreateSP(this, &FT4RehearsalEntityEditor::SpawnTab_EntityBrowser))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.TabEntityBrowser_16px"));

	TabManager->RegisterTabSpawner(MainViewportTabId, FOnSpawnTab::CreateSP(this, &FT4RehearsalEntityEditor::SpawnTab_Viewport))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.TabMainViewport_16px"));

	TabManager->RegisterTabSpawner(EntityDetailTabId, FOnSpawnTab::CreateSP(this, &FT4RehearsalEntityEditor::SpawnTab_EntityDetail))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.TabDetail_16px"));

	bool IsPlayable = (EntityAssetPtr.IsValid()) ? (ET4EntityType::Character == EntityAssetPtr->GetEntityType()) : false;
	if (IsPlayable)
	{
		TabManager->RegisterTabSpawner(AnimSetDetailTabId, FOnSpawnTab::CreateSP(this, &FT4RehearsalEntityEditor::SpawnTab_AnimSetDetail))
			.SetGroup(WorkspaceMenuCategoryRef)
			.SetIcon(FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.TabAnimSetDetail_16px")); // #39

		TabManager->RegisterTabSpawner(ContiBrowserTabId, FOnSpawnTab::CreateSP(this, &FT4RehearsalEntityEditor::SpawnTab_ContiBrowser))
			.SetGroup(WorkspaceMenuCategoryRef)
			.SetIcon(FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.TabContiBrowser_16px")); // #71
	}
}

void FT4RehearsalEntityEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(PreviewTabId);
	InTabManager->UnregisterTabSpawner(EntityBrowserTabId);
	InTabManager->UnregisterTabSpawner(MainViewportTabId);
	InTabManager->UnregisterTabSpawner(EntityDetailTabId);

	bool IsPlayable = (EntityAssetPtr.IsValid()) ? (ET4EntityType::Character == EntityAssetPtr->GetEntityType()) : false;
	if (IsPlayable)
	{
		InTabManager->UnregisterTabSpawner(ContiBrowserTabId); // #71
		InTabManager->UnregisterTabSpawner(AnimSetDetailTabId); // #39
	}
}

FName FT4RehearsalEntityEditor::GetToolkitFName() const
{
	return FName("T4RehearsalEntity Editor");
}

FText FT4RehearsalEntityEditor::GetBaseToolkitName() const
{
	return LOCTEXT("T4EntityEditorToolKitName", "T4RehearsalEntity Editor");
}

FString FT4RehearsalEntityEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("T4EntityEditorWorldCentricTabPrefix", "T4Entity ").ToString();
}

FLinearColor FT4RehearsalEntityEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.0f, 0.0f, 0.2f, 0.5f);
}

void FT4RehearsalEntityEditor::DoUpdateThumbnail(UObject* InAsset)
{
	// #39
	if (MainViewportPtr.IsValid())
	{
		MainViewportPtr->CreateThumbnail(InAsset);
	}
}

ET4LayerType FT4RehearsalEntityEditor::GetLayerType() const // #68
{
	if (!MainViewModelPtr.IsValid())
	{
		return ET4LayerType::Max;
	}
	return MainViewModelPtr->GetLayerType();
}

bool FT4RehearsalEntityEditor::HasActionPlaybackController() const // #68
{
	if (!MainViewModelPtr.IsValid())
	{
		return false;
	}
	return MainViewModelPtr->HasActionPlaybackController();
}

UT4EditorActionPlaybackController* FT4RehearsalEntityEditor::GetActionPlaybackController() const // #68
{
	if (!MainViewModelPtr.IsValid())
	{
		return nullptr;
	}
	return MainViewModelPtr->GetActionPlaybackController();
}

bool FT4RehearsalEntityEditor::CanSaveAsset() const 
{
	return true;
}

void FT4RehearsalEntityEditor::SaveAsset_Execute()
{
	if (!EntityAssetPtr.IsValid())
	{
		return;
	}
	if (MainViewModelPtr.IsValid())
	{
		FString ErrorMsg;
		MainViewModelPtr->DoSave(ErrorMsg); // #39
	}
}

bool FT4RehearsalEntityEditor::CanSaveAssetAs() const
{
	return false;
}

void FT4RehearsalEntityEditor::SaveAssetAs_Execute()
{
}

void FT4RehearsalEntityEditor::HandleOnSaveThumbnailImage() // #79
{
	if (MainViewportPtr.IsValid())
	{
		MainViewportPtr->CreateThumbnail(Cast<UObject>(EntityAssetPtr.Get()));
	}
}

void FT4RehearsalEntityEditor::HandleOnReload()
{
	if (MainViewModelPtr.IsValid())
	{
		MainViewModelPtr->ReloadPlayerSpawn();
	}
	if (PreviewViewModelPtr.IsValid())
	{
		PreviewViewModelPtr->HandleOnEntityPropertiesChanged();
	}
}

void FT4RehearsalEntityEditor::HandleSelectAsset(UObject* InAsset)
{
	if (PreviewViewModelPtr.IsValid())
	{
		PreviewViewModelPtr->OnChangeViewTarget(InAsset, 5.0f);
	}
}

void FT4RehearsalEntityEditor::HandleSpawnAsset(UObject* InAsset)
{
	UT4EntityAsset* SpawnEntityAsset = Cast<UT4EntityAsset>(InAsset);
	if (nullptr != SpawnEntityAsset)
	{
		MainViewModelPtr->DoEntitySpawn(SpawnEntityAsset);
	}
}

void FT4RehearsalEntityEditor::HandleOpenAsset(UObject* InAsset)
{
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(InAsset);
}

void FT4RehearsalEntityEditor::HandleSaveAsset(UObject* InAsset)
{
	T4AssetUtil::SaveAsset(InAsset, true); // #73
}

void FT4RehearsalEntityEditor::HandleOnUpdateThumbnail(UObject* InAsset)
{
	if (PreviewViewportPtr.IsValid())
	{
		PreviewViewportPtr->CreateThumbnail(InAsset);
	}
	HandleOnSavePreviewCameraInfo(); // 썸네일을 찍으면 해당 위치를 저장해둔다.
}

void FT4RehearsalEntityEditor::HandleOnSavePreviewCameraInfo()
{
	if (!PreviewViewportPtr.IsValid())
	{
		return;
	}
	PreviewViewModelPtr->SaveThumbnailCameraInfo();
}

void FT4RehearsalEntityEditor::HandleOnDoubleClickedEntityAsset(UObject* InAsset)
{
	UT4EntityAsset* ConvertEntityAsset = Cast<UT4EntityAsset>(InAsset);
	if (nullptr != ConvertEntityAsset)
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(InAsset);
	}
}

void FT4RehearsalEntityEditor::HandleOnEquipItem(
	UObject* InAsset, 
	FName InActionPointName,
	bool bEquip
) // #72
{
	if (!MainViewModelPtr.IsValid())
	{
		return;
	}
	UT4EntityAsset* ConvertEntityAsset = Cast<UT4EntityAsset>(InAsset);
	if (nullptr != ConvertEntityAsset)
	{
		MainViewModelPtr->ClientEquipWeapon(ConvertEntityAsset, InActionPointName, bEquip);
	}
}

void FT4RehearsalEntityEditor::HandleOnExchangeItem(
	UObject* InAsset, 
	FName InCompositePartName,
	bool bSet
) // #72
{
	if (!MainViewModelPtr.IsValid())
	{
		return;
	}
	UT4EntityAsset* ConvertEntityAsset = Cast<UT4EntityAsset>(InAsset);
	if (nullptr != ConvertEntityAsset)
	{
		MainViewModelPtr->ClientExchangeCostume(ConvertEntityAsset, InCompositePartName, bSet);
	}
}

void FT4RehearsalEntityEditor::HandleSelectContiAsset(UObject* InAsset) // #71
{
	if (!MainViewModelPtr.IsValid())
	{
		return;
	}
	UT4ContiAsset* HandleContiAsset = Cast<UT4ContiAsset>(InAsset);
	if (nullptr != HandleContiAsset)
	{
		MainViewModelPtr->ClientPlayConti(HandleContiAsset, nullptr);
	}
}

void FT4RehearsalEntityEditor::HandleOpenContiAsset(UObject* InAsset) // #71
{
	UT4ContiAsset* HandleContiAsset = Cast<UT4ContiAsset>(InAsset);
	if (nullptr != HandleContiAsset)
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(InAsset);
	}
}

void FT4RehearsalEntityEditor::HandleOnGoPreviewScene() // #87
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

TSharedRef<SDockTab> FT4RehearsalEntityEditor::SpawnTab_Preview(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == PreviewTabId);
	return SNew(SDockTab)
		[
			PreviewViewportPtr.ToSharedRef()
		];
}

TSharedRef<SDockTab> FT4RehearsalEntityEditor::SpawnTab_EntityBrowser(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == EntityBrowserTabId);
	return SNew(SDockTab)
		[
			EntityBrowserPtr.ToSharedRef()
		];
}

TSharedRef<SDockTab> FT4RehearsalEntityEditor::SpawnTab_ContiBrowser(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == ContiBrowserTabId);
	return SNew(SDockTab)
		[
			ContiBrowserPtr.ToSharedRef()
		];
}

TSharedRef<SDockTab> FT4RehearsalEntityEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == MainViewportTabId);
	return SNew(SDockTab)
		[
			MainViewportPtr.ToSharedRef()
		];
}

TSharedRef<SDockTab> FT4RehearsalEntityEditor::SpawnTab_EntityDetail(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == EntityDetailTabId);
	TSharedRef<SDockTab> SpawnedTab =
		SNew(SDockTab)
		[
			SNew(ST4EntityObjectWidget, MainViewModelPtr)
		];

	return SpawnedTab;
}

TSharedRef<SDockTab> FT4RehearsalEntityEditor::SpawnTab_AnimSetDetail(const FSpawnTabArgs& Args)
{
	// #39
	check(Args.GetTabId() == AnimSetDetailTabId);
	TSharedRef<SDockTab> SpawnedTab =
		SNew(SDockTab)
		[
			SNew(ST4AnimSetObjectsWidget, MainViewModelPtr)
		];

	return SpawnedTab;
}

#undef LOCTEXT_NAMESPACE