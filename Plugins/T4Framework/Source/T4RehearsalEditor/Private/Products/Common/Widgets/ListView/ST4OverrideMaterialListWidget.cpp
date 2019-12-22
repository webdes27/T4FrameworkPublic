// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4OverrideMaterialListWidget.h"

#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h" // #39

#include "T4Engine/Public/T4EngineConstants.h" // #39
#include "T4Engine/Public/Playback/T4ActionPlaybackAPI.h" // #74

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4OverrideMaterialListWidget"

/**
  * #73
 */
ST4OverrideMaterialListWidget::ST4OverrideMaterialListWidget()
	: OverrideMaterialData(nullptr)
{
}

ST4OverrideMaterialListWidget::~ST4OverrideMaterialListWidget()
{
}

void ST4OverrideMaterialListWidget::Construct(
	const FArguments& InArgs,
	const FT4EntityOverrideMaterialData* InOverrideMaterialData
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	OnDoubleClicked = InArgs._OnDoubleClicked;
	OverrideMaterialData = InOverrideMaterialData;

	Create();
};

void ST4OverrideMaterialListWidget::UpdateLists()
{
	if (nullptr == OverrideMaterialData)
	{
		return;
	}

	FT4EngineConstants* EngineConstants = T4EngineConstantsGet();
	check(nullptr != EngineConstants);

	for (TMap<FName, TSoftObjectPtr<UMaterialInterface>>::TConstIterator It = OverrideMaterialData->MaterialMap.CreateConstIterator(); It; ++It)
	{
		const FName SlotName = It.Key();
		TSoftObjectPtr<UMaterialInterface> OverrideMaterial = It.Value();
		TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
		if (!OverrideMaterial.IsNull())
		{
			const FString AssetName = T4ActionPlaybackAPI::GetAssetNameFromObjectPath(
				OverrideMaterial.ToString()
			);
			NewItem->DisplayString = FString::Printf(
				TEXT("[%s] %s"),
				*SlotName.ToString(),
				*AssetName
			);
		}
		else
		{
			NewItem->DisplayString = FString::Printf(TEXT("[%s] No Override"), *SlotName.ToString());
		}
		NewItem->ValueName = SlotName;
		ItemList.Add(NewItem);
		if (InitializeValue == NAME_None)
		{
			ItemSelected = NewItem;
			InitializeValue = NewItem->ValueName;
		}
		else if (SlotName == InitializeValue)
		{
			ItemSelected = NewItem;
		}
	}
}

#undef LOCTEXT_NAMESPACE