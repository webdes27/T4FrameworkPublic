// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Products/T4RehearsalEditor.h"

#include "T4RehearsalEditorCommands.h"
#include "Products/T4RehearsalEditorUtils.h"

#include "Common/ViewModel/T4RehearsalViewModel.h" // #104
#include "Common/Helper/T4EditorActionPlaybackController.h" // #68

#include "T4Asset/Public/T4AssetUtils.h" // #39, #62
#include "T4Engine/Classes/Playback/T4ActionPlaybackAsset.h" // #68

#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Framework/Application/SlateApplication.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4RehearsalEditor"

/**
  * 
 */
FT4RehearsalEditor::FT4RehearsalEditor()
{
}

FT4RehearsalEditor::~FT4RehearsalEditor()
{
}

void FT4RehearsalEditor::CommonSetupCommands(TSharedRef<FUICommandList> InCommandList)
{
	// #68
	InCommandList->MapAction(
		FT4RehearsalEditorCommands::Get().ActionPlaybackRec,
		FExecuteAction::CreateRaw(this, &FT4RehearsalEditor::HandleOnActionPlaybackRec),
		FCanExecuteAction::CreateRaw(this, &FT4RehearsalEditor::HandleOnCanExecuteActionPlaybackRec)
	);
	InCommandList->MapAction(
		FT4RehearsalEditorCommands::Get().ActionPlaybackPlay,
		FExecuteAction::CreateRaw(this, &FT4RehearsalEditor::HandleOnActionPlaybackPlay),
		FCanExecuteAction::CreateRaw(this, &FT4RehearsalEditor::HandleOnCanExecuteActionPlaybackPlay)
	);
	InCommandList->MapAction(
		FT4RehearsalEditorCommands::Get().ActionPlaybackPause,
		FExecuteAction::CreateRaw(this, &FT4RehearsalEditor::HandleOnActionPlaybackPause),
		FCanExecuteAction::CreateRaw(this, &FT4RehearsalEditor::HandleOnCanExecuteActionPlaybackPause)
	);
	InCommandList->MapAction(
		FT4RehearsalEditorCommands::Get().ActionPlaybackStop,
		FExecuteAction::CreateRaw(this, &FT4RehearsalEditor::HandleOnActionPlaybackStop),
		FCanExecuteAction::CreateRaw(this, &FT4RehearsalEditor::HandleOnCanExecuteActionPlaybackStop)
	);

	InCommandList->MapAction(
		FT4RehearsalEditorCommands::Get().DespawnAll,
		FExecuteAction::CreateRaw(this, &FT4RehearsalEditor::HandleOnDespawnAll)
	);
	// ~#68
}

void FT4RehearsalEditor::CommonExtendToolbar(FToolBarBuilder& InToolbarBuilder)
{
	// #68
	InToolbarBuilder.AddToolBarButton(
		FT4RehearsalEditorCommands::Get().ActionPlaybackRec,
		NAME_None,
		LOCTEXT("T4RehearsalEditorActionPlaybackRec", "Rec"),
		LOCTEXT("T4RehearsalEditorActionPlaybackRec_Tooltip", "Action Playback Rec"),
		FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.ToolbarActionPlaybackRec_40x")
	);

	InToolbarBuilder.AddToolBarButton(
		FT4RehearsalEditorCommands::Get().ActionPlaybackPlay,
		NAME_None,
		LOCTEXT("T4RehearsalEditorActionPlaybackPlay", "Play"),
		LOCTEXT("T4RehearsalEditorActionPlaybackPlay_Tooltip", "Action Playback Play"),
		FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.ToolbarActionPlaybackPlay_40x")
	);

	InToolbarBuilder.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateRaw(this, &FT4RehearsalEditor::HandleOnActionPlaybackPlayMenu),
		LOCTEXT("T4RehearsalEditorActionPlaybackPlayMenu", "Play Menu"),
		LOCTEXT("T4RehearsalEditorActionPlaybackPlayMenu_Tooltip", "Action Playback Play"),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.RepeatLastLaunch"),
		true
	);

	InToolbarBuilder.AddToolBarButton(
		FT4RehearsalEditorCommands::Get().ActionPlaybackPause,
		NAME_None,
		LOCTEXT("T4RehearsalEditorActionPlaybackPause", "Pause"),
		LOCTEXT("T4RehearsalEditorActionPlaybackPause_Tooltip", "Action Playback Pause"),
		FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.ToolbarActionPlaybackPause_40x")
	);

	InToolbarBuilder.AddToolBarButton(
		FT4RehearsalEditorCommands::Get().ActionPlaybackStop,
		NAME_None,
		LOCTEXT("T4RehearsalEditorActionPlaybackStop", "Stop"),
		LOCTEXT("T4RehearsalEditorActionPlaybackStop_Tooltip", "Action Playback 'Play or Rec' Stop"),
		FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.ToolbarActionPlaybackStop_40x")
	);

	InToolbarBuilder.AddSeparator();

	InToolbarBuilder.AddToolBarButton(
		FT4RehearsalEditorCommands::Get().DespawnAll,
		NAME_None,
		LOCTEXT("T4RehearsalEditorDespawnAll", "DespawnAll"),
		LOCTEXT("T4RehearsalEditorDespawnAll_Tooltip", "Leave all SpawnObjects"),
		FSlateIcon(FT4RehearsalEditorStyle::GetStyleSetName(), "T4RehearsalEditorStyle.ToolbarDespawnAll_40x")
	);
	// ~#68
}

void FT4RehearsalEditor::HandleOnThumbnailCaptured(UObject* InOwner, UTexture2D* InThumbnail)
{
	T4AssetUtil::SaveThumbnailImage(InOwner, InThumbnail);
}

TSharedRef<SWidget> FT4RehearsalEditor::HandleOnActionPlaybackPlayMenu()
{
	const bool bShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, nullptr);

	MenuBuilder.BeginSection("T4ActionPlaybackPlayOptions", LOCTEXT("T4ActionPlaybackPlayOptionsSection", "Play Options"));
	{
		FUIAction ActionPlaybackRepeatAction(
			FExecuteAction::CreateRaw(this, &FT4RehearsalEditor::HandleOnToogleActionPlaybackRepeatEnable),
			FCanExecuteAction(),
			FIsActionChecked::CreateRaw(this, &FT4RehearsalEditor::HandleOnIsActionPlaybackRepeatEnabled)
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("T4ActionPlaybackPlayRepeat", "Repeat Enabled"),
			LOCTEXT("T4ActionPlaybackPlayRepeat_Tooltip", "If Enabled, Action Playback Repeat Enabled."),
			FSlateIcon(),
			ActionPlaybackRepeatAction,
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
		FUIAction ActionPlaybackPlayerPossessedAction(
			FExecuteAction::CreateRaw(this, &FT4RehearsalEditor::HandleOnToogleActionPlaybackPlayerPossessed),
			FCanExecuteAction(),
			FIsActionChecked::CreateRaw(this, &FT4RehearsalEditor::HandleOnIsActionPlaybackPlayerPossessed)
		);
		MenuBuilder.AddMenuEntry(
			LOCTEXT("T4ActionPlaybackPlayPlayerPossessed", "Player Possessed"),
			LOCTEXT("T4ActionPlaybackPlayPlayerPossessed_Tooltip", "If Enabled, Action Playback Player Possessed."),
			FSlateIcon(),
			ActionPlaybackPlayerPossessedAction,
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
	}
	MenuBuilder.EndSection();

	FAssetData SelectAssetData;
	UT4EditorActionPlaybackController* EditorActionPlaybackController = GetActionPlaybackController();
	if (nullptr != EditorActionPlaybackController)
	{
		if (!EditorActionPlaybackController->SelectActionPlaybackAsset.IsNull())
		{
			UT4ActionPlaybackAsset* ActionPlaybackAsset = EditorActionPlaybackController->SelectActionPlaybackAsset.LoadSynchronous();
			if (nullptr != ActionPlaybackAsset)
			{
				SelectAssetData = FAssetData(ActionPlaybackAsset);
			}
		}

		MenuBuilder.BeginSection("T4ActionPlaybackPlayAsset", LOCTEXT("T4ActionPlaybackPlayAssetSection", "ActionPlayback Asset select"));
		{
			FAssetPickerConfig AssetPickerConfig;
			AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateLambda([EditorActionPlaybackController](const FAssetData& AssetData)
			{
				check(nullptr != EditorActionPlaybackController);
				EditorActionPlaybackController->SelectActionPlaybackAsset = Cast<UT4ActionPlaybackAsset>(AssetData.GetAsset());
				FSlateApplication::Get().DismissAllMenus();
			});
			AssetPickerConfig.bAllowNullSelection = false;
			AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
			AssetPickerConfig.Filter.bRecursiveClasses = true;
			AssetPickerConfig.Filter.ClassNames.Add(UT4ActionPlaybackAsset::StaticClass()->GetFName());
			if (SelectAssetData.IsValid())
			{
				AssetPickerConfig.InitialAssetSelection = SelectAssetData;
			}

			FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

			TSharedPtr<SBox> MenuEntry = SNew(SBox)
				.WidthOverride(300.0f)
				.HeightOverride(300.0f)
				[
					ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
				];

			MenuBuilder.AddWidget(MenuEntry.ToSharedRef(), FText::GetEmpty(), true);
		}
		MenuBuilder.EndSection();
	}

	return MenuBuilder.MakeWidget();
}

// #68
void FT4RehearsalEditor::HandleOnActionPlaybackRec()
{
	UT4EditorActionPlaybackController* EditorActionPlaybackController = GetActionPlaybackController();
	if (nullptr == EditorActionPlaybackController)
	{
		return;
	}
	bool bResult = EditorActionPlaybackController->DoRec();
	if (bResult)
	{
		IT4RehearsalViewModel* MainViewModel = GetMainViewModelInterface();
		if (nullptr != MainViewModel)
		{
			MainViewModel->NotifyActionPlaybackRec(); // #104
		}
	}
}

void FT4RehearsalEditor::HandleOnActionPlaybackPlay()
{
	UT4EditorActionPlaybackController* EditorActionPlaybackController = GetActionPlaybackController();
	if (nullptr == EditorActionPlaybackController)
	{
		return;
	}
	bool bResult = EditorActionPlaybackController->DoPlay();
	if (bResult)
	{
		IT4RehearsalViewModel* MainViewModel = GetMainViewModelInterface();
		if (nullptr != MainViewModel)
		{
			MainViewModel->NotifyActionPlaybackPlay(); // #104
		}
	}
}

void FT4RehearsalEditor::HandleOnToogleActionPlaybackRepeatEnable()
{
	UT4EditorActionPlaybackController* EditorActionPlaybackController = GetActionPlaybackController();
	if (nullptr == EditorActionPlaybackController)
	{
		return;
	}
	EditorActionPlaybackController->TooglePlayRepeat();
}

bool FT4RehearsalEditor::HandleOnIsActionPlaybackRepeatEnabled() const
{
	UT4EditorActionPlaybackController* EditorActionPlaybackController = GetActionPlaybackController();
	if (nullptr == EditorActionPlaybackController)
	{
		return false;
	}
	return EditorActionPlaybackController->bPlayRepeat;
}

void FT4RehearsalEditor::HandleOnToogleActionPlaybackPlayerPossessed()
{
	UT4EditorActionPlaybackController* EditorActionPlaybackController = GetActionPlaybackController();
	if (nullptr == EditorActionPlaybackController)
	{
		return;
	}
	EditorActionPlaybackController->TooglePlayerPossessed();
}

bool FT4RehearsalEditor::HandleOnIsActionPlaybackPlayerPossessed() const
{
	UT4EditorActionPlaybackController* EditorActionPlaybackController = GetActionPlaybackController();
	if (nullptr == EditorActionPlaybackController)
	{
		return false;
	}
	return EditorActionPlaybackController->bPlayerPossessed;
}

void FT4RehearsalEditor::HandleOnActionPlaybackPause()
{
	UT4EditorActionPlaybackController* EditorActionPlaybackController = GetActionPlaybackController();
	if (nullptr == EditorActionPlaybackController)
	{
		return;
	}
	EditorActionPlaybackController->DoPause();
}

void FT4RehearsalEditor::HandleOnActionPlaybackStop()
{
	UT4EditorActionPlaybackController* EditorActionPlaybackController = GetActionPlaybackController();
	if (nullptr == EditorActionPlaybackController)
	{
		return;
	}
	EditorActionPlaybackController->DoStop();
}

bool FT4RehearsalEditor::HandleOnCanExecuteActionPlaybackRec()
{
	if (!HasActionPlaybackController()) // #104 : 사용하지 않을 때 인스턴스 생성을 하지 않기 위한 처리
	{
		return true;
	}
	UT4EditorActionPlaybackController* EditorActionPlaybackController = GetActionPlaybackController();
	if (nullptr == EditorActionPlaybackController)
	{
		return false;
	}
	return EditorActionPlaybackController->CanRec();
}

bool FT4RehearsalEditor::HandleOnCanExecuteActionPlaybackPlay()
{
	if (!HasActionPlaybackController()) // #104 : 사용하지 않을 때 인스턴스 생성을 하지 않기 위한 처리
	{
		return true;
	}
	UT4EditorActionPlaybackController* EditorActionPlaybackController = GetActionPlaybackController();
	if (nullptr == EditorActionPlaybackController)
	{
		return false;
	}
	return EditorActionPlaybackController->CanPlay();
}

bool FT4RehearsalEditor::HandleOnCanExecuteActionPlaybackPause()
{
	if (!HasActionPlaybackController()) // #104 : 사용하지 않을 때 인스턴스 생성을 하지 않기 위한 처리
	{
		return false;
	}
	UT4EditorActionPlaybackController* EditorActionPlaybackController = GetActionPlaybackController();
	if (nullptr == EditorActionPlaybackController)
	{
		return false;   
	}
	return EditorActionPlaybackController->CanPause();
}

bool FT4RehearsalEditor::HandleOnCanExecuteActionPlaybackStop()
{
	if (!HasActionPlaybackController()) // #104 : 사용하지 않을 때 인스턴스 생성을 하지 않기 위한 처리
	{
		return false;
	}
	UT4EditorActionPlaybackController* EditorActionPlaybackController = GetActionPlaybackController();
	if (nullptr == EditorActionPlaybackController)
	{
		return false;
	}
	return EditorActionPlaybackController->CanStop();
}

void FT4RehearsalEditor::HandleOnDespawnAll()
{
	const ET4LayerType LayerType = GetLayerType();
	check(ET4LayerType::Max != LayerType);
	T4EditorUtil::ServerDespawnAll(LayerType, false);
}
// ~#68

#undef LOCTEXT_NAMESPACE