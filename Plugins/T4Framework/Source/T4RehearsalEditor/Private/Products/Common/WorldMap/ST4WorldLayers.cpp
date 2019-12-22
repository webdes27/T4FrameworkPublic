// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4WorldLayers.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorStyleSet.h"
#include "T4WorldTileCollectionModel.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"

#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "WorldMap"

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void ST4NewWorldLayerPopup::Construct(const FArguments& InArgs)
{
	OnCreateLayer	= InArgs._OnCreateLayer;
	LayerData.Name	= InArgs._DefaultName;

	// store set of currently existing layer names
	{
		const auto& AllLayersList = InArgs._InWorldModel->GetLayers();
		for (const auto& WorldLayer : AllLayersList)
		{
			ExistingLayerNames.Add(WorldLayer.Name);
		}
	}
	
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
		.Padding(10)
		[
			SNew(SVerticalBox)

			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2,2,0,0)
			[
				SNew(SHorizontalBox)

				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Layer_Name", "Name:"))
				]

				+SHorizontalBox::Slot()
				.Padding(4,0,0,0)
				[
					SNew(SEditableTextBox)
					.Text(this, &ST4NewWorldLayerPopup::GetLayerName)
					.SelectAllTextWhenFocused(true)
					.OnTextChanged(this, &ST4NewWorldLayerPopup::SetLayerName)
				]

			]

			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2,2,0,0)
			[
				SNew(SHorizontalBox)

				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SCheckBox)
					.IsChecked(this, &ST4NewWorldLayerPopup::GetDistanceStreamingState)
					.OnCheckStateChanged(this, &ST4NewWorldLayerPopup::OnDistanceStreamingStateChanged)
				]

				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SNumericEntryBox<int32>)
					.IsEnabled(this, &ST4NewWorldLayerPopup::IsDistanceStreamingEnabled)
					.Value(this, &ST4NewWorldLayerPopup::GetStreamingDistance)
					.MinValue(1)
					.MaxValue(TNumericLimits<int32>::Max())
					.OnValueChanged(this, &ST4NewWorldLayerPopup::SetStreamingDistance)
					.LabelPadding(0)
					.Label()
					[
						SNumericEntryBox<int32>::BuildLabel(
							LOCTEXT("LayerStreamingDistance", "Streaming distance"), 
							FLinearColor::White, SNumericEntryBox<int32>::RedLabelBackgroundColor
							)

					]
				]
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2,2,0,0)
			[
				SNew(SButton)
				.OnClicked(this, &ST4NewWorldLayerPopup::OnClickedCreate)
				.IsEnabled(this, &ST4NewWorldLayerPopup::CanCreateLayer)
				.Text(LOCTEXT("Layer_Create", "Create"))
			]

		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply ST4NewWorldLayerPopup::OnClickedCreate()
{
	if (OnCreateLayer.IsBound())
	{
		return OnCreateLayer.Execute(LayerData);
	}
	
	return FReply::Unhandled();
}

bool ST4NewWorldLayerPopup::CanCreateLayer() const
{
	return LayerData.Name.Len() > 0 && !ExistingLayerNames.Contains(LayerData.Name);
}

// #86
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void ST4EditWorldLayerPopup::Construct(const FArguments& InArgs)
{
	OnEditLayer = InArgs._OnEditLayer;
	LayerData = InArgs._TileLayerData;
	WorldModel = InArgs._InWorldModel;

	// store set of currently existing layer names
	{
		const auto& AllLayersList = WorldModel->GetLayers();
		for (const auto& WorldLayer : AllLayersList)
		{
			ExistingLayers.Add(WorldLayer);
		}
	}
	
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
		.Padding(10)
		[
			SNew(SVerticalBox)

			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2,2,0,0)
			[
				SNew(SHorizontalBox)

				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Layer_Name", "Name:"))
				]

				+SHorizontalBox::Slot()
				.Padding(4,0,0,0)
				[
					SNew(SEditableTextBox)
					.Text(this, &ST4EditWorldLayerPopup::GetLayerName)
					.SelectAllTextWhenFocused(true)
					.OnTextChanged(this, &ST4EditWorldLayerPopup::SetLayerName)
				]

			]

			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2,2,0,0)
			[
				SNew(SHorizontalBox)

				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SCheckBox)
					.IsChecked(this, &ST4EditWorldLayerPopup::GetDistanceStreamingState)
					.OnCheckStateChanged(this, &ST4EditWorldLayerPopup::OnDistanceStreamingStateChanged)
				]

				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SNumericEntryBox<int32>)
					.IsEnabled(this, &ST4EditWorldLayerPopup::IsDistanceStreamingEnabled)
					.Value(this, &ST4EditWorldLayerPopup::GetStreamingDistance)
					.MinValue(1)
					.MaxValue(TNumericLimits<int32>::Max())
					.OnValueChanged(this, &ST4EditWorldLayerPopup::SetStreamingDistance)
					.LabelPadding(0)
					.Label()
					[
						SNumericEntryBox<int32>::BuildLabel(
							LOCTEXT("LayerStreamingDistance", "Streaming distance"), 
							FLinearColor::White, SNumericEntryBox<int32>::RedLabelBackgroundColor
							)

					]
				]
			]

			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2,2,0,0)
			[
				SNew(SButton)
				.OnClicked(this, &ST4EditWorldLayerPopup::OnClickedUpdate)
				.IsEnabled(this, &ST4EditWorldLayerPopup::CanUpdateLayer)
				.Text(LOCTEXT("Layer_Update", "Update Layer and SubLevels"))
			]

		]
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply ST4EditWorldLayerPopup::OnClickedUpdate()
{
	if (WorldModel->IsSimulating())
	{
		return FReply::Unhandled();
	}
	if (OnEditLayer.IsBound())
	{
		return OnEditLayer.Execute(LayerData);
	}
	
	return FReply::Unhandled();
}

bool ST4EditWorldLayerPopup::CanUpdateLayer() const
{
	for (const FWorldTileLayer& WorldTileLayer : ExistingLayers)
	{
		if (LayerData == WorldTileLayer)
		{
			return false;
		}
	}
	return true;
	//return LayerData.Name.Len() > 0 && !ExistingLayers.Contains(LayerData);
}


/** A class for check boxes in the layer list. 
 *	If you double click a layer checkbox, you will enable it and disable all others 
 *	If you Ctrl+Click a layer checkbox, you will add/remove it from selection list
 */
class SLayerCheckBox : public SCheckBox
{
public:
	void SetOnLayerDoubleClicked(const FOnClicked& NewLayerDoubleClicked)
	{
		OnLayerDoubleClicked = NewLayerDoubleClicked;
	}

	void SetOnLayerCtrlClicked(const FOnClicked& NewLayerCtrlClicked)
	{
		OnLayerCtrlClicked = NewLayerCtrlClicked;
	}
	
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override
	{
		if (OnLayerDoubleClicked.IsBound())
		{
			return OnLayerDoubleClicked.Execute();
		}
		else
		{
			return SCheckBox::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
		}
	}

	virtual FReply OnMouseButtonUp(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override
	{
		if (!InMouseEvent.IsControlDown())
		{
			return SCheckBox::OnMouseButtonUp(InMyGeometry, InMouseEvent);
		}
		
		if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			bIsPressed = false;

			if (IsHovered() && HasMouseCapture())
			{
				if (OnLayerCtrlClicked.IsBound())
				{
					return OnLayerCtrlClicked.Execute();
				}
			}
		}

		return FReply::Handled().ReleaseMouseCapture();
	}


private:
	FOnClicked OnLayerDoubleClicked;
	FOnClicked OnLayerCtrlClicked;
};

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------
ST4WorldLayerButton::~ST4WorldLayerButton()
{
}

void ST4WorldLayerButton::Construct(const FArguments& InArgs)
{
	WorldModel = InArgs._InWorldModel;
	WorldLayer = InArgs._WorldLayer;
	OnSelectLayer = InArgs._OnSelectLayer; // #86
	OnEditLayer = InArgs._OnEditLayer; // #86

	TSharedPtr<SLayerCheckBox> CheckBox;
	
	ChildSlot
		[
			SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.2f, 0.2f))
				.BorderImage(FEditorStyle::GetBrush("ContentBrowser.FilterButtonBorder"))
				[
					SAssignNew(CheckBox, SLayerCheckBox)
						.Style(FEditorStyle::Get(), "ToggleButtonCheckbox")
						.OnCheckStateChanged(this, &ST4WorldLayerButton::OnCheckStateChanged)
						.IsChecked(this, &ST4WorldLayerButton::IsChecked)
						.OnGetMenuContent(this, &ST4WorldLayerButton::GetRightClickMenu)
						.ToolTipText(this, &ST4WorldLayerButton::GetToolTipText)
						.Padding(3)
						[
							SNew(STextBlock)
								.Font(FEditorStyle::GetFontStyle("ContentBrowser.FilterNameFont"))
								.ShadowOffset(FVector2D(1.f, 1.f))
								.Text(FText::FromString(WorldLayer.Name))
						]
				]
		];

	CheckBox->SetOnLayerCtrlClicked(FOnClicked::CreateSP(this, &ST4WorldLayerButton::OnCtrlClicked));
	CheckBox->SetOnLayerDoubleClicked(FOnClicked::CreateSP(this, &ST4WorldLayerButton::OnDoubleClicked));
}

FReply ST4WorldLayerButton::HandleOnEditLayerClicked(const FWorldTileLayer& InWorldTileLayer) // #86
{
	OnEditLayer.ExecuteIfBound(WorldLayer, InWorldTileLayer);
	return FReply::Handled();
}

void ST4WorldLayerButton::OnCheckStateChanged(ECheckBoxState NewState)
{
	if (NewState == ECheckBoxState::Checked)
	{
		WorldModel->SetSelectedLayer(WorldLayer);
	}
	else
	{
		WorldModel->SetSelectedLayers(TArray<FWorldTileLayer>());
	}
	OnSelectLayer.ExecuteIfBound(); // #86
}

ECheckBoxState ST4WorldLayerButton::IsChecked() const
{
	return WorldModel->IsLayerSelected(WorldLayer) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

/** Handler for when the filter checkbox is double clicked */
FReply ST4WorldLayerButton::OnDoubleClicked()
{
	return FReply::Handled().ReleaseMouseCapture();
}

FReply ST4WorldLayerButton::OnCtrlClicked()
{
	WorldModel->ToggleLayerSelection(WorldLayer);
	return FReply::Handled().ReleaseMouseCapture();
}

TSharedRef<SWidget> ST4WorldLayerButton::GetRightClickMenu()
{
	// #86
#if 1
	return SNew(ST4EditWorldLayerPopup)
		//.OnCreateLayer(this, &ST4WorldMap::CreateNewLayer)
		.TileLayerData(WorldLayer)
		.InWorldModel(WorldModel)
		.OnEditLayer(this, &ST4WorldLayerButton::HandleOnEditLayerClicked); // #86
#else
	return SNullWidget::NullWidget;
#endif
}

FText ST4WorldLayerButton::GetToolTipText() const
{
	if (WorldLayer.DistanceStreamingEnabled)
	{
		return FText::Format(LOCTEXT("Layer_Distance_Tooltip", "Streaming Distance: {0}"), FText::AsNumber(WorldLayer.StreamingDistance));
	}
	else
	{
		return FText(LOCTEXT("Layer_DisabledDistance_Tooltip", "Distance Streaming Disabled"));
	}
}

#undef LOCTEXT_NAMESPACE
