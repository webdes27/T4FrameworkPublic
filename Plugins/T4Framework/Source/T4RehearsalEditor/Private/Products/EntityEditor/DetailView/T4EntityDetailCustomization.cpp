// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4EntityDetailCustomization.h"
#include "Products/Common/DetailView/T4DetailCustomizationMacros.h"

#include "Products/EntityEditor/ViewModel/T4EntityViewModel.h"

#include "Products/Common/Widgets/DropListView/ST4EquipPointDropListWidget.h" // #74
#include "Products/Common/Widgets/DropListView/ST4LayerTagDropListWidget.h" // #74
#include "Products/Common/Widgets/DropListView/ST4CompositePartDropListWidget.h" // #38
#include "Products/Common/Widgets/DropListView/ST4ActionPointDropListWidget.h" // #76

#include "Products/Common/Widgets/ListView/ST4EntityLayerTagListWidget.h" // #74
#include "Products/Common/Widgets/ListView/ST4PointOfInterestListWidget.h" // #103

#include "T4Asset/Classes/Entity/T4MapEntityAsset.h" // #79
#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h" // #71

#include "Animation/Skeleton.h" // #74

#include "AssetData.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4EntityDetailCustomization"

static const TCHAR* T4DefaultCategoryName = TEXT("Default");

/**
  * http://api.unrealengine.com/KOR/Programming/Slate/DetailsCustomization/
 */

TSharedRef<IDetailCustomization> FT4EntityDetailCustomization::MakeInstance(
	TSharedPtr<FT4EntityViewModel> InEntityViewModel
)
{
	return MakeShared<FT4EntityDetailCustomization>(InEntityViewModel);
}

FT4EntityDetailCustomization::FT4EntityDetailCustomization(
	TSharedPtr<FT4EntityViewModel> InEntityViewModel
)	: ViewModelPtr(InEntityViewModel)
	, DetailLayoutPtr(nullptr)
{
}

void FT4EntityDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& InBuilder)
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	DetailLayoutPtr = &InBuilder;
	UT4EntityAsset* EntityAsset = ViewModelPtr->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return;
	}

	EditorTransientEntityDataHandle = DEFINE_DETAIL_GET_CLASS_PROPERTY_MACRO(UT4EntityAsset, EditorTransientData); // #81
	if (EditorTransientEntityDataHandle.IsValid())
	{
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientEntityDataHandle, TransientLayerTagWeaponAsset); // #74
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientEntityDataHandle, TransientLayerTagContiAsset); // #74

		InBuilder.HideProperty(EditorTransientEntityDataHandle);
	}

	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4EntityAsset, LayerTagData); // #74, #81

	{
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4EntityAsset, TestAutomation);
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4EntityAsset, ThumbnailCameraInfo);

		if (ET4EntityType::Map != EntityAsset->GetEntityType())
		{
			// #103
			static const TCHAR* EditorCategoryName = TEXT("Editor");
			IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(EditorCategoryName);

			PointOfInterestListWidgetPtr = SNew(ST4PointOfInterestListWidget, &EntityAsset->TestAutomation)
				.OnSelectedByIndex(this, &FT4EntityDetailCustomization::HandleOnAutomationPointOfInterestSelected)
				.OnDoubleClickedByIndex(this, &FT4EntityDetailCustomization::HandleOnAutomationPointOfInterestGo);

			DetailCategoryBuilder
				.AddCustomRow(LOCTEXT("T4EntityAutomationPointOfInterestList", "POI"))
				.NameContent()
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(LOCTEXT("T4EntityAutomationPointOfInterestListBoxTitle", "Point Of Interests"))
				]
			.ValueContent()
				.HAlign(HAlign_Fill)
				.MaxDesiredWidth(400.0f)
				[
					PointOfInterestListWidgetPtr.ToSharedRef()
				];

			DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(TestAutomationHandle, TransientName); // #103

			{
				DetailCategoryBuilder
					.AddCustomRow(LOCTEXT("T4EntityAutomationPointOfInterestSelector", "EntityAutomationPointOfInterest"))
					.NameContent()
					[
						SNew(STextBlock)
					]
					.ValueContent()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(2.0f, 0.0f)
						[
							SNew(SButton)
							.Text(LOCTEXT("T4ContiAutomationTeleportUpdateBtn", "Update POI"))
							.ToolTipText(LOCTEXT("T4ContiAutomationTeleportUpdateBtn_Tooltip", "Update Point Of Interest"))
							.OnClicked(this, &FT4EntityDetailCustomization::HandleOnAutomationPointOfInterestUpdate)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(2.0f, 0.0f)
						[
							SNew(SButton)
							.Text(LOCTEXT("T4EntityAutomationTeleportAddBtn", "Add POI"))
							.ToolTipText(LOCTEXT("T4EntityAutomationTeleportAddBtn_Tooltip", "Add Point Of Interest"))
							.OnClicked(this, &FT4EntityDetailCustomization::HandleOnAutomationPointOfInterestAdd)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(2.0f, 0.0f)
						[
							SNew(SButton)
							.Text(LOCTEXT("T4EntityAutomationTeleportRemoveBtn", "Remove POI"))
							.ToolTipText(LOCTEXT("T4EntityAutomationTeleportRemoveBtn_Tooltip", "Remove Selected POI"))
							.OnClicked(this, &FT4EntityDetailCustomization::HandleOnAutomationPointOfInterestRemove)
						]
					];
			}

			PointOfInterestListWidgetPtr->OnRefresh(true);
		}
		//DetailCategoryBuilder.AddProperty(ThumbnailCameraInfoHandle);
	}

	// #T4_ADD_ENTITY_TAG

	// #72 : Warning : ST4EntityObjectWidget::Construct 에 ClassType 을 추가해주어야 정상적으로 동작함!!

	const ET4EntityType EntityType = EntityAsset->GetEntityType();
	switch (EntityType)
	{
		case ET4EntityType::Character:
			CustomizeCharacterEntityDetails(InBuilder, EntityAsset);
			break;

		case ET4EntityType::Costume:
			CustomizeCostumeEntityDetails(InBuilder, EntityAsset); // #72
			CustomizeItemCommonEntityDetails(InBuilder, EntityAsset); // #80
			break;

		case ET4EntityType::Weapon:
			CustomizeWeaponEntityDetails(InBuilder, EntityAsset); // #80
			CustomizeItemCommonEntityDetails(InBuilder, EntityAsset); // #80
			break;

		case ET4EntityType::Map:
			CustomizeMapEntityDetails(InBuilder, EntityAsset); // #79
			break;

		case ET4EntityType::Zone:
			CustomizeZoneEntityDetails(InBuilder, EntityAsset); // #94
			break;

		case ET4EntityType::Prop:
			{
				// TODO
			}
			break;

		default:
			{
				UE_LOG(
					LogT4RehearsalEditor,
					Error,
					TEXT("FT4EntityDetailCustomization : Unknown EntityType '%u'"),
					uint8(EntityType)
				);
			}
			break;
	}
}

void FT4EntityDetailCustomization::CustomizeMapEntityDetails(
	IDetailLayoutBuilder& InBuilder, 
	UT4EntityAsset* InEntityAsset
) // #79
{
	check(nullptr != InEntityAsset);

	UT4MapEntityAsset* MapEntityAsset = Cast<UT4MapEntityAsset>(InEntityAsset);
	if (nullptr == MapEntityAsset)
	{
		return;
	}
	{
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4MapEntityAsset, MapData);
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4MapEntityAsset, LevelThumbnailDatas); // #84, #104 : 에디터 전용

		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(T4DefaultCategoryName);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(MapDataHandle, LevelAsset);
		if (LevelAssetHandle.IsValid())
		{
			LevelAssetHandle->SetOnPropertyValueChanged(
				FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnMapChange)
			);
		}
	}
}

void FT4EntityDetailCustomization::HandleOnAutomationPointOfInterestSelected(int32 InSelectedIndex) // #103
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	if (-1 == InSelectedIndex)
	{
		return;
	}
	ViewModelPtr->SelectPointOfInterest(InSelectedIndex);
}

void FT4EntityDetailCustomization::HandleOnAutomationPointOfInterestGo(int32 InSelectedIndex) // #103
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	if (-1 == InSelectedIndex)
	{
		return;
	}
	ViewModelPtr->TravelPointOfInterest(InSelectedIndex);
}

FReply FT4EntityDetailCustomization::HandleOnAutomationPointOfInterestUpdate() // #103
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	if (!PointOfInterestListWidgetPtr.IsValid())
	{
		return FReply::Handled();
	}
	int32 POISelected = PointOfInterestListWidgetPtr->GetItemValueIndexSelected();
	if (-1 == POISelected)
	{
		return FReply::Handled();
	}
	ViewModelPtr->UpdatePointOfInterest(POISelected);
	PointOfInterestListWidgetPtr->OnRefresh(false);
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnAutomationPointOfInterestAdd() // #103
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	if (!PointOfInterestListWidgetPtr.IsValid())
	{
		return FReply::Handled();
	}
	int32 AddIndex = ViewModelPtr->AddPointOfInterest();
	PointOfInterestListWidgetPtr->SetInitializeIndex(AddIndex);
	PointOfInterestListWidgetPtr->OnRefresh(false);
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnAutomationPointOfInterestRemove() // #103
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	if (!PointOfInterestListWidgetPtr.IsValid())
	{
		return FReply::Handled();
	}
	int32 ValueIndexSelected = PointOfInterestListWidgetPtr->GetItemValueIndexSelected();
	if (-1 == ValueIndexSelected)
	{
		return FReply::Handled();
	}
	ViewModelPtr->RemovePointOfInterest(ValueIndexSelected - 1);
	PointOfInterestListWidgetPtr->OnRefresh(false);
	return FReply::Handled();
}

void FT4EntityDetailCustomization::HandleOnMapChange() // #79
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	UT4EntityAsset* EntityAsset = ViewModelPtr->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return;
	}
	UT4MapEntityAsset* MapEntityAsset = Cast<UT4MapEntityAsset>(EntityAsset);
	if (nullptr == MapEntityAsset)
	{
		return;
	}
	const FT4EntityMapData& MapData = MapEntityAsset->MapData;
	if (MapData.LevelAsset.IsNull())
	{
		return;
	}
	ViewModelPtr->ClientWorldTravel(EntityAsset);
}

void FT4EntityDetailCustomization::HandleOnRefreshLayout()
{
	if (nullptr != DetailLayoutPtr)
	{
		DetailLayoutPtr->ForceRefreshDetails();
	}
}

bool FT4EntityDetailCustomization::HandleFilterCostumeEntityAsset(
	const FAssetData& InAssetData,
	USkeleton* InSkeleton
) const // #71, #72
{
	FString SkeletonString = FAssetData(InSkeleton).ToSoftObjectPath().ToString();
	if (InAssetData.TagsAndValues.FindRef(TEXT("SkeletonAsset")) != SkeletonString)
	{
		return true;
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return false;
	}
	FName TransientCompositePartName = CharacterEntityAsset->EditorTransientCharacterData.TransientCompositePartName; // #72
	if (TransientCompositePartName != NAME_None &&
		InAssetData.TagsAndValues.FindRef(TEXT("CompositePartName")) != TransientCompositePartName.ToString())
	{
		return true;
	}
	return false;
}

bool FT4EntityDetailCustomization::HandleFilterSkeletalMeshAsset(
	const FAssetData& InAssetData,
	USkeleton* InSkeleton
) const
{
	FString SkeletonName;
	InAssetData.GetTagValue("Skeleton", SkeletonName);
	FAssetData SkeletonData(InSkeleton);
	return (SkeletonName != SkeletonData.GetExportTextName());
}

bool FT4EntityDetailCustomization::HandleFilterAnimSetAsset(
	const FAssetData& InAssetData,
	USkeleton* InSkeleton
) const // #73
{
	FString SkeletonString = FAssetData(InSkeleton).ToSoftObjectPath().ToString();
	return (InAssetData.TagsAndValues.FindRef(TEXT("SkeletonAsset")) != SkeletonString);
}

#undef LOCTEXT_NAMESPACE
