// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4ListViewWidget.h"

#include "PropertyCustomizationHelpers.h"

#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SScrollBox.h"

#include "SListViewSelectorDropdownMenu.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4ListViewWidget"

/**
  * #39 : 
 */
ST4ListViewWidget::ST4ListViewWidget()
	: InitializeValue(NAME_None) // #88
	, NoNameDescription(TEXT("None")) // #58
	, bItemSorting(true) // #92
	, MaxHeight(77) // #104
	, SelectionMode(ESelectionMode::SingleToggle) // #104
	, bShowSearchButton(false)
	, bShowSearchAndRefreshButton(false)
	, bShowUpAndDownButton(false)
	, bUpdating(false)
{
}

ST4ListViewWidget::~ST4ListViewWidget()
{
}

void ST4ListViewWidget::OnRefresh(bool bInExecuteSelection)
{
	Refresh();
	if (bInExecuteSelection && ItemSelected.IsValid())
	{
		// #104 : 초기 선택시 선택 정보를 전달한다. 
		//        bInExecuteSelection = true 를 사용할때는 OnRefresh 를 하위 Property 를 모두 업데이트 한 후 호출할 것!
		OnItemSelected(ItemSelected);
	}
}

const FName ST4ListViewWidget::GetItemValueSelected() const // #90
{
	if (!ItemSelected.IsValid())
	{
		return NAME_None;
	}
	return ItemSelected->ValueName;
}

const int32 ST4ListViewWidget::GetItemValueIndexSelected() const // #100
{
	if (!ItemSelected.IsValid())
	{
		return -1;
	}
	return ItemSelected->ValueIndex;
}

bool ST4ListViewWidget::GetItemValueMultiSelected(
	TArray<FName>& OutItemMultiSelected
) // #104
{
	TArray<TSharedPtr<FT4ListViewItem>> SelectedItems = ListViewPtr->GetSelectedItems();
	for (TSharedPtr<FT4ListViewItem> Item : SelectedItems)
	{
		OutItemMultiSelected.Add(Item->ValueName);
	}
	return (0 < OutItemMultiSelected.Num()) ? true : false;
}

void ST4ListViewWidget::Create()
{
	SAssignNew(ListViewPtr, SListView<TSharedPtr<FT4ListViewItem>>)
		.ListItemsSource(&FilteredItemList)
		.OnSelectionChanged(this, &ST4ListViewWidget::HandleOnSelectionChanged)
		.OnMouseButtonDoubleClick(this, &ST4ListViewWidget::HandleOnDoubleClicked)
		.OnGenerateRow(this, &ST4ListViewWidget::GenerateAddElementRow)
		.SelectionMode(SelectionMode);

	if (bShowSearchAndRefreshButton)
	{
		SAssignNew(SearchBox, SSearchBox)
		.HintText(LOCTEXT("T4WidgetListViewSearchBoxHint", "Search Elements"))
		.OnTextChanged(this, &ST4ListViewWidget::HandleOnSearchBoxTextChanged)
		.OnTextCommitted(this, &ST4ListViewWidget::HandleOnSearchBoxTextCommitted);

		TSharedPtr<SHorizontalBox> ButtonBox;
		
		if (bShowUpAndDownButton)
		{
			ButtonBox = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("T4WidgetListViewRefreshBtn", "Refresh Lists"))
				.ToolTipText(LOCTEXT("T4WidgetListViewRefreshBtn_Tooltip", "Refresh Lists"))
				.OnClicked(this, &ST4ListViewWidget::HandleOnRefreshButton)
				.HAlign(HAlign_Center)
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("T4WidgetListViewUpBtn", "Move Up"))
				.ToolTipText(LOCTEXT("T4WidgetListViewUpBtn_Tooltip", "Move Up a Selected Item"))
				.OnClicked(this, &ST4ListViewWidget::HandleOnUpButton)
				.HAlign(HAlign_Center)
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("T4WidgetListViewDownBtn", "Move Down"))
				.ToolTipText(LOCTEXT("T4WidgetListViewUpBtn_Tooltip", "Move Down a Selected Item"))
				.OnClicked(this, &ST4ListViewWidget::HandleOnDownButton)
				.HAlign(HAlign_Center)
			];
		}
		else
		{
			ButtonBox = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			//.Padding(2.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("T4WidgetListViewRefreshBtn", "Refresh Lists"))
				.ToolTipText(LOCTEXT("T4WidgetListViewRefreshBtn_Tooltip", "Refresh Lists"))
				.OnClicked(this, &ST4ListViewWidget::HandleOnRefreshButton)
				.HAlign(HAlign_Center)
			];
		}

		ChildSlot
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.Padding(0.0f)
			[
				SNew(SBorder)
				//.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
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
						.MaxHeight(MaxHeight)
						.Padding(4.0f)
						[
							ListViewPtr.ToSharedRef()
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(1.0f)
						[
							ButtonBox.ToSharedRef()
						]
					]
				]
			]
		];
	}
	else if (bShowSearchButton)
	{

		SAssignNew(SearchBox, SSearchBox)
		.HintText(LOCTEXT("T4WidgetListViewSearchBoxHint", "Search Elements"))
		.OnTextChanged(this, &ST4ListViewWidget::HandleOnSearchBoxTextChanged)
		.OnTextCommitted(this, &ST4ListViewWidget::HandleOnSearchBoxTextCommitted);

		ChildSlot
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.Padding(0.0f)
			[
				SNew(SBorder)
				//.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
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
						.MaxHeight(MaxHeight)
						.Padding(4.0f)
						[
							ListViewPtr.ToSharedRef()
						]
					]
				]
			]
		];
	}
	else
	{
		ChildSlot
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.Padding(0.0f)
			[
				SNew(SBorder)
				//.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
				.Padding(2)
				[
					SNew(SBox)
					.WidthOverride(175)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.MaxHeight(200)
						.Padding(4.0f)
						[
							ListViewPtr.ToSharedRef()
						]
					]
				]
			]
		];
	}

	// Refresh(); #58 : 생성후 OnRefresh 를 직접 호출하는 것으로 정리!
};

void ST4ListViewWidget::OnItemSelected(TSharedPtr<FT4ListViewItem> InSelectedItem) // #74
{
	InitializeValue = (InSelectedItem.IsValid()) ? InSelectedItem->ValueName : NAME_None; // #88
	if (OnSelected.IsBound())
	{
		OnSelected.ExecuteIfBound(InSelectedItem->ValueName);
	}
}

void ST4ListViewWidget::OnItemDoubleClicked(TSharedPtr<FT4ListViewItem> InSelectedItem) // #81
{
	InitializeValue = (InSelectedItem.IsValid()) ? InSelectedItem->ValueName : NAME_None; // #88
	if (OnDoubleClicked.IsBound())
	{
		OnDoubleClicked.ExecuteIfBound(InSelectedItem->ValueName);
	}
}

void ST4ListViewWidget::OnItemMultiSelected(const TArray<FName>& InMultiSelected) // #104
{
	OnMultiSelected.ExecuteIfBound(InMultiSelected);
}

void ST4ListViewWidget::Refresh()
{
	if (bUpdating)
	{
		return;
	}
	ItemList.Empty();
	ItemSelected.Reset();
	{
		UpdateLists();
	}
	if (0 < ItemList.Num())
	{
		if (bItemSorting) // #92
		{
			ItemList.Sort([](const TSharedPtr<FT4ListViewItem> A, const TSharedPtr<FT4ListViewItem> B)
			{
				return A->SortOrder < B->SortOrder;
			});
		}
		if (!ItemSelected.IsValid())
		{
			ItemSelected = ItemList[0];
			InitializeValue = ItemSelected->ValueName;
		}
	}
	else
	{
		AddEmptyItem(); // #59
	}
	GenerateFilteredElementList(CurrentSearchString.ToString());
	if (ListViewPtr.IsValid())
	{
		ListViewPtr->RequestListRefresh();
		if (ItemSelected.IsValid())
		{
			TGuardValue<bool> UpdateGuard(bUpdating, true); // #71 : 자기 이벤트에 대한 업데이트 방지
			ListViewPtr->SetItemSelection(ItemSelected, true);
		}
	}
}


FReply ST4ListViewWidget::HandleOnRefreshButton()
{
	Refresh();
	return FReply::Handled();
}

FReply ST4ListViewWidget::HandleOnUpButton()
{
	if (!ItemSelected.IsValid())
	{
		return FReply::Handled();
	}
	if (OnMoveUpItem.IsBound())
	{
		OnMoveUpItem.ExecuteIfBound(ItemSelected->ValueName);
	}
	return FReply::Handled();
}

FReply ST4ListViewWidget::HandleOnDownButton()
{
	if (!ItemSelected.IsValid())
	{
		return FReply::Handled();
	}
	if (OnMoveDownItem.IsBound())
	{
		OnMoveDownItem.ExecuteIfBound(ItemSelected->ValueName);
	}
	return FReply::Handled();
}

void ST4ListViewWidget::HandleOnSelectionChanged(
	TSharedPtr<FT4ListViewItem> InNewSelection,
	ESelectInfo::Type SelectInfo
)
{
	if (bUpdating)
	{
		return;
	}
	TGuardValue<bool> UpdateGuard(bUpdating, true); // #71 : 자기 이벤트에 대한 업데이트 방지
	if (ESelectionMode::Multi == SelectionMode) // #104
	{
		TArray<FName> ItemMultiSelected;
		GetItemValueMultiSelected(ItemMultiSelected);
		OnItemMultiSelected(ItemMultiSelected);
	}
	else
	{
		if (InNewSelection.IsValid() && (SelectInfo != ESelectInfo::OnNavigation))
		{
			if (InNewSelection->ValueName != NAME_None)
			{
				ItemSelected = InNewSelection;
				OnItemSelected(ItemSelected); // #74
			}
		}
	}
}

void ST4ListViewWidget::HandleOnDoubleClicked(TSharedPtr<FT4ListViewItem> InNewSelection)
{
	if (bUpdating)
	{
		return;
	}
#if 0 // Select 가 된 상황이라 DobuleClick 이벤트가 가지 않는 문제가 있다. 복붙하다 들어간 듯...
	if (InNewSelection == ItemSelected)
	{
		return; // 같다면 진행할 필요없다.
	}
#endif
	if (InNewSelection.IsValid())
	{
		if (InNewSelection->ValueName != NAME_None)
		{
			TGuardValue<bool> UpdateGuard(bUpdating, true); // #71 : 자기 이벤트에 대한 업데이트 방지
			ItemSelected = InNewSelection;
			OnItemDoubleClicked(ItemSelected); // #81
		}
	}
}

void ST4ListViewWidget::HandleOnSearchBoxTextChanged(const FText& InSearchText)
{
	CurrentSearchString = InSearchText;

	// Generate a filtered list
	ListViewPtr->ClearSelection();
	GenerateFilteredElementList(CurrentSearchString.ToString());
	// Select first element, if any
	if (FilteredItemList.Num() > 0)
	{
		ListViewPtr->SetSelection(FilteredItemList[0], ESelectInfo::OnNavigation);
	}
	// Ask the combo to update its contents on next tick
	ListViewPtr->RequestListRefresh();
}

void ST4ListViewWidget::HandleOnSearchBoxTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
{
	// no need to handle this for now, due to the way SListViewSelectorDropdownMenu works (it eats up the Enter key and sends it to the list)
}

void ST4ListViewWidget::GenerateFilteredElementList(const FString& InSearchText)
{
	if (InSearchText.IsEmpty())
	{
		FilteredItemList.Empty();
		FilteredItemList.Append(ItemList);
	}
	else
	{
		FilteredItemList.Empty();
		for (TSharedPtr<FT4ListViewItem> Item : ItemList)
		{
			if (Item->DisplayString.Contains(InSearchText))
			{
				FilteredItemList.AddUnique(Item);
			}
		}
	}
}

TSharedRef<ITableRow> ST4ListViewWidget::GenerateAddElementRow(
	TSharedPtr<FT4ListViewItem> Entry,
	const TSharedRef<STableViewBase> &OwnerTable
) const
{
	return
		SNew(STableRow< TSharedPtr<FString> >, OwnerTable)
		.Style(&FEditorStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row"))
		.ShowSelection(true)
		[
			SNew(SBox)
			.Padding(1.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Entry->DisplayString))
				.TextStyle(FEditorStyle::Get(), TEXT("Menu.Heading"))
				.HighlightText(this, &ST4ListViewWidget::GetCurrentSearchString)
			]
		];
}

void ST4ListViewWidget::AddEmptyItem() // #95
{
	TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
	NewItem->DisplayString = FString::Printf(TEXT("Empty"));
	NewItem->ValueName = NAME_None;
	NewItem->SortOrder = -1;
	ItemList.Add(NewItem);
}

#undef LOCTEXT_NAMESPACE