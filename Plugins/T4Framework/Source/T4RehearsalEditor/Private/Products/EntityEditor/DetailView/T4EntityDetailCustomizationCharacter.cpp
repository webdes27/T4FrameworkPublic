// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4EntityDetailCustomization.h"
#include "Products/Common/DetailView/T4DetailCustomizationMacros.h"

#include "Products/EntityEditor/ViewModel/T4EntityViewModel.h"

#include "Products/Common/Widgets/DropListView/ST4LayerTagDropListWidget.h" // #74
#include "Products/Common/Widgets/DropListView/ST4EquipPointDropListWidget.h" // #74
#include "Products/Common/Widgets/DropListView/ST4CompositePartDropListWidget.h" // #38
#include "Products/Common/Widgets/DropListView/ST4StanceDropListWidget.h" // #73
#include "Products/Common/Widgets/DropListView/ST4ActionPointDropListWidget.h" // #76
#include "Products/Common/Widgets/DropListView/ST4ReactionDropListWidget.h" // #76
#include "Products/Common/Widgets/DropListView/ST4AnimSectionDropListWidget.h" // #39

#include "Products/Common/Widgets/ListView/ST4CompositePartListWidget.h" // #71
#include "Products/Common/Widgets/ListView/ST4StanceListWidget.h" // #73
#include "Products/Common/Widgets/ListView/ST4ReactionListWidget.h" // #76
#include "Products/Common/Widgets/ListView/ST4OverrideMaterialListWidget.h" // #80

#include "Products/EntityEditor/Utility/T4AssetEntityUtils.h" // #88

#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h" // #73
#include "T4Asset/Classes/Entity/T4EntityAsset.h"
#include "T4Asset/Classes/Entity/T4CostumeEntityAsset.h" // #71
#include "T4Asset/Classes/Entity/T4WeaponEntityAsset.h" // #74
#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h" // #71
#include "T4Asset/Classes/Conti/T4ContiAsset.h" // #74

#include "Engine/SkeletalMesh.h" // #74
#include "PhysicsEngine/PhysicsAsset.h" // #76

#include "ScopedTransaction.h" // #77

#include "Widgets/Input/SButton.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"

#include "Misc/MessageDialog.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4EntityCustomizeCharacterDetails"

static const FText T4CharacterEntityFullbodyDetailErrorTitleText = LOCTEXT("T4CharacterEntityFullbodyDetailsError", "Character CustomDetail Error"); // #71

static const FText T4EntityCompositePartDetailErrorTitleText = LOCTEXT("T4EntityCompositePartDetailsError", "Character CustomDetail Error"); // #71
static const FText T4EntityCompositePartDetailErrorResultNoCompositeSelectedText = LOCTEXT("T4EntityCompositePartDetailNoCompositeSelectedError", "No Composite Mesh"); // #71
static const FText T4EntityCompositePartDetailErrorResultNoPartSelectedText = LOCTEXT("T4EntityCompositePartDetailNoPartSelectedError", "No PartName Selected"); // #71
static const FText T4EntityCompositePartDetailErrorResultNoSetCostumeText = LOCTEXT("T4EntityCompositePartDetailNoSetCostumeError", "No Set Costume Asset"); // #71

static const FText T4EntityStanceDetailErrorTitleText = LOCTEXT("T4EntityStanceDetailsError", "Character CustomDetail Error"); // #71
static const FText T4EntityStanceDetailErrorResultNoStanceSelectedText = LOCTEXT("T4EntityStanceDetailNoStanceSelectedError", "No Stance Selected"); // #71
static const FText T4EntityStanceDetailErrorResultNoSetAnimSetText = LOCTEXT("T4EntityStanceDetailNoSetAnimSetError", "No Set AnimSet Asset"); // #71

static const FText T4EntityReactionDetailErrorTitleText = LOCTEXT("T4EntityReactionDetailsError", "Character CustomDetail Error"); // #76
static const FText T4EntityReactionDetailErrorResultNoReactionSelectedText = LOCTEXT("T4EntityReactionDetailNoReactionSelectedError", "No Reaction Selected"); // #76

/**
  * http://api.unrealengine.com/KOR/Programming/Slate/DetailsCustomization/
 */
void FT4EntityDetailCustomization::CustomizeCharacterEntityDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4EntityAsset* InEntityAsset
) // #71
{
	check(nullptr != InEntityAsset);

	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(InEntityAsset);
	if (nullptr == CharacterEntityAsset)
	{
		return;
	}

	// WARN : 편집을 위해 Context 는 유지하고 화면 출력은 막는다.
	EditorTransientEntitySelectDataHandle = DEFINE_DETAIL_GET_CLASS_PROPERTY_MACRO(UT4CharacterEntityAsset, EditorTransientCharacterData);
	if (EditorTransientEntitySelectDataHandle.IsValid())
	{
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientEntitySelectDataHandle, TransientFullbodyOverrideMaterialAsset); // #80

		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientEntitySelectDataHandle, TransientCompositePartName);
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientEntitySelectDataHandle, TransientCompositePartAsset);

		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientEntitySelectDataHandle, TransientStanceName); // #73
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientEntitySelectDataHandle, TransientStanceAsset); // #73
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientEntitySelectDataHandle, TransientStanceActiveLayerTag); // #73, #74

		InBuilder.HideProperty(EditorTransientEntitySelectDataHandle);
	}
	// ~WARN : 편집을 위해 Context 는 유지하고 화면 출력은 막는다.

	{
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CharacterEntityAsset, Physical);

		static const TCHAR* PhysicalAttributesCategoryName = TEXT("Physical");
		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(PhysicalAttributesCategoryName);

		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(PhysicalHandle, CapsuleHeight);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(PhysicalHandle, CapsuleRadius);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(PhysicalHandle, RunSpeed);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(PhysicalHandle, WalkSpeed);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(PhysicalHandle, LockOnSpeed);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(PhysicalHandle, JumpZVelocity);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(PhysicalHandle, RollZVelocity);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(PhysicalHandle, RotationYawRate);
	}

	{
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CharacterEntityAsset, Rendering);

		static const TCHAR* RenderingAttributesCategoryName = TEXT("Rendering");
		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(RenderingAttributesCategoryName);

		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(RenderingHandle, Scale);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(RenderingHandle, ImportRotationYaw);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(RenderingHandle, bReceivesDecals);
	}

	// #38 : Entity 순서를 정렬해준다.
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CharacterEntityAsset, SkeletonAsset);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CharacterEntityAsset, AnimBlueprintAsset);
	{
		static const TCHAR* CharacterDefaultAttributeCategoryName = TEXT("Default");
		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(CharacterDefaultAttributeCategoryName);
		{
			DetailCategoryBuilder.AddProperty(SkeletonAssetHandle);
			SkeletonAssetHandle->SetOnPropertyValueChanged(
				FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnRefreshLayout)
			);
		}
		DetailCategoryBuilder.AddProperty(AnimBlueprintAssetHandle);
	}

	if (ET4EntityCharacterMeshType::FullBody == CharacterEntityAsset->MeshType)
	{
		CustomizeCharacterFullbodyMeshDetails(InBuilder, CharacterEntityAsset);
	}
	else if (ET4EntityCharacterMeshType::Composite == CharacterEntityAsset->MeshType)
	{
		CustomizeCharacterCompositeMeshDetails(InBuilder, CharacterEntityAsset);
	}
	else
	{
		static const TCHAR* MeshDataCategoryName = TEXT("Mesh");
		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(MeshDataCategoryName);
		{
			DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CharacterEntityAsset, MeshType);
			DetailCategoryBuilder.AddProperty(MeshTypeHandle);
			MeshTypeHandle->SetOnPropertyValueChanged(
				FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnRefreshLayout)
			);
		}
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CharacterEntityAsset, CopmpositeMeshData);
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CharacterEntityAsset, FullBodyMeshData);
	}

	CustomizeCharacterStanceSetDetails(InBuilder, CharacterEntityAsset); // #73
	CustomizeCharacterReactionDetails(InBuilder, CharacterEntityAsset); // #76

	{
		CustomizeEntityLayerTagMaterialDetails(InBuilder, InEntityAsset); // #80
		CustomizeEntityLayerTagWeaponDetails(InBuilder, InEntityAsset); // #74
		CustomizeEntityLayerTagContiDetails(InBuilder, InEntityAsset); // #74
	}
}

void FT4EntityDetailCustomization::CustomizeCharacterFullbodyMeshDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4CharacterEntityAsset* InCharacterEntityAsset
) // #71, #74
{
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CharacterEntityAsset, CopmpositeMeshData);

	USkeleton* Skeleton = nullptr;
	if (!InCharacterEntityAsset->SkeletonAsset.IsNull())
	{
		Skeleton = InCharacterEntityAsset->SkeletonAsset.LoadSynchronous();
	}

	static const TCHAR* FullbodyMeshDataCategoryName = TEXT("Mesh (Fullbody)");
	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(FullbodyMeshDataCategoryName);

	{
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CharacterEntityAsset, MeshType);
		DetailCategoryBuilder.AddProperty(MeshTypeHandle);
		MeshTypeHandle->SetOnPropertyValueChanged(
			FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnRefreshLayout)
		);
	}

	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CharacterEntityAsset, FullBodyMeshData); // all hided

	TSharedPtr<IPropertyHandle> FullbodySkeletalMeshHandle 
		= FullBodyMeshDataHandle->GetChildHandle(TEXT("SkeletalMeshAsset"), false);

	DetailCategoryBuilder
		.AddCustomRow(LOCTEXT("T4CharacterEntityFullbodySkeletalMeshSelectorEntryBox", "FullbodyMesh"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("T4CharacterEntityFullbodySkeletalMeshSelectorEntryBoxTitle", "Skeletal Mesh Asset"))
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

	// #80
	IDetailGroup& MaterialDetailGroup = DetailCategoryBuilder.AddGroup(FName("Override Materials"), FText::FromString("Override Materials"), false);

	MaterialDetailGroup.HeaderRow()
	.NameContent()
	[
		SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(LOCTEXT("T4CharacterEntityFullbodyGenerateOverrideMaterialsHeader", "Override Material Data"))
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("T4CharacterEntityFullbodyGetMaterialSlotsBtn", "Get Material Slots"))
			.ToolTipText(LOCTEXT("T4CharacterEntityFullbodyGetMaterialSlotsBtn_Tooltip", "Get Material Slots"))
			.OnClicked(this, &FT4EntityDetailCustomization::HandleOnCharacterFullbodyGetMateralSlots)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f, 0.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("T4CharacterEntityFullbodyClearMaterialSlotsBtn", "Clear Material Slots"))
			.ToolTipText(LOCTEXT("T4CharacterEntityFullbodyClearMaterialSlotsBtn_Tooltip", "Clear Material Slots"))
			.OnClicked(this, &FT4EntityDetailCustomization::HandleOnCharacterFullbodyClearMateralSlots)
		]
	];

	OverrideMaterialListWidget = SNew(ST4OverrideMaterialListWidget, &InCharacterEntityAsset->FullBodyMeshData.OverrideMaterialData)
		.OnSelected(this, &FT4EntityDetailCustomization::HandleOnCharacterFullbodyOverrideMaterialSelected)
		.OnDoubleClicked(nullptr);

	MaterialDetailGroup.AddWidgetRow()
	.WholeRowContent()
	.HAlign(HAlign_Fill)
	.MaxDesiredWidth(400.0f)
	[
		OverrideMaterialListWidget.ToSharedRef()
	];

	{
		DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(MaterialDetailGroup, EditorTransientEntitySelectDataHandle, TransientFullbodyOverrideMaterialSlotName);
		DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(MaterialDetailGroup, EditorTransientEntitySelectDataHandle, TransientFullbodyOverrideMaterialAsset);

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
					.Text(LOCTEXT("T4CharacterEntityFullbodyOverrideMaterialUpdateBtn", "Update a Material Slot"))
					.ToolTipText(LOCTEXT("T4CharacterEntityFullbodyOverrideMaterialUpdateBtn_Tooltip", "Update a Material Slot"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnCharacterFullbodyUpdateOverrideMaterial)
				]
			];
		// ~#80

		// #76 : Fullbody SK 라면 기본 세팅된 PhsycisAsset 을 그대로 사용하고, Override 할 경우만 재설정한다.
		TSharedPtr<IPropertyHandle> FullbodyOverridePhysicsAssetHandlePtr
			= FullBodyMeshDataHandle->GetChildHandle(TEXT("OverridePhysicsAsset"), false);

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4CharacterEntityFullbodyOverridePhysicsAssetSelectorEntryBox", "OverridePhysicsAsset"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4CharacterEntityFullbodyOverridePhysicsAssetSelectorEntryBoxTitle", "Override Physics Asset"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				SNew(SObjectPropertyEntryBox)
				.PropertyHandle(FullbodyOverridePhysicsAssetHandlePtr)
				.AllowedClass(UPhysicsAsset::StaticClass())
				.ThumbnailPool(InBuilder.GetThumbnailPool())
				//.OnShouldFilterAsset(this, &FT4EntityDetailCustomization::HandleFilterPhysicsAsset, Skeleton)
			];
	}

	OverrideMaterialListWidget->OnRefresh(true);
}

void FT4EntityDetailCustomization::CustomizeCharacterCompositeMeshDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4CharacterEntityAsset* InCharacterEntityAsset
) // #71
{
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CharacterEntityAsset, FullBodyMeshData);

	USkeleton* Skeleton = nullptr;
	if (!InCharacterEntityAsset->SkeletonAsset.IsNull())
	{
		Skeleton = InCharacterEntityAsset->SkeletonAsset.LoadSynchronous();
	}

	static const TCHAR* CompositeMeshDataCategoryName = TEXT("Mesh (Composite)");
	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(CompositeMeshDataCategoryName);

	{
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CharacterEntityAsset, MeshType);
		DetailCategoryBuilder.AddProperty(MeshTypeHandle);
		MeshTypeHandle->SetOnPropertyValueChanged(
			FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnRefreshLayout)
		);
	}

	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CharacterEntityAsset, CopmpositeMeshData); // all hided

	DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(CopmpositeMeshDataHandle, ModularType);

	CharacterCompositePartListWidget = SNew(ST4CompositePartListWidget, &InCharacterEntityAsset->CopmpositeMeshData)
		.OnSelected(this, &FT4EntityDetailCustomization::HandleOnCharacterCompositePartSelected)
		.OnDoubleClicked(nullptr);

	DetailCategoryBuilder
		.AddCustomRow(LOCTEXT("T4EntityCompositePartsList", "Composite Parts"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("T4EntityCompositePartsListBoxTitleList", "Parts"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			CharacterCompositePartListWidget.ToSharedRef()
		];

	{
		CompositePartDropListWidget = SNew(ST4CompositePartDropListWidget)
			.OnSelected(nullptr) // #95 : Name 변경은 지원하지 않는다.
			.PropertyHandle(TransientCompositePartNameHandlePtr);
		CompositePartDropListWidget->OnRefresh();

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EntityCompositePartSelector", "Composite Part"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EntityCompositePartSelectorTitle", "Part Name"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				CompositePartDropListWidget.ToSharedRef()
			];

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EntityCompositePartSelectorEntryBox", "CostumeEntity Asset"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EntityCompositePartSelectorEntryBoxTitle", "CostumeEntity Asset"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				SNew(SObjectPropertyEntryBox)
				.PropertyHandle(TransientCompositePartAssetHandlePtr)
				.AllowedClass(UT4CostumeEntityAsset::StaticClass())
				.ThumbnailPool(InBuilder.GetThumbnailPool())
				.OnObjectChanged(this, &FT4EntityDetailCustomization::HandleOnCharacterCompositePartAssetChanged) // #95
				.OnShouldFilterAsset(this, &FT4EntityDetailCustomization::HandleFilterCostumeEntityAsset, Skeleton)
			];

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EntityCompositePartSelector", "CompositePartSelector"))
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
					.Text(LOCTEXT("T4EntityCompositePartBtn", "Add"))
					.ToolTipText(LOCTEXT("T4EntityCompositePartBtn_Tooltip", "Add Selected Part"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnCharacterAddCompositePart)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EntityCompositePartRemoveBtn", "Remove"))
					.ToolTipText(LOCTEXT("T4EntityCompositePartRemoveBtn_Tooltip", "Remove Selected Part"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnCharacterRemoveSelectedCompositePart)
				]
			];
	}

	CharacterCompositePartListWidget->OnRefresh(true);
}

void FT4EntityDetailCustomization::CustomizeCharacterStanceSetDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4CharacterEntityAsset* InCharacterEntityAsset
) // #73
{
	static const TCHAR* T4CharacterStanceSetCategoryName = TEXT("Stance");

	USkeleton* Skeleton = nullptr;
	if (!InCharacterEntityAsset->SkeletonAsset.IsNull())
	{
		Skeleton = InCharacterEntityAsset->SkeletonAsset.LoadSynchronous();
	}

	TSharedRef<FT4EditorViewTargetSelector> ViewTargetSelector = ViewModelPtr->GetViewTargetSelector(); // #57

	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CharacterEntityAsset, StanceSetData); // all hided

	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(T4CharacterStanceSetCategoryName);

	CharacterStanceListWidget = SNew(ST4StanceListWidget, &InCharacterEntityAsset->StanceSetData)
		.OnSelected(this, &FT4EntityDetailCustomization::HandleOnCharacterStanceSelected)
		.OnDoubleClicked(nullptr);

	DetailCategoryBuilder
		.AddCustomRow(LOCTEXT("T4EntityStancesList", "Stances"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("T4EntityStancesListEntryBoxTitle", "Stances"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			CharacterStanceListWidget.ToSharedRef()
		];

	{
		TransientStanceAssetHandlePtr->SetOnPropertyValueChanged(
			FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterStanceChanged)
		); // #104
		TransientStanceActiveLayerTagHandlePtr->SetOnPropertyValueChanged(
			FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterStanceChanged)
		); // #104

		CharacterStanceDropListWidget = SNew(ST4StanceDropListWidget)
			.OnSelected(nullptr) // #95 : Name 변경은 지원하지 않는다.
			.PropertyHandle(TransientStanceNameHandlePtr);
		CharacterStanceDropListWidget->OnRefresh();

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EntityStanceSelector", "StanceName"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EntityStanceSelectorTitle", "Stance Name"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				CharacterStanceDropListWidget.ToSharedRef()
			];

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EntityStanceSelectorEntryBox", "SelectAnimSetAsset"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EntityStanceSelectorEntryBoxTitle", "AnimSetAsset"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				SNew(SObjectPropertyEntryBox)
				.PropertyHandle(TransientStanceAssetHandlePtr)
				.AllowedClass(UT4AnimSetAsset::StaticClass())
				.ThumbnailPool(InBuilder.GetThumbnailPool())
				.OnShouldFilterAsset(this, &FT4EntityDetailCustomization::HandleFilterAnimSetAsset, Skeleton)
			];

		CharacterStanceActiveLayerTagDropListWidget = SNew(ST4LayerTagDropListWidget, ET4LayerTagType::All, ViewTargetSelector)
			.PropertyHandle(TransientStanceActiveLayerTagHandlePtr);
		CharacterStanceActiveLayerTagDropListWidget->OnRefresh();

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EntityStanceActiveLayerTagSelector", "Active LayerTag"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EntityStanceActiveLayerTagSelectorTitle", "Active LayerTag"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				CharacterStanceActiveLayerTagDropListWidget.ToSharedRef()
			];

		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EntityStanceSelectorAddBtn", "Add"))
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
					.Text(LOCTEXT("T4EntityStanceBtn", "Add"))
					.ToolTipText(LOCTEXT("T4EntityStanceBtn_Tooltip", "Add Selected Part"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnCharacterAddSelectedStance)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EntityStanceRemoveBtn", "Remove"))
					.ToolTipText(LOCTEXT("T4EntityStanceRemoveBtn_Tooltip", "Remove Selected Part"))
					.OnClicked(this, &FT4EntityDetailCustomization::HandleOnCharacterRemoveSelectedStance)
				]
			];
	}

	CharacterStanceListWidget->OnRefresh(true);
}

void FT4EntityDetailCustomization::CustomizeCharacterReactionDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4CharacterEntityAsset* InCharacterEntityAsset
) // #76
{
	static const TCHAR* T4ReactionDataCategoryName = TEXT("Reaction");

	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(T4ReactionDataCategoryName);

	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CharacterEntityAsset, ReactionSetData);

	TSharedPtr<IPropertyHandle> TransientReactionNameHandlePtr;
	TSharedPtr<IPropertyHandle> ImpulseMainActionPointHandlePtr;
	TSharedPtr<IPropertyHandle> ImpulseSubActionPointHandlePtr;
	TSharedPtr<IPropertyHandle> StartAnimSectionNameHandlePtr;
	TSharedPtr<IPropertyHandle> LoopAnimSectionNameHandlePtr;

	check(EditorTransientEntitySelectDataHandle.IsValid());
	{
		// #76
		DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(EditorTransientEntitySelectDataHandle, TransientReactionName);
		TSharedPtr<IPropertyHandle> PhysicsStartDataHandle = EditorTransientEntitySelectDataHandle->GetChildHandle(TEXT("TransientReactionPhysicsStartData"), false);
		if (PhysicsStartDataHandle.IsValid())
		{
			DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(PhysicsStartDataHandle, ImpulseMainActionPoint);
			DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(PhysicsStartDataHandle, ImpulseSubActionPoint);
		}
		TSharedPtr<IPropertyHandle> AnimationDataHandle = EditorTransientEntitySelectDataHandle->GetChildHandle(TEXT("TransientReactionAnimationData"), false);
		if (AnimationDataHandle.IsValid())
		{
			DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(AnimationDataHandle, StartAnimSectionName);
			DEFINE_DETAIL_GET_HANDLE_PTR_CHILD_PROPERTY_MACRO(AnimationDataHandle, LoopAnimSectionName);
		}
	}

	TSharedPtr<IPropertyHandle> PhysicsStartDataHandle = EditorTransientEntitySelectDataHandle->GetChildHandle(TEXT("TransientReactionPhysicsStartData"), false);
	if (!PhysicsStartDataHandle.IsValid())
	{
		check(false);
	}
	TSharedPtr<IPropertyHandle> PhysicsStopDataHandle = EditorTransientEntitySelectDataHandle->GetChildHandle(TEXT("TransientReactionPhysicsStopData"), false);
	if (!PhysicsStopDataHandle.IsValid())
	{
		check(false);
	}
	TSharedPtr<IPropertyHandle> AnimationDataHandle = EditorTransientEntitySelectDataHandle->GetChildHandle(TEXT("TransientReactionAnimationData"), false);
	if (!AnimationDataHandle.IsValid())
	{
		check(false);
	}

	TSharedRef<FT4EditorViewTargetSelector> ViewTargetSelector = ViewModelPtr->GetViewTargetSelector(); // #57
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = ViewModelPtr->GetAnimSetAssetSelector();

	CharacterReactionListWidget = SNew(ST4ReactionListWidget, &InCharacterEntityAsset->ReactionSetData)
		.OnSelected(this, &FT4EntityDetailCustomization::HandleOnCharacterReactionSelected)
		.OnDoubleClicked(this, &FT4EntityDetailCustomization::HandleOnCharacterReactionDoubleClicked);

	DetailCategoryBuilder
		.AddCustomRow(LOCTEXT("T4EntityReactionList", "Reactions"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("T4EntityReactionListEntryBoxTitle", "Reactions"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			CharacterReactionListWidget.ToSharedRef()
		];

	{
		CharacterReactionDropListWidget = SNew(ST4ReactionDropListWidget, ViewTargetSelector)
			.OnSelected(nullptr) // #95 : Name 변경은 지원하지 않는다.
			.PropertyHandle(TransientReactionNameHandlePtr);
		CharacterReactionDropListWidget->OnRefresh();

		{
			DetailCategoryBuilder
				.AddCustomRow(LOCTEXT("T4EntityReactionNameSelector", "ReactionName"))
				.NameContent()
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(LOCTEXT("T4EntityReactionNameSelectorTitle", "Reaction Name"))
				]
				.ValueContent()
				.HAlign(HAlign_Fill)
				.MaxDesiredWidth(400.0f)
				[
					CharacterReactionDropListWidget.ToSharedRef()
				];

			DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(EditorTransientEntitySelectDataHandle, TransientReactionType);
			DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(EditorTransientEntitySelectDataHandle, TransientReactionMaxPlayTimeSec);

			TransientReactionTypeHandle->SetOnPropertyValueChanged(
				FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
			); // #95
			TransientReactionMaxPlayTimeSecHandle->SetOnPropertyValueChanged(
				FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
			); // #95

			{
				IDetailGroup& DetailGroup = DetailCategoryBuilder.AddGroup(FName("Physics Start Settings"), FText::FromString("Physics Start Settings"), false);

				DEFINE_DETAIL_HEADER_GROUP_CHILD_PROPERTY_MACRO(DetailGroup, EditorTransientEntitySelectDataHandle, bTransientReactionPhysicsStartUsed);
				DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(DetailGroup, PhysicsStartDataHandle, DelayTimeSec);

				bTransientReactionPhysicsStartUsedHandle->SetOnPropertyValueChanged(
					FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
				); // #95

				DelayTimeSecHandle->SetOnPropertyValueChanged(
					FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
				); // #95

				CharacterReactionImpulseMainActionPointDropListWidget = SNew(ST4ActionPointDropListWidget, ViewTargetSelector)
					.PropertyHandle(ImpulseMainActionPointHandlePtr);
				CharacterReactionImpulseMainActionPointDropListWidget->OnRefresh();

				DetailGroup.AddWidgetRow()
					.NameContent()
					[
						SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text(LOCTEXT("T4EntityReactionImpulseMainActionPointSelectorTitle", "Impulse Main ActionPoint"))
					]
					.ValueContent()
					.HAlign(HAlign_Fill)
					.MaxDesiredWidth(400.0f)
					[
						CharacterReactionImpulseMainActionPointDropListWidget.ToSharedRef()
					];

				CharacterReactionImpulseSubActionPointDropListWidget = SNew(ST4ActionPointDropListWidget, ViewTargetSelector)
					.PropertyHandle(ImpulseSubActionPointHandlePtr);
				CharacterReactionImpulseSubActionPointDropListWidget->OnRefresh();

				DetailGroup.AddWidgetRow()
					.NameContent()
					[
						SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text(LOCTEXT("T4EntityReactionImpulseSubActionPointSelectorTitle", "Impulse Sub ActionPoint"))
					]
					.ValueContent()
					.HAlign(HAlign_Fill)
					.MaxDesiredWidth(400.0f)
					[
						CharacterReactionImpulseSubActionPointDropListWidget.ToSharedRef()
					];

				ImpulseMainActionPointHandlePtr->SetOnPropertyValueChanged(
					FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
				); // #95

				ImpulseSubActionPointHandlePtr->SetOnPropertyValueChanged(
					FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
				); // #95

				DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(DetailGroup, PhysicsStartDataHandle, ImpulsePower);
				DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(DetailGroup, PhysicsStartDataHandle, CenterOfMass);
				DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(DetailGroup, PhysicsStartDataHandle, MassOverrideInKg);

				ImpulsePowerHandle->SetOnPropertyValueChanged(
					FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
				); // #95
				CenterOfMassHandle->SetOnPropertyValueChanged(
					FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
				); // #95
				MassOverrideInKgHandle->SetOnPropertyValueChanged(
					FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
				); // #95

				{
					IDetailGroup& BlendDetailGroup = DetailGroup.AddGroup(FName("Physics Blend Data"), FText::FromString("Physics Blend Data"), false);

					DEFINE_DETAIL_HEADER_GROUP_CHILD_PROPERTY_MACRO(BlendDetailGroup, PhysicsStartDataHandle, bSimulateBodiesBelow);
					DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(BlendDetailGroup, PhysicsStartDataHandle, BlendData);

					bSimulateBodiesBelowHandle->SetOnPropertyValueChanged(
						FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
					); // #95
					BlendDataHandle->SetOnChildPropertyValueChanged(
						FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
					); // #95
				}
			}

			{
				IDetailGroup& DetailGroup = DetailCategoryBuilder.AddGroup(FName("Physics Stop Settings"), FText::FromString("Physics Stop Settings"), false);

				DEFINE_DETAIL_HEADER_GROUP_CHILD_PROPERTY_MACRO(DetailGroup, EditorTransientEntitySelectDataHandle, bTransientReactionPhysicsStopUsed);
				DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(DetailGroup, PhysicsStopDataHandle, DelayTimeSec);

				bTransientReactionPhysicsStopUsedHandle->SetOnPropertyValueChanged(
					FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
				); // #95
				DelayTimeSecHandle->SetOnPropertyValueChanged(
					FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
				); // #95
			}

			{
				IDetailGroup& DetailGroup = DetailCategoryBuilder.AddGroup(FName("Animation Settings"), FText::FromString("Animation Settings"), false);

				DEFINE_DETAIL_HEADER_GROUP_CHILD_PROPERTY_MACRO(DetailGroup, EditorTransientEntitySelectDataHandle, bTransientReactionAnimationUsed);

				DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(DetailGroup, AnimationDataHandle, DelayTimeSec);

				bTransientReactionAnimationUsedHandle->SetOnPropertyValueChanged(
					FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
				); // #95
				DelayTimeSecHandle->SetOnPropertyValueChanged(
					FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
				); // #95

				CharacterReactionStartAnimSectionDropListWidget = SNew(
					ST4AnimSectionDropListWidget, 
					AnimSetAssetSelector, 
					ET4EngineConstantType::AdditiveSection
				).PropertyHandle(StartAnimSectionNameHandlePtr);

				DetailGroup.AddWidgetRow()
					.NameContent()
					[
						SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text(LOCTEXT("T4EntityReactionStartAnimSequenceTitle", "Start Animation (Additive Layer)"))
					]
					.ValueContent()
					.HAlign(HAlign_Fill)
					.MaxDesiredWidth(400.0f)
					[
						CharacterReactionStartAnimSectionDropListWidget.ToSharedRef()
					];

				CharacterReactionLoopAnimSectionDropListWidget = SNew(
					ST4AnimSectionDropListWidget, 
					AnimSetAssetSelector, 
					ET4EngineConstantType::AdditiveSection
				).PropertyHandle(LoopAnimSectionNameHandlePtr);

				DetailGroup.AddWidgetRow()
					.NameContent()
					[
						SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text(LOCTEXT("T4EntityReactionLoopAnimSequenceTitle", "Loop Animation (Additive Layer)"))
					]
					.ValueContent()
					.HAlign(HAlign_Fill)
					.MaxDesiredWidth(400.0f)
					[
						CharacterReactionLoopAnimSectionDropListWidget.ToSharedRef()
					];

				StartAnimSectionNameHandlePtr->SetOnPropertyValueChanged(
					FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
				); // #95
				LoopAnimSectionNameHandlePtr->SetOnPropertyValueChanged(
					FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
				); // #95

				DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(DetailGroup, AnimationDataHandle, BlendInTimeSec);
				DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(DetailGroup, AnimationDataHandle, BlendOutTimeSec);

				BlendInTimeSecHandle->SetOnPropertyValueChanged(
					FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
				); // #95
				BlendOutTimeSecHandle->SetOnPropertyValueChanged(
					FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction)
				); // #95
			}

			{
				IDetailGroup& DetailGroup = DetailCategoryBuilder.AddGroup(FName("Test Settings"), FText::FromString("Test Settings"), false);

				DEFINE_DETAIL_ADD_GROUP_CHILD_PROPERTY_MACRO(DetailGroup, EditorTransientEntitySelectDataHandle, TransientReactionTestShotDirection);
			}

			DetailCategoryBuilder
				.AddCustomRow(LOCTEXT("T4EntityReactionSelectorButtons", "ReactionButtons"))
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
						.Text(LOCTEXT("T4EntityReactionBtn", "Add"))
						.ToolTipText(LOCTEXT("T4EntityReactionBtn_Tooltip", "Add Reaction"))
						.OnClicked(this, &FT4EntityDetailCustomization::HandleOnCharacterAddReaction)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2.0f, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("T4EntityReactionRemoveBtn", "Remove"))
						.ToolTipText(LOCTEXT("T4EntityReactionRemoveBtn_Tooltip", "Remove Selected Reaction"))
						.OnClicked(this, &FT4EntityDetailCustomization::HandleOnCharacterRemoveSelectedReaction)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2.0f, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("T4EntityReactionPlayBtn", "Test Play"))
						.ToolTipText(LOCTEXT("T4EntityReactionPlayBtn_Tooltip", "Test play reaction"))
						.OnClicked(this, &FT4EntityDetailCustomization::HandleOnCharacterPlayReaction)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2.0f, 0.0f)
					[
						SNew(SButton)
						.Text(LOCTEXT("T4EntityReactionResotreBtn", "Restore"))
						.ToolTipText(LOCTEXT("T4EntityReactionResotreBtn_Tooltip", "Restore body"))
						.OnClicked(this, &FT4EntityDetailCustomization::HandleOnCharacterResotreReaction)
					]
				];
		}
	}

	CharacterReactionListWidget->OnRefresh(true);
}

FReply FT4EntityDetailCustomization::HandleOnCharacterFullbodyGetMateralSlots() // #80
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnCharacterFullbodyGetMateralSlots_Transaction", "Get Material Slots"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityCharacterGetFullbodyMeterialSlots(CharacterEntityAsset, ErrorMessage);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4CharacterEntityFullbodyDetailErrorTitleText
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

FReply FT4EntityDetailCustomization::HandleOnCharacterFullbodyClearMateralSlots() // #80
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnCharacterFullbodyClearMateralSlots_Transaction", "Clear Material Slots"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityCharacterClearFullbodyMeterialSlots(
		CharacterEntityAsset,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4CharacterEntityFullbodyDetailErrorTitleText
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

void FT4EntityDetailCustomization::HandleOnCharacterFullbodyOverrideMaterialSelected(const FName InName)
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return;
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnCharacterFullbodyOverrideMaterialSelected_Transaction", "Select a Material Slot"));
	T4AssetUtil::EntityCharacterSelectFullbodyOverrideMaterialBySlotName(CharacterEntityAsset, InName); // #71
}

FReply FT4EntityDetailCustomization::HandleOnCharacterFullbodyUpdateOverrideMaterial() // #80
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return FReply::Handled();
	}
	const FT4EntityCharacterEditorTransientData& EditorTransientData = CharacterEntityAsset->EditorTransientCharacterData;
	const FScopedTransaction Transaction(LOCTEXT("HandleOnCharacterFullbodyUpdateOverrideMaterial_Transaction", "Update a Material Slot"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityCharacterUpdateFullbodyOverrideMaterialBySlotName(
		CharacterEntityAsset,
		EditorTransientData.TransientFullbodyOverrideMaterialSlotName,
		EditorTransientData.TransientFullbodyOverrideMaterialAsset,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4CharacterEntityFullbodyDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (ViewModelPtr.IsValid())
	{
		ViewModelPtr->ClientEditorAction(ET4EditorAction::UpdateOverrideMaterials);
	}
	if (CharacterCompositePartListWidget.IsValid())
	{
		CharacterCompositePartListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

void FT4EntityDetailCustomization::HandleOnCharacterCompositePartAssetChanged(const FAssetData& InAssetData) // #95
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return;
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnCharacterCompositePartAssetChanged_Transaction", "Select a Composite Part"));
	const FT4EntityCharacterEditorTransientData& EditorTransientData = CharacterEntityAsset->EditorTransientCharacterData;
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityCharacterUpdateCompositePartByPartName(
		CharacterEntityAsset,
		EditorTransientData.TransientCompositePartName,
		EditorTransientData.TransientCompositePartAsset,
		ErrorMessage
	); // #95
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityCompositePartDetailErrorTitleText
		);
		return;
	}
	if (ViewModelPtr.IsValid())
	{
		ViewModelPtr->ClientExchangeCostume(nullptr, EditorTransientData.TransientCompositePartName, false); // #72
	}
	if (CharacterCompositePartListWidget.IsValid())
	{
		CharacterCompositePartListWidget->OnRefresh(false);
	}
}

void FT4EntityDetailCustomization::HandleOnCharacterCompositePartSelected(const FName InName)
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return;
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnCharacterCompositePartSelected_Transaction", "Select a Composite Part"));
	T4AssetUtil::EntityCharacterSelectCompositePartByPartName(CharacterEntityAsset, InName); // #71
	if (CompositePartDropListWidget.IsValid())
	{
		CompositePartDropListWidget->OnRefresh();
	}
}

FReply FT4EntityDetailCustomization::HandleOnCharacterAddCompositePart()
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return FReply::Handled();
	}
	if (ET4EntityCharacterMeshType::Composite != CharacterEntityAsset->MeshType)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityCompositePartDetailErrorResultNoCompositeSelectedText,
			&T4EntityCompositePartDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FT4EntityCharacterEditorTransientData& EditorTransientData = CharacterEntityAsset->EditorTransientCharacterData;
	FName SelectedPartName = EditorTransientData.TransientCompositePartName;
	if (SelectedPartName == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityCompositePartDetailErrorResultNoPartSelectedText,
			&T4EntityCompositePartDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (EditorTransientData.TransientCompositePartAsset.IsNull())
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityCompositePartDetailErrorResultNoSetCostumeText,
			&T4EntityCompositePartDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnCharacterAddCompositePart_Transaction", "Add to Composite Part"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityCharacterAddCompositePartByPartName(
		CharacterEntityAsset,
		SelectedPartName,
		EditorTransientData.TransientCompositePartAsset,
		ErrorMessage
	); // #71
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityCompositePartDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (ViewModelPtr.IsValid())
	{
		ViewModelPtr->ClientExchangeCostume(nullptr, SelectedPartName, false); // #72
	}
	if (CharacterCompositePartListWidget.IsValid())
	{
		CharacterCompositePartListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnCharacterRemoveSelectedCompositePart()
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return FReply::Handled();
	}
	if (ET4EntityCharacterMeshType::Composite != CharacterEntityAsset->MeshType)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityCompositePartDetailErrorResultNoCompositeSelectedText,
			&T4EntityCompositePartDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FT4EntityCharacterEditorTransientData& EditorTransientData = CharacterEntityAsset->EditorTransientCharacterData;
	FName SelectedPartName = EditorTransientData.TransientCompositePartName;
	if (SelectedPartName == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityCompositePartDetailErrorResultNoPartSelectedText,
			&T4EntityCompositePartDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnCharacterRemoveSelectedCompositePart_Transaction", "Remove a Composite Part"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityCharacterRemoveCompositePartByPartName(
		CharacterEntityAsset,
		SelectedPartName,
		ErrorMessage
	); // #71
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityCompositePartDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (ViewModelPtr.IsValid())
	{
		ViewModelPtr->ClientExchangeCostume(nullptr, SelectedPartName, false); // #72
	}
	if (CharacterCompositePartListWidget.IsValid())
	{
		CharacterCompositePartListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

void FT4EntityDetailCustomization::HandleOnCharacterFullbodyMesh() // #74
{

}

void FT4EntityDetailCustomization::HandleOnCharacterStanceChanged() // #95, #105
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return;
	}
	const FT4EntityCharacterEditorTransientData& EditorTransientData = CharacterEntityAsset->EditorTransientCharacterData;
	FString ErrorMessage;
	FT4EntityCharacterStanceData NewStanceData; // #74, #73
	NewStanceData.AnimSetAsset = EditorTransientData.TransientStanceAsset;
	NewStanceData.ActiveLayerTag = EditorTransientData.TransientStanceActiveLayerTag;
	bool bResult = T4AssetUtil::EntityCharacterUpdateStanceData(
		CharacterEntityAsset,
		EditorTransientData.TransientStanceName,
		&NewStanceData,
		ErrorMessage
	); // #73
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityStanceDetailErrorTitleText
		);
		return;
	}
	ViewModelPtr->ClientChangeStance(EditorTransientData.TransientStanceName);
	if (CharacterStanceListWidget.IsValid())
	{
		CharacterStanceListWidget->OnRefresh(false);
	}
}

void FT4EntityDetailCustomization::HandleOnCharacterStanceSelected(const FName InName) // #73
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return;
	}
	FT4EntityCharacterStanceSetData& StanceSetData = CharacterEntityAsset->StanceSetData;
	if (StanceSetData.StanceMap.Contains(InName))
	{
		const FScopedTransaction Transaction(LOCTEXT("HandleOnCharacterStanceSelected_Transaction", "Select a Stance Data"));
		T4AssetUtil::EntityCharacterSelectStanceDataByName(CharacterEntityAsset, InName); // #71
		ViewModelPtr->ClientChangeStance(InName);
	}
	if (CharacterStanceDropListWidget.IsValid())
	{
		CharacterStanceDropListWidget->OnRefresh();
	}
	if (CharacterStanceActiveLayerTagDropListWidget.IsValid())
	{
		CharacterStanceActiveLayerTagDropListWidget->OnRefresh();
	}
}

FReply FT4EntityDetailCustomization::HandleOnCharacterAddSelectedStance() // #73
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return FReply::Handled();
	}
	const FT4EntityCharacterEditorTransientData& EditorTransientData = CharacterEntityAsset->EditorTransientCharacterData;
	FName SelectedStanceName = EditorTransientData.TransientStanceName;
	if (SelectedStanceName == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityStanceDetailErrorResultNoStanceSelectedText,
			&T4EntityStanceDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (EditorTransientData.TransientStanceAsset.IsNull())
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityStanceDetailErrorResultNoSetAnimSetText,
			&T4EntityStanceDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnCharacterAddSelectedStance_Transaction", "Add to Stance Data"));
	FString ErrorMessage;
	FT4EntityCharacterStanceData NewStanceData; // #74, #73
	NewStanceData.AnimSetAsset = EditorTransientData.TransientStanceAsset;
	NewStanceData.ActiveLayerTag = EditorTransientData.TransientStanceActiveLayerTag;
	bool bResult = T4AssetUtil::EntityCharacterAddStanceData(
		CharacterEntityAsset,
		SelectedStanceName,
		&NewStanceData,
		ErrorMessage
	); // #73
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityStanceDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (CharacterStanceListWidget.IsValid())
	{
		CharacterStanceListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnCharacterRemoveSelectedStance() // #73
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return FReply::Handled();
	}
	const FT4EntityCharacterEditorTransientData& EditorTransientData = CharacterEntityAsset->EditorTransientCharacterData;
	FName SelectedStanceName = EditorTransientData.TransientStanceName;
	if (SelectedStanceName == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityStanceDetailErrorResultNoStanceSelectedText,
			&T4EntityStanceDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnCharacterRemoveSelectedStance_Transaction", "Remove a Stance"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityCharacterRemoveStanceDataByName(
		CharacterEntityAsset,
		SelectedStanceName,
		ErrorMessage
	); // #73
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityStanceDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (CharacterStanceListWidget.IsValid())
	{
		CharacterStanceListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

void FT4EntityDetailCustomization::HandleOnCharacterReactionSelected(const FName InName) // #76
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return;
	}
	FT4EntityCharacterReactionSetData& ReactionSetData = CharacterEntityAsset->ReactionSetData;
	if (ReactionSetData.ReactionMap.Contains(InName))
	{
		const FScopedTransaction Transaction(LOCTEXT("HandleOnCharacterReactionSelected_Transaction", "Select a Reaction"));
		T4AssetUtil::EntityCharacterSelectReactionDataByName(CharacterEntityAsset, InName);
	}
	if (CharacterReactionDropListWidget.IsValid())
	{
		CharacterReactionDropListWidget->OnRefresh();
	}
	if (CharacterReactionImpulseMainActionPointDropListWidget.IsValid())
	{
		CharacterReactionImpulseMainActionPointDropListWidget->OnRefresh();
	}
	if (CharacterReactionImpulseSubActionPointDropListWidget.IsValid())
	{
		CharacterReactionImpulseSubActionPointDropListWidget->OnRefresh();
	}
	if (CharacterReactionStartAnimSectionDropListWidget.IsValid())
	{
		CharacterReactionStartAnimSectionDropListWidget->OnRefresh();
	}
	if (CharacterReactionLoopAnimSectionDropListWidget.IsValid())
	{
		CharacterReactionLoopAnimSectionDropListWidget->OnRefresh();
	}
}

void FT4EntityDetailCustomization::HandleOnCharacterReactionDoubleClicked(const FName InName) // #76
{
	HandleOnCharacterReactionSelected(InName);
	HandleOnCharacterPlayReaction();
}

void FT4EntityDetailCustomization::HandleOnCharacterUpdateSelectedReaction() // #95
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return;
	}
	const FT4EntityCharacterEditorTransientData& EditorTransientData = CharacterEntityAsset->EditorTransientCharacterData;
	FName SelectedReactionName = EditorTransientData.TransientReactionName;
	if (SelectedReactionName == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityReactionDetailErrorResultNoReactionSelectedText,
			&T4EntityReactionDetailErrorTitleText
		);
		return;
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnCharacterUpdateSelectedReaction_Transaction", "Add to Reaction"));
	FString ErrorMessage;
	FT4EntityCharacterReactionData NewReactionData;
	NewReactionData.ReactionType = EditorTransientData.TransientReactionType;
	NewReactionData.MaxPlayTimeSec = EditorTransientData.TransientReactionMaxPlayTimeSec;
	NewReactionData.bUsePhysicsStart = EditorTransientData.bTransientReactionPhysicsStartUsed;
	NewReactionData.PhysicsStartData = EditorTransientData.TransientReactionPhysicsStartData;
	NewReactionData.bUsePhysicsStop = EditorTransientData.bTransientReactionPhysicsStopUsed;
	NewReactionData.PhysicsStopData = EditorTransientData.TransientReactionPhysicsStopData;
	NewReactionData.bUseAnimation = EditorTransientData.bTransientReactionAnimationUsed;
	NewReactionData.AnimationData = EditorTransientData.TransientReactionAnimationData;
	bool bResult = T4AssetUtil::EntityCharacterUpdateReactionDataByName(
		CharacterEntityAsset,
		SelectedReactionName,
		&NewReactionData,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityReactionDetailErrorTitleText
		);
		return;
	}
	if (CharacterReactionListWidget.IsValid())
	{
		CharacterReactionListWidget->OnRefresh(false);
	}
}

FReply FT4EntityDetailCustomization::HandleOnCharacterAddReaction() // #76
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return FReply::Handled();
	}
	const FT4EntityCharacterEditorTransientData& EditorTransientData = CharacterEntityAsset->EditorTransientCharacterData;
	FName SelectedReactionName = EditorTransientData.TransientReactionName;
	if (SelectedReactionName == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityReactionDetailErrorResultNoReactionSelectedText,
			&T4EntityReactionDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnCharacterAddReaction_Transaction", "Add to Reaction"));
	FString ErrorMessage;
	FT4EntityCharacterReactionData NewReactionData;
	NewReactionData.ReactionType = EditorTransientData.TransientReactionType;
	NewReactionData.MaxPlayTimeSec = EditorTransientData.TransientReactionMaxPlayTimeSec;
	NewReactionData.bUsePhysicsStart = EditorTransientData.bTransientReactionPhysicsStartUsed;
	NewReactionData.PhysicsStartData = EditorTransientData.TransientReactionPhysicsStartData;
	NewReactionData.bUsePhysicsStop = EditorTransientData.bTransientReactionPhysicsStopUsed;
	NewReactionData.PhysicsStopData = EditorTransientData.TransientReactionPhysicsStopData;
	NewReactionData.bUseAnimation = EditorTransientData.bTransientReactionAnimationUsed;
	NewReactionData.AnimationData = EditorTransientData.TransientReactionAnimationData;
	bool bResult = T4AssetUtil::EntityCharacterAddReactionData(
		CharacterEntityAsset,
		SelectedReactionName,
		&NewReactionData,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityReactionDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (CharacterReactionListWidget.IsValid())
	{
		CharacterReactionListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnCharacterRemoveSelectedReaction() // #76
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return FReply::Handled();
	}
	const FT4EntityCharacterEditorTransientData& EditorTransientData = CharacterEntityAsset->EditorTransientCharacterData;
	FName SelectedReactionName = EditorTransientData.TransientReactionName;
	if (SelectedReactionName == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityReactionDetailErrorResultNoReactionSelectedText,
			&T4EntityReactionDetailErrorTitleText
		);
		return FReply::Handled();
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnCharacterRemoveSelectedReaction_Transaction", "Remove a Reaction"));
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EntityCharacterRemoveReactionDataByName(
		CharacterEntityAsset,
		SelectedReactionName,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4EntityReactionDetailErrorTitleText
		);
		return FReply::Handled();
	}
	if (CharacterReactionListWidget.IsValid())
	{
		CharacterReactionListWidget->OnRefresh(false);
	}
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnCharacterPlayReaction() // #76
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == CharacterEntityAsset)
	{
		return FReply::Handled();
	}
	const FT4EntityCharacterEditorTransientData& EditorTransientData = CharacterEntityAsset->EditorTransientCharacterData;
	FName SelectedReactionName = EditorTransientData.TransientReactionName;
	if (SelectedReactionName == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			T4EntityReactionDetailErrorResultNoReactionSelectedText,
			&T4EntityReactionDetailErrorTitleText
		);
		return FReply::Handled();
	}
	ViewModelPtr->ClientPlayReaction(
		SelectedReactionName, 
		EditorTransientData.TransientReactionType,
		EditorTransientData.TransientReactionTestShotDirection
	);
	return FReply::Handled();
}

FReply FT4EntityDetailCustomization::HandleOnCharacterResotreReaction() // #76
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	ViewModelPtr->ClientEditorAction(ET4EditorAction::RestoreReaction);
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
