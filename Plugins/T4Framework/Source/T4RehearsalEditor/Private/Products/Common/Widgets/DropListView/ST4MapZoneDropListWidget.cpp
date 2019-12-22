// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4MapZoneDropListWidget.h"

#include "T4Asset/Public/T4AssetDefinitions.h"
#include "T4Engine/Public/T4Engine.h"
#include "T4Engine/Public/T4EngineConstants.h" // #39

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4MapZoneDropListWidget"

/**
  * #92
 */
ST4MapZoneDropListWidget::ST4MapZoneDropListWidget()
{
}

ST4MapZoneDropListWidget::~ST4MapZoneDropListWidget()
{
}

void ST4MapZoneDropListWidget::Construct(
	const FArguments& InArgs
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	PropertyHandle = InArgs._PropertyHandle;

	Create();
}


void ST4MapZoneDropListWidget::UpdateItemLists()
{
	FName SelectedValue = InitializeValue;
	if (PropertyHandle.IsValid())
	{
		FString ValueString;
		PropertyHandle->GetValue(ValueString);
		SelectedValue = *ValueString;
	}

	FT4EngineConstants* EngineConstants = T4EngineConstantsGet();
	check(nullptr != EngineConstants);

	if (SelectedValue == NAME_None)
	{
		AddNoSelectionItem(SelectedValue); // #95
	}

	TArray<FT4ConstantDataRow>& ConstantDatas = EngineConstants->GetConstantDatas(ET4EngineConstantType::MapZone);
	for (const FT4ConstantDataRow& Data : ConstantDatas)
	{
		TSharedPtr<FT4DropListViewItem> NewItem = MakeShareable(new FT4DropListViewItem);
		NewItem->DisplayString = FString::Printf(
			TEXT("[%s] %s"),
			*Data.Name.ToString(),
			*Data.Description
		);
		NewItem->ValueName = Data.Name;
		NewItem->SortOrder = Data.SortOrder;
		ItemList.Add(NewItem);
		if (Data.Name == SelectedValue)
		{
			ItemSelected = NewItem;
		}
	}

	if (!ItemSelected.IsValid() && SelectedValue != NAME_None)
	{
		// #92 : 등록되지 않은 Value 가 선택되어 있을 경우에 대한 처리!
		TSharedPtr<FT4DropListViewItem> NewItem = MakeShareable(new FT4DropListViewItem);
		NewItem->DisplayString = FString::Printf(
			TEXT("[%s] Unregistered Name"),
			*SelectedValue.ToString()
		);
		NewItem->ValueName = SelectedValue;
		NewItem->SortOrder = 0;
		ItemList.Add(NewItem);
		ItemSelected = NewItem;
	}
}

#undef LOCTEXT_NAMESPACE