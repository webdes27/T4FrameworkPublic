// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4ReactionDropListWidget.h"

#include "Products/Common/Helper/T4EditorViewTargetSelector.h"

#include "T4Asset/Public/T4AssetDefinitions.h"
#include "T4Engine/Public/T4Engine.h"
#include "T4Engine/Public/T4EngineConstants.h" // #39

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4ReactionDropListWidget"

/**
  * #73
 */

ST4ReactionDropListWidget::ST4ReactionDropListWidget()
	: bCheckValidReactionList(false)
{
}

ST4ReactionDropListWidget::~ST4ReactionDropListWidget()
{
}

void ST4ReactionDropListWidget::Construct(
	const FArguments& InArgs,
	TSharedRef<FT4EditorViewTargetSelector> InViewTargetSelector
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	PropertyHandle = InArgs._PropertyHandle;
	ViewTargetSelector = InViewTargetSelector;

	Create();
}

void ST4ReactionDropListWidget::Construct(
	const FArguments& InArgs,
	const TSet<FName>& InReactionNamelist,
	TSharedRef<FT4EditorViewTargetSelector> InViewTargetSelector
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	PropertyHandle = InArgs._PropertyHandle;
	ViewTargetSelector = InViewTargetSelector;
	ValidReactionNamelist = InReactionNamelist;
	bCheckValidReactionList = true;

	Create();
}

void ST4ReactionDropListWidget::HandleOnViewTargetChanged(IT4GameObject* InViewTarget)
{
	Refresh();
}

void ST4ReactionDropListWidget::UpdateItemLists()
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

	if (!bCheckValidReactionList)
	{
		AddNoSelectionItem(SelectedValue); // #95
	}

	IT4GameObject* ViewTargetObject = nullptr;
	if (ViewTargetSelector.IsValid() && !ViewTargetSelector.Pin()->IsNull())
	{
		ViewTargetObject = ViewTargetSelector.Pin()->GetViewTarget();
		check(nullptr != ViewTargetObject)
	}

	TArray<FT4ConstantDataRow>& ConstantDatas = EngineConstants->GetConstantDatas(ET4EngineConstantType::Reaction);
	for (const FT4ConstantDataRow& Data : ConstantDatas)
	{
		if (bCheckValidReactionList && !ValidReactionNamelist.Contains(Data.Name))
		{
			continue;
		}
		FString PrefixString = TEXT("");
		if (nullptr != ViewTargetObject)
		{
			ET4LayerTagType ReturnLayerTagType = ET4LayerTagType::All;
			if (!ViewTargetObject->IsLoaded())
			{
				PrefixString = TEXT("(?) "); // 아직 로딩이 안됨
			}
			else if (ViewTargetObject->HasReaction(Data.Name))
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