// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4ActionPointDropListWidget.h"

#include "Products/Common/Helper/T4EditorViewTargetSelector.h"

#include "T4Asset/Public/T4AssetDefinitions.h"
#include "T4Engine/Public/T4Engine.h"
#include "T4Engine/Public/T4EngineConstants.h" // #39

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4ActionPointDropListWidget"

/**
  * #57 
 */

ST4ActionPointDropListWidget::ST4ActionPointDropListWidget()
{
}

ST4ActionPointDropListWidget::~ST4ActionPointDropListWidget()
{
	if (ViewTargetSelector.IsValid() && ViewTargetSelectDelegate.IsValid())
	{
		ViewTargetSelector.Pin()->GetOnViewTargetChanged().Remove(ViewTargetSelectDelegate);
		ViewTargetSelector.Reset();
	}
}

void ST4ActionPointDropListWidget::Construct(
	const FArguments& InArgs,
	TSharedRef<FT4EditorViewTargetSelector> InViewTargetSelector
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	PropertyHandle = InArgs._PropertyHandle;
	ViewTargetSelector = InViewTargetSelector;

	if (ViewTargetSelector.IsValid())
	{
		ViewTargetSelectDelegate = ViewTargetSelector.Pin()->GetOnViewTargetChanged().AddRaw(
			this,
			&ST4ActionPointDropListWidget::HandleOnViewTargetChanged
		);
	}

	Create();
};

void ST4ActionPointDropListWidget::HandleOnViewTargetChanged(IT4GameObject* InViewTarget)
{
	Refresh();
}

void ST4ActionPointDropListWidget::UpdateItemLists()
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

	IT4GameObject* ViewTargetObject = nullptr;
	if (ViewTargetSelector.IsValid() && !ViewTargetSelector.Pin()->IsNull())
	{
		ViewTargetObject = ViewTargetSelector.Pin()->GetViewTarget();
		check(nullptr != ViewTargetObject)
	}

	TArray<FT4ConstantDataRow>& ConstantDatas = EngineConstants->GetConstantDatas(ET4EngineConstantType::ActionPoint);
	for (const FT4ConstantDataRow& Data : ConstantDatas)
	{
		FString PrefixString = TEXT("");
		if (nullptr != ViewTargetObject)
		{
			if (!ViewTargetObject->IsLoaded())
			{
				PrefixString = TEXT("(?) "); // 아직 로딩이 안됨
			}
			else if (ViewTargetObject->HasActionPoint(Data.Name)) // #57
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