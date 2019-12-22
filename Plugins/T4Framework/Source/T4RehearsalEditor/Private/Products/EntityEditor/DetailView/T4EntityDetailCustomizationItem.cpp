// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4EntityDetailCustomization.h"
#include "Products/Common/DetailView/T4DetailCustomizationMacros.h"

#include "Products/EntityEditor/ViewModel/T4EntityViewModel.h"

#include "Products/Common/Widgets/DropListView/ST4CompositePartDropListWidget.h" // #38
#include "Products/Common/Widgets/ListView/ST4OverrideMaterialListWidget.h" // #80

#include "Products/EntityEditor/Utility/T4AssetEntityUtils.h" // #88

#include "T4Asset/Classes/Entity/T4EntityAsset.h"
#include "T4Asset/Classes/Entity/T4CostumeEntityAsset.h" // #71
#include "T4Asset/Classes/Entity/T4WeaponEntityAsset.h" // #71

#include "Animation/Skeleton.h" // #74
#include "Engine/SkeletalMesh.h" // #74

#include "AssetData.h"
#include "ScopedTransaction.h" // #77

#include "Widgets/Input/SButton.h"

#include "DetailLayoutBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"

#include "Misc/MessageDialog.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4EntityCustomizeItemDetails"

static const FText T4EntityDropMeshDetailErrorTitleText = LOCTEXT("T4EntityDropMeshDetailsError", "DropMesh CustomDetail Error"); // #80
static const FText T4EntityCostumeDetailErrorTitleText = LOCTEXT("T4EntityCostumeDetailsError", "Costume CustomDetail Error"); // #71
static const FText T4EntityWeaponDetailErrorTitleText = LOCTEXT("T4EntityWeaponDetailsError", "Weapon CustomDetail Error"); // #80

/**
  * http://api.unrealengine.com/KOR/Programming/Slate/DetailsCustomization/
 */
void FT4EntityDetailCustomization::CustomizeItemEntityOverrideMaterialDetails(
	IDetailLayoutBuilder& InBuilder,
	IDetailCategoryBuilder& InCategoryBuilder,
	const FT4EntityOverrideMaterialData* InOverrideMaterialData
) // #72, #80
{
	// #80 : TODO : 중복코드 제거!!

	IDetailGroup& MaterialDetailGroup = InCategoryBuilder.AddGroup(FName("Override Materials"), FText::FromString("Override Materials"), false);

	MaterialDetailGroup.HeaderRow()
	.NameContent()
	[
		SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(LOCTEXT("T4ItemEntityGenerateOverrideMaterialsHeader", "Override Material Data"))
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("T4ItemEntityGetMaterialSlotsBtn", "Get Material Slots"))
			.ToolTipText(LOCTEXT("T4ItemEntityGetMaterialSlotsBtn_Tooltip", "Get Material Slots"))
			.OnClicked(this, &FT4EntityDetailCustomization::HandleOnWeaponGetMateralSlots)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("T4ItemEntityClearMaterialSlotsBtn", "Clear Material Slots"))
			.ToolTipText(LOCTEXT("T4ItemEntityClearMaterialSlotsBtn_Tooltip", "Clear Material Slots"))
			.OnClicked(this, &FT4EntityDetailCustomization::HandleOnWeaponClearMateralSlots)
		]
	];

	OverrideMaterialListWidget = SNew(ST4OverrideMaterialListWidget, InOverrideMaterialData)
		.OnSelected(this, &FT4EntityDetailCustomization::HandleOnWeaponOverrideMaterialSelected)
		.OnDoubleClicked(nullptr);

	MaterialDetailGroup.AddWidgetRow()
	.WholeRowContent()
	.HAlign(HAlign_Fill)
	.MaxDesiredWidth(400.0f)
	[
		OverrideMaterialListWidget.ToSharedRef()
	];

	{
		DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(MaterialDetailGroup, EditorTransientEntitySelectDataHandle, TransientItemOverrideMaterialSlotName);
		DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(MaterialDetailGroup, EditorTransientEntitySelectDataHandle, TransientItemOverrideMaterialAsset);

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
					.Text(LOCTEXT("T4ItemEntityOverrideMaterialUpdateBtn", "Update a Material Slot"))
					.ToolTipText(LOCTEXT("T4ItemEntityOverrideMaterialUpdateBtn_Tooltip", "Update a Material Slot"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnWeaponUpdateOverrideMaterial)
				]
			];
		// ~#80
	}

	OverrideMaterialListWidget->OnRefresh(true);
}

void FT4EntityDetailCustomization::CustomizeItemEntityDropMeshOverrideMaterialDetails(
	IDetailLayoutBuilder& InBuilder,
	IDetailCategoryBuilder& InCategoryBuilder,
	const FT4EntityOverrideMaterialData* InOverrideMaterialData
) // #72, #80
{
	// #80
	IDetailGroup& MaterialDetailGroup = InCategoryBuilder.AddGroup(FName("Override Materials"), FText::FromString("Override Materials"), false);

	MaterialDetailGroup.HeaderRow()
	.NameContent()
	[
		SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(LOCTEXT("T4ItemEntityDropMeshGenerateOverrideMaterialsHeader", "Override Material Data"))
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("T4ItemEntityDropMeshGetMaterialSlotsBtn", "Get Material Slots"))
			.ToolTipText(LOCTEXT("T4ItemEntityDropMeshGetMaterialSlotsBtn_Tooltip", "Get Material Slots"))
			.OnClicked(this, &FT4EntityDetailCustomization::HandleOnDropMeshGetMateralSlots)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("T4ItemEntityDropMeshClearMaterialSlotsBtn", "Clear Material Slots"))
			.ToolTipText(LOCTEXT("T4ItemEntityDropMeshClearMaterialSlotsBtn_Tooltip", "Clear Material Slots"))
			.OnClicked(this, &FT4EntityDetailCustomization::HandleOnDropMeshClearMateralSlots)
		]
	];

	OverrideDropMeshMaterialListWidget = SNew(ST4OverrideMaterialListWidget, InOverrideMaterialData)
		.OnSelected(this, &FT4EntityDetailCustomization::HandleOnDropMeshOverrideMaterialSelected)
		.OnDoubleClicked(nullptr);

	MaterialDetailGroup.AddWidgetRow()
	.WholeRowContent()
	.HAlign(HAlign_Fill)
	.MaxDesiredWidth(400.0f)
	[
		OverrideDropMeshMaterialListWidget.ToSharedRef()
	];

	{
		DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(MaterialDetailGroup, EditorTransientEntityItemDataHandle, TransientDropMeshOverrideMaterialSlotName);
		DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(MaterialDetailGroup, EditorTransientEntityItemDataHandle, TransientDropMeshOverrideMaterialAsset);

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
					.Text(LOCTEXT("T4ItemEntityDropMeshOverrideMaterialUpdateBtn", "Update a Material Slot"))
					.ToolTipText(LOCTEXT("T4ItemEntityDropMeshOverrideMaterialUpdateBtn_Tooltip", "Update a Material Slot"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnDropMeshUpdateOverrideMaterial)
				]
			];
		// ~#80
	}

	OverrideDropMeshMaterialListWidget->OnRefresh(true);
}

void FT4EntityDetailCustomization::CustomizeItemCommonEntityDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4EntityAsset* InEntityAsset
) // #72, #80
{
	UT4ItemEntityAsset* ItemEntityAsset = Cast<UT4ItemEntityAsset>(InEntityAsset);
	if (nullptr == ItemEntityAsset)
	{
		return;
	}
	const FT4EntityItemDropMeshData& ItemDropMeshData = ItemEntityAsset->DropMeshData;
	{
		static const TCHAR* DropMeshEditorAttributeCategoryName = TEXT("DropMesh");
		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(DropMeshEditorAttributeCategoryName);

		// WARN : 편집을 위해 Context 는 유지하고 화면 출력은 막는다.
		EditorTransientEntityItemDataHandle = DEFINE_DETAIL_GET_CLASS_PROPERTY_MACRO(UT4ItemEntityAsset, EditorTransientItemData);
		if (EditorTransientEntityItemDataHandle.IsValid())
		{
			DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientEntityItemDataHandle, TransientDropMeshOverrideMaterialAsset); // #80
			InBuilder.HideProperty(EditorTransientEntityItemDataHandle);
		}
		// ~WARN : 편집을 위해 Context 는 유지하고 화면 출력은 막는다.

		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4ItemEntityAsset, DropMeshData);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(DropMeshDataHandle, MeshType);
		if (MeshTypeHandle.IsValid())
		{
			MeshTypeHandle->SetOnPropertyValueChanged(
				FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnRefreshLayout)
			);
		}
		if (ET4EntityMeshType::StaticMesh == ItemDropMeshData.MeshType)
		{
			DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(DropMeshDataHandle, StaticMeshAsset);
			CustomizeItemEntityDropMeshOverrideMaterialDetails(
				InBuilder, 
				DetailCategoryBuilder, 
				&ItemDropMeshData.StaticMeshOverrideMaterialData
			);
		}
		else if (ET4EntityMeshType::SkeletalMesh == ItemDropMeshData.MeshType)
		{
			DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(DropMeshDataHandle, SkeletalMeshAsset);
			CustomizeItemEntityDropMeshOverrideMaterialDetails(
				InBuilder,
				DetailCategoryBuilder,
				&ItemDropMeshData.SkeletalMeshOverrideMaterialData
			);
		}
		else if (ET4EntityMeshType::SkeletalMesh == ItemDropMeshData.MeshType)
		{
			DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(DropMeshDataHandle, ParticleSystemAsset);
		}
	}
	{
		static const TCHAR* DropMeshPhysicalAttributeCategoryName = TEXT("Physical (DropMesh)");
		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(DropMeshPhysicalAttributeCategoryName);
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4ItemEntityAsset, DropMeshPhysical);

		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(DropMeshPhysicalHandle, CapsuleHeight);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(DropMeshPhysicalHandle, CapsuleRadius);
	}
	{
		static const TCHAR* DropMeshRenderingAttributeCategoryName = TEXT("Rendering (DropMesh)");
		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(DropMeshRenderingAttributeCategoryName);
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4ItemEntityAsset, DropMeshRendering);

		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(DropMeshRenderingHandle, Scale);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(DropMeshRenderingHandle, ImportRotationYaw);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(DropMeshRenderingHandle, bReceivesDecals);
	}
}

void FT4EntityDetailCustomization::CustomizeWeaponEntityDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4EntityAsset* InEntityAsset
) // #72
{
	check(nullptr != InEntityAsset);

	UT4WeaponEntityAsset* WeaponEntityAsset = Cast<UT4WeaponEntityAsset>(InEntityAsset);
	if (nullptr == WeaponEntityAsset)
	{
		return;
	}
	const FT4EntityItemWeaponMeshData& MeshData = WeaponEntityAsset->MeshData;
	static const TCHAR* T4WeaponDefaultAttributeCategoryName = TEXT("Default (Weapon)");
	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(T4WeaponDefaultAttributeCategoryName);
	{
		// WARN : 편집을 위해 Context 는 유지하고 화면 출력은 막는다.
		EditorTransientEntitySelectDataHandle = DEFINE_DETAIL_GET_CLASS_PROPERTY_MACRO(UT4WeaponEntityAsset, EditorTransientWeaponData);
		if (EditorTransientEntitySelectDataHandle.IsValid())
		{
			DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientEntitySelectDataHandle, TransientItemOverrideMaterialAsset); // #80
			InBuilder.HideProperty(EditorTransientEntitySelectDataHandle);
		}
		// ~WARN : 편집을 위해 Context 는 유지하고 화면 출력은 막는다.

		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4WeaponEntityAsset, MeshData);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(MeshDataHandle, MeshType);
		if (MeshTypeHandle.IsValid())
		{
			MeshTypeHandle->SetOnPropertyValueChanged(
				FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnRefreshLayout)
			);
		}
		if (ET4EntityMeshType::StaticMesh == MeshData.MeshType)
		{
			DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(MeshDataHandle, StaticMeshAsset);
			CustomizeItemEntityOverrideMaterialDetails(
				InBuilder,
				DetailCategoryBuilder,
				&MeshData.StaticMeshOverrideMaterialData
			);
		}
		else if (ET4EntityMeshType::SkeletalMesh == MeshData.MeshType)
		{
			DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(MeshDataHandle, SkeletalMeshAsset);
			CustomizeItemEntityOverrideMaterialDetails(
				InBuilder,
				DetailCategoryBuilder,
				&MeshData.SkeletalMeshOverrideMaterialData
			);
		}
		else if (ET4EntityMeshType::SkeletalMesh == MeshData.MeshType)
		{
			DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(MeshDataHandle, ParticleSystemAsset);
		}
	}
}

void FT4EntityDetailCustomization::CustomizeCostumeEntityDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4EntityAsset* InEntityAsset
) // #72
{
	check(nullptr != InEntityAsset);

	UT4CostumeEntityAsset* CostumeEntityAsset = Cast<UT4CostumeEntityAsset>(InEntityAsset);
	if (nullptr == CostumeEntityAsset)
	{
		return;
	}

	USkeleton* Skeleton = nullptr;
	if (!CostumeEntityAsset->MeshData.SkeletonAsset.IsNull())
	{
		Skeleton = CostumeEntityAsset->MeshData.SkeletonAsset.LoadSynchronous();
	}

	// WARN : 편집을 위해 Context 는 유지하고 화면 출력은 막는다.
	EditorTransientEntitySelectDataHandle = DEFINE_DETAIL_GET_CLASS_PROPERTY_MACRO(UT4CostumeEntityAsset, EditorTransientCostumeData);
	if (EditorTransientEntitySelectDataHandle.IsValid())
	{
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientEntitySelectDataHandle, TransientItemOverrideMaterialAsset); // #80
		InBuilder.HideProperty(EditorTransientEntitySelectDataHandle);
	}
	// ~WARN : 편집을 위해 Context 는 유지하고 화면 출력은 막는다.

	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CostumeEntityAsset, MeshData);

	static const TCHAR* T4CostumeDefaultAttributeCategoryName = TEXT("Default (Costume)");
	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(T4CostumeDefaultAttributeCategoryName);

	{
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(MeshDataHandle, SkeletonAsset);
	}

	TSharedPtr<IPropertyHandle> CompositePartNameHandle = MeshDataHandle->GetChildHandle(TEXT("CompositePartName"));
	if (CompositePartNameHandle.IsValid())
	{
		CompositePartDropListWidget = SNew(ST4CompositePartDropListWidget)
			.OnSelected(nullptr)
			.PropertyHandle(CompositePartNameHandle);
		CompositePartDropListWidget->OnRefresh();

		InBuilder.EditCategory(T4CostumeDefaultAttributeCategoryName)
		.AddCustomRow(LOCTEXT("T4CostumeEntityCompositePartSelector", "Composite Part"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("T4CostumeEntityCompositePartSelectorTitle", "Part Name"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			CompositePartDropListWidget.ToSharedRef()
		];
	}

	TSharedPtr<IPropertyHandle> FullbodySkeletalMeshHandle = MeshDataHandle->GetChildHandle(TEXT("SkeletalMeshAsset"));
	if (FullbodySkeletalMeshHandle.IsValid())
	{
		InBuilder.EditCategory(T4CostumeDefaultAttributeCategoryName)
			.AddCustomRow(LOCTEXT("T4CostumeEntitySkeletalMeshSelectorEntryBox", "Skeletal Mesh"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4CostumeEntitySkeletalMeshSelectorEntryBoxTitle", "Skeletal Mesh Asset"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				SNew(SObjectPropertyEntryBox)
				.PropertyHandle(FullbodySkeletalMeshHandle)
				.AllowedClass(USkeletalMesh::StaticClass())
				.ThumbnailPool(InBuilder.GetThumbnailPool())
				.OnShouldFilterAsset(this, &FT4EntityDetailCustomization::HandleFilterSkeletalMeshAsset, Skeleton)
			];
		}

	// #80
	IDetailGroup& MaterialDetailGroup = DetailCategoryBuilder.AddGroup(FName("Override Materials"), FText::FromString("Override Materials"), false);

	MaterialDetailGroup.HeaderRow()
	.NameContent()
	[
		SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(LOCTEXT("T4CostumeEntityGenerateOverrideMaterialsHeader", "Override Material Data"))
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("T4CostumeEntityGetMaterialSlotsBtn", "Get Material Slots"))
			.ToolTipText(LOCTEXT("T4CostumeEntityGetMaterialSlotsBtn_Tooltip", "Get Material Slots"))
			.OnClicked(this, &FT4EntityDetailCustomization::HandleOnCostumeGetMateralSlots)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("T4CostumeEntityClearMaterialSlotsBtn", "Clear Material Slots"))
			.ToolTipText(LOCTEXT("T4CostumeEntityClearMaterialSlotsBtn_Tooltip", "Clear Material Slots"))
			.OnClicked(this, &FT4EntityDetailCustomization::HandleOnCostumeClearMateralSlots)
		]
	];

	OverrideMaterialListWidget = SNew(ST4OverrideMaterialListWidget, &CostumeEntityAsset->MeshData.OverrideMaterialData)
		.OnSelected(this, &FT4EntityDetailCustomization::HandleOnCostumeOverrideMaterialSelected)
		.OnDoubleClicked(nullptr);

	MaterialDetailGroup.AddWidgetRow()
	.WholeRowContent()
	.HAlign(HAlign_Fill)
	.MaxDesiredWidth(400.0f)
	[
		OverrideMaterialListWidget.ToSharedRef()
	];

	{
		DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(MaterialDetailGroup, EditorTransientEntitySelectDataHandle, TransientItemOverrideMaterialSlotName);
		DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(MaterialDetailGroup, EditorTransientEntitySelectDataHandle, TransientItemOverrideMaterialAsset);

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
					.Text(LOCTEXT("T4CostumeEntityOverrideMaterialUpdateBtn", "Update a Material Slot"))
					.ToolTipText(LOCTEXT("T4CostumeEntityOverrideMaterialUpdateBtn_Tooltip", "Update a Material Slot"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnCostumeUpdateOverrideMaterial)
				]
			];
		// ~#80

		//DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(MeshDataHandle, bUseDropMesh);
	}

	OverrideMaterialListWidget->OnRefresh(true);
}

FReply FT4EntityDetailCustomization::HandleOnDropMeshGetMateralSlots() // #80
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4ItemEntityAsset* ItemEntityAsset = Cast<UT4ItemEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == ItemEntityAsset)
	{
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnDropMeshGetMateralSlots_Transaction", "Get Material Slots"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityItemGetOverrideMeterialSlots(ItemEntityAsset, ErrorMessage);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityDropMeshDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (ViewModelPtr.IsValid())
	{
		ViewModelPtr->ClientEditorAction(ET4EditorAction::UpdateOverrideMaterials);
	}
	if (OverrideDropMeshMaterialListWidget.IsValid())
	{
		OverrideDropMeshMaterialListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnDropMeshClearMateralSlots() // #80
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4ItemEntityAsset* ItemEntityAsset = Cast<UT4ItemEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == ItemEntityAsset)
	{
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnDropMeshClearMateralSlots_Transaction", "Clear Material Slots"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityItemClearOverrideMeterialSlots(ItemEntityAsset, ErrorMessage);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityDropMeshDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (ViewModelPtr.IsValid())
	{
		ViewModelPtr->ClientEditorAction(ET4EditorAction::UpdateOverrideMaterials);
	}
	if (OverrideDropMeshMaterialListWidget.IsValid())
	{
		OverrideDropMeshMaterialListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

void FT4EntityDetailCustomization::HandleOnDropMeshOverrideMaterialSelected(const FName InName)
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	UT4ItemEntityAsset* ItemEntityAsset = Cast<UT4ItemEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == ItemEntityAsset)
	{
		return;
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnDropMeshOverrideMaterialSelected_Transaction", "Select a Override Material"));
	T4AssetUtil::EntityItemSelectOverrideMaterialBySlotName(ItemEntityAsset, InName); // #71
}

FReply FT4EntityDetailCustomization::HandleOnDropMeshUpdateOverrideMaterial() // #80
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4ItemEntityAsset* ItemEntityAsset = Cast<UT4ItemEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == ItemEntityAsset)
	{
		return FReply::Handled();
	}
	const FT4EntityItemEditorTransientData& EditorTransientData = ItemEntityAsset->EditorTransientItemData;
	const FScopedTransaction Transaction(LOCTEXT("HandleOnDropMeshUpdateOverrideMaterial_Transaction", "Updata a Override Material"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityItemUpdateOverrideMaterialBySlotName(
		ItemEntityAsset,
		EditorTransientData.TransientDropMeshOverrideMaterialSlotName,
		EditorTransientData.TransientDropMeshOverrideMaterialAsset,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityDropMeshDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (ViewModelPtr.IsValid())
	{
		ViewModelPtr->ClientEditorAction(ET4EditorAction::UpdateOverrideMaterials);
	}
	if (OverrideDropMeshMaterialListWidget.IsValid())
	{
		OverrideDropMeshMaterialListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnWeaponGetMateralSlots() // #80
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4WeaponEntityAsset* WeaponEntityAsset = Cast<UT4WeaponEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == WeaponEntityAsset)
	{
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnWeaponGetMateralSlots_Transaction", "Get Material Slots"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityWeaponGetOverrideMeterialSlots(WeaponEntityAsset, ErrorMessage);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityWeaponDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (ViewModelPtr.IsValid())
	{
		ViewModelPtr->ClientEditorAction(ET4EditorAction::UpdateOverrideMaterials);
	}
	if (OverrideMaterialListWidget.IsValid())
	{
		// #104 : 새로 추가된 Slot 을 자동 선택 해준다.
		if (ET4EntityMeshType::StaticMesh == WeaponEntityAsset->MeshData.MeshType)
		{
			FT4EntityOverrideMaterialData& OverrideMaterialData = WeaponEntityAsset->MeshData.StaticMeshOverrideMaterialData;
			if (0 < OverrideMaterialData.MaterialSortedSlotNames.Num())
			{
				FName AddMaterialSlots = OverrideMaterialData.MaterialSortedSlotNames[OverrideMaterialData.MaterialSortedSlotNames.Num() - 1];
				OverrideMaterialListWidget->SetInitializeValue(AddMaterialSlots);
			}
		}
		else if(ET4EntityMeshType::SkeletalMesh == WeaponEntityAsset->MeshData.MeshType)
		{
			FT4EntityOverrideMaterialData& OverrideMaterialData = WeaponEntityAsset->MeshData.SkeletalMeshOverrideMaterialData;
			if (0 < OverrideMaterialData.MaterialSortedSlotNames.Num())
			{
				FName AddMaterialSlots = OverrideMaterialData.MaterialSortedSlotNames[OverrideMaterialData.MaterialSortedSlotNames.Num() - 1];
				OverrideMaterialListWidget->SetInitializeValue(AddMaterialSlots);
			}
		}
		OverrideMaterialListWidget->OnRefresh(true); // #104 : 하위 메뉴 업데이트를 위해 재선택 해준다.
	}
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnWeaponClearMateralSlots() // #80
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4WeaponEntityAsset* WeaponEntityAsset = Cast<UT4WeaponEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == WeaponEntityAsset)
	{
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnWeaponClearMateralSlots_Transaction", "Clear Material Slots"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityWeaponClearOverrideMeterialSlots(
		WeaponEntityAsset,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityWeaponDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (ViewModelPtr.IsValid())
	{
		ViewModelPtr->ClientEditorAction(ET4EditorAction::UpdateOverrideMaterials);
	}
	if (OverrideMaterialListWidget.IsValid())
	{
		OverrideMaterialListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

void FT4EntityDetailCustomization::HandleOnWeaponOverrideMaterialSelected(const FName InName)
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	UT4WeaponEntityAsset* WeaponEntityAsset = Cast<UT4WeaponEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == WeaponEntityAsset)
	{
		return;
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnWeaponOverrideMaterialSelected_Transaction", "Select a Override Material"));
	T4AssetUtil::EntityWeaponSelectOverrideMaterialBySlotName(WeaponEntityAsset, InName); // #71
}

FReply FT4EntityDetailCustomization::HandleOnWeaponUpdateOverrideMaterial() // #80
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4WeaponEntityAsset* WeaponEntityAsset = Cast<UT4WeaponEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == WeaponEntityAsset)
	{
		return FReply::Handled();
	}
	const FT4EntityWeaponEditorTransientData& EditorTransientData = WeaponEntityAsset->EditorTransientWeaponData;
	const FScopedTransaction Transaction(LOCTEXT("HandleOnWeaponUpdateOverrideMaterial_Transaction", "Update a Override Material"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityWeaponUpdateOverrideMaterialBySlotName(
		WeaponEntityAsset,
		EditorTransientData.TransientItemOverrideMaterialSlotName,
		EditorTransientData.TransientItemOverrideMaterialAsset,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityWeaponDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (ViewModelPtr.IsValid())
	{
		ViewModelPtr->ClientEditorAction(ET4EditorAction::UpdateOverrideMaterials);
	}
	if (OverrideMaterialListWidget.IsValid())
	{
		OverrideMaterialListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnCostumeGetMateralSlots() // #80
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4CostumeEntityAsset* CostumeEntityAsset = Cast<UT4CostumeEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CostumeEntityAsset)
	{
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnCostumeGetMateralSlots_Transaction", "Get Material Slots"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityCostumeGetOverrideMeterialSlots(CostumeEntityAsset, ErrorMessage);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityCostumeDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (ViewModelPtr.IsValid())
	{
		ViewModelPtr->ClientEditorAction(ET4EditorAction::UpdateOverrideMaterials);
	}
	if (OverrideMaterialListWidget.IsValid())
	{
		// #104 : 새로 추가된 Slot 을 자동 선택 해준다.
		FT4EntityOverrideMaterialData& OverrideMaterialData = CostumeEntityAsset->MeshData.OverrideMaterialData;
		if (0 < OverrideMaterialData.MaterialSortedSlotNames.Num())
		{
			FName AddMaterialSlots = OverrideMaterialData.MaterialSortedSlotNames[OverrideMaterialData.MaterialSortedSlotNames.Num() - 1];
			OverrideMaterialListWidget->SetInitializeValue(AddMaterialSlots);
		}
		OverrideMaterialListWidget->OnRefresh(true); // #104 : 하위 메뉴 업데이트를 위해 재선택 해준다.
	}
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnCostumeClearMateralSlots() // #80
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4CostumeEntityAsset* CostumeEntityAsset = Cast<UT4CostumeEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CostumeEntityAsset)
	{
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnCostumeClearMateralSlots_Transaction", "Clear Material Slots"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityCostumeClearOverrideMeterialSlots(
		CostumeEntityAsset,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityCostumeDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (ViewModelPtr.IsValid())
	{
		ViewModelPtr->ClientEditorAction(ET4EditorAction::UpdateOverrideMaterials);
	}
	if (OverrideMaterialListWidget.IsValid())
	{
		OverrideMaterialListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

void FT4EntityDetailCustomization::HandleOnCostumeOverrideMaterialSelected(const FName InName)
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	UT4CostumeEntityAsset* CostumeEntityAsset = Cast<UT4CostumeEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CostumeEntityAsset)
	{
		return;
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnCostumeOverrideMaterialSelected_Transaction", "Select a Override Material"));
	T4AssetUtil::EntityCostumeSelectOverrideMaterialBySlotName(CostumeEntityAsset, InName); // #71
}

FReply FT4EntityDetailCustomization::HandleOnCostumeUpdateOverrideMaterial() // #80
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4CostumeEntityAsset* CostumeEntityAsset = Cast<UT4CostumeEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CostumeEntityAsset)
	{
		return FReply::Handled();
	}
	const FT4EntityCostumeEditorTransientData& EditorTransientData = CostumeEntityAsset->EditorTransientCostumeData;
	const FScopedTransaction Transaction(LOCTEXT("HandleOnCostumeUpdateOverrideMaterial_Transaction", "Select a Override Material"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityCostumeUpdateOverrideMaterialBySlotName(
		CostumeEntityAsset,
		EditorTransientData.TransientItemOverrideMaterialSlotName,
		EditorTransientData.TransientItemOverrideMaterialAsset,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityCostumeDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (ViewModelPtr.IsValid())
	{
		ViewModelPtr->ClientEditorAction(ET4EditorAction::UpdateOverrideMaterials);
	}
	if (OverrideMaterialListWidget.IsValid())
	{
		OverrideMaterialListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
