// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4RehearsalViewport.h"
#include "T4RehearsalViewportClient.h"
#include "T4RehearsalViewportToolBar.h"

#include "T4RehearsalEditorCommands.h"

#include "Products/Common/ViewModel/T4RehearsalViewModel.h"

#include "Settings/LevelEditorViewportSettings.h" // #94

#include "ShowFlagMenuCommands.h" // #94

#include "Slate/SceneViewport.h"
#include "EditorStyleSet.h"
#include "EditorViewportCommands.h"
#include "SViewportToolBarIconMenu.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SSlider.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4RehearsalViewport"

/**
  * 
 */
ST4RehearsalViewport::ST4RehearsalViewport()
	: ViewModelRef(nullptr)
	, bAlwaysTickViewport(false) // #76
{
}

ST4RehearsalViewport::~ST4RehearsalViewport()
{
	OnCleanup();
}

void ST4RehearsalViewport::Construct(const FArguments& InArgs)
{
	ViewModelRef = InArgs._ViewModel;
	OnThumbnailCaptured = InArgs._OnThumbnailCaptured;
	OnRefreshButtonClicked = InArgs._OnRefreshButtonClicked; // #86
	OnSimulationButtonClicked = InArgs._OnSimulationButtonClicked; // #86
	OnHotKeyJumpToPlay = InArgs._OnHotKeyJumpToPlay; // #99
	OnHotKeyJumpToEnd = InArgs._OnHotKeyJumpToEnd; // #99
	OnHotKeyTogglePlay = InArgs._OnHotKeyTogglePlay; // #99

	SEditorViewport::Construct(SEditorViewport::FArguments());
}

void ST4RehearsalViewport::CreateThumbnail(UObject* InScreenShotOwner)
{
	if (ViewportClientPtr.IsValid())
	{
		ViewportClientPtr->bCaptureScreenShot = true;
		ViewportClientPtr->ScreenShotOwner = InScreenShotOwner;
		ViewportClientPtr->SetUpdateViewport(5.0f); // #72 : 썸네일 업데이트 시간동안 Runtime 으로 변경
	}
}

void ST4RehearsalViewport::OnCleanup() // #97
{
	if (ViewportClientPtr.IsValid())
	{
		ViewportClientPtr->OnReset();
		ViewportClientPtr.Reset();
	}
}

TSharedPtr<FExtender> ST4RehearsalViewport::GetExtenders() const
{
	TSharedPtr<FExtender> Result(MakeShareable(new FExtender));
	return Result;
}

void ST4RehearsalViewport::AddReferencedObjects(FReferenceCollector & Collector)
{
}

TSharedRef<FEditorViewportClient> ST4RehearsalViewport::MakeEditorViewportClient()
{
	check(nullptr != ViewModelRef);
	ViewportClientPtr = MakeShareable(
		new FT4RehearsalViewportClient(
			SharedThis(this), 
			ViewModelRef,
			FT4RehearsalViewportClient::FT4OnScreenShotCaptured::CreateSP(this, &ST4RehearsalViewport::OnScreenShotCaptured)
		)
	);

	return ViewportClientPtr.ToSharedRef();
}

TSharedPtr<SWidget> ST4RehearsalViewport::MakeViewportToolbar()
{
	check(nullptr != ViewModelRef);
	const ET4ViewModelEditMode CurrentEditMode = ViewModelRef->GetEditMode();
	if (ET4ViewModelEditMode::Preview == CurrentEditMode)
	{
		return nullptr;
	}
	return SNew(SBox);
}

void ST4RehearsalViewport::BindCommands()
{
	check(nullptr != ViewModelRef);
	if (ET4ViewModelEditMode::Preview == ViewModelRef->GetEditMode())
	{
		// #94 : preview 는 불필요한 처리를 하지 않도록 한다.
		return;
	}

	SEditorViewport::BindCommands();

	FShowFlagMenuCommands::Get().BindCommands(*CommandList, Client);
	//FBufferVisualizationMenuCommands::Get().BindCommands(*CommandList, Client);

	// Unbind the CycleTransformGizmos since niagara currently doesn't use the gizmos and it prevents resetting the system with
	// spacebar when the viewport is focused.
	CommandList->UnmapAction(FEditorViewportCommands::Get().CycleTransformGizmos);

	const FT4RehearsalEditorCommands& Commands = FT4RehearsalEditorCommands::Get();

	CommandList->MapAction(
		Commands.UseDefaultShowFlags,
		FExecuteAction::CreateSP(this, &ST4RehearsalViewport::HandleOnUseDefaultShowFlags)
	); // #94

	// #76
	CommandList->MapAction(
		Commands.ViewportShowCapsule,
		FExecuteAction::CreateSP(this, &ST4RehearsalViewport::HandleOnToggleShowCapsule),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &ST4RehearsalViewport::HandleOnIsToggleShowCapsuleChecked)
	);
	CommandList->MapAction(
		Commands.ViewportAlwaysTick,
		FExecuteAction::CreateSP(this, &ST4RehearsalViewport::HandleOnToggleAlwaysTick),
		FCanExecuteAction(),
		FIsActionChecked::CreateSP(this, &ST4RehearsalViewport::HandleOnIsToggleAlwaysTickChecked)
	); 
	// ~#76

	// #99
	CommandList->MapAction(
		Commands.ViewportJumpToPlay,
		FExecuteAction::CreateSP(this, &ST4RehearsalViewport::HandleOnJumpToPlay)
	);
	CommandList->MapAction(
		Commands.ViewportJumpToEnd,
		FExecuteAction::CreateSP(this, &ST4RehearsalViewport::HandleOnJumpToEnd)
	);
	CommandList->MapAction(
		Commands.ViewportTogglePlay,
		FExecuteAction::CreateSP(this, &ST4RehearsalViewport::HandleOnTogglePlay)
	);
	// ~#99
}

void ST4RehearsalViewport::PopulateViewportOverlays(TSharedRef<class SOverlay> Overlay)
{
	check(nullptr != ViewModelRef);
	if (ET4ViewModelEditMode::Preview == ViewModelRef->GetEditMode())
	{
		return; 
	}

	TSharedPtr<ST4RehearsalViewportToolBar> ViewportToolBar = SNew(ST4RehearsalViewportToolBar, this);

	Overlay->AddSlot()
		.VAlign(VAlign_Top)
		[
			ViewportToolBar.ToSharedRef()
		];

	{
		FName ToolBarStyle = "ViewportMenu";
		Overlay->AddSlot()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Right)
			.Padding(5.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot() // #93
				.AutoWidth()
				.Padding(2.0f)
				[
					SNew(SViewportToolBarIconMenu)
					.Cursor(EMouseCursor::Default)
					.Style(ToolBarStyle)
					.Label(this, &ST4RehearsalViewport::HandleOnGetTimelapseLabel)
					.OnGetMenuContent(this, &ST4RehearsalViewport::HandleOnFillTimelapseScaleMenu)
					.ToolTipText(LOCTEXT("TimelapseScale_ToolTip", "Timelapse Scale"))
					.Icon(FSlateIcon(FEditorStyle::GetStyleSetName(), "EditorViewport.RelativeCoordinateSystem_World"))
					.ParentToolBar(ViewportToolBar)
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("CameraSpeedButton")))
				]

				+ SHorizontalBox::Slot() // #83
				.AutoWidth()
				.Padding(2.0f)
				[
					SNew(SViewportToolBarIconMenu)
					.Cursor(EMouseCursor::Default)
					.Style(ToolBarStyle)
					.Label(this, &ST4RehearsalViewport::HandleOnGetCameraSpeedLabel)
					.OnGetMenuContent(this, &ST4RehearsalViewport::HandleOnFillCameraSpeedMenu)
					.ToolTipText(LOCTEXT("CameraSpeed_ToolTip", "Camera Speed"))
					.Icon(FSlateIcon(FEditorStyle::GetStyleSetName(), "EditorViewport.CamSpeedSetting"))
					.ParentToolBar(ViewportToolBar)
					.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("CameraSpeedButton")))
				]
			];
	}

	const ET4ViewModelEditMode CurrentEditMode = ViewModelRef->GetEditMode();
	if (ET4ViewModelEditMode::WorldPreview == CurrentEditMode) // #83
	{
		Overlay->AddSlot()
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Right)
			.Padding(5.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4RehearsalViewportRefreshWorldBtn", "Refresh World"))
					.ToolTipText(LOCTEXT("T4RehearsalViewportRefreshWorldBtn_Tooltip", "Refresh World"))
					.OnClicked(this, &ST4RehearsalViewport::HandleOnRefreshButtonClicked)
					//.ParentToolBar(ViewportToolBar)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4RehearsalViewportSimulationWorldBtn", "Toggle Simulation"))
					.ToolTipText(LOCTEXT("T4RehearsalViewportSimulationWorldBtn_Tooltip", "Toggle Simulation"))
					.OnClicked(this, &ST4RehearsalViewport::HandleOnSimulationButtonClicked)
					//.ParentToolBar(ViewportToolBar)
				] // #86
			];
	}
	else if (ET4ViewModelEditMode::Conti == CurrentEditMode) // #102
	{
		Overlay->AddSlot()
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Right)
			.Padding(5.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4RehearsalViewportSimulationWorldBtn", "Toggle Simulation"))
					.ToolTipText(LOCTEXT("T4RehearsalViewportSimulationWorldBtn_Tooltip", "Toggle Simulation"))
					.OnClicked(this, &ST4RehearsalViewport::HandleOnSimulationButtonClicked)
					//.ParentToolBar(ViewportToolBar)
				]
			];
	}
}

void ST4RehearsalViewport::RefreshViewport()
{
	//reregister the preview components, so if the preview material changed it will be propagated to the render thread
	//PreviewComponent->MarkRenderStateDirty();
	SceneViewport->InvalidateDisplay();
}

bool ST4RehearsalViewport::IsVisible() const // #76
{
	if (bAlwaysTickViewport)
	{
		return true;
	}
	return SEditorViewport::IsVisible();
}

void ST4RehearsalViewport::HandleOnUseDefaultShowFlags()
{
	// cache off the current viewmode as it gets trashed when applying FEngineShowFlags()
	const EViewModeIndex CachedViewMode = ViewportClientPtr->GetViewMode();

	// Setting show flags to the defaults should not stomp on the current viewmode settings.
	ViewportClientPtr->SetGameView(false);

	// Get default save flags
	FEngineShowFlags EditorShowFlags(ESFIM_Editor);
	FEngineShowFlags GameShowFlags(ESFIM_Game);

	// this trashes the current viewmode!
	ViewportClientPtr->EngineShowFlags = EditorShowFlags;
	// Restore the state of SelectionOutline based on user settings
	ViewportClientPtr->EngineShowFlags.SetSelectionOutline(GetDefault<ULevelEditorViewportSettings>()->bUseSelectionOutline);
	ViewportClientPtr->LastEngineShowFlags = GameShowFlags;

	// re-apply the cached viewmode, as it was trashed with FEngineShowFlags()
	ApplyViewMode(CachedViewMode, ViewportClientPtr->IsPerspective(), ViewportClientPtr->EngineShowFlags);
	ApplyViewMode(CachedViewMode, ViewportClientPtr->IsPerspective(), ViewportClientPtr->LastEngineShowFlags);

	ViewportClientPtr->Invalidate();
}

// #76
void ST4RehearsalViewport::HandleOnToggleShowCapsule()
{ 
	if (nullptr == ViewModelRef)
	{
		return;
	}
	ViewModelRef->SetViewportShowOptionCapsule(!ViewModelRef->IsShownViewportShowOptionCapsule());
} 

bool ST4RehearsalViewport::HandleOnIsToggleShowCapsuleChecked() const 
{ 
	if (nullptr == ViewModelRef)
	{
		return false;
	}
	return ViewModelRef->IsShownViewportShowOptionCapsule();
} 
// ~#76

// #93
TSharedRef<SWidget> ST4RehearsalViewport::HandleOnFillTimelapseScaleMenu()
{
	TSharedRef<SWidget> ReturnWidget = SNew(SBorder)
	.BorderImage(FEditorStyle::GetBrush(TEXT("Menu.Background")))
	[
		SNew(SVerticalBox)
		// Timelapse Scale
		+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(8.0f, 2.0f, 60.0f, 2.0f))
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("MouseSettingsTimelapseScale", "Timelapse Scale"))
				.Font(FEditorStyle::GetFontStyle(TEXT("MenuItem.Font")))
			]
		+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(8.0f, 4.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(FMargin(0.0f, 2.0f))
				[
					SAssignNew(TimelapseScaleBox, SSpinBox<float>)
					.MinValue(0.0f)
					.MaxValue(7000.0f)
					.MinSliderValue(0)
					.MaxSliderValue(7000)
					.Value(this, &ST4RehearsalViewport::HandleOnGetTimelapseScaleBoxValue)
					.OnValueChanged(this, &ST4RehearsalViewport::HandleOnSetTimelapseScaleBoxValue)
					.ToolTipText(LOCTEXT("TimelapseScale_ToolTip", "Scalar to increase Timelapse Scale range"))
				]
			]

		// Timelapse SetHour
		+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(8.0f, 2.0f, 60.0f, 2.0f))
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("MouseSettingsTimelapseSetHour", "Timelapse SetHour"))
				.Font(FEditorStyle::GetFontStyle(TEXT("MenuItem.Font")))
			]
		+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(8.0f, 4.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(FMargin(0.0f, 2.0f))
				[
					SAssignNew(TimelapseSetHourBox, SSpinBox<float>)
					.MinValue(0.0f)
					.MaxValue(24.0f)
					.MinSliderValue(0)
					.MaxSliderValue(24)
					.Value(this, &ST4RehearsalViewport::HandleOnGetTimelapseSetHourBoxValue)
					.OnValueChanged(this, &ST4RehearsalViewport::HandleOnSetTimelapseSetHourBoxValue)
					.ToolTipText(LOCTEXT("TimelapseSetHour_ToolTip", "Timelapse Set Hour"))
				]
			]
	];

	return ReturnWidget;
}

FText ST4RehearsalViewport::HandleOnGetTimelapseLabel() const
{
	if (nullptr != ViewModelRef)
	{
		return FText::AsNumber(ViewModelRef->GetGameWorldTimelapseScale());
	}

	return FText();
}

float ST4RehearsalViewport::HandleOnGetTimelapseScaleBoxValue() const
{
	float TimelapseScale = 1.f;

	if (nullptr != ViewModelRef)
	{
		TimelapseScale = (ViewModelRef->GetGameWorldTimelapseScale());
	}

	return TimelapseScale;
}

void ST4RehearsalViewport::HandleOnSetTimelapseScaleBoxValue(float NewValue)
{
	if (nullptr != ViewModelRef)
	{
		ViewModelRef->SetGameWorldTimelapseScale(NewValue);
	}
}

float ST4RehearsalViewport::HandleOnGetTimelapseSetHourBoxValue() const
{
	float TimelapseScale = 1.f;

	if (nullptr != ViewModelRef)
	{
		TimelapseScale = (ViewModelRef->GetGameWorldTimeHour());
	}

	return TimelapseScale;
}

void ST4RehearsalViewport::HandleOnSetTimelapseSetHourBoxValue(float NewValue)
{
	if (nullptr != ViewModelRef)
	{
		ViewModelRef->SetGameWorldTimeHour(NewValue);
	}
}
// ~#93

// #83
TSharedRef<SWidget> ST4RehearsalViewport::HandleOnFillCameraSpeedMenu()
{
	TSharedRef<SWidget> ReturnWidget = SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush(TEXT("Menu.Background")))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(8.0f, 2.0f, 60.0f, 2.0f))
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("MouseSettingsCamSpeed", "Camera Speed"))
				.Font(FEditorStyle::GetFontStyle(TEXT("MenuItem.Font")))
			]
			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(8.0f, 4.0f))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(FMargin(0.0f, 2.0f))
				[
					SAssignNew(CamSpeedSlider, SSlider)
					.Value(this, &ST4RehearsalViewport::HandleOnGetCamSpeedSliderPosition)
					.OnValueChanged(this, &ST4RehearsalViewport::HandleOnSetCamSpeed)
				]
			+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(8.0f, 2.0f, 0.0f, 2.0f)
				[
					SNew(STextBlock)
					.Text(this, &ST4RehearsalViewport::HandleOnGetCameraSpeedLabel)
				.Font(FEditorStyle::GetFontStyle(TEXT("MenuItem.Font")))
				]
			] 
			
		// Camera Speed Scalar
		+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(8.0f, 2.0f, 60.0f, 2.0f))
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("MouseSettingsCamSpeedScalar", "Camera Speed Scalar"))
			.Font(FEditorStyle::GetFontStyle(TEXT("MenuItem.Font")))
			]
		+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(8.0f, 4.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				.Padding(FMargin(0.0f, 2.0f))
				[
					SAssignNew(CamSpeedScalarBox, SSpinBox<float>)
					.MinValue(1)
					.MaxValue(TNumericLimits<int32>::Max())
					.MinSliderValue(1)
					.MaxSliderValue(128)
					.Value(this, &ST4RehearsalViewport::HandleOnGetCamSpeedScalarBoxValue)
					.OnValueChanged(this, &ST4RehearsalViewport::HandleOnSetCamSpeedScalarBoxValue)
					.ToolTipText(LOCTEXT("CameraSpeedScalar_ToolTip", "Scalar to increase camera movement range"))
				]
			]
		];

	return ReturnWidget;
}

FText ST4RehearsalViewport::HandleOnGetCameraSpeedLabel() const
{
	if (ViewportClientPtr.IsValid())
	{
		return FText::AsNumber(ViewportClientPtr->GetCameraSpeedSetting());
	}

	return FText();
}

float ST4RehearsalViewport::HandleOnGetCamSpeedSliderPosition() const
{
	float SliderPos = 0.f;

	if (ViewportClientPtr.IsValid())
	{
		SliderPos = (ViewportClientPtr->GetCameraSpeedSetting() - 1) / ((float)FEditorViewportClient::MaxCameraSpeeds - 1);
	}

	return SliderPos;
}


void ST4RehearsalViewport::HandleOnSetCamSpeed(float NewValue)
{
	if (ViewportClientPtr.IsValid())
	{
		const int32 SpeedSetting = NewValue * ((float)FEditorViewportClient::MaxCameraSpeeds - 1) + 1;
		ViewportClientPtr->SetCameraSpeedSetting(SpeedSetting);
	}
}

FText ST4RehearsalViewport::HandleOnGetCameraSpeedScalarLabel() const
{
	if (ViewportClientPtr.IsValid())
	{
		return FText::AsNumber(ViewportClientPtr->GetCameraSpeedScalar());
	}

	return FText();
}

float ST4RehearsalViewport::HandleOnGetCamSpeedScalarBoxValue() const
{
	float CamSpeedScalar = 1.f;

	if (ViewportClientPtr.IsValid())
	{
		CamSpeedScalar = (ViewportClientPtr->GetCameraSpeedScalar());
	}

	return CamSpeedScalar;
}

void ST4RehearsalViewport::HandleOnSetCamSpeedScalarBoxValue(float NewValue)
{
	if (ViewportClientPtr.IsValid())
	{
		ViewportClientPtr->SetCameraSpeedScalar(NewValue);
	}
}
// ~#83

FReply ST4RehearsalViewport::HandleOnRefreshButtonClicked() // #83
{
	OnRefreshButtonClicked.ExecuteIfBound();
	return FReply::Handled();
}

FReply ST4RehearsalViewport::HandleOnSimulationButtonClicked() // #86
{
	OnSimulationButtonClicked.ExecuteIfBound();
	return FReply::Handled();
}

void ST4RehearsalViewport::OnScreenShotCaptured(UObject* InOwner, UTexture2D* InScreenShot)
{
	OnThumbnailCaptured.ExecuteIfBound(InOwner, InScreenShot);
}

// #99
void ST4RehearsalViewport::HandleOnJumpToPlay() // #99 : Keys::Up
{
	OnHotKeyJumpToPlay.ExecuteIfBound();
}

void ST4RehearsalViewport::HandleOnJumpToEnd() // #99 : Keys::Up + CTRL
{
	OnHotKeyJumpToEnd.ExecuteIfBound();
}

void ST4RehearsalViewport::HandleOnTogglePlay() // #99 : Keys::Down
{
	OnHotKeyTogglePlay.ExecuteIfBound();
}
// ~#99

#undef LOCTEXT_NAMESPACE