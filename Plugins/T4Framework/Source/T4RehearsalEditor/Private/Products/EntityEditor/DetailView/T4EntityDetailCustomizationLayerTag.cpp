// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4EntityDetailCustomization.h"
#include "Products/Common/DetailView/T4DetailCustomizationMacros.h"

#include "Products/EntityEditor/ViewModel/T4EntityViewModel.h"

#include "Products/Common/Widgets/DropListView/ST4EquipPointDropListWidget.h" // #74
#include "Products/Common/Widgets/DropListView/ST4LayerTagDropListWidget.h" // #74
#include "Products/Common/Widgets/DropListView/ST4CompositePartDropListWidget.h" // #38
#include "Products/Common/Widgets/DropListView/ST4ActionPointDropListWidget.h" // #76
#include "Products/Common/Widgets/ListView/ST4EntityLayerTagListWidget.h" // #74
#include "Products/Common/Widgets/ListView/ST4OverrideMaterialListWidget.h" // #80

#include "Products/EntityEditor/Utility/T4AssetEntityUtils.h" // #88

#include "T4Asset/Classes/Entity/T4EntityAsset.h"
#include "T4Asset/Classes/Entity/T4EntityAsset.h" // #71
#include "T4Asset/Classes/Entity/T4WeaponEntityAsset.h" // #74
#include "T4Asset/Classes/Conti/T4ContiAsset.h" // #74

#include "Animation/Skeleton.h" // #74

#include "AssetData.h"
#include "ScopedTransaction.h" // #77

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"

#include "Misc/MessageDialog.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4EntityCustomizeLayerTagDetails"

static const FText T4EntityLayerTagWeaponDetailErrorTitleText = LOCTEXT("T4EntityLayerTagWeaponDetailsError", "EquipWeapon LayerTag Error"); // #74
static const FText T4EntityLayerTagWeaponDetailErrorResultNoLayerTagSelectedText = LOCTEXT("T4EntityLayerTagWeaponDetailNoLayerTagSelectedError", "No LayerTag Selected"); // #74
static const FText T4EntityLayerTagWeaponDetailErrorResultNoEquipPointSelectedText = LOCTEXT("T4EntityLayerTagWeaponDetailNoEquipPointSelectedError", "No EquipPoint Selected"); // #74
static const FText T4EntityLayerTagWeaponDetailErrorResultNoSetWeaponText = LOCTEXT("T4EntityLayerTagWeaponDetailNoSetWeaponError", "No Set Weapon Asset"); // #74

static const FText T4EntityLayerTagContiDetailErrorTitleText = LOCTEXT("T4EntityLayerTagContiDetailsError", "Conti LayerTag Error"); // #74
static const FText T4EntityLayerTagContiDetailErrorResultNoLayerTagSelectedText = LOCTEXT("T4EntityLayerTagContiDetailNoLayerTagSelectedError", "No LayerTag Selected"); // #74
static const FText T4EntityLayerTagContiDetailErrorResultNoSetContiText = LOCTEXT("T4EntityLayerTagContinDetailNoSetContiError", "No Set Conti Asset"); // #74

static const FText T4EntityLayerTagMaterialDetailErrorTitleText = LOCTEXT("T4EntityLayerTagMaterialDetailsError", "OverrideMaterial LayerTag Error"); // #81
static const FText T4EntityLayerTagMaterialDetailErrorResultNoLayerTagSelectedText = LOCTEXT("T4EntityLayerTagMaterialDetailNoLayerTagSelectedError", "No LayerTag Selected"); // #81
static const FText T4EntityLayerTagMaterialDetailErrorResultAlreadySameLayerTagText = LOCTEXT("T4EntityLayerTagMaterialDetailAlreadySameLayerTagError", "LayerTag has already been added"); // #81

/**
  * http://api.unrealengine.com/KOR/Programming/Slate/DetailsCustomization/
 */


void FT4EntityDetailCustomization::CustomizeEntityLayerTagMaterialDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4EntityAsset* InEntityAsset
) // #74
{
	check(nullptr != InEntityAsset);

	const FT4EntityLayerTagData& LayerTagData = InEntityAsset->LayerTagData;

	static const TCHAR* T4EntityLayerTagMaterialCategoryName = TEXT("LayerTag (Material)");

	TSharedRef<FT4EditorViewTargetSelector> ViewTargetSelector = ViewModelPtr->GetViewTargetSelector(); // #57

	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(T4EntityLayerTagMaterialCategoryName);

	EntityLayerTagMaterialListWidget = SNew(ST4EntityLayerTagListWidget, &LayerTagData, ET4LayerTagType::Material)
		.OnSelectedByIndex(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagSelected)
		.OnDoubleClickedByIndex(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagDoubleClicked);

	DetailCategoryBuilder
		.AddCustomRow(LOCTEXT("T4EntityLayerTagMaterialList", "Material Tags"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("T4EntityLayerTagMaterialListEntryBoxTitle", "Tags"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(200.0f)
		[
			EntityLayerTagMaterialListWidget.ToSharedRef()
		];

	{
		TSharedPtr<IPropertyHandle> TransientLayerTagNameOfMaterialHandlePtr; // #74
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientEntityDataHandle, TransientLayerTagNameOfMaterial); // #74

		EntityLayerTagMaterialDropListWidget = SNew(ST4LayerTagDropListWidget, ET4LayerTagType::Material, ViewTargetSelector)
			.PropertyHandle(TransientLayerTagNameOfMaterialHandlePtr);
		EntityLayerTagMaterialDropListWidget->OnRefresh();

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EntityLayerTagMaterialSelector", "LayerTag"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EntityLayerTagMaterialSelectorTitle", "LayerTag"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				EntityLayerTagMaterialDropListWidget.ToSharedRef()
			];

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EntityLayerTagMaterialAddOrRemove", "AddOrRemove"))
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
					.Text(LOCTEXT("T4EntityLayerTagMaterialAddBtn", "Add"))
					.ToolTipText(LOCTEXT("T4EntityLayerTagMaterialAddBtn_Tooltip", "Add to Material LayerTag"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagAddMaterial)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EntityLayerTagMaterialRemoveBtn", "Remove"))
					.ToolTipText(LOCTEXT("T4EntityLayerTagMaterialRemoveBtn_Tooltip", "Remove a Material LayerTag"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagRemoveMaterial)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EntityLayerTagMaterialTestPlayBtn", "Test Play"))
					.ToolTipText(LOCTEXT("T4EntityLayerTagMaterialTestPlayBtn_Tooltip", "Test Play"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagPlayMaterialByName)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EntityLayerTagMaterialRestoreBtn", "Restore"))
					.ToolTipText(LOCTEXT("T4EntityLayerTagMaterialRestoreBtn_Tooltip", "Restore"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagRestoreMaterialByName)
				]
			];

		// #80, #81
		IDetailGroup& MaterialDetailGroup = DetailCategoryBuilder.AddGroup(FName("Materials"), FText::FromString("Materials"), false);

		MaterialDetailGroup.HeaderRow()
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EntityLayerTagMaterialGenerateOverrideMaterialsHeader", "Material Data"))
			]
			.ValueContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EntityLayerTagMaterialGetMaterialSlotsBtn", "Get Material Slots"))
					.ToolTipText(LOCTEXT("T4EntityLayerTagMaterialGetMaterialSlotsBtn_Tooltip", "Get Material Slots"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagGetMateralSlots)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EntityLayerTagMaterialClearMaterialSlotsBtn", "Clear Material Slots"))
					.ToolTipText(LOCTEXT("T4EntityLayerTagMaterialClearMaterialSlotsBtn_Tooltip", "Clear Material Slots"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagClearMateralSlots)
				]
			];

		EntityLayerOverrideMaterialListWidget = SNew(ST4OverrideMaterialListWidget, &InEntityAsset->EditorTransientData.TransientLayerTagMaterialData)
			.OnSelected(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagMaterialSlotSelected)
			.OnDoubleClicked(nullptr);
		EntityLayerOverrideMaterialListWidget->OnRefresh(false);

		MaterialDetailGroup.AddWidgetRow()
			.WholeRowContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				EntityLayerOverrideMaterialListWidget.ToSharedRef()
			];

		DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(MaterialDetailGroup, EditorTransientEntityDataHandle, TransientLayerTagMaterialSlotName);
		DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(MaterialDetailGroup, EditorTransientEntityDataHandle, TransientLayerTagMaterialAsset);

		MaterialDetailGroup.AddWidgetRow()
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
					.Text(LOCTEXT("T4EntityLayerTagMaterialUpdateBtn", "Update a Material Slot"))
					.ToolTipText(LOCTEXT("T4EntityLayerTagMaterialUpdateBtn_Tooltip", "Update a Material Slot"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagUpdateMaterialBySlot)
				]
			];
	}

	EntityLayerTagMaterialListWidget->OnRefresh(true);
}

void FT4EntityDetailCustomization::CustomizeEntityLayerTagWeaponDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4EntityAsset* InEntityAsset
) // #74
{
	check(nullptr != InEntityAsset);

	const FT4EntityLayerTagData& LayerTagData = InEntityAsset->LayerTagData;

	static const TCHAR* T4EntityLayerTagWeaponCategoryName = TEXT("LayerTag (Weapon)");
	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(T4EntityLayerTagWeaponCategoryName);

	TSharedRef<FT4EditorViewTargetSelector> ViewTargetSelector = ViewModelPtr->GetViewTargetSelector(); // #57

	EntityLayerTagWeaponListWidget = SNew(ST4EntityLayerTagListWidget, &LayerTagData, ET4LayerTagType::Weapon)
		.OnSelectedByIndex(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagSelected)
		.OnDoubleClickedByIndex(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagDoubleClicked);

	DetailCategoryBuilder
		.AddCustomRow(LOCTEXT("T4EntityLayerTagWeaponList", "Weapon Tags"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("T4EntityLayerTagWeaponListEntryBoxTitle", "Tags"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(200.0f)
		[
			EntityLayerTagWeaponListWidget.ToSharedRef()
		];

	{
		TSharedPtr<IPropertyHandle> TransientLayerTagNameOfWeaponHandlePtr; // #74
		TSharedPtr<IPropertyHandle> TransientLayerTagWeaponEquipPointHandlePtr; // #74
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientEntityDataHandle, TransientLayerTagNameOfWeapon); // #74
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientEntityDataHandle, TransientLayerTagWeaponEquipPoint); // #74

		TransientLayerTagNameOfWeaponHandlePtr->SetOnPropertyValueChanged(
			FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagUpdateWeapon)
		); // #104
		TransientLayerTagWeaponEquipPointHandlePtr->SetOnPropertyValueChanged(
			FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagUpdateWeapon)
		); // #104
		TransientLayerTagWeaponAssetHandlePtr->SetOnPropertyValueChanged(
			FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagUpdateWeapon)
		); // #104

		EntityLayerTagWeaponDropListWidget = SNew(ST4LayerTagDropListWidget, ET4LayerTagType::Weapon, ViewTargetSelector)
			.PropertyHandle(TransientLayerTagNameOfWeaponHandlePtr);
		EntityLayerTagWeaponDropListWidget->OnRefresh();

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EntityLayerTagWeaponSelector", "LayerTag"))
			.NameContent()

			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EntityLayerTagWeaponSelectorTitle", "LayerTag"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				EntityLayerTagWeaponDropListWidget.ToSharedRef()
			];

		EntityLayerTagWeaponEquipPointDropListWidget = SNew(ST4EquipPointDropListWidget, ViewTargetSelector)
			.PropertyHandle(TransientLayerTagWeaponEquipPointHandlePtr);
		EntityLayerTagWeaponEquipPointDropListWidget->OnRefresh();

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EntityLayerTagEquipPointSelector", "EquipPoint"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EntityLayerTagEquipPointSelectorTitle", "EquipPoint"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				EntityLayerTagWeaponEquipPointDropListWidget.ToSharedRef()
			];

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EntityLayerTagWeaponSelectorEntryBox", "EquipWeaponItem"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EntityLayerTagWeaponSelectorEntryBoxTitle", "Weapon Entity Asset"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				SNew(SObjectPropertyEntryBox)
				.PropertyHandle(TransientLayerTagWeaponAssetHandlePtr)
				.AllowedClass(UT4WeaponEntityAsset::StaticClass())
				.ThumbnailPool(InBuilder.GetThumbnailPool())
			];

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EntityLayerTagWeaponSelector", "LayerTagWeaponSelector"))
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
					.Text(LOCTEXT("T4EntityLayerTagWeaponAddOrUpdateBtn", "Add"))
					.ToolTipText(LOCTEXT("T4EntityLayerTagWeaponAddOrUpdateBtn_Tooltip", "Add to Selected Weapon"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagAddWeapon)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EntityLayerTagWeaponRemoveBtn", "Remove"))
					.ToolTipText(LOCTEXT("T4EntityLayerTagWeaponRemoveBtn_Tooltip", "Remove a Selected Weapon"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagRemoveWeapon)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EntityLayerTagWeaponTestPlayBtn", "Test Play"))
					.ToolTipText(LOCTEXT("T4EntityLayerTagWeaponTestPlayBtn_Tooltip", "Test Play"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagPlayWeaponByName)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EntityLayerTagWeaponStopBtn", "Stop"))
					.ToolTipText(LOCTEXT("T4EntityLayerTagWeaponStopBtn_Tooltip", "Stop"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagStopWeaponByName)
				]
			];
	}

	EntityLayerTagWeaponListWidget->OnRefresh(true);
}

void FT4EntityDetailCustomization::CustomizeEntityLayerTagContiDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4EntityAsset* InEntityAsset
) // #74
{
	check(nullptr != InEntityAsset);

	const FT4EntityLayerTagData& LayerTagData = InEntityAsset->LayerTagData;

	static const TCHAR* T4EntityLayerTagContiCategoryName = TEXT("LayerTag (Conti)");
	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(T4EntityLayerTagContiCategoryName);

	TSharedRef<FT4EditorViewTargetSelector> ViewTargetSelector = ViewModelPtr->GetViewTargetSelector(); // #57

	EntityLayerTagContiListWidget = SNew(ST4EntityLayerTagListWidget, &LayerTagData, ET4LayerTagType::Conti)
		.OnSelectedByIndex(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagSelected)
		.OnDoubleClickedByIndex(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagDoubleClicked);

	DetailCategoryBuilder
		.AddCustomRow(LOCTEXT("T4EntityLayerTagContiList", "Conti Tags"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("T4EntityLayerTagContiListEntryBoxTitle", "Tags"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(200.0f)
		[
			EntityLayerTagContiListWidget.ToSharedRef()
		];

	{
		TSharedPtr<IPropertyHandle> TransientLayerTagNameOfContiHandlePtr; // #74
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientEntityDataHandle, TransientLayerTagNameOfConti); // #74

		EntityLayerTagContiDropListWidget = SNew(ST4LayerTagDropListWidget, ET4LayerTagType::Conti, ViewTargetSelector)
			.PropertyHandle(TransientLayerTagNameOfContiHandlePtr);
		EntityLayerTagContiDropListWidget->OnRefresh();

		TransientLayerTagNameOfContiHandlePtr->SetOnPropertyValueChanged(
			FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagUpdateConti)
		); // #104
		TransientLayerTagContiAssetHandlePtr->SetOnPropertyValueChanged(
			FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagUpdateConti)
		); // #104

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EntityLayerTagContiSelector", "LayerTag"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EntityLayerTagContiSelectorTitle", "LayerTag"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				EntityLayerTagContiDropListWidget.ToSharedRef()
			];

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EntityLayerTagContiSelectorEntryBox", "Conti"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EntityLayerTagContiSelectorEntryBoxTitle", "Conti Asset"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				SNew(SObjectPropertyEntryBox)
				.PropertyHandle(TransientLayerTagContiAssetHandlePtr)
				.AllowedClass(UT4ContiAsset::StaticClass())
				.ThumbnailPool(InBuilder.GetThumbnailPool())
			];

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EntityLayerTagContiSelector", "LayerTagContiSelector"))
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
					.Text(LOCTEXT("T4EntityLayerTagContiAddOrUpdateBtn", "Add"))
					.ToolTipText(LOCTEXT("T4EntityLayerTagContiAddOrUpdateBtn_Tooltip", "Add Conti"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagAddConti)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EntityLayerTagContiRemoveBtn", "Remove"))
					.ToolTipText(LOCTEXT("T4EntityLayerTagContiRemoveBtn_Tooltip", "Remove a Selected Conti"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagRemoveConti)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EntityLayerTagContiTestPlayBtn", "Test Play"))
					.ToolTipText(LOCTEXT("T4EntityLayerTagContiTestPlayBtn_Tooltip", "Test Play"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagPlayContiByName)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EntityLayerTagContiStopBtn", "Stop"))
					.ToolTipText(LOCTEXT("T4EntityLayerTagContiStopBtn_Tooltip", "Stop"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnEntityLayerTagStopContiByName)
				]
			];
	}

	EntityLayerTagContiListWidget->OnRefresh(true);
}

void FT4EntityDetailCustomization::HandleOnEntityLayerTagSelected(
	ET4LayerTagType InLayerTagType, 
	int32 InSelectIndex
) // #74, // #81
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	UT4EntityAsset* EntityAsset = Cast<UT4EntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == EntityAsset)
	{
		return;
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnEntityLayerTagSelected_Transaction", "LayerTag Selection"));
	T4AssetUtil::EntityLayerTagSelectionByIndex(EntityAsset, InLayerTagType, InSelectIndex); // #74
	switch (InLayerTagType)
	{
		case ET4LayerTagType::Weapon: // #74
			{
				if (EntityLayerTagWeaponDropListWidget.IsValid())
				{
					EntityLayerTagWeaponDropListWidget->OnRefresh();
				}
				if (EntityLayerTagWeaponEquipPointDropListWidget.IsValid())
				{
					EntityLayerTagWeaponEquipPointDropListWidget->OnRefresh();
				}
			}
			break;

		case ET4LayerTagType::Conti: // #74
			{
				if (EntityLayerTagContiDropListWidget.IsValid())
				{
					EntityLayerTagContiDropListWidget->OnRefresh();
				}
			}
			break;

		case ET4LayerTagType::Material: // #81
			{
				if (EntityLayerOverrideMaterialListWidget.IsValid())
				{
					EntityLayerOverrideMaterialListWidget->OnRefresh(false);
				}
				if (EntityLayerTagMaterialDropListWidget.IsValid())
				{
					EntityLayerTagMaterialDropListWidget->OnRefresh();
				}
			}
			break;

		default:
			{
				UE_LOG(
					LogT4RehearsalEditor,
					Error,
					TEXT("HandleOnEntityLayerTagSelected : Unknown LayerTag type '%u'"),
					uint8(InLayerTagType)
				);
			}
			break;
	}
}

void FT4EntityDetailCustomization::HandleOnEntityLayerTagDoubleClicked(
	ET4LayerTagType InLayerTagType, 
	int32 InSelectIndex
) // #74, // #81
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	UT4EntityAsset* EntityAsset = Cast<UT4EntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == EntityAsset)
	{
		return;
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnEntityLayerTagDoubleClicked_Transaction", "LayerTag Selection"));
	T4AssetUtil::EntityLayerTagSelectionByIndex(EntityAsset, InLayerTagType, InSelectIndex); // #74
	switch (InLayerTagType)
	{
		case ET4LayerTagType::Weapon: // #74
			HandleOnEntityLayerTagPlayWeaponByName();
			break;

		case ET4LayerTagType::Conti: // #74
			HandleOnEntityLayerTagPlayContiByName();
			break;

		case ET4LayerTagType::Material: // #81
			HandleOnEntityLayerTagPlayMaterialByName();
			break;

		default:
			{
				UE_LOG(
					LogT4RehearsalEditor,
					Error,
					TEXT("HandleOnEntityLayerTagSelected : Unknown LayerTag type '%u'"),
					uint8(InLayerTagType)
				);
			}
			break;
	}
}

void FT4EntityDetailCustomization::HandleOnEntityLayerTagUpdateWeapon() // #104
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
	int32 SelectedIndex = -1;
	if (EntityLayerTagWeaponListWidget.IsValid())
	{
		SelectedIndex = EntityLayerTagWeaponListWidget->GetSelectedIndex();
	}
	if (0 > SelectedIndex)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagWeaponDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagWeaponDetailErrorTitleText
		);
		return;
	}
	const FT4EntityEditorTransientData& EditorTransientData = EntityAsset->EditorTransientData;
	FName SelectedLayerTag = EditorTransientData.TransientLayerTagNameOfWeapon;
	if (SelectedLayerTag == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagWeaponDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagWeaponDetailErrorTitleText
		);
		return;
	}
	FName SelectedEquipPoint = EditorTransientData.TransientLayerTagWeaponEquipPoint;
	if (SelectedEquipPoint == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagWeaponDetailErrorResultNoEquipPointSelectedText,
			&T4EntityLayerTagWeaponDetailErrorTitleText
		);
		return;
	}
	if (EditorTransientData.TransientLayerTagWeaponAsset.IsNull())
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagWeaponDetailErrorResultNoSetWeaponText,
			&T4EntityLayerTagWeaponDetailErrorTitleText
		);
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("HandleOnEntityLayerTagUpdateWeapon_Transaction", "Update Weapon LayerTag"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityLayerTagUpdateWeaponData(
		EntityAsset,
		SelectedIndex,
		SelectedLayerTag,
		SelectedEquipPoint,
		EditorTransientData.TransientLayerTagWeaponAsset,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityLayerTagWeaponDetailErrorTitleText
		);
		return;
	}
	if (EntityLayerTagWeaponListWidget.IsValid())
	{
		EntityLayerTagWeaponListWidget->OnRefresh(false);
	}
}

FReply FT4EntityDetailCustomization::HandleOnEntityLayerTagAddWeapon() // #74
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4EntityAsset* EntityAsset = ViewModelPtr->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return FReply::Handled();
	}
	const FT4EntityEditorTransientData& EditorTransientData = EntityAsset->EditorTransientData;
	FName SelectedLayerTag = EditorTransientData.TransientLayerTagNameOfWeapon;
	if (SelectedLayerTag == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagWeaponDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagWeaponDetailErrorTitleText
		);
		return FReply::Handled();
	}
	FName SelectedEquipPoint = EditorTransientData.TransientLayerTagWeaponEquipPoint;
	if (SelectedEquipPoint == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagWeaponDetailErrorResultNoEquipPointSelectedText,
			&T4EntityLayerTagWeaponDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (EditorTransientData.TransientLayerTagWeaponAsset.IsNull())
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagWeaponDetailErrorResultNoSetWeaponText,
			&T4EntityLayerTagWeaponDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnEntityLayerTagAddWeapon_Transaction", "Add to Weapon LayerTag"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityLayerTagAddWeaponData(
		EntityAsset,
		SelectedLayerTag,
		SelectedEquipPoint,
		EditorTransientData.TransientLayerTagWeaponAsset,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityLayerTagWeaponDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (EntityLayerTagWeaponListWidget.IsValid())
	{
		EntityLayerTagWeaponListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnEntityLayerTagRemoveWeapon() // #74
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4EntityAsset* EntityAsset = ViewModelPtr->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return FReply::Handled();
	}
	int32 SelectedIndex = EntityLayerTagWeaponListWidget->GetSelectedIndex();
	if (0 > SelectedIndex)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagWeaponDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagWeaponDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnEntityLayerTagRemoveWeapon_Transaction", "Remove a Weapon LayerTag"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityLayerTagRemoveWeaponDataByIndex(
		EntityAsset,
		SelectedIndex,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityLayerTagWeaponDetailErrorTitleText
		);
		return FReply::Handled();
	}
	HandleOnEntityLayerTagStopWeaponByName();
	if (EntityLayerTagWeaponListWidget.IsValid())
	{
		EntityLayerTagWeaponListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnEntityLayerTagPlayWeaponByName() // #81
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4EntityAsset* EntityAsset = ViewModelPtr->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return FReply::Handled();
	}
	int32 SelectedIndex = -1;
	if (EntityLayerTagWeaponListWidget.IsValid())
	{
		SelectedIndex = EntityLayerTagWeaponListWidget->GetSelectedIndex();
	}
	if (0 > SelectedIndex)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagWeaponDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagWeaponDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FT4EntityEditorTransientData& EditorTransientData = EntityAsset->EditorTransientData;
	ViewModelPtr->ClientPlayLayerTag(
		EditorTransientData.TransientLayerTagNameOfWeapon,
		ET4LayerTagType::Weapon
	);
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnEntityLayerTagStopWeaponByName() // #81
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4EntityAsset* EntityAsset = ViewModelPtr->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return FReply::Handled();
	}
	int32 SelectedIndex = -1;
	if (EntityLayerTagWeaponListWidget.IsValid())
	{
		SelectedIndex = EntityLayerTagWeaponListWidget->GetSelectedIndex();
	}
	if (0 > SelectedIndex)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagWeaponDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagWeaponDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FT4EntityEditorTransientData& EditorTransientData = EntityAsset->EditorTransientData;
	ViewModelPtr->ClientStopLayerTag(
		EditorTransientData.TransientLayerTagNameOfWeapon,
		ET4LayerTagType::Weapon
	);
	return FReply::Handled();
}

void FT4EntityDetailCustomization::HandleOnEntityLayerTagUpdateConti() // #104
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
	int32 SelectedIndex = -1;
	if (EntityLayerTagContiListWidget.IsValid())
	{
		SelectedIndex = EntityLayerTagContiListWidget->GetSelectedIndex();
	}
	if (0 > SelectedIndex)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagContiDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagContiDetailErrorTitleText
		);
		return;
	}
	const FT4EntityEditorTransientData& EditorTransientData = EntityAsset->EditorTransientData;
	FName SelectedLayerTag = EditorTransientData.TransientLayerTagNameOfConti;
	if (SelectedLayerTag == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagContiDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagContiDetailErrorTitleText
		);
		return;
	}
	if (EditorTransientData.TransientLayerTagContiAsset.IsNull())
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagContiDetailErrorResultNoSetContiText,
			&T4EntityLayerTagContiDetailErrorTitleText
		);
		return;
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnEntityLayerTagUpdateConti_Transaction", "Update Conti LayerTag"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityLayerTagUpdateContiData(
		EntityAsset,
		SelectedIndex,
		SelectedLayerTag,
		EditorTransientData.TransientLayerTagContiAsset,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityLayerTagContiDetailErrorTitleText
		);
		return;
	}
	if (EntityLayerTagContiListWidget.IsValid())
	{
		EntityLayerTagContiListWidget->OnRefresh(false);
	}
}

FReply FT4EntityDetailCustomization::HandleOnEntityLayerTagAddConti() // #74
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4EntityAsset* EntityAsset = ViewModelPtr->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return FReply::Handled();
	}
	const FT4EntityEditorTransientData& EditorTransientData = EntityAsset->EditorTransientData;
	FName SelectedLayerTag = EditorTransientData.TransientLayerTagNameOfConti;
	if (SelectedLayerTag == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagContiDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagContiDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (EditorTransientData.TransientLayerTagContiAsset.IsNull())
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagContiDetailErrorResultNoSetContiText,
			&T4EntityLayerTagContiDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnEntityLayerTagAddConti_Transaction", "Add to Conti LayerTag"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityLayerTagAddContiData(
		EntityAsset,
		SelectedLayerTag,
		EditorTransientData.TransientLayerTagContiAsset,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityLayerTagContiDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (EntityLayerTagContiListWidget.IsValid())
	{
		EntityLayerTagContiListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnEntityLayerTagRemoveConti() // #74
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4EntityAsset* EntityAsset = ViewModelPtr->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return FReply::Handled();
	}
	int32 SelectedIndex = EntityLayerTagContiListWidget->GetSelectedIndex();
	if (0 > SelectedIndex)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagWeaponDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagWeaponDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnEntityLayerTagRemoveConti_Transaction", "Remove a Conti LayerTag"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityLayerTagRemoveContiDataByIndex(
		EntityAsset,
		SelectedIndex,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityLayerTagContiDetailErrorTitleText
		);
		return FReply::Handled();
	}
	HandleOnEntityLayerTagStopContiByName();
	if (EntityLayerTagContiListWidget.IsValid())
	{
		EntityLayerTagContiListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnEntityLayerTagPlayContiByName() // #81
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4EntityAsset* EntityAsset = ViewModelPtr->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return FReply::Handled();
	}
	int32 SelectedIndex = -1;
	if (EntityLayerTagContiListWidget.IsValid())
	{
		SelectedIndex = EntityLayerTagContiListWidget->GetSelectedIndex();
	}
	if (0 > SelectedIndex)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagContiDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagContiDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FT4EntityEditorTransientData& EditorTransientData = EntityAsset->EditorTransientData;
	ViewModelPtr->ClientPlayLayerTag(
		EditorTransientData.TransientLayerTagNameOfConti,
		ET4LayerTagType::Conti
	);
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnEntityLayerTagStopContiByName() // #81
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4EntityAsset* EntityAsset = ViewModelPtr->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return FReply::Handled();
	}
	int32 SelectedIndex = -1;
	if (EntityLayerTagContiListWidget.IsValid())
	{
		SelectedIndex = EntityLayerTagContiListWidget->GetSelectedIndex();
	}
	if (0 > SelectedIndex)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagContiDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagContiDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FT4EntityEditorTransientData& EditorTransientData = EntityAsset->EditorTransientData;
	ViewModelPtr->ClientStopLayerTag(
		EditorTransientData.TransientLayerTagNameOfConti,
		ET4LayerTagType::Conti
	);
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnEntityLayerTagGetMateralSlots() // #81
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4EntityAsset* EntityAsset = ViewModelPtr->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return FReply::Handled();
	}
	int32 SelectedIndex = -1;
	if (EntityLayerTagMaterialListWidget.IsValid())
	{
		SelectedIndex = EntityLayerTagMaterialListWidget->GetSelectedIndex();
	}
	if (0 > SelectedIndex)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagMaterialDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagMaterialDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FT4EntityEditorTransientData& EditorTransientData = EntityAsset->EditorTransientData;
	const FScopedTransaction Transaction(LOCTEXT("HandleOnEntityLayerTagGetMateralSlots_Transaction", "Get Material Slots"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityLayerTagGetMeterialSlots(
		EntityAsset,
		SelectedIndex,
		EditorTransientData.TransientLayerTagNameOfMaterial,
		EditorTransientData.TransientLayerTagMaterialSlotName,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityLayerTagMaterialDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (EntityLayerOverrideMaterialListWidget.IsValid())
	{
		EntityLayerOverrideMaterialListWidget->OnRefresh(false);
	}
	if (EntityLayerTagMaterialListWidget.IsValid())
	{
		EntityLayerTagMaterialListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnEntityLayerTagClearMateralSlots() // #81
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4EntityAsset* EntityAsset = ViewModelPtr->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return FReply::Handled();
	}
	int32 SelectedIndex = -1;
	if (EntityLayerTagMaterialListWidget.IsValid())
	{
		SelectedIndex = EntityLayerTagMaterialListWidget->GetSelectedIndex();
	}
	if (0 > SelectedIndex)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagMaterialDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagMaterialDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FT4EntityEditorTransientData& EditorTransientData = EntityAsset->EditorTransientData;
	FString ErrorMessage;
	const FScopedTransaction Transaction(LOCTEXT("HandleOnEntityLayerTagClearMateralSlots_Transaction", "Clear Material Slots"));
	bool bResult = T4AssetUtil::EntityLayerTagClearMeterialSlots(
		EntityAsset,
		SelectedIndex,
		EditorTransientData.TransientLayerTagNameOfMaterial,
		EditorTransientData.TransientLayerTagMaterialSlotName,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityLayerTagMaterialDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (EntityLayerOverrideMaterialListWidget.IsValid())
	{
		EntityLayerOverrideMaterialListWidget->OnRefresh(false);
	}
	if (EntityLayerTagMaterialListWidget.IsValid())
	{
		EntityLayerTagMaterialListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

void FT4EntityDetailCustomization::HandleOnEntityLayerTagMaterialSlotSelected(const FName InName) // #81
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
	int32 SelectedIndex = -1;
	if (EntityLayerTagMaterialListWidget.IsValid())
	{
		SelectedIndex = EntityLayerTagMaterialListWidget->GetSelectedIndex();
	}
	if (0 > SelectedIndex)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagMaterialDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagMaterialDetailErrorTitleText
		);
		return;
	}
	const FT4EntityEditorTransientData& EditorTransientData = EntityAsset->EditorTransientData;
	const FScopedTransaction Transaction(LOCTEXT("HandleOnEntityLayerTagMaterialSlotSelected_Transaction", "Material Selected"));
	T4AssetUtil::EntityLayerTagSelectMaterialBySlotName(
		EntityAsset, 
		SelectedIndex, 
		EditorTransientData.TransientLayerTagNameOfMaterial,
		InName
	);
}

FReply FT4EntityDetailCustomization::HandleOnEntityLayerTagUpdateMaterialBySlot() // #81
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4EntityAsset* EntityAsset = ViewModelPtr->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return FReply::Handled();
	}
	int32 SelectedIndex = -1;
	if (EntityLayerTagMaterialListWidget.IsValid())
	{
		SelectedIndex = EntityLayerTagMaterialListWidget->GetSelectedIndex();
	}
	if (0 > SelectedIndex)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagMaterialDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagMaterialDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FT4EntityEditorTransientData& EditorTransientData = EntityAsset->EditorTransientData;
	const FScopedTransaction Transaction(LOCTEXT("HandleOnEntityLayerTagUpdateMaterialBySlot_Transaction", "Update a Material Slot"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityLayerTagUpdatMaterialBySlotName(
		EntityAsset,
		SelectedIndex,
		EditorTransientData.TransientLayerTagNameOfMaterial,
		EditorTransientData.TransientLayerTagMaterialSlotName,
		EditorTransientData.TransientLayerTagMaterialAsset,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityLayerTagMaterialDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (EntityLayerOverrideMaterialListWidget.IsValid())
	{
		EntityLayerOverrideMaterialListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnEntityLayerTagAddMaterial() // #81
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4EntityAsset* EntityAsset = ViewModelPtr->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return FReply::Handled();
	}
	const FT4EntityEditorTransientData& EditorTransientData = EntityAsset->EditorTransientData;
	FName SelectedLayerTag = EditorTransientData.TransientLayerTagNameOfMaterial;
	if (SelectedLayerTag == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagMaterialDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagMaterialDetailErrorTitleText
		);
		return FReply::Handled();
	}

	// WARN: 머트리얼은 같은 이름의 LayerTag 등록을 막는다.
	for (const FT4EntityLayerTagMaterialData& MaterialData : EntityAsset->LayerTagData.MaterialTags)
	{
		if (SelectedLayerTag == MaterialData.LayerTag)
		{
			FMessageDialog::Open(
				EAppMsgType::Ok,
				EAppReturnType::Ok,
				T4EntityLayerTagMaterialDetailErrorResultAlreadySameLayerTagText,
				&T4EntityLayerTagMaterialDetailErrorTitleText
			);
			return FReply::Handled();
		}
	}

	const FScopedTransaction Transaction(LOCTEXT("HandleOnEntityLayerTagAddMaterial_Transaction", "Add to Material LayerTag"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityLayerTagAddOrUpdateMaterialData(
		EntityAsset,
		SelectedLayerTag,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityLayerTagMaterialDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (EntityLayerTagMaterialListWidget.IsValid())
	{
		EntityLayerTagMaterialListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnEntityLayerTagRemoveMaterial() // #81
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4EntityAsset* EntityAsset = ViewModelPtr->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return FReply::Handled();
	}
	int32 SelectedIndex = -1;
	if (EntityLayerTagMaterialListWidget.IsValid())
	{
		SelectedIndex = EntityLayerTagMaterialListWidget->GetSelectedIndex();
	}
	if (0 > SelectedIndex)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagMaterialDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagMaterialDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnEntityLayerTagRemoveMaterial_Transaction", "Remove a Material LayerTag"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityLayerTagRemoveMaterialDataByIndex(
		EntityAsset,
		SelectedIndex,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityLayerTagMaterialDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (EntityLayerOverrideMaterialListWidget.IsValid())
	{
		EntityLayerOverrideMaterialListWidget->OnRefresh(false);
	}
	if (EntityLayerTagMaterialListWidget.IsValid())
	{ 
		EntityLayerTagMaterialListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnEntityLayerTagPlayMaterialByName() // #81
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4EntityAsset* EntityAsset = ViewModelPtr->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return FReply::Handled();
	}
	int32 SelectedIndex = -1;
	if (EntityLayerTagMaterialListWidget.IsValid())
	{
		SelectedIndex = EntityLayerTagMaterialListWidget->GetSelectedIndex();
	}
	if (0 > SelectedIndex)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagMaterialDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagMaterialDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FT4EntityEditorTransientData& EditorTransientData = EntityAsset->EditorTransientData;
	ViewModelPtr->ClientPlayLayerTag(
		EditorTransientData.TransientLayerTagNameOfMaterial,
		ET4LayerTagType::Material
	);
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnEntityLayerTagRestoreMaterialByName() // #81
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4EntityAsset* EntityAsset = ViewModelPtr->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return FReply::Handled();
	}
	int32 SelectedIndex = -1;
	if (EntityLayerTagMaterialListWidget.IsValid())
	{
		SelectedIndex = EntityLayerTagMaterialListWidget->GetSelectedIndex();
	}
	if (0 > SelectedIndex)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityLayerTagMaterialDetailErrorResultNoLayerTagSelectedText,
			&T4EntityLayerTagMaterialDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FT4EntityEditorTransientData& EditorTransientData = EntityAsset->EditorTransientData;
	ViewModelPtr->ClientStopLayerTag(
		EditorTransientData.TransientLayerTagNameOfMaterial,
		ET4LayerTagType::Material
	);
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
