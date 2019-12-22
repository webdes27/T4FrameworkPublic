// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4BlendSpaceDropListWidget.h"

#include "Products/Common/Helper/T4EditorAnimSetAssetSelector.h"

#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h"
#include "T4Engine/Public/T4EngineConstants.h" // #39

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4BlendSpaceDropListWidget"

/**
  * #39 : 
 */
ST4BlendSpaceDropListWidget::ST4BlendSpaceDropListWidget()
{
}

ST4BlendSpaceDropListWidget::~ST4BlendSpaceDropListWidget()
{
	if (AnimSetAssetSelector.IsValid() && AnimSetSelectDelegate.IsValid())
	{
		AnimSetAssetSelector.Pin()->GetOnAnimSetChanged().Remove(AnimSetSelectDelegate);
		AnimSetAssetSelector.Reset();
	}
}

void ST4BlendSpaceDropListWidget::Construct(
	const FArguments& InArgs,
	TSharedRef<FT4EditorAnimSetAssetSelector> InAnimSetAssetSelector
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	PropertyHandle = InArgs._PropertyHandle;
	AnimSetAssetSelector = InAnimSetAssetSelector;

	if (AnimSetAssetSelector.IsValid())
	{
		AnimSetSelectDelegate = AnimSetAssetSelector.Pin()->GetOnAnimSetChanged().AddRaw(
			this,
			&ST4BlendSpaceDropListWidget::HandleOnAnimSetChanged
		);
	}

	Create();
};

void ST4BlendSpaceDropListWidget::HandleOnAnimSetChanged(UT4AnimSetAsset* InAnimSetAsset)
{
	Refresh();
}

void ST4BlendSpaceDropListWidget::UpdateItemLists()
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

	{
		AddNoSelectionItem(SelectedValue); // #95
	}

	TArray<FT4ConstantDataRow>& ConstantDatas = EngineConstants->GetConstantDatas(ET4EngineConstantType::BlendSpace);
	for (const FT4ConstantDataRow& Data : ConstantDatas)
	{
		FString PrefixString = TEXT("");
		if (AnimSetAssetSelector.IsValid() && !AnimSetAssetSelector.Pin()->IsNull())
		{
			UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector.Pin()->GetAnimSetAsset();
			check(nullptr != AnimSetAsset)
			FT4BlendSpaceInfo* SelectedInfo = AnimSetAsset->BlendSpaceArray.FindByKey(Data.Name);
			if (nullptr != SelectedInfo)
			{
				PrefixString = TEXT("(+) ");
			}
		}
		TSharedPtr<FT4DropListViewItem> NewItem = MakeShareable(new FT4DropListViewItem);
		NewItem->DisplayString = FString::Printf(
			TEXT("%s[%s] %s"),
			*PrefixString,
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