// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4TimeTagListWidget.h"

#include "T4Asset/Classes/World/T4EnvironmentAsset.h"
#include "T4Asset/Classes/World/T4WorldAsset.h"

#include "T4Engine/Public/T4EngineConstants.h" // #39

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4TimeTagListWidget"

/**
  * #90
 */
ST4TimeTagListWidget::ST4TimeTagListWidget()
{
}

ST4TimeTagListWidget::~ST4TimeTagListWidget()
{
}

void ST4TimeTagListWidget::Construct(
	const FArguments& InArgs,
	const UT4EnvironmentAsset* InEnvironmentAsset
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	OnDoubleClicked = InArgs._OnDoubleClicked;
	EnvironmentAssetPtr = InEnvironmentAsset;

	Create();
};

void ST4TimeTagListWidget::SetEnvironmentAsset(const UT4EnvironmentAsset* InEnvironmentAsset) // #94
{
	if (EnvironmentAssetPtr != InEnvironmentAsset)
	{
		EnvironmentAssetPtr = InEnvironmentAsset;
		OnRefresh(false);
	}
}

void ST4TimeTagListWidget::UpdateLists()
{
	if (!EnvironmentAssetPtr.IsValid())
	{
		return;
	}

	FT4EngineConstants* EngineConstants = T4EngineConstantsGet();
	check(nullptr != EngineConstants);

	const FT4EnvTimeTagSetData& TimeTagSetData = EnvironmentAssetPtr->TimeTagSetData;
	for (TMap<FName, FT4EnvTimeTagData>::TConstIterator It = TimeTagSetData.TimeTagMap.CreateConstIterator(); It; ++It)
	{
		const FName TimeTagName = It.Key();
		const FT4EnvTimeTagData& TimeTagData = It.Value();

		const FT4ConstantDataRow& ConstantData = EngineConstants->GetConstantData(
			ET4EngineConstantType::TimeTag,
			TimeTagName
		);
		TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
		NewItem->DisplayString = FString::Printf(
			TEXT("[%s] %s"),
			*TimeTagName.ToString(),
			*ConstantData.Description
		);
		NewItem->ValueName = TimeTagName;
		NewItem->SortOrder = ConstantData.SortOrder;
		ItemList.Add(NewItem);
		if (InitializeValue == NAME_None)
		{
			ItemSelected = NewItem;
			InitializeValue = NewItem->ValueName;
		}
		else if (TimeTagName == InitializeValue)
		{
			ItemSelected = NewItem;
		}
	}
}

#undef LOCTEXT_NAMESPACE