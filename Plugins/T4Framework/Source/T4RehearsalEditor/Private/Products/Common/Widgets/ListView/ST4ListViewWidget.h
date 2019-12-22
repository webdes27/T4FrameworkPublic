// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Types/SlateEnums.h"
#include "PropertyCustomizationHelpers.h"

/**
  * #39
 */

struct FT4ListViewItem // #88
{
	FT4ListViewItem()
		: ValueName(NAME_None)
		, ValueIndex(0)
		, SortOrder(0)
	{
	}

	FString DisplayString;
	FName ValueName;
	uint32 ValueIndex;
	int32 SortOrder;
};

class SComboButton;
class IPropertyHandle;
class ST4ListViewWidget : public SCompoundWidget
{
public:
	DECLARE_DELEGATE_OneParam(FT4OnSelected, const FName);
	DECLARE_DELEGATE_OneParam(FT4OnMultiSelected, const TArray<FName>&); // #104
	DECLARE_DELEGATE_OneParam(FT4OnDoubleClicked, const FName);
	DECLARE_DELEGATE_OneParam(FT4OnMoveUpItem, const FName);
	DECLARE_DELEGATE_OneParam(FT4OnMoveDownItem, const FName);

public:
	ST4ListViewWidget();
	virtual ~ST4ListViewWidget();

	// #104 : 초기 선택시 선택 정보를 전달한다. 
	//        bInExecuteSelection = true 를 사용할때는 OnRefresh 를 하위 Property 를 모두 업데이트 한 후 호출할 것!
	void OnRefresh(bool bInExecuteSelection); // #104

	const FName GetItemValueSelected() const; // #90
	const int32 GetItemValueIndexSelected() const; // #100
	bool GetItemValueMultiSelected(TArray<FName>& OutItemMultiSelected); // #104

	int32 GetItemCount() const { return ItemList.Num(); } // #92

	void SetInitializeValue(FName InValue) { InitializeValue = InValue; } // #88
	void SetNoNameDescription(const TCHAR* InNoNameDescription) { NoNameDescription = InNoNameDescription; } // #58 : None 대신 출력할 Description

protected:
	virtual void UpdateLists() {}

	virtual void OnItemSelected(TSharedPtr<FT4ListViewItem> InSelectedItem); // #74
	virtual void OnItemDoubleClicked(TSharedPtr<FT4ListViewItem> InSelectedItem); // #81

	virtual void OnItemMultiSelected(const TArray<FName>& InMultiSelected); // #104

protected:
	void Create();
	void Refresh();

	FReply HandleOnRefreshButton();
	FReply HandleOnUpButton();
	FReply HandleOnDownButton();

	void HandleOnSelectionChanged(TSharedPtr<FT4ListViewItem> InNewSelection, ESelectInfo::Type);
	void HandleOnDoubleClicked(TSharedPtr<FT4ListViewItem> InNewSelection);

	void HandleOnSearchBoxTextChanged(const FText& InSearchText);
	void HandleOnSearchBoxTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo);

	TSharedRef<ITableRow> GenerateAddElementRow(
		TSharedPtr<FT4ListViewItem> Entry,
		const TSharedRef<STableViewBase> &OwnerTable
	) const;

	void GenerateFilteredElementList(const FString& InSearchText);
	
	FText GetCurrentSearchString() const { return CurrentSearchString; };

	void AddEmptyItem();// #95

protected:
	FT4OnSelected OnSelected;
	FT4OnDoubleClicked OnDoubleClicked;

	FT4OnMultiSelected OnMultiSelected; // #104

	FT4OnMoveUpItem OnMoveUpItem;
	FT4OnMoveDownItem OnMoveDownItem;

	TArray<TSharedPtr<FT4ListViewItem>> ItemList;
	TArray<TSharedPtr<FT4ListViewItem>> FilteredItemList;

	TSharedPtr<FT4ListViewItem> ItemSelected;

	TSharedPtr<SSearchBox> SearchBox;
	TSharedPtr<SListView<TSharedPtr<FT4ListViewItem>>> ListViewPtr;
	
	FText CurrentSearchString;
	FName InitializeValue; // #88
	FString NoNameDescription; // #58 : None 대신 출력할 Description

	bool bItemSorting; // #92

	int32 MaxHeight; // #104
	ESelectionMode::Type SelectionMode; // #104

	bool bShowSearchButton;
	bool bShowSearchAndRefreshButton;
	bool bShowUpAndDownButton;

	bool bUpdating; // #71 : 자기 이벤트에 대한 업데이트 방지
};
