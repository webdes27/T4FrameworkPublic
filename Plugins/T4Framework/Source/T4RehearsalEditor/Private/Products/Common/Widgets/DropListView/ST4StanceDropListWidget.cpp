// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4StanceDropListWidget.h"

#include "Products/Common/Helper/T4EditorViewTargetSelector.h"

#include "T4Asset/Public/T4AssetDefinitions.h"
#include "T4Engine/Public/T4Engine.h"
#include "T4Engine/Public/T4EngineConstants.h" // #39

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4StanceDropListWidget"

/**
  * #73
 */

ST4StanceDropListWidget::ST4StanceDropListWidget()
	: bCheckValidStanceList(false)
{
}

ST4StanceDropListWidget::~ST4StanceDropListWidget()
{
}

void ST4StanceDropListWidget::Construct(
	const FArguments& InArgs
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	PropertyHandle = InArgs._PropertyHandle;

	Create();
}

void ST4StanceDropListWidget::Construct(
	const FArguments& InArgs,
	const TSet<FName>& InStanceNamelist
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	PropertyHandle = InArgs._PropertyHandle;
	ValidStanceNamelist = InStanceNamelist;
	bCheckValidStanceList = true;

	Create();
}

void ST4StanceDropListWidget::UpdateItemLists()
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

	if (!bCheckValidStanceList)
	{
		AddNoSelectionItem(SelectedValue); // #95
	}

	TArray<FT4ConstantDataRow>& ConstantDatas = EngineConstants->GetConstantDatas(ET4EngineConstantType::Stance);
	for (const FT4ConstantDataRow& Data : ConstantDatas)
	{
		if (bCheckValidStanceList && !ValidStanceNamelist.Contains(Data.Name))
		{
			continue;
		}
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
}

#undef LOCTEXT_NAMESPACE