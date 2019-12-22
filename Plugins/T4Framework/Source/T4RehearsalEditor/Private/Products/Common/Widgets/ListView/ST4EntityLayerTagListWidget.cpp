// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4EntityLayerTagListWidget.h"

#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h" // #39
#include "T4Engine/Public/Playback/T4ActionPlaybackAPI.h" // #74

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4EntityLayerTagListWidget"

/**
  * #74
 */
ST4EntityLayerTagListWidget::ST4EntityLayerTagListWidget()
	: SelectedIndex(-1)
	, LayerTagData(nullptr)
{
}

ST4EntityLayerTagListWidget::~ST4EntityLayerTagListWidget()
{
}

void ST4EntityLayerTagListWidget::Construct(
	const FArguments& InArgs,
	const FT4EntityLayerTagData* InLayerTagData,
	const ET4LayerTagType InLayerTagType
)
{
	// #39
	OnSelectedByIndex = InArgs._OnSelectedByIndex; // #74, #81
	OnDoubleClickedByIndex = InArgs._OnDoubleClickedByIndex; // #81
	LayerTagData = InLayerTagData;
	LayerTagType = InLayerTagType;

	Create();
};

void ST4EntityLayerTagListWidget::UpdateLists()
{
	if (nullptr == LayerTagData)
	{
		return;
	}

	switch (LayerTagType)
	{
		case ET4LayerTagType::Weapon: // #74
			{
				uint32 NumCount = 0;
				for (const FT4EntityLayerTagWeaponData& WeaponData : LayerTagData->WeaponTags)
				{
					const FString AssetName = T4ActionPlaybackAPI::GetAssetNameFromObjectPath(
						WeaponData.WeaponEntityAsset.ToString()
					);
					TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
					const int32 CurrentIndexCount = NumCount++;
					NewItem->DisplayString = FString::Printf(
						TEXT("[%s] %s (%s)"),
						*(WeaponData.LayerTag.ToString()),
						*AssetName,
						*(WeaponData.EquipPoint.ToString())
					);
					NewItem->ValueIndex = CurrentIndexCount;
					NewItem->ValueName = *FString::Printf(TEXT("%i"), CurrentIndexCount);
					ItemList.Add(NewItem);
					if (0 <= SelectedIndex && CurrentIndexCount == SelectedIndex)
					{
						ItemSelected = NewItem;
					}
					else
					{
						ItemSelected = NewItem;
						SelectedIndex = 0;
					}
				}
			}
			break;

		case ET4LayerTagType::Conti:
			{
				uint32 NumCount = 0;
				for (const FT4EntityLayerTagContiData& ContiData : LayerTagData->ContiTags)
				{
					const FString AssetName = T4ActionPlaybackAPI::GetAssetNameFromObjectPath(
						ContiData.ContiAsset.ToString()
					);
					TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
					const int32 CurrentIndexCount = NumCount++;
					NewItem->DisplayString = FString::Printf(
						TEXT("[%s] %s"),
						*(ContiData.LayerTag.ToString()),
						*AssetName
					);
					NewItem->ValueIndex = CurrentIndexCount;
					NewItem->ValueName = *FString::Printf(TEXT("%i"), CurrentIndexCount);
					ItemList.Add(NewItem);
					if (0 <= SelectedIndex && CurrentIndexCount == SelectedIndex)
					{
						ItemSelected = NewItem;
					}
					else
					{
						ItemSelected = NewItem;
						SelectedIndex = 0;
					}
				}
			}
			break;

		case ET4LayerTagType::Material: // #81
			{
				uint32 NumCount = 0;
				for (const FT4EntityLayerTagMaterialData& OverrideMaterialData : LayerTagData->MaterialTags)
				{
					TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
					const int32 CurrentIndexCount = NumCount++;
					FString DisplayString = FString::Printf(
						TEXT("[%s] %u slots"),
						*(OverrideMaterialData.LayerTag.ToString()),
						OverrideMaterialData.OverrideMaterialData.MaterialSortedSlotNames.Num()
					);
					NewItem->ValueIndex = CurrentIndexCount;
					NewItem->ValueName = *FString::Printf(TEXT("%i"), CurrentIndexCount);
					ItemList.Add(NewItem);
					if (0 <= SelectedIndex && CurrentIndexCount == SelectedIndex)
					{
						ItemSelected = NewItem;
					}
					else
					{
						ItemSelected = NewItem;
						SelectedIndex = 0;
					}
				}
			}
			break;

		default:
			{
				UE_LOG(
					LogT4RehearsalEditor,
					Error,
					TEXT("ST4EntityLayerTagListWidget::UpdateLists : Unknown LayerTag type '%u'"),
					uint8(LayerTagType)
				);
			}
			break;
	}
}

void ST4EntityLayerTagListWidget::OnItemSelected(TSharedPtr<FT4ListViewItem> InSelectedItem) // #74
{
	//ST4ListViewWidget::OnItemSelected(InSelectedItem);
	SelectedIndex = (InSelectedItem.IsValid()) ? InSelectedItem->ValueIndex : -1;
	InitializeValue = (InSelectedItem.IsValid()) ? InSelectedItem->ValueName : NAME_None; // #88
	if (OnSelectedByIndex.IsBound())
	{
		OnSelectedByIndex.ExecuteIfBound(LayerTagType, SelectedIndex);
	}
}

void ST4EntityLayerTagListWidget::OnItemDoubleClicked(TSharedPtr<FT4ListViewItem> InSelectedItem) // #81
{
	//ST4ListViewWidget::OnItemDoubleClicked(InSelectedItem);
	SelectedIndex = (InSelectedItem.IsValid()) ? InSelectedItem->ValueIndex : -1;
	InitializeValue = (InSelectedItem.IsValid()) ? InSelectedItem->ValueName : NAME_None; // #88
	if (OnDoubleClickedByIndex.IsBound())
	{
		OnDoubleClickedByIndex.ExecuteIfBound(LayerTagType, SelectedIndex);
	}
}

#undef LOCTEXT_NAMESPACE