// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4PointOfInterestListWidget.h"

#include "T4Asset/Classes/Common/T4CommonAssetStructs.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4PointOfInterestListWidget"

/**
  * #100
 */
ST4PointOfInterestListWidget::ST4PointOfInterestListWidget()
	: SelectedIndex(-1)
	, TestAutomatioRef(nullptr)
{
}

ST4PointOfInterestListWidget::~ST4PointOfInterestListWidget()
{
}

void ST4PointOfInterestListWidget::Construct(
	const FArguments& InArgs,
	const FT4EditorTestAutomation* InTestAutomation
)
{
	// #39
	OnSelectedByIndex = InArgs._OnSelectedByIndex; // #74, #81
	OnDoubleClickedByIndex = InArgs._OnDoubleClickedByIndex; // #81
	TestAutomatioRef = InTestAutomation;

	Create();
};

void ST4PointOfInterestListWidget::UpdateLists()
{
	if (nullptr == TestAutomatioRef)
	{
		return;
	}

	uint32 NumCount = 0;
	for (const FT4EditorPointOfInterest& PointOfInterest : TestAutomatioRef->PointOfInterests)
	{
		const int32 CurrentIndexCount = NumCount++;
		TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
		NewItem->DisplayString = FString::Printf(
			TEXT("[%s] %s (%.1f Hour)"),
			(PointOfInterest.MapEntityName == NAME_None) ? TEXT("Preview") : *FPackageName::ObjectPathToObjectName(PointOfInterest.MapEntityName.ToString()),
			(!PointOfInterest.Name.IsEmpty()) ? *(PointOfInterest.Name) : *(PointOfInterest.SpawnLocation.ToString()),
			PointOfInterest.GameTimeHour
		);
		NewItem->ValueIndex = CurrentIndexCount;
		NewItem->ValueName = *FString::Printf(TEXT("%i"), CurrentIndexCount);
		NewItem->SortOrder = NumCount;
		ItemList.Add(NewItem);
		if (-1 == SelectedIndex || SelectedIndex == CurrentIndexCount)
		{
			ItemSelected = NewItem;
			SelectedIndex = CurrentIndexCount;
		}
	}
}

void ST4PointOfInterestListWidget::OnItemSelected(TSharedPtr<FT4ListViewItem> InSelectedItem) // #74
{
	//ST4ListViewWidget::OnItemSelected(InSelectedItem);
	SelectedIndex = (InSelectedItem.IsValid()) ? InSelectedItem->ValueIndex : -1;
	InitializeValue = (InSelectedItem.IsValid()) ? InSelectedItem->ValueName : NAME_None; // #88
	if (OnSelectedByIndex.IsBound())
	{
		OnSelectedByIndex.ExecuteIfBound(SelectedIndex);
	}
}

void ST4PointOfInterestListWidget::OnItemDoubleClicked(TSharedPtr<FT4ListViewItem> InSelectedItem) // #81
{
	//ST4ListViewWidget::OnItemDoubleClicked(InSelectedItem);
	SelectedIndex = (InSelectedItem.IsValid()) ? InSelectedItem->ValueIndex : -1;
	InitializeValue = (InSelectedItem.IsValid()) ? InSelectedItem->ValueName : NAME_None; // #88
	if (OnDoubleClickedByIndex.IsBound())
	{
		OnDoubleClickedByIndex.ExecuteIfBound(SelectedIndex);
	}
}

#undef LOCTEXT_NAMESPACE