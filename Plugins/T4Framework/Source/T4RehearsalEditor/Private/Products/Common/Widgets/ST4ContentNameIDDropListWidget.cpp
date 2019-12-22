// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4ContentNameIDDropListWidget.h"

#include "Products/Common/Helper/T4EditorGameplaySettingObject.h" // #60

#include "T4Frame/Public/T4Frame.h" // #60

#include "PropertyCustomizationHelpers.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SComboButton.h"
#include "SListViewSelectorDropdownMenu.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4ContentNameIDDropListWidget"

/**
  * #60
 */

void ST4ContentNameIDDropListWidget::Construct(
	const FArguments& InArgs,
	UT4EditorGameplaySettingObject* InEditorPlaySettings,
	ET4EditorGameDataType InEditorGameDataType
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	PropertyHandle = InArgs._PropertyHandle;
	EditorPlaySettings = InEditorPlaySettings;
	EditorGameDataType = InEditorGameDataType;

	if (EditorPlaySettings.IsValid())
	{
		EditorPlaySettingsDelegate = InEditorPlaySettings->GetEditorPlaySettingsChanged().AddRaw(
			this,
			&ST4ContentNameIDDropListWidget::HandleOnEditorPlaySettingsChanged
		);
	}
	
	OnRefresh();

	SAssignNew(ElementsListView, SListView<TSharedPtr<FName>>)
		.ListItemsSource(&FilteredContentNameIDList)
		.OnSelectionChanged(this, &ST4ContentNameIDDropListWidget::HandleOnSelectionChanged)
		.OnGenerateRow(this, &ST4ContentNameIDDropListWidget::GenerateAddElementRow)
		.SelectionMode(ESelectionMode::Single);

	ElementsListView->RequestListRefresh();

	SAssignNew(SearchBox, SSearchBox)
		.HintText(LOCTEXT("T4ContentNameIDDropListWidgetBoxHint", "Search Elements"))
		.OnTextChanged(this, &ST4ContentNameIDDropListWidget::HandleOnSearchBoxTextChanged)
		.OnTextCommitted(this, &ST4ContentNameIDDropListWidget::HandleOnSearchBoxTextCommitted);

	// Create the Construct arguments for SComboButton
	SComboButton::FArguments Args;
	Args.ButtonContent()
		[
			SNew(STextBlock)
			.Text(this, &ST4ContentNameIDDropListWidget::GetComboText)
		]
		.MenuContent()
		[
			SNew(SListViewSelectorDropdownMenu<TSharedPtr<FName>>, SearchBox, ElementsListView)
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
				.Padding(2)
				[
					SNew(SBox)
					.WidthOverride(175)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.Padding(1.0f)
						.AutoHeight()
						[
							SearchBox.ToSharedRef()
						]
						+ SVerticalBox::Slot()
						.MaxHeight(400)
						.Padding(4.0f)
						[
							ElementsListView.ToSharedRef()
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(1.0f)
						[
							SNew(SButton)
							.Text(LOCTEXT("T4ContentNameIDDropListWidgetRefreshBtn", "Refresh Content NameID Lists"))
							.ToolTipText(LOCTEXT("T4ContentNameIDDropListWidgetRefreshBtn_Tooltip", "Refresh Content NameID Lists"))
							.OnClicked(this, &ST4ContentNameIDDropListWidget::HandleOnRefresh)
							.HAlign(HAlign_Center)
						]
					]
				]
			]
		]
		.IsFocusable(true)
		.ContentPadding(FMargin(5, 0))
		.OnComboBoxOpened(this, &ST4ContentNameIDDropListWidget::HandleOnComboOpening);

	SAssignNew(ElementButton, SComboButton);
	ElementButton->Construct(Args);

	ElementsListView->EnableToolTipForceField(true);
	// SComboButton can automatically handle setting focus to a specified control when the combo button is opened
	ElementButton->SetMenuContentWidgetToFocus(SearchBox);

	ChildSlot
	[
		ElementButton.ToSharedRef()
	];
};

ST4ContentNameIDDropListWidget::ST4ContentNameIDDropListWidget()
	: SelectContentNameIDDesc(NAME_None)
{
}

ST4ContentNameIDDropListWidget::~ST4ContentNameIDDropListWidget()
{
	if (EditorPlaySettings.IsValid() && EditorPlaySettingsDelegate.IsValid())
	{
		EditorPlaySettings->GetEditorPlaySettingsChanged().Remove(EditorPlaySettingsDelegate);
		EditorPlaySettingsDelegate.Reset();
	}
}

void ST4ContentNameIDDropListWidget::OnRefresh()
{
	FName SelectedAnimSequenceName = NAME_None;
	if (PropertyHandle.IsValid())
	{
		FString ValueString;
		PropertyHandle->GetValue(ValueString);
		SelectedAnimSequenceName = *ValueString;
	}
	UpdateContentNameIDLists(SelectedAnimSequenceName);
	GenerateFilteredElementList(CurrentSearchString.ToString());
	if (ElementsListView.IsValid())
	{
		ElementsListView->RequestListRefresh();
	}
}

void ST4ContentNameIDDropListWidget::UpdateContentNameIDLists(const FName& InSelectedName)
{
	check(EditorPlaySettings.IsValid());
	IT4EditorGameData* EditorGameData = EditorPlaySettings->GetEditorGameData();
	if (nullptr == EditorGameData)
	{
		return;
	}
	ContentNameIDList.Empty();
	TArray<FName> ContentDataNameIDList;
	EditorGameData->GetNameIDList(EditorGameDataType, ContentDataNameIDList);
	ContentDataNameIDList.Add(NAME_None);
	for (const FName& CurrentName : ContentDataNameIDList)
	{
		ContentNameIDList.Add(MakeShareable(new FName(CurrentName)));
		if (CurrentName == InSelectedName)
		{
			SelectContentNameIDDesc = CurrentName;
		}
	}
}

void ST4ContentNameIDDropListWidget::HandleOnEditorPlaySettingsChanged()
{
	OnRefresh();
}

FReply ST4ContentNameIDDropListWidget::HandleOnRefresh()
{
	OnRefresh();
	return FReply::Handled();
}

void ST4ContentNameIDDropListWidget::HandleOnComboOpening()
{
	SearchBox->SetText(FText::GetEmpty());
}

void ST4ContentNameIDDropListWidget::HandleOnSelectionChanged(
	TSharedPtr<FName> InNewSelection,
	ESelectInfo::Type SelectInfo
)
{
	if (InNewSelection.IsValid() && (SelectInfo != ESelectInfo::OnNavigation))
	{
		SelectContentNameIDDesc = *InNewSelection;
		FString AnimSequenceName = SelectContentNameIDDesc.ToString();
		int32 StartIdx = 0;
		if (AnimSequenceName.FindChar(TEXT('['), StartIdx))
		{
			int32 LenString = AnimSequenceName.Len();
			AnimSequenceName.RemoveAt(0, StartIdx + 1);
		}
		int32 EndIdx = 0;
		if (AnimSequenceName.FindChar(TEXT(']'), EndIdx))
		{
			int32 LenString = AnimSequenceName.Len();
			AnimSequenceName.RemoveAt(EndIdx, LenString - EndIdx);
		}
		PropertyHandle->SetValue(AnimSequenceName);
		OnSelected.ExecuteIfBound(*AnimSequenceName);
		ElementButton->SetIsOpen(false, false);
	}
}

void ST4ContentNameIDDropListWidget::HandleOnSearchBoxTextChanged(const FText& InSearchText)
{
	CurrentSearchString = InSearchText;

	// Generate a filtered list
	ElementsListView->ClearSelection();
	GenerateFilteredElementList(CurrentSearchString.ToString());
	// Select first element, if any
	if (FilteredContentNameIDList.Num() > 0)
	{
		ElementsListView->SetSelection(FilteredContentNameIDList[0], ESelectInfo::OnNavigation);
	}
	// Ask the combo to update its contents on next tick
	ElementsListView->RequestListRefresh();
}

void ST4ContentNameIDDropListWidget::HandleOnSearchBoxTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
{
	// no need to handle this for now, due to the way SListViewSelectorDropdownMenu works (it eats up the Enter key and sends it to the list)
}

void ST4ContentNameIDDropListWidget::GenerateFilteredElementList(const FString& InSearchText)
{
	if (InSearchText.IsEmpty())
	{
		FilteredContentNameIDList.Empty();
		FilteredContentNameIDList.Append(ContentNameIDList);
	}
	else
	{
		FilteredContentNameIDList.Empty();
		for (TSharedPtr<FName> Name : ContentNameIDList)
		{
			if (Name->ToString().Contains(InSearchText))
			{
				FilteredContentNameIDList.AddUnique(Name);
			}
		}
	}
}

TSharedRef<ITableRow> ST4ContentNameIDDropListWidget::GenerateAddElementRow(
	TSharedPtr<FName> Entry, 
	const TSharedRef<STableViewBase> &OwnerTable
) const
{
	return
		SNew(STableRow< TSharedPtr<FString> >, OwnerTable)
		.Style(&FEditorStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.NoHoverTableRow"))
		.ShowSelection(true)
		[
			SNew(SBox)
			.Padding(1.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Entry->ToString()))
				.TextStyle(FEditorStyle::Get(), TEXT("Menu.Heading"))
				.HighlightText(this, &ST4ContentNameIDDropListWidget::GetCurrentSearchString)
			]
		];
}

FText ST4ContentNameIDDropListWidget::GetComboText() const
{
	FText ValueText = FText::FromName(SelectContentNameIDDesc);
	return ValueText;
}

#undef LOCTEXT_NAMESPACE