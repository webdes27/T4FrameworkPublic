// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4AnimSectionDropListWidget.h"

#include "Products/Common/Helper/T4EditorAnimSetAssetSelector.h"

#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4AnimSectionDropListWidget"

/**
  * #39
 */
ST4AnimSectionDropListWidget::ST4AnimSectionDropListWidget()
	: NameTableType(ET4EngineConstantType::Nums)
{
}

ST4AnimSectionDropListWidget::~ST4AnimSectionDropListWidget()
{
	if (AnimSetAssetSelector.IsValid() && AnimSetSelectDelegate.IsValid())
	{
		AnimSetAssetSelector.Pin()->GetOnAnimSetChanged().Remove(AnimSetSelectDelegate);
		AnimSetAssetSelector.Reset();
	}
}

void ST4AnimSectionDropListWidget::Construct(
	const FArguments& InArgs,
	TSharedRef<FT4EditorAnimSetAssetSelector> InAnimSetAssetSelector,
	ET4EngineConstantType InConstantTypeType
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	PropertyHandle = InArgs._PropertyHandle;
	AnimSetAssetSelector = InAnimSetAssetSelector;
	NameTableType = InConstantTypeType;

	if (AnimSetAssetSelector.IsValid())
	{
		AnimSetSelectDelegate = AnimSetAssetSelector.Pin()->GetOnAnimSetChanged().AddRaw(
			this,
			&ST4AnimSectionDropListWidget::HandleOnAnimSetChanged
		);
	}
	
	check(ET4EngineConstantType::Nums != NameTableType);

	Create();
};

void ST4AnimSectionDropListWidget::HandleOnAnimSetChanged(UT4AnimSetAsset* InAnimSetAsset)
{
	Refresh();
}

void ST4AnimSectionDropListWidget::UpdateItemLists()
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

	TArray<FT4ConstantDataRow>& ConstantDatas = EngineConstants->GetConstantDatas(NameTableType);
	for (const FT4ConstantDataRow& Data : ConstantDatas)
	{
		FString PrefixString = TEXT("(-) ");
		FString DurationSecString = TEXT("");
		if (AnimSetAssetSelector.IsValid() && !AnimSetAssetSelector.Pin()->IsNull())
		{
			UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector.Pin()->GetAnimSetAsset();
			check(nullptr != AnimSetAsset)
			const TArray<FT4AnimSequenceInfo>* AnimSequenceInfoArray = nullptr;
			if (ET4EngineConstantType::SkillSection == NameTableType)
			{
				AnimSequenceInfoArray = &AnimSetAsset->SkillAnimSequenceArray;
			}
			else if (ET4EngineConstantType::AdditiveSection == NameTableType)
			{
				AnimSequenceInfoArray = &AnimSetAsset->AdditiveAnimSequenceArray;
			}
			else if (ET4EngineConstantType::DefaultSection == NameTableType)
			{
				AnimSequenceInfoArray = &AnimSetAsset->DefaultAnimSequenceArray;
			}
			const FT4AnimSequenceInfo* FoundInfo = AnimSequenceInfoArray->FindByKey(Data.Name);
			if (nullptr != FoundInfo)
			{
				PrefixString = TEXT("(+) ");
				DurationSecString = FString::Printf(TEXT(" (%.2f Sec)"), FoundInfo->DurationSec);
			}
		}
		TSharedPtr<FT4DropListViewItem> NewItem = MakeShareable(new FT4DropListViewItem);
		NewItem->DisplayString = FString::Printf(
			TEXT("%s[%s] %s%s"),
			*PrefixString,
			*Data.Name.ToString(),
			*Data.Description,
			*DurationSecString
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