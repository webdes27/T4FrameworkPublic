// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4CompositePartListWidget.h"

#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h" // #39

#include "T4Engine/Public/T4EngineConstants.h" // #39

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4CompositePartListWidget"

/**
  * #39
 */
ST4CompositePartListWidget::ST4CompositePartListWidget()
	: CompositeMeshData(nullptr)
{
}

ST4CompositePartListWidget::~ST4CompositePartListWidget()
{
}

void ST4CompositePartListWidget::Construct(
	const FArguments& InArgs,
	const FT4EntityCharacterCompositeMeshData* InCompositeMeshData
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	OnDoubleClicked = InArgs._OnDoubleClicked;
	CompositeMeshData = InCompositeMeshData;

	Create();
};

void ST4CompositePartListWidget::UpdateLists()
{
	if (nullptr == CompositeMeshData)
	{
		return;
	}

	FT4EngineConstants* EngineConstants = T4EngineConstantsGet();
	check(nullptr != EngineConstants);

	for (TMap<FName, FT4EntityCharacterCompositePartMeshData>::TConstIterator It = CompositeMeshData->DefaultPartsData.CreateConstIterator(); It; ++It)
	{
		FString PrefixString = TEXT("");

		const FName CompositePartName = It.Key();
		const FT4EntityCharacterCompositePartMeshData& CompositePartData = It.Value();

		const FT4ConstantDataRow& ConstantData = EngineConstants->GetConstantData(
			ET4EngineConstantType::CompositePart,
			CompositePartName
		);

		TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
		NewItem->DisplayString = FString::Printf(
			TEXT("%s[%s] %s"),
			*PrefixString,
			*CompositePartName.ToString(),
			*ConstantData.Description
		);
		NewItem->ValueName = CompositePartName;
		NewItem->SortOrder = ConstantData.SortOrder;
		ItemList.Add(NewItem);
		if (InitializeValue == NAME_None)
		{
			ItemSelected = NewItem;
			InitializeValue = NewItem->ValueName;
		}
		else if (CompositePartName == InitializeValue)
		{
			ItemSelected = NewItem;
		}
	}
}

#undef LOCTEXT_NAMESPACE