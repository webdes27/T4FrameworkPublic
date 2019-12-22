// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4RehearsalViewportToolBar.h"
#include "T4RehearsalViewport.h"
#include "T4RehearsalViewportClient.h"
#include "T4RehearsalViewportViewMenu.h" // #97

#include "T4RehearsalEditorCommands.h"

#include "Products/Common/Viewport/T4RehearsalViewport.h"
#include "Products/Common/Viewport/T4RehearsalViewportClient.h"

#include "Widgets/Layout/SBorder.h"
#include "EditorStyleSet.h"
#include "EditorViewportCommands.h"

#include "ShowFlagMenuCommands.h" // #94

#include "SEditorViewport.h"
#include "SEditorViewportViewMenu.h"
#include "STransformViewportToolbar.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4RehearsalViewportToolBar"

/**
  * #94 : refer ST4RehearsalViewportToolBar.h
 */
void ST4RehearsalViewportToolBar::Construct(const FArguments& InArgs, ST4RehearsalViewport* InViewport)
{
	ViewportRef = InViewport;
	TSharedPtr<SHorizontalBox> MainBoxPtr;

	const FMargin ToolbarSlotPadding(2.0f, 2.0f);
	const FMargin ToolbarButtonPadding(2.0f, 0.0f);

	static const FName DefaultForegroundName("DefaultForeground");

	ChildSlot
	[
		SNew( SBorder )
		.BorderImage( FEditorStyle::GetBrush("NoBorder") )
		// Color and opacity is changed based on whether or not the mouse cursor is hovering over the toolbar area
		.ColorAndOpacity( this, &SViewportToolBar::OnGetColorAndOpacity )
		.ForegroundColor( FEditorStyle::GetSlateColor(DefaultForegroundName) )
		[
			SNew( SVerticalBox )
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew( MainBoxPtr, SHorizontalBox )
			]
		]
	];

	// Options menu
	MainBoxPtr->AddSlot()
		.AutoWidth()
		.Padding(ToolbarSlotPadding)
		[
			SNew(SEditorViewportToolbarMenu)
			.ParentToolBar(SharedThis(this))
			.Cursor(EMouseCursor::Default)
			.Image("EditorViewportToolBar.MenuDropdown")
			.OnGetMenuContent(this, &ST4RehearsalViewportToolBar::GenerateOptionsMenu)
		];

	// Camera mode menu
	MainBoxPtr->AddSlot()
		.AutoWidth()
		.Padding(ToolbarSlotPadding)
		[
			SNew(SEditorViewportToolbarMenu)
			.ParentToolBar(SharedThis(this))
			.Cursor(EMouseCursor::Default)
			.Label(this, &ST4RehearsalViewportToolBar::GetCameraMenuLabel)
			.LabelIcon(this, &ST4RehearsalViewportToolBar::GetCameraMenuLabelIcon)
			.OnGetMenuContent(this, &ST4RehearsalViewportToolBar::GenerateCameraMenu)
		];

	// View menu
	MainBoxPtr->AddSlot()
		.AutoWidth()
		.Padding(ToolbarSlotPadding)
		[
			MakeViewMenu()
		];

	// Show menu
	MainBoxPtr->AddSlot()
		.AutoWidth()
		.Padding(ToolbarSlotPadding)
		[
			SNew(SEditorViewportToolbarMenu)
			.Label(LOCTEXT("T4RehearsalViewportShowMenuTitle", "Show"))
			.Cursor(EMouseCursor::Default)
			.ParentToolBar(SharedThis(this))
			.OnGetMenuContent(this, &ST4RehearsalViewportToolBar::GenerateShowMenu)
		];

	MainBoxPtr->AddSlot()
		.AutoWidth()
		.Padding(ToolbarSlotPadding)
		[
			SNew(SEditorViewportToolbarMenu)
			.Label(LOCTEXT("T4RehearsalViewportViewParamMenuTitle", "View Mode Options"))
			.Cursor(EMouseCursor::Default)
			.ParentToolBar(SharedThis(this))
			.Visibility(this, &ST4RehearsalViewportToolBar::GetViewModeOptionsVisibility)
			.OnGetMenuContent(this, &ST4RehearsalViewportToolBar::GenerateViewModeOptionsMenu)
		];

	SViewportToolBar::Construct(SViewportToolBar::FArguments());
}

TSharedRef<SWidget> ST4RehearsalViewportToolBar::GenerateOptionsMenu() const
{
	const bool bIsPerspective = GetViewportClient()->GetViewportType() == LVT_Perspective;

	const bool bInShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder OptionsMenuBuilder(bInShouldCloseWindowAfterMenuSelection, ViewportRef->GetCommandList());
	{
		OptionsMenuBuilder.BeginSection("T4RehearsalViewportOptions", LOCTEXT("OptionsMenuHeader", "Viewport Options"));
		{
			OptionsMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().ToggleRealTime);
			OptionsMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().ToggleStats);
			OptionsMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().ToggleFPS);

			if (bIsPerspective)
			{
				OptionsMenuBuilder.AddWidget(GenerateFOVMenu(), LOCTEXT("FOVAngle", "Field of View (H)"));
				OptionsMenuBuilder.AddWidget(GenerateFarViewPlaneMenu(), LOCTEXT("FarViewPlane", "Far View Plane"));
			}

			OptionsMenuBuilder.AddWidget(GenerateScreenPercentageMenu(), LOCTEXT("ScreenPercentage", "Screen Percentage"));
		}
		OptionsMenuBuilder.EndSection();
		ExtendOptionsMenu(OptionsMenuBuilder);
	}

	return OptionsMenuBuilder.MakeWidget();
}

void ST4RehearsalViewportToolBar::ExtendOptionsMenu(FMenuBuilder& OptionsMenuBuilder) const
{
	OptionsMenuBuilder.BeginSection("T4RehearsalViewportOptions", LOCTEXT("T4FrameworkViewportToolBarSection", "T4Framework Options"));

	const FT4RehearsalEditorCommands& Commands = FT4RehearsalEditorCommands::Get();
	OptionsMenuBuilder.AddMenuEntry(Commands.ViewportAlwaysTick);

	OptionsMenuBuilder.EndSection();
}

TSharedRef<SWidget> ST4RehearsalViewportToolBar::GenerateFOVMenu() const
{
	const float FOVMin = 5.f;
	const float FOVMax = 170.f;

	return
		SNew(SBox)
		.HAlign(HAlign_Right)
		[
			SNew(SBox)
			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
			.WidthOverride(100.0f)
			[
				SNew(SSpinBox<float>)
				.Font(FEditorStyle::GetFontStyle(TEXT("MenuItem.Font")))
				.MinValue(FOVMin)
				.MaxValue(FOVMax)
				.Value(this, &ST4RehearsalViewportToolBar::OnGetFOVValue)
				.OnValueChanged(this, &ST4RehearsalViewportToolBar::OnFOVValueChanged)
			]
		];
}

float ST4RehearsalViewportToolBar::OnGetFOVValue() const
{
	return GetViewportClient()->ViewFOV;
}

void ST4RehearsalViewportToolBar::OnFOVValueChanged(float NewValue) const
{
	FT4RehearsalViewportClient& ViewportClient = *GetViewportClient();
	ViewportClient.FOVAngle = NewValue;
	ViewportClient.ViewFOV = NewValue;
	ViewportClient.Invalidate();
}

TSharedRef<SWidget> ST4RehearsalViewportToolBar::GenerateFarViewPlaneMenu() const
{
	return
		SNew(SBox)
		.HAlign(HAlign_Right)
		[
			SNew(SBox)
			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
			.WidthOverride(100.0f)
			[
				SNew(SSpinBox<float>)
				.ToolTipText(LOCTEXT("FarViewPlaneTooltip", "Distance to use as the far view plane, or zero to enable an infinite far view plane"))
				.MinValue(0.0f)
				.MaxValue(100000.0f)
				.Font(FEditorStyle::GetFontStyle(TEXT("MenuItem.Font")))
				.Value(this, &ST4RehearsalViewportToolBar::OnGetFarViewPlaneValue)
				.OnValueChanged(const_cast<ST4RehearsalViewportToolBar*>(this), &ST4RehearsalViewportToolBar::OnFarViewPlaneValueChanged)
			]
		];
}

float ST4RehearsalViewportToolBar::OnGetFarViewPlaneValue() const
{
	return GetViewportClient()->GetFarClipPlaneOverride();
}

void ST4RehearsalViewportToolBar::OnFarViewPlaneValueChanged(float NewValue)
{
	GetViewportClient()->OverrideFarClipPlane(NewValue);
}

TSharedRef<SWidget> ST4RehearsalViewportToolBar::GenerateScreenPercentageMenu() const
{
	const int32 PreviewScreenPercentageMin = FSceneViewScreenPercentageConfig::kMinTAAUpsampleResolutionFraction * 100.0f;
	const int32 PreviewScreenPercentageMax = FSceneViewScreenPercentageConfig::kMaxTAAUpsampleResolutionFraction * 100.0f;

	return
		SNew(SBox)
		.HAlign(HAlign_Right)
		.IsEnabled(this, &ST4RehearsalViewportToolBar::OnScreenPercentageIsEnabled)
		[
			SNew(SBox)
			.Padding(FMargin(4.0f, 0.0f, 0.0f, 0.0f))
			.WidthOverride(100.0f)
			[
				SNew(SSpinBox<int32>)
				.Font(FEditorStyle::GetFontStyle(TEXT("MenuItem.Font")))
				.MinValue(PreviewScreenPercentageMin)
				.MaxValue(PreviewScreenPercentageMax)
				.Value(this, &ST4RehearsalViewportToolBar::OnGetScreenPercentageValue)
				.OnValueChanged(const_cast<ST4RehearsalViewportToolBar*>(this), &ST4RehearsalViewportToolBar::OnScreenPercentageValueChanged)
			]
		];
}

int32 ST4RehearsalViewportToolBar::OnGetScreenPercentageValue() const
{
	return GetViewportClient()->GetPreviewScreenPercentage();
}

bool ST4RehearsalViewportToolBar::OnScreenPercentageIsEnabled() const
{
	return GetViewportClient()->SupportsPreviewResolutionFraction();
}

void ST4RehearsalViewportToolBar::OnScreenPercentageValueChanged(int32 NewValue)
{
	FT4RehearsalViewportClient& ViewportClient = *GetViewportClient();
	ViewportClient.SetPreviewScreenPercentage(NewValue);
	ViewportClient.Invalidate();
}

FText ST4RehearsalViewportToolBar::GetCameraMenuLabel() const
{
	return GetCameraMenuLabelFromViewportType(GetViewportClient()->GetViewportType());
}

const FSlateBrush* ST4RehearsalViewportToolBar::GetCameraMenuLabelIcon() const
{
	return GetCameraMenuLabelIconFromViewportType(GetViewportClient()->GetViewportType());
}

TSharedRef<SWidget> ST4RehearsalViewportToolBar::GenerateCameraMenu() const
{
	const bool bInShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder CameraMenuBuilder(bInShouldCloseWindowAfterMenuSelection, ViewportRef->GetCommandList());

	// Camera types
	CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Perspective);

	CameraMenuBuilder.BeginSection("T4RehearsalViewportCameraType_Ortho", LOCTEXT("CameraTypeHeader_Ortho", "Orthographic"));
	CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Top);
	CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Bottom);
	CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Left);
	CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Right);
	CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Front);
	CameraMenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().Back);
	CameraMenuBuilder.EndSection();

	return CameraMenuBuilder.MakeWidget();
}

TSharedRef<ST4RehearsalViewportViewMenu> ST4RehearsalViewportToolBar::MakeViewMenu()
{
	return SNew(ST4RehearsalViewportViewMenu, ViewportRef, SharedThis(this))
		.Cursor(EMouseCursor::Default)
		.MenuExtenders(GetViewMenuExtender());
}

TSharedPtr<FExtender> ST4RehearsalViewportToolBar::GetViewMenuExtender() const
{
	TSharedRef<FExtender> ViewModeExtender(new FExtender());
	ViewModeExtender->AddMenuExtension(
		TEXT("ViewMode"),
		EExtensionHook::After,
		ViewportRef->GetCommandList(),
		FMenuExtensionDelegate::CreateSP(this, &ST4RehearsalViewportToolBar::CreateViewMenuExtensions));

	return GetCombinedExtenderList(ViewModeExtender);
}

void ST4RehearsalViewportToolBar::CreateViewMenuExtensions(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("T4RehearsalViewportDeferredRendering", LOCTEXT("DeferredRenderingHeader", "Deferred Rendering"));
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("T4RehearsalViewportCollision", LOCTEXT("CollisionViewModeHeader", "Collision"));
	{
		MenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().CollisionPawn, NAME_None, LOCTEXT("CollisionPawnViewModeDisplayName", "Player Collision"));
		MenuBuilder.AddMenuEntry(FEditorViewportCommands::Get().CollisionVisibility, NAME_None, LOCTEXT("CollisionVisibilityViewModeDisplayName", "Visibility Collision"));
	}
	MenuBuilder.EndSection();
}

TSharedPtr<FExtender> ST4RehearsalViewportToolBar::GetCombinedExtenderList(TSharedRef<FExtender> MenuExtender) const
{
	TSharedPtr<FExtender> HostEditorExtenders = ViewportRef->GetExtenders();

	TArray<TSharedPtr<FExtender>> Extenders;
	Extenders.Reserve(2);
	Extenders.Add(HostEditorExtenders);
	Extenders.Add(MenuExtender);

	return FExtender::Combine(Extenders);
}

TSharedRef<SWidget> ST4RehearsalViewportToolBar::GenerateShowMenu() const
{
	const bool bInShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder ShowMenuBuilder(bInShouldCloseWindowAfterMenuSelection, ViewportRef->GetCommandList());

	const FT4RehearsalEditorCommands& Commands = FT4RehearsalEditorCommands::Get();
	{
		ShowMenuBuilder.AddMenuEntry(Commands.UseDefaultShowFlags);
		FShowFlagMenuCommands::Get().BuildShowFlagsMenu(ShowMenuBuilder);
	}
	{
		ShowMenuBuilder.BeginSection("T4RehearsalViewportShowOptions", LOCTEXT("T4RehearsalViewportShowOptionsSection", "T4Framework Show Flags"));
		{
			ShowMenuBuilder.AddMenuEntry(Commands.ViewportShowCapsule); // #76
		}
		ShowMenuBuilder.EndSection();
	}

	return ShowMenuBuilder.MakeWidget();
}

EVisibility ST4RehearsalViewportToolBar::GetViewModeOptionsVisibility() const
{
	const FT4RehearsalViewportClient& ViewClient = *GetViewportClient();
	if (ViewClient.GetViewMode() == VMI_MeshUVDensityAccuracy || ViewClient.GetViewMode() == VMI_MaterialTextureScaleAccuracy || ViewClient.GetViewMode() == VMI_RequiredTextureResolution)
	{
		return EVisibility::SelfHitTestInvisible;
	}
	else
	{
		return EVisibility::Collapsed;
	}
}

TSharedRef<SWidget> ST4RehearsalViewportToolBar::GenerateViewModeOptionsMenu() const
{
	FT4RehearsalViewportClient& ViewClient = *GetViewportClient();
	const UWorld* World = ViewClient.GetWorld();
	return BuildViewModeOptionsMenu(
		ViewportRef->GetCommandList(),
		ViewClient.GetViewMode(),
		World ? World->FeatureLevel.GetValue() : GMaxRHIFeatureLevel,
		ViewClient.GetViewModeParamNameMap()
	);
}

FT4RehearsalViewportClient* ST4RehearsalViewportToolBar::GetViewportClient() const
{
	return ViewportRef->GetViewportClient();
}

#undef LOCTEXT_NAMESPACE