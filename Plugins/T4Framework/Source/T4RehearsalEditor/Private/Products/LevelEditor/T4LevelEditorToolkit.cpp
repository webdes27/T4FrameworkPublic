// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4LevelEditorToolkit.h"
#include "T4LevelEditorMode.h"

#include "Products/Common/Helper/T4EditorActionPlaybackController.h"

#include "Products/T4RehearsalEditorUtils.h"

#include "Widgets/Text/STextBlock.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SToolTip.h"
#include "Widgets/Layout/SScrollBox.h"
#include "EditorStyleSet.h"
#include "EditorModeManager.h"
#include "EditorModes.h"

#include "PropertyEditorModule.h"
#include "IDetailsView.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4LevelEditorToolkit"

/**
  * #68
 */
void ST4LevelEditorModeControls::SelectionChanged()
{
#if 0
	// If the currently selected modifier is being disabled, change the selection to Edit
	for (int32 Idx = 0; Idx < ModifierControls.Num(); ++Idx)
	{
		if (ModifierControls[Idx]->IsChecked() && !GetGeometryModeTool()->GetModifier(Idx)->Supports())
		{
			if (GetGeometryModeTool()->GetNumModifiers() > 0)
			{
				GetGeometryModeTool()->SetCurrentModifier(GetGeometryModeTool()->GetModifier(0));
			}
		}
	}
#endif
}

void ST4LevelEditorModeControls::Construct(const FArguments& InArgs)
{
	CreateLayout();
}

void ST4LevelEditorModeControls::CreateLayout()
{
	this->ChildSlot
	[
		SNew(SScrollBox)
		+SScrollBox::Slot()
		.Padding(0.0f)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(3.0f)
				[
					SNew(SSeparator)
					.Orientation(Orient_Horizontal)
				]	
				+SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					CreateDespawnAllObjectButtons()
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(3.0f)
				[
					SNew(SSeparator)
					.Orientation(Orient_Horizontal)
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					CreateEditorActionPlaybackController()
				]	
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(3.0f)
				[
					SNew(SSeparator)
					.Orientation(Orient_Horizontal)
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					CreateActionPlaybackButtons()
				]
			]
		]
	];
}

TSharedRef<SVerticalBox> ST4LevelEditorModeControls::CreateDespawnAllObjectButtons()
{
	TSharedPtr<SVerticalBox> Vbox;
	SAssignNew(Vbox, SVerticalBox)
	+SVerticalBox::Slot()
	.AutoHeight()
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Center)
	[
		SNew(SButton)
		.Text(LOCTEXT("T4LevelEditorModeDespawnAllSpawnObjectsBtn", "Leave all SpawnObjects"))
		.ToolTipText(LOCTEXT("T4LevelEditorModeDespawnAllSpawnObjectsBtn_Tooltip", "Leave all SpawnObjects"))
		.OnClicked(this, &ST4LevelEditorModeControls::HandleOnDespawnAllSpawnObjectsClicked)
	];

	return Vbox.ToSharedRef();
}

TSharedRef<IDetailsView> ST4LevelEditorModeControls::CreateEditorActionPlaybackController()
{
	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.bAllowSearch = false;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertiesControl = PropertyModule.CreateDetailView(Args);

	TArray<UObject*> PropertyObjects;
	PropertyObjects.Add(GetActionPlaybackController());
	PropertiesControl->SetObjects(PropertyObjects);

	return PropertiesControl.ToSharedRef();
}

TSharedRef<SHorizontalBox> ST4LevelEditorModeControls::CreateActionPlaybackButtons()
{
	TSharedPtr<SHorizontalBox> Hbox;
	SAssignNew(Hbox, SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("T4LevelEditorModeActionPlaybackRecBtn", "Rec"))
			.ToolTipText(LOCTEXT("T4LevelEditorModeActionPlaybackRecBtn_Tooltip", "ActionPlayback Rec"))
			.OnClicked(this, &ST4LevelEditorModeControls::HandleOnActionPlaybackRecClicked)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("T4LevelEditorModeActionPlaybackPlayBtn", "Play"))
			.ToolTipText(LOCTEXT("T4LevelEditorModeActionPlaybackPlayBtn_Tooltip", "ActionPlayback Play"))
			.OnClicked(this, &ST4LevelEditorModeControls::HandleOnActionPlaybackPlayClicked)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("T4LevelEditorModeActionPlaybackPauseBtn", "Pause"))
			.ToolTipText(LOCTEXT("T4LevelEditorModeActionPlaybackPauseBtn_Tooltip", "ActionPlayback Pause"))
			.OnClicked(this, &ST4LevelEditorModeControls::HandleOnActionPlaybackPauseClicked)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("T4LevelEditorModeActionPlaybackStopBtn", "Stop"))
			.ToolTipText(LOCTEXT("T4LevelEditorModeActionPlaybackStopBtn_Tooltip", "ActionPlayback Stop"))
			.OnClicked(this, &ST4LevelEditorModeControls::HandleOnActionPlaybackStopClicked)
		];

	return Hbox.ToSharedRef();
}

UT4EditorActionPlaybackController* ST4LevelEditorModeControls::GetActionPlaybackController() const
{
	FT4LevelEditorMode* Mode = (FT4LevelEditorMode*)GLevelEditorModeTools().GetActiveMode(FT4LevelEditorMode::EM_T4LevelEditorMode);
	if (nullptr == Mode)
	{
		return nullptr;
	}
	return Mode->GetActionPlaybackController();
}

FReply ST4LevelEditorModeControls::HandleOnDespawnAllSpawnObjectsClicked()
{
	T4EditorUtil::ServerDespawnAll(ET4LayerType::LevelEditor, true);
	return FReply::Handled();
}

FReply ST4LevelEditorModeControls::HandleOnActionPlaybackRecClicked()
{
	UWorld* EditorWorld = T4EditorUtil::GetWorld(ET4LayerType::LevelEditor);
	if (nullptr != EditorWorld)
	{
		UT4EditorActionPlaybackController* EditorActionPlaybackController = GetActionPlaybackController();
		check(nullptr != EditorActionPlaybackController);
		EditorActionPlaybackController->DoRec();
	}
	return FReply::Handled();
}

FReply ST4LevelEditorModeControls::HandleOnActionPlaybackPlayClicked()
{
	UWorld* EditorWorld = T4EditorUtil::GetWorld(ET4LayerType::LevelEditor);
	if (nullptr != EditorWorld)
	{
		UT4EditorActionPlaybackController* EditorActionPlaybackController = GetActionPlaybackController();
		check(nullptr != EditorActionPlaybackController);
		EditorActionPlaybackController->DoPlay();
	}
	return FReply::Handled();
}

FReply ST4LevelEditorModeControls::HandleOnActionPlaybackPauseClicked()
{
	UT4EditorActionPlaybackController* EditorActionPlaybackController = GetActionPlaybackController();
	check(nullptr != EditorActionPlaybackController);
	EditorActionPlaybackController->DoPause();
	return FReply::Handled();
}

FReply ST4LevelEditorModeControls::HandleOnActionPlaybackStopClicked()
{
	UT4EditorActionPlaybackController* EditorActionPlaybackController = GetActionPlaybackController();
	check(nullptr != EditorActionPlaybackController);
	EditorActionPlaybackController->DoStop();
	return FReply::Handled();
}

void FT4LevelEditorToolkit::RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager)
{

}

void FT4LevelEditorToolkit::UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager)
{

}

void FT4LevelEditorToolkit::Init(const TSharedPtr< class IToolkitHost >& InitToolkitHost)
{
	LevelEditorModeWidget = SNew(ST4LevelEditorModeControls);

	FModeToolkit::Init(InitToolkitHost);
}

FName FT4LevelEditorToolkit::GetToolkitFName() const
{
	return FName("T4LevelEditorMode");
}

FText FT4LevelEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT( "ToolkitName", "T4LevelEditor Mode" );
}

class FEdMode* FT4LevelEditorToolkit::GetEditorMode() const
{
	return (FT4LevelEditorMode*)GLevelEditorModeTools().GetActiveMode(FT4LevelEditorMode::EM_T4LevelEditorMode);
}

void FT4LevelEditorToolkit::SelectionChanged()
{
	LevelEditorModeWidget->SelectionChanged();
}

TSharedPtr<SWidget> FT4LevelEditorToolkit::GetInlineContent() const
{
	return SNew(ST4LevelEditorModeControls);
}

#undef LOCTEXT_NAMESPACE
