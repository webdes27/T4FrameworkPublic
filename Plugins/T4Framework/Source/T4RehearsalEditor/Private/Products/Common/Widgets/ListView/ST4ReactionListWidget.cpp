// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4ReactionListWidget.h"

#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h" // #39

#include "T4Engine/Public/T4EngineConstants.h" // #39

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4ReactionListWidget"

/**
  * #73
 */
ST4ReactionListWidget::ST4ReactionListWidget()
	: ReactionSetData(nullptr)
{
}

ST4ReactionListWidget::~ST4ReactionListWidget()
{
}

void ST4ReactionListWidget::Construct(
	const FArguments& InArgs,
	const FT4EntityCharacterReactionSetData* InReactionSetData
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	OnDoubleClicked = InArgs._OnDoubleClicked;
	ReactionSetData = InReactionSetData;

	Create();
};

void ST4ReactionListWidget::UpdateLists()
{
	if (nullptr == ReactionSetData)
	{
		return;
	}

	FT4EngineConstants* EngineConstants = T4EngineConstantsGet();
	check(nullptr != EngineConstants);

	for (TMap<FName, FT4EntityCharacterReactionData>::TConstIterator It = ReactionSetData->ReactionMap.CreateConstIterator(); It; ++It)
	{
		const FName ReactionName = It.Key();
		const FT4EntityCharacterReactionData& ReactionData = It.Value();

		const FT4ConstantDataRow& ConstantData = EngineConstants->GetConstantData(
			ET4EngineConstantType::Reaction,
			ReactionName
		);
		TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
		NewItem->DisplayString = FString::Printf(
			TEXT("[%s] %s"),
			*ReactionName.ToString(),
			*ConstantData.Description
		);
		NewItem->ValueName = ReactionName;
		NewItem->SortOrder = ConstantData.SortOrder;
		ItemList.Add(NewItem);
		if (InitializeValue == NAME_None)
		{
			ItemSelected = NewItem;
			InitializeValue = NewItem->ValueName;
		}
		else if (ReactionName == InitializeValue)
		{
			ItemSelected = NewItem;
		}
	}
}

#undef LOCTEXT_NAMESPACE