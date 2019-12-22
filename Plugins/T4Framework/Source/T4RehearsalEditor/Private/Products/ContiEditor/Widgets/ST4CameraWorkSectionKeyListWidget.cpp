// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4CameraWorkSectionKeyListWidget.h"

#include "T4Asset/Public/Action/T4ActionContiStructs.h"
#include "T4Engine/Public/T4EngineConstants.h" // #39

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "ST4CameraWorkSectionKeyListWidget"

/**
  * #100
 */
ST4CameraWorkSectionKeyListWidget::ST4CameraWorkSectionKeyListWidget()
	: SelectedChannelKey(INDEX_NONE)
	, CameraWorkSectionDataRef(nullptr)
{
}

ST4CameraWorkSectionKeyListWidget::~ST4CameraWorkSectionKeyListWidget()
{
}

void ST4CameraWorkSectionKeyListWidget::Construct(
	const FArguments& InArgs,
	const FT4CameraWorkSectionData* InCameraWorkSectionData
)
{
	// #39
	OnSelectedByIndex = InArgs._OnSelectedByIndex; // #74, #81
	OnDoubleClickedByIndex = InArgs._OnDoubleClickedByIndex; // #81
	OnGetValueIndexSelcted = InArgs._OnGetValueIndexSelcted; // #81
	CameraWorkSectionDataRef = InCameraWorkSectionData;

	Create();
};

inline FString GetEnumToString(ET4BuiltInEasing InBuiltInEasing)
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("ET4BuiltInEasing"), true);
	if (nullptr == EnumPtr)
	{
		return FString("Invalid");
	}
	return EnumPtr->GetNameStringByIndex((int32)InBuiltInEasing);
}

void ST4CameraWorkSectionKeyListWidget::UpdateLists()
{
	if (nullptr == CameraWorkSectionDataRef)
	{
		return;
	}

	FT4EngineConstants* EngineConstants = T4EngineConstantsGet();
	check(nullptr != EngineConstants);

	const int32 ValueIndexSelected = OnGetValueIndexSelcted.Execute();

	for (const FT4CameraWorkSectionKeyData& KeyData : CameraWorkSectionDataRef->KeyDatas)
	{
		const FT4ConstantDataRow& ConstantData = EngineConstants->GetConstantData(
			ET4EngineConstantType::ActionPoint,
			KeyData.LookAtPoint
		);

		TSharedPtr<FT4ListViewItem> NewItem = MakeShareable(new FT4ListViewItem);
		NewItem->DisplayString = FString::Printf(
			TEXT("[%.2f] %s (%s, %s%s, %.1f, %.0f)"),
			KeyData.StartTimeSec,
			*(ConstantData.Description),
			*(KeyData.LookAtPoint.ToString()),
			(KeyData.bInverse) ? TEXT("INV, ") : TEXT(""),
			*GetEnumToString(KeyData.EasingCurve),
			KeyData.Distance,
			KeyData.FOVDegree
		);
		NewItem->ValueIndex = KeyData.ChannelKey;
		NewItem->ValueName = *FString::Printf(TEXT("%i"), KeyData.ChannelKey);
		NewItem->SortOrder = KeyData.ChannelKey;
		ItemList.Add(NewItem);
		{
			if (INDEX_NONE != ValueIndexSelected && KeyData.ChannelKey == ValueIndexSelected)
			{
				ItemSelected = NewItem;
				SelectedChannelKey = KeyData.ChannelKey;
			}
			else if (!ItemSelected.IsValid())
			{
				ItemSelected = NewItem;
				SelectedChannelKey = KeyData.ChannelKey;
			}
		}
	}
}

void ST4CameraWorkSectionKeyListWidget::OnItemSelected(TSharedPtr<FT4ListViewItem> InSelectedItem) // #74
{
	//ST4ListViewWidget::OnItemSelected(InSelectedItem);
	SelectedChannelKey = (InSelectedItem.IsValid()) ? InSelectedItem->ValueIndex : -1;
	InitializeValue = (InSelectedItem.IsValid()) ? InSelectedItem->ValueName : NAME_None; // #88
	if (OnSelectedByIndex.IsBound())
	{
		OnSelectedByIndex.ExecuteIfBound(SelectedChannelKey);
	}
}

void ST4CameraWorkSectionKeyListWidget::OnItemDoubleClicked(TSharedPtr<FT4ListViewItem> InSelectedItem) // #81
{
	//ST4ListViewWidget::OnItemDoubleClicked(InSelectedItem);
	SelectedChannelKey = (InSelectedItem.IsValid()) ? InSelectedItem->ValueIndex : -1;
	InitializeValue = (InSelectedItem.IsValid()) ? InSelectedItem->ValueName : NAME_None; // #88
	if (OnDoubleClickedByIndex.IsBound())
	{
		OnDoubleClickedByIndex.ExecuteIfBound(SelectedChannelKey);
	}
}

#undef LOCTEXT_NAMESPACE