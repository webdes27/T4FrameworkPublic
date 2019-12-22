// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4StanceListWidget.h"

#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h" // #39

#include "T4Engine/Public/T4EngineConstants.h" // #39

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4StanceListWidget"

/**
  * #73
 */
ST4StanceListWidget::ST4StanceListWidget()
	: StanceSetData(nullptr)
{
}

ST4StanceListWidget::~ST4StanceListWidget()
{
}

void ST4StanceListWidget::Construct(
	const FArguments& InArgs,
	const FT4EntityCharacterStanceSetData* InStanceSetData
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	OnDoubleClicked = InArgs._OnDoubleClicked;
	StanceSetData = InStanceSetData;

	Create();
};

void ST4StanceListWidget::UpdateLists()
{
	if (nullptr == StanceSetData)
	{
		return;
	}

	FT4EngineConstants* EngineConstants = T4EngineConstantsGet();
	check(nullptr != EngineConstants);

	for (TMap<FName, FT4EntityCharacterStanceData>::TConstIterator It = StanceSetData->StanceMap.CreateConstIterator(); It; ++It)
	{
		const FName StanceName = It.Key();
		const FT4EntityCharacterStanceData& StanceData = It.Value();

		const FT4ConstantDataRow& ConstantData = EngineConstants->GetConstantData(
			ET4EngineConstantType::Stance,
			StanceName
		);
		TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
		NewItem->DisplayString = FString::Printf(
			TEXT("[%s] %s"),
			*StanceName.ToString(),
			*ConstantData.Description
		);
		NewItem->ValueName = StanceName;
		NewItem->SortOrder = ConstantData.SortOrder;
		ItemList.Add(NewItem);
		if (InitializeValue == NAME_None)
		{
			ItemSelected = NewItem;
			InitializeValue = NewItem->ValueName;
		}
		else if (StanceName == InitializeValue)
		{
			ItemSelected = NewItem;
		}
	}
}

#undef LOCTEXT_NAMESPACE