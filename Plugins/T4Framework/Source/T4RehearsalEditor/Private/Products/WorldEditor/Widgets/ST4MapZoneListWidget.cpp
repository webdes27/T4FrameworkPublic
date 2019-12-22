// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4MapZoneListWidget.h"

#include "T4Engine/Classes/World/T4MapZoneVolume.h" // #92
#include "T4Engine/Public/T4EngineConstants.h" // #39
#include "T4Engine/Public/T4EngineUtility.h" // #92

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4MapZoneListWidget"

/**
  * #90
 */
ST4MapZoneListWidget::ST4MapZoneListWidget()
	: WorldRef(nullptr)
{
	bShowSearchButton = true;
}

ST4MapZoneListWidget::~ST4MapZoneListWidget()
{
}

void ST4MapZoneListWidget::Construct(
	const FArguments& InArgs,
	UWorld* InWorld
)
{
	// #39
	OnSelected = InArgs._OnSelected;
	OnDoubleClicked = InArgs._OnDoubleClicked;
	InitializeValue = InArgs._InInitializeValue; // #92
	WorldRef = InWorld;

	Create();
};

bool ST4MapZoneListWidget::HasMapZoneName(FName InName) const // #92
{
	for (TSharedPtr<FT4ListViewItem> Item : ItemList)
	{
		if (Item->ValueName == InName)
		{
			return true;
		}
	}
	return false;
}

void ST4MapZoneListWidget::UpdateLists()
{
	if (nullptr == WorldRef)
	{
		return;
	}

	FT4EngineConstants* EngineConstants = T4EngineConstantsGet();
	check(nullptr != EngineConstants);

	TArray<AT4MapZoneVolume*> MapZoneVolumes;
	if (!T4EngineUtility::GetMapZomeVolumesOnWorld(WorldRef, MapZoneVolumes))
	{
		return;
	}

	for (AT4MapZoneVolume* ZoneVolume  : MapZoneVolumes)
	{
		check(nullptr != ZoneVolume);

		const FT4ConstantDataRow& ConstantData = EngineConstants->GetConstantData(
			ET4EngineConstantType::MapZone,
			ZoneVolume->ZoneName
		);
		const int32 SortOrder = ZoneVolume->GetBlendPriority(); // ConstantData.SortOrder;
		TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
		NewItem->DisplayString = FString::Printf(
			TEXT("[%s] %s (BlendOrder:%i)"),
			*ZoneVolume->ZoneName.ToString(),
			(!ConstantData.Description.IsEmpty()) ? *ConstantData.Description : TEXT("Unregistered Zone Name"),
			SortOrder
		);
		NewItem->ValueName = ZoneVolume->ZoneName;
		NewItem->SortOrder = SortOrder; // ConstantData.SortOrder;
		ItemList.Add(NewItem);
		if (InitializeValue == NAME_None)
		{
			ItemSelected = NewItem;
			InitializeValue = NewItem->ValueName;
		}
		else if (ZoneVolume->ZoneName == InitializeValue)
		{
			ItemSelected = NewItem;
		}
	}
}

#undef LOCTEXT_NAMESPACE