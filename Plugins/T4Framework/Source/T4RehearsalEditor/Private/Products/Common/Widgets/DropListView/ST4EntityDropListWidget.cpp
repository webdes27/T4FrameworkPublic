// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4EntityDropListWidget.h"

#include "T4Asset/Public/Entity/T4Entity.h"
#include "T4Asset/Classes/Entity/T4EntityAsset.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4EntityDropListWidget"

/**
  * #87
 */

ST4EntityDropListWidget::ST4EntityDropListWidget()
	: EntityType(ET4EntityType::None)
{
}

ST4EntityDropListWidget::~ST4EntityDropListWidget()
{
}

void ST4EntityDropListWidget::Construct(
	const FArguments& InArgs,
	ET4EntityType InEntityType
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	PropertyHandle = InArgs._PropertyHandle;
	EntityType = InEntityType;

	Create();
}

void ST4EntityDropListWidget::UpdateItemLists()
{
	FName SelectedValue = InitializeValue;
	if (PropertyHandle.IsValid())
	{
		FString ValueString;
		PropertyHandle->GetValue(ValueString);
		SelectedValue = *ValueString;
	}

	IT4EntityManager* EntityManager = T4AssetEntityManagerGet();
	check(nullptr != EntityManager);

	{
		AddNoSelectionItem(SelectedValue); // #95
	}

	TArray<UT4EntityAsset*> SelectEntities;
	EntityManager->GetEntities(EntityType, SelectEntities);
	for (const UT4EntityAsset* EntityAsset : SelectEntities)
	{
		TSharedPtr<FT4DropListViewItem> NewItem = MakeShareable(new FT4DropListViewItem);
		NewItem->DisplayString = FString::Printf(
			TEXT("[%s] %s"),
			EntityAsset->GetEntityTypeString(),
			*EntityAsset->GetEntityDisplayName()
		);
		NewItem->ValueName = EntityAsset->GetEntityKeyPath();
		ItemList.Add(NewItem);
		if (NewItem->ValueName == SelectedValue)
		{
			ItemSelected = NewItem;
		}
	}
}

#undef LOCTEXT_NAMESPACE