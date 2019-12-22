// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4RehearsalWorldEditor.h"

#include "T4RehearsalEditorCommands.h"
#include "T4RehearsalEditorModule.h"

#include "Products/Common/ViewModel/T4WorldPreviewViewModel.h"

#include "Products/WorldEditor/ViewModel/T4WorldViewModel.h"

#include "Products/WorldEditor/DetailView/ST4WorldObjectWidget.h"
#include "Products/WorldEditor/DetailView/ST4WorldPreviewObjectWidget.h" // #85

#include "Products/Common/Viewport/T4RehearsalViewport.h"

#include "T4RehearsalEditorStyle.h"

#include "T4Asset/Public/T4AssetUtils.h" // #39, #62
#include "T4Asset/Classes/World/T4WorldAsset.h"

#include "Subsystems/AssetEditorSubsystem.h" // #4.24
#include "Toolkits/AssetEditorManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "FileHelpers.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"

#include "T4RehearsalEditorInternal.h"

const FName FT4RehearsalWorldEditor::AppIdentifier = FName(TEXT("T4RehearsaleWorldEditorApp"));
const FName FT4RehearsalWorldEditor::PreviewViewportTabId = FName(TEXT("PreviewViewport"));
const FName FT4RehearsalWorldEditor::PreviewDetailTabId = FName(TEXT("PreviewDetail")); // #85
const FName FT4RehearsalWorldEditor::WorldMapTabId = FName(TEXT("WorldMap"));
const FName FT4RehearsalWorldEditor::WorldDetailTabId = FName(TEXT("WorldDetail"));

#define LOCTEXT_NAMESPACE "T4RehearsalWorldEditor"

/**
  * #83
 */
FT4RehearsalWorldEditor::FT4RehearsalWorldEditor()
	: bInitializing(false)
	, bUpdatePreviewSubLevels(false)
{
}

FT4RehearsalWorldEditor::~FT4RehearsalWorldEditor()
{
	CleanUp();
}

void FT4RehearsalWorldEditor::CleanUp() // #90
{
	if (PreviewViewportPtr.IsValid())
	{
		PreviewViewportPtr->OnCleanup(); // #97 : 안전하게 Viewport 먼저 삭제한다. World 는 ViewportClient 와 완전히 분리되었다.
		PreviewViewportPtr.Reset();
	}
	if (WorldObjectDetailsPtr.IsValid())
	{
		WorldObjectDetailsPtr.Reset();
	}
	if (PreviewObjectDetailsPtr.IsValid()) // #85
	{
		PreviewObjectDetailsPtr.Reset();
	}
	if (PreviewViewModelPtr.IsValid())
	{
		PreviewViewModelPtr->OnCleanup();
		PreviewViewModelPtr.Reset();
	}
	if (WorldViewModelPtr.IsValid())
	{
		WorldViewModelPtr->OnCleanup();
		WorldViewModelPtr.Reset();
	}
	WorldAssetPtr.Reset();
}

void FT4RehearsalWorldEditor::InitializeWithWorld(
	const EToolkitMode::Type InMode,
	const TSharedPtr<IToolkitHost>& InInitToolkitHost,
	UT4WorldAsset* InWorldAsset
)
{
	// #83, #104 : World 툴이 오픈된 상황에서 일반 레벨을 로드하거나 다른 World 를 열 경우 현재의
	//             World Editor 를 Close 하도록 처리한다.
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OnAssetEditorRequestedOpen().AddSP(
		this,
		&FT4RehearsalWorldEditor::HandleOnAssetEditorRequestedOpen
	);

	bInitializing = true;

	WorldAssetPtr = InWorldAsset;
	WorldAssetPtr->SetFlags(RF_Transactional); // Undo, Redo

	if (!InWorldAsset->MapEntityAsset.IsNull())
	{
		// #83 : 편집을 위해 Editor World 를 열어준다.
		UT4MapEntityAsset* MapEntityAsset = InWorldAsset->MapEntityAsset.LoadSynchronous();
		if (nullptr != MapEntityAsset)
		{
			if (!MapEntityAsset->MapData.LevelAsset.IsNull())
			{
				FWorldContext& EditorContext = GEditor->GetEditorWorldContext();
				const FString EditorWorldPackageName = EditorContext.World()->GetOutermost()->GetName();
				{
					UWorld* World = MapEntityAsset->MapData.LevelAsset.LoadSynchronous();
					check(nullptr != World);
					const FString WorldAssetPackageName = World->GetOutermost()->GetName();
					if (EditorWorldPackageName != WorldAssetPackageName)
					{
						GEditor->EditObject(World);
					}
				}

			}
		}
	}

	const bool bIsUpdatable = false;
	const bool bIsLockable = false;
	const bool bAllowFavorites = true;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	const FDetailsViewArgs DetailsViewArgs(bIsUpdatable, bIsLockable, bAllowFavorites, FDetailsViewArgs::ObjectsUseNameArea, false);

	{
		FT4WorldPreviewViewModelOptions ViewModelOptions;
		ViewModelOptions.MapEntityAsset = (!InWorldAsset->MapEntityAsset.IsNull()) ? InWorldAsset->MapEntityAsset.LoadSynchronous() : nullptr;
		ViewModelOptions.ActionPlaybackAssetName = InWorldAsset->GetName(); // #104
		ViewModelOptions.ActionPlaybackFolderName = TEXT("World"); // #104

		PreviewViewModelPtr = MakeShareable(new FT4WorldPreviewViewModel(ViewModelOptions));
		PreviewViewModelPtr->GetOnSubLevelUpdate().AddRaw(this, &FT4RehearsalWorldEditor::HandleOnSubLevelAddOrRemovePreviewWorld);

		PreviewViewportPtr = SNew(ST4RehearsalViewport)
			.ViewModel(PreviewViewModelPtr.Get())
			.OnThumbnailCaptured(this, &FT4RehearsalEditor::HandleOnThumbnailCaptured)
			.OnRefreshButtonClicked(ST4RehearsalViewport::FT4OnOverlayButtonCallback::CreateRaw(this, &FT4RehearsalWorldEditor::HandleOnRefreshPreviewWorld)) // #86
			.OnSimulationButtonClicked(ST4RehearsalViewport::FT4OnOverlayButtonCallback::CreateRaw(this, &FT4RehearsalWorldEditor::HandleToggleSimulation)); // #86

		// #79
		PreviewViewModelPtr->OnStartPlay(PreviewViewportPtr->GetViewportClient());
	}

	{
		FT4WorldViewModelOptions ModelOptions;
		ModelOptions.WorldAsset = InWorldAsset;
		ModelOptions.PreviewViewModelPtr = PreviewViewModelPtr; // #85
		WorldViewModelPtr = MakeShareable(new FT4WorldViewModel(ModelOptions));
		WorldViewModelPtr->GetOnSubLevelSelection().AddRaw(this, &FT4RehearsalWorldEditor::HandleOnEditorSubLevelSelection);
		WorldViewModelPtr->GetOnSubLevelChanged().AddRaw(this, &FT4RehearsalWorldEditor::HandleOnSubLevelChanged);
		WorldViewModelPtr->GetOnEditorSubLevelChanged().AddRaw(this, &FT4RehearsalWorldEditor::HandleOnEditorSubLevelChanged);
		WorldViewModelPtr->GetOnWorldEditorRefresh().BindRaw(this, &FT4RehearsalWorldEditor::HandleOnWorldEditorRefresh); // #90
		//WorldViewModelPtr->OnStartPlay(nullptr); // #86 : WorldViewModel 은 Viewport Clinet 를 사용하지 않아 호출하면 안된다.
	}

	{
		WorldObjectDetailsPtr = SNew(ST4WorldObjectWidget, WorldViewModelPtr); // #85
		PreviewObjectDetailsPtr = SNew(ST4WorldPreviewObjectWidget, WorldViewModelPtr); // #85
	}

	TSharedPtr<FTabManager::FLayout> EditorDefaultLayout;

	{
		EditorDefaultLayout = FTabManager::NewLayout("T4RehearsalWorld_WorldLayout_v2")
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
					->SetSizeCoefficient(0.25f)
					->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewStack()
						->AddTab(PreviewDetailTabId, ETabState::OpenedTab)->SetHideTabWell(true)
					)
				)
				->Split
				(
					FTabManager::NewSplitter()
					->SetSizeCoefficient(0.5f)
					->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.5f)
						->AddTab(PreviewViewportTabId, ETabState::OpenedTab)->SetHideTabWell(true)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.5f)
						->AddTab(WorldMapTabId, ETabState::OpenedTab)->SetHideTabWell(true)
					)
				)
				->Split
				(
					FTabManager::NewSplitter()
					->SetSizeCoefficient(0.25f)
					->SetOrientation(Orient_Vertical)
					->Split
					(
						FTabManager::NewStack()
						->AddTab(WorldDetailTabId, ETabState::OpenedTab)->SetHideTabWell(false)
					)					
				)
			)
		);
	}

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	FAssetEditorToolkit::InitAssetEditor(
		InMode, 
		InInitToolkitHost,
		AppIdentifier, 
		EditorDefaultLayout.ToSharedRef(), 
		bCreateDefaultStandaloneMenu, 
		bCreateDefaultToolbar, 
		WorldAssetPtr.Get()
	);

	SetupCommands();

	ExtendMenu();
	ExtendToolbar();
	RegenerateMenusAndToolbars();

	bInitializing = false;
}

void FT4RehearsalWorldEditor::Tick(float DeltaTime)
{
	if (bUpdatePreviewSubLevels)
	{
		if (WorldViewModelPtr.IsValid())
		{
			WorldViewModelPtr->UpdatePreviewWorldSubLevel();
		}
		if (PreviewObjectDetailsPtr.IsValid())
		{
			PreviewObjectDetailsPtr->OnRefreshWorld(); // #85 : SubLevel 정보를 업데이트 해준다.
		}
		bUpdatePreviewSubLevels = false;
	}
}

TStatId FT4RehearsalWorldEditor::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FT4RehearsalWorldEditor, STATGROUP_Tickables);
}

void FT4RehearsalWorldEditor::SetupCommands()
{
	TSharedRef<FUICommandList> CommandList = GetToolkitCommands();

	CommandList->MapAction(
		FT4RehearsalEditorCommands::Get().SaveThumbnailImage,
		FExecuteAction::CreateSP(this, &FT4RehearsalWorldEditor::HandleOnSaveThumbnailImage)
	);
	CommandList->MapAction(
		FT4RehearsalEditorCommands::Get().SaveAllModifiedLevels,
		FExecuteAction::CreateSP(this, &FT4RehearsalWorldEditor::HandleOnSaveAllModifiedLevels)
	); // #86

	CommonSetupCommands(CommandList); // #68
}

void FT4RehearsalWorldEditor::ExtendMenu()
{

}

void FT4RehearsalWorldEditor::ExtendToolbar()
{
	struct Local
	{
		static void FillToolbar(
			FToolBarBuilder& InToolbarBuilder, 
			FT4RehearsalWorldEditor* InWorldEditor
		)
		{
			InToolbarBuilder.BeginSection("WorldEditorToolBar");
			{
				InToolbarBuilder.AddToolBarButton(
					FT4RehearsalEditorCommands::Get().SaveThumbnailImage,
					NAME_None,
					LOCTEXT("T4WorldEditorGenerateThumbnail", "Thumbnail"),
					LOCTEXT("T4WorldEditorGenerateThumbnail_Tooltip", "Generate a thumbnail image."),
					FSlateIcon(FEditorStyle::GetStyleSetName(), "Cascade.SaveThumbnailImage")
				);

				InToolbarBuilder.AddSeparator();

				InToolbarBuilder.AddToolBarButton(
					FT4RehearsalEditorCommands::Get().SaveAllModifiedLevels,
					NAME_None,
					LOCTEXT("T4WorldEditorSaveAllModified", "Save All"),
					LOCTEXT("T4WorldEditorSaveAllModified_Tooltip", "Save all modified assets."),
					FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.WorldEditorSaveAllModified_40x")
				); // #86

				InToolbarBuilder.AddSeparator();

				InWorldEditor->CommonExtendToolbar(InToolbarBuilder); // #68
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
			this
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

void FT4RehearsalWorldEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = TabManager->AddLocalWorkspaceMenuCategory(
		LOCTEXT("T4WorldEditorTabSpawnersMenu", "T4RehearsalWorld Editor")
	);
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	TabManager->RegisterTabSpawner(PreviewViewportTabId, FOnSpawnTab::CreateSP(this, &FT4RehearsalWorldEditor::SpawnTab_PreviewViewport))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.TabMainViewport_16px"));

	TabManager->RegisterTabSpawner(WorldMapTabId, FOnSpawnTab::CreateSP(this, &FT4RehearsalWorldEditor::SpawnTab_WorldMap))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.TabMainViewport_16px"));

	TabManager->RegisterTabSpawner(WorldDetailTabId, FOnSpawnTab::CreateSP(this, &FT4RehearsalWorldEditor::SpawnTab_WorldDetail))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.TabDetail_16px"));

	TabManager->RegisterTabSpawner(PreviewDetailTabId, FOnSpawnTab::CreateSP(this, &FT4RehearsalWorldEditor::SpawnTab_PreviewDetail))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.TabDetail_16px"));
}

void FT4RehearsalWorldEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(PreviewViewportTabId);
	InTabManager->UnregisterTabSpawner(WorldMapTabId);
	InTabManager->UnregisterTabSpawner(WorldDetailTabId);
	InTabManager->UnregisterTabSpawner(PreviewDetailTabId);
}

FName FT4RehearsalWorldEditor::GetToolkitFName() const
{
	return FName("T4RehearsalWorld Editor");
}

FText FT4RehearsalWorldEditor::GetBaseToolkitName() const
{
	return LOCTEXT("T4WorldEditorToolKitName", "T4RehearsalWorld Editor");
}

FString FT4RehearsalWorldEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("T4WorldEditorWorldCentricTabPrefix", "T4World ").ToString();
}

FLinearColor FT4RehearsalWorldEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.0f, 0.0f, 0.2f, 0.5f);
}

void FT4RehearsalWorldEditor::DoUpdateThumbnail(UObject* InAsset)
{
	// #39
	if (PreviewViewportPtr.IsValid())
	{
		PreviewViewportPtr->CreateThumbnail(InAsset);
	}
}

bool FT4RehearsalWorldEditor::IsWorldCompositionEnabled() const // #91 : World Single
{
	if (!WorldViewModelPtr.IsValid())
	{
		return false;
	}
	return WorldViewModelPtr->IsWorldCompositionEnabled();
}

void FT4RehearsalWorldEditor::HandleOnAssetEditorRequestedOpen(UObject* InWorldObject) // #104
{
	// #83, #104 : World 툴이 오픈된 상황에서 일반 레벨을 로드하거나 다른 World 를 열 경우 현재의
	//             World Editor 를 Close 하도록 처리한다.
	if (nullptr == Cast<UWorld>(InWorldObject)) // 월드가 아니면 처리하지 않는다.
	{
		return;
	}
	if (bInitializing)
	{
		return;
	}
	// World 툴이 오픈된 상황에서 일반 레벨을 로드할 경우 World Editor 를 강제로 닫는다.
	CleanUp();
	GEngine->TrimMemory();
	CloseWindow();
}

void FT4RehearsalWorldEditor::HandleOnSaveThumbnailImage()
{
	if (PreviewViewportPtr.IsValid())
	{
		PreviewViewportPtr->CreateThumbnail(Cast<UObject>(WorldAssetPtr.Get()));
	}
}

void FT4RehearsalWorldEditor::HandleOnSaveAllModifiedLevels()
{
	if (PreviewViewModelPtr.IsValid())
	{
		// #86 : Editor World 편집 이슈로 Preview World 를 리로드 해준다.
		PreviewViewModelPtr->OnRefreshWorld();
	}
	if (PreviewObjectDetailsPtr.IsValid())
	{
		PreviewObjectDetailsPtr->OnRefreshWorld(); // #86 : SubLevel 정보도 업데이트 해준다.
	}
	if (WorldViewModelPtr.IsValid())
	{
		// #86 : Editor World 편집 이슈로 Preview World 에서 로드한 SubLevel 을 리로드 처리해준다.
		//       WorldViewModelPtr 의 다음 Tick 에 복구 메세지가 간다.
		WorldViewModelPtr->ResetSubLevelSelection();
	}

	const bool bPromptUserToSave = true;
	const bool bSaveMapPackages = true;
	const bool bSaveContentPackages = true;
	const bool bFastSave = false;
	const bool bNotifyNoPackagesSaved = false;
	const bool bCanBeDeclined = false;
	FEditorFileUtils::SaveDirtyPackages(
		bPromptUserToSave, 
		bSaveMapPackages, 
		bSaveContentPackages, 
		bFastSave, 
		bNotifyNoPackagesSaved, 
		bCanBeDeclined
	);
}

void FT4RehearsalWorldEditor::HandleOnSubLevelAddOrRemovePreviewWorld() // #86
{
	bUpdatePreviewSubLevels = true; // N개 만큼 Notify 가 오기 때문에 모아서 업데이트 해준다.
}

void FT4RehearsalWorldEditor::HandleOnRefreshPreviewWorld() // #86
{
	if (PreviewViewModelPtr.IsValid())
	{
		PreviewViewModelPtr->OnRefreshWorld();
	}
	if (PreviewObjectDetailsPtr.IsValid())
	{
		PreviewObjectDetailsPtr->OnRefreshWorld(); // #85 : SubLevel 정보를 업데이트 해준다.
	}
}

void FT4RehearsalWorldEditor::HandleToggleSimulation() // #86
{
	if (PreviewViewModelPtr.IsValid())
	{
		PreviewViewModelPtr->OnToggleSimulation();
	}
	if (WorldViewModelPtr.IsValid())
	{
		WorldViewModelPtr->ToggleSimulation();
	}
}

void FT4RehearsalWorldEditor::HandleOnEditorSubLevelSelection(
	const TArray<FName>& InSubLevelPackageNames,
	bool bFlushLevelStreaming
) // #86
{
	if (PreviewViewModelPtr.IsValid())
	{
		PreviewViewModelPtr->OnSubLevelSelection(InSubLevelPackageNames, bFlushLevelStreaming);
	}
	HandleOnEditorSubLevelChanged(); // #104 : Editor 의 SubLevel 에 표시
}

void FT4RehearsalWorldEditor::HandleOnSubLevelChanged() // #86
{
	if (PreviewViewModelPtr.IsValid())
	{
		PreviewViewModelPtr->OnDisplayEditorWorldModified();
	}
}

void FT4RehearsalWorldEditor::HandleOnEditorSubLevelChanged() // #104
{
	if (WorldObjectDetailsPtr.IsValid())
	{
		WorldObjectDetailsPtr->OnRefreshWorld();
	}
}

void FT4RehearsalWorldEditor::HandleOnWorldEditorRefresh() // #90
{
	UT4WorldAsset* CurrentWorldAsset = WorldAssetPtr.Get();
	CleanUp();
	GEngine->TrimMemory();
	CloseWindow();
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(CurrentWorldAsset); // #4.24
}

ET4LayerType FT4RehearsalWorldEditor::GetLayerType() const // #68
{
	if (!PreviewViewModelPtr.IsValid())// WARN : World Editor 는 PreviewWorld 를 ActionPlayback 으로 사용한다.
	{
		return ET4LayerType::Max;
	}
	return PreviewViewModelPtr->GetLayerType();
}

IT4RehearsalViewModel* FT4RehearsalWorldEditor::GetMainViewModelInterface() const // #104
{
	if (!PreviewViewModelPtr.IsValid())// WARN : World Editor 는 PreviewWorld 를 ActionPlayback 으로 사용한다.
	{
		return nullptr;
	}
	return PreviewViewModelPtr.Get();
}

bool FT4RehearsalWorldEditor::HasActionPlaybackController() const // #68
{
	if (!PreviewViewModelPtr.IsValid())// WARN : World Editor 는 PreviewWorld 를 ActionPlayback 으로 사용한다.
	{
		return false;
	}
	return PreviewViewModelPtr->HasActionPlaybackController();
}

UT4EditorActionPlaybackController* FT4RehearsalWorldEditor::GetActionPlaybackController() const // #68
{
	if (!PreviewViewModelPtr.IsValid()) // WARN : World Editor 는 PreviewWorld 를 ActionPlayback 으로 사용한다.
	{
		return nullptr;
	}
	return PreviewViewModelPtr->GetActionPlaybackController();
}

TSharedRef<SDockTab> FT4RehearsalWorldEditor::SpawnTab_PreviewViewport(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == PreviewViewportTabId);
	return SNew(SDockTab)
		[
			PreviewViewportPtr.ToSharedRef()
		];
}


TSharedRef<SDockTab> FT4RehearsalWorldEditor::SpawnTab_WorldMap(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == WorldMapTabId);
	if (!WorldViewModelPtr.IsValid())
	{
		return SNew(SDockTab);
	}
	return SNew(SDockTab)
		[
			WorldViewModelPtr->GetWorldMapRef()
		];
}

TSharedRef<SDockTab> FT4RehearsalWorldEditor::SpawnTab_WorldDetail(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == WorldDetailTabId);
	TSharedRef<SDockTab> SpawnedTab =
		SNew(SDockTab)
		[
			WorldObjectDetailsPtr.ToSharedRef() // #85
		];

	return SpawnedTab;
}

TSharedRef<SDockTab> FT4RehearsalWorldEditor::SpawnTab_PreviewDetail(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == PreviewDetailTabId);
	TSharedRef<SDockTab> SpawnedTab =
		SNew(SDockTab)
		[
			PreviewObjectDetailsPtr.ToSharedRef() // #85
		];

	return SpawnedTab;
}

#undef LOCTEXT_NAMESPACE