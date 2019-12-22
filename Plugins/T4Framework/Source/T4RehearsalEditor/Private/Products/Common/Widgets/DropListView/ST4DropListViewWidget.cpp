// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4DropListViewWidget.h"

#include "PropertyCustomizationHelpers.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SComboButton.h"
#include "SListViewSelectorDropdownMenu.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4DropListViewWidget"

/**
  * #39 : 
 */
ST4DropListViewWidget::ST4DropListViewWidget()
	: InitializeValue(NAME_None) // #90
	, NoNameDescription(TEXT("None")) // #58
	, bItemSorting(true)
	, bUpdating(false)
{
}

ST4DropListViewWidget::~ST4DropListViewWidget()
{
}

const FName ST4DropListViewWidget::GetItemValueSelected() const // #90
{
	if (!ItemSelected.IsValid())
	{
		return NAME_None;
	}
	return ItemSelected->ValueName;
}

void ST4DropListViewWidget::Create()
{
	SAssignNew(ElementsListView, SListView<TSharedPtr<FT4DropListViewItem>>)
		.ListItemsSource(&FilteredItemList)
		.OnSelectionChanged(this, &ST4DropListViewWidget::HandleOnSelectionChanged)
		.OnGenerateRow(this, &ST4DropListViewWidget::GenerateAddElementRow)
		.SelectionMode(ESelectionMode::Single);

	SAssignNew(SearchBox, SSearchBox)
		.HintText(LOCTEXT("T4DropdownListViewSearchBoxHint", "Search Elements"))
		.OnTextChanged(this, &ST4DropListViewWidget::HandleOnSearchBoxTextChanged)
		.OnTextCommitted(this, &ST4DropListViewWidget::HandleOnSearchBoxTextCommitted);

	// Create the Construct arguments for SComboButton
	SComboButton::FArguments Args;
	Args.ButtonContent()
		[
			SNew(STextBlock)
			.Text(this, &ST4DropListViewWidget::GetComboText)
		]
		.MenuContent()
		[
			SNew(SListViewSelectorDropdownMenu<TSharedPtr<FT4DropListViewItem>>, SearchBox, ElementsListView)
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
							.Text(LOCTEXT("T4DropdownListViewRefreshBtn", "Refresh Name Lists"))
							.ToolTipText(LOCTEXT("T4DropdownListViewRefreshBtn_Tooltip", "Refresh Name Lists"))
							.OnClicked(this, &ST4DropListViewWidget::HandleOnRefreshButton)
							.HAlign(HAlign_Center)
						]
					]
				]
			]
		]
		.IsFocusable(true)
		.ContentPadding(FMargin(5, 0))
		.OnComboBoxOpened(this, &ST4DropListViewWidget::HandleOnComboOpening);

	SAssignNew(ElementButton, SComboButton);
	ElementButton->Construct(Args);

	ElementsListView->EnableToolTipForceField(true);
	ElementButton->SetMenuContentWidgetToFocus(SearchBox);

	ChildSlot
	[
		ElementButton.ToSharedRef()
	];

	// Refresh(); #58 : 생성후 OnRefresh 를 직접 호출하는 것으로 정리!
};

void ST4DropListViewWidget::Refresh()
{
	if (bUpdating)
	{
		return;
	}
	ItemList.Empty();
	ItemSelected.Reset();
	{
		UpdateItemLists();
	}
	if (bItemSorting) // #92
	{
		ItemList.Sort([](const TSharedPtr<FT4DropListViewItem> A, const TSharedPtr<FT4DropListViewItem> B)
		{
			return A->SortOrder < B->SortOrder;
		});
	}
	GenerateFilteredElementList(CurrentSearchString.ToString());
	if (ElementsListView.IsValid())
	{
		ElementsListView->RequestListRefresh();
		if (ItemSelected.IsValid())
		{
			TGuardValue<bool> UpdateGuard(bUpdating, true); // #71 : 자기 이벤트에 대한 업데이트 방지
			ElementsListView->SetItemSelection(ItemSelected, true);
		}
	}
}

FReply ST4DropListViewWidget::HandleOnRefreshButton()
{
	Refresh();
	return FReply::Handled();
}

void ST4DropListViewWidget::HandleOnComboOpening()
{
	SearchBox->SetText(FText::GetEmpty());
}

void ST4DropListViewWidget::HandleOnSelectionChanged(
	TSharedPtr<FT4DropListViewItem> InNewSelection,
	ESelectInfo::Type SelectInfo
)
{
	if (bUpdating)
	{
		return;
	}
	if (InNewSelection == ItemSelected)
	{
		return; // 같다면 진행할 필요없다.
	}
	if (InNewSelection.IsValid() && (SelectInfo != ESelectInfo::OnNavigation))
	{
		TGuardValue<bool> UpdateGuard(bUpdating, true); // #71 : 자기 이벤트에 대한 업데이트 방지
		ItemSelected = InNewSelection;
		InitializeValue = (InNewSelection.IsValid()) ? InNewSelection->ValueName : NAME_None; // #90
		if (PropertyHandle.IsValid())
		{
			PropertyHandle->SetValue(ItemSelected->ValueName);
		}
		OnSelected.ExecuteIfBound(ItemSelected->ValueName);
		ElementButton->SetIsOpen(false, false);
	}
}

void ST4DropListViewWidget::HandleOnSearchBoxTextChanged(const FText& InSearchText)
{
	CurrentSearchString = InSearchText;

	// Generate a filtered list
	ElementsListView->ClearSelection();
	GenerateFilteredElementList(CurrentSearchString.ToString());
	// Select first element, if any
	if (FilteredItemList.Num() > 0)
	{
		ElementsListView->SetSelection(FilteredItemList[0], ESelectInfo::OnNavigation);
	}
	// Ask the combo to update its contents on next tick
	ElementsListView->RequestListRefresh();
}

void ST4DropListViewWidget::HandleOnSearchBoxTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
{
	// no need to handle this for now, due to the way SListViewSelectorDropdownMenu works (it eats up the Enter key and sends it to the list)
}

void ST4DropListViewWidget::GenerateFilteredElementList(const FString& InSearchText)
{
	if (InSearchText.IsEmpty())
	{
		FilteredItemList.Empty();
		FilteredItemList.Append(ItemList);
	}
	else
	{
		FilteredItemList.Empty();
		for (TSharedPtr<FT4DropListViewItem> Name : ItemList)
		{
			if (Name->DisplayString.Contains(InSearchText))
			{
				FilteredItemList.AddUnique(Name);
			}
		}
	}
}

TSharedRef<ITableRow> ST4DropListViewWidget::GenerateAddElementRow(
	TSharedPtr<FT4DropListViewItem> Entry,
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
				.Text(FText::FromString(Entry->DisplayString))
				.TextStyle(FEditorStyle::Get(), TEXT("Menu.Heading"))
				.HighlightText(this, &ST4DropListViewWidget::GetCurrentSearchString)
			]
		];
}

FText ST4DropListViewWidget::GetComboText() const
{
	FText ValueText = ItemSelected.IsValid() ? FText::FromString(ItemSelected->DisplayString) : FText::FromName(NAME_None);
	return ValueText;
}

void ST4DropListViewWidget::AddNoSelectionItem(FName InDefaultValue) // #95
{
	TSharedPtr<FT4DropListViewItem> NewItem = MakeShareable(new FT4DropListViewItem);
	NewItem->DisplayString = NoNameDescription;
	NewItem->ValueName = NAME_None;
	ItemList.Add(NewItem);
	if (InDefaultValue == NAME_None)
	{
		ItemSelected = NewItem;
	}
}

#undef LOCTEXT_NAMESPACE