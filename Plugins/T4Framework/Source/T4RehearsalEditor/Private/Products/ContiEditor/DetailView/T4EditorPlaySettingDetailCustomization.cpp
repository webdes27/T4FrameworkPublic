// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4EditorPlaySettingDetailCustomization.h"
#include "Products/Common/DetailView/T4DetailCustomizationMacros.h"

#include "Products/ContiEditor/ViewModel/T4ContiViewModel.h"

#include "Products/Common/Widgets/DropListView/ST4ReactionDropListWidget.h" // #76
#include "Products/Common/Widgets/ST4ContentNameIDDropListWidget.h" // #60

#include "Products/Common/Helper/T4EditorGameplaySettingObject.h" // #60

#include "T4Asset/Classes/Conti/T4ContiAsset.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"

#include "Misc/MessageDialog.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4EditorPlaySettingDetailCustomization"

static const FText T4EditorPlayCustomizeErrorTitleText = LOCTEXT("T4EditorPlaySettingDetailCustomizationError", "EditorSettins Error");

/**
  * http://api.unrealengine.com/KOR/Programming/Slate/DetailsCustomization/
 */

TSharedRef<IDetailCustomization> FT4EditorPlaySettingDetailCustomization::MakeInstance(
	TSharedPtr<FT4ContiViewModel> InContiViewModel
)
{
	return MakeShared<FT4EditorPlaySettingDetailCustomization>(InContiViewModel);
}

FT4EditorPlaySettingDetailCustomization::FT4EditorPlaySettingDetailCustomization(
	TSharedPtr<FT4ContiViewModel> InContiViewModel
)	: ViewModel(InContiViewModel)
	, DetailLayoutPtr(nullptr)
{
}

void FT4EditorPlaySettingDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& InBuilder)
{
	if (!ViewModel.IsValid())
	{
		return;
	}

	DetailLayoutPtr = &InBuilder;

	UT4EditorGameplaySettingObject* EditorPlaySettings = ViewModel->GetEditorPlaySettings(); // #60
	check(nullptr != EditorPlaySettings);

	CustomizePlayerSettingsDetails(InBuilder, EditorPlaySettings);
	CustomizeSandbackSettingsDetails(InBuilder, EditorPlaySettings);
	CustomizeAttackerDataDetails(InBuilder, EditorPlaySettings);
	CustomizeDefenderDataDetails(InBuilder, EditorPlaySettings);
}

void FT4EditorPlaySettingDetailCustomization::CustomizePlayerSettingsDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4EditorGameplaySettingObject* InEditorPlaySettings
) // #60
{
	check(nullptr != InEditorPlaySettings);
	{
		static const TCHAR* PlayerSettingsCategoryName = TEXT("Player Settings");

		TSharedRef<SWidget> EditorSettingsHeaderWidget =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Right)
			[
				SNew(SButton)
				.ButtonStyle(FEditorStyle::Get(), "RoundButton")
				.ForegroundColor(FEditorStyle::GetSlateColor("DefaultForeground"))
				.ContentPadding(FMargin(2, 0))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Text(LOCTEXT("T4EditorGameplaySettingObjectSaveBtn", "Save the Modified Settings"))
				.ToolTipText(LOCTEXT("T4EditorGameplaySettingObjectSave_Tooltip", "Save the Modified Settings"))
				.OnClicked(this, &FT4EditorPlaySettingDetailCustomization::HandleOnSaveEditorPlaySettings)
			];

		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(PlayerSettingsCategoryName);
		DetailCategoryBuilder.HeaderContent(EditorSettingsHeaderWidget);

		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4EditorGameplaySettingObject, WeaponContentNameIDSelected);
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4EditorGameplaySettingObject, CostumeContentNameIDSelected);

		InBuilder.EditCategory(PlayerSettingsCategoryName)
			.AddCustomRow(LOCTEXT("T4EditorGameplaySettingObjectUseWeapon", "Weapon NameIDs"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EditorGameplaySettingObjectUseWeaponTitle", "Select a Weapon NameID"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				SNew(ST4ContentNameIDDropListWidget, InEditorPlaySettings, ET4EditorGameDataType::EdData_Weapon)
				.OnSelected(this, &FT4EditorPlaySettingDetailCustomization::HandleOnWeaponContentNameIDSelected)
				.PropertyHandle(WeaponContentNameIDSelectedHandle)
			];

		InBuilder.EditCategory(PlayerSettingsCategoryName)
			.AddCustomRow(LOCTEXT("T4EditorGameplaySettingObjectUseWeaponBtn", "Weapon"))
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
					.Text(LOCTEXT("T4EditorGameplaySettingObjectUseWeaponShowBtn", "Show Preview"))
					.ToolTipText(LOCTEXT("T4EditorGameplaySettingObjectUseWeaponShowBtn_Tooltip", "Show Preview"))
					.OnClicked(this, &FT4EditorPlaySettingDetailCustomization::HandleOnButtonWeaponShowPreview)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EditorGameplaySettingObjectUseWeaponEquipBtn", "Equip"))
					.ToolTipText(LOCTEXT("T4EditorGameplaySettingObjectUseWeaponEquipBtnBtn_Tooltip", "Do Equip Weapon"))
					.OnClicked(this, &FT4EditorPlaySettingDetailCustomization::HandleOnButtonWeaponEquip)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EditorGameplaySettingObjectUseWeaponUnEquipBtn", "UnEquip"))
					.ToolTipText(LOCTEXT("T4EditorGameplaySettingObjectUseWeaponUnEquipBtn_Tooltip", "Do UnEquip Weapon"))
					.OnClicked(this, &FT4EditorPlaySettingDetailCustomization::HandleOnButtonWeaponUnequip)
				]
			];

		InBuilder.EditCategory(PlayerSettingsCategoryName)
			.AddCustomRow(LOCTEXT("T4EditorGameplaySettingObjectUseCostume", "Costume NameIDs"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EditorGameplaySettingObjectUseCostumeTitle", "Select a Costume NameID"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				SNew(ST4ContentNameIDDropListWidget, InEditorPlaySettings, ET4EditorGameDataType::EdData_Costume)
				.OnSelected(this, &FT4EditorPlaySettingDetailCustomization::HandleOnCostumeContentNameIDSelected)
				.PropertyHandle(CostumeContentNameIDSelectedHandle)
			];

		InBuilder.EditCategory(PlayerSettingsCategoryName)
			.AddCustomRow(LOCTEXT("T4EditorGameplaySettingObjectUseCostumeBtn", "Costume"))
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
					.Text(LOCTEXT("T4EditorGameplaySettingObjectUseCostumeShowBtn", "Show Preview"))
					.ToolTipText(LOCTEXT("T4EditorGameplaySettingObjectUseCostumeShowBtn_Tooltip", "Show Preview"))
					.OnClicked(this, &FT4EditorPlaySettingDetailCustomization::HandleOnButtonCostumeShowPreview)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EditorGameplaySettingObjectUseCostumeExchangeBtn", "Exchange"))
					.ToolTipText(LOCTEXT("T4EditorGameplaySettingObjectUseCostumeExchangeBtn_Tooltip", "Do Exchange Costume"))
					.OnClicked(this, &FT4EditorPlaySettingDetailCustomization::HandleOnButtonCostumeExchange)
				]
			];
	}
}

void FT4EditorPlaySettingDetailCustomization::CustomizeSandbackSettingsDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4EditorGameplaySettingObject* InEditorPlaySettings
) // #60
{
	check(nullptr != InEditorPlaySettings);

	static const TCHAR* SandbackSpawnCategoryName = TEXT("Sandback Settings");

	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4EditorGameplaySettingObject, NPCContentNameIDSelected);

	InBuilder.EditCategory(SandbackSpawnCategoryName)
		.AddCustomRow(LOCTEXT("T4EditorGameplaySettingObjectNPCSpawn", "NPC NameIDs"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("T4EditorGameplaySettingObjectNPCSpawnTitle", "Select a NPC NameID"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			SNew(ST4ContentNameIDDropListWidget, InEditorPlaySettings, ET4EditorGameDataType::EdData_NPC)
			.OnSelected(this, &FT4EditorPlaySettingDetailCustomization::HandleOnNPCContentNameIDSelected)
			.PropertyHandle(NPCContentNameIDSelectedHandle)
		];

	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4EditorGameplaySettingObject, NPCEntityAsset); // #76
	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(SandbackSpawnCategoryName);
	DetailCategoryBuilder.AddProperty(NPCEntityAssetHandle); // #76

	InBuilder.EditCategory(SandbackSpawnCategoryName)
		.AddCustomRow(LOCTEXT("T4EditorGameplaySettingObjectNPCDoSpawn", "NPC Spawn"))
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
				.Text(LOCTEXT("T4EditorGameplaySettingObjectNPCShowPreviewBtn", "Show Preview"))
				.ToolTipText(LOCTEXT("T4EditorGameplaySettingObjectNPCShowPreviewBtn_Tooltip", "Show Preview"))
				.OnClicked(this, &FT4EditorPlaySettingDetailCustomization::HandleOnButtonNPCShowPreview)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("T4EditorGameplaySettingObjectNPCSpawnBtn", "Spawn"))
				.ToolTipText(LOCTEXT("T4EditorGameplaySettingObjectPCSpawnBtn_Tooltip", "Do NPC Spawn"))
				.OnClicked(this, &FT4EditorPlaySettingDetailCustomization::HandleOnButtonNPCDoSpawn)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("T4EditorGameplaySettingObjectNPCSpawnQuadBtn", "Spawn4"))
				.ToolTipText(LOCTEXT("T4EditorGameplaySettingObjectPCSpawnBtn_Tooltip", "Do NPC Spawn Quad"))
				.OnClicked(this, &FT4EditorPlaySettingDetailCustomization::HandleOnButtonNPCDoSpawn4)
			]
		];
	
	// #T4_ADD_EDITOR_PLAY_TAG

	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4EditorGameplaySettingObject, bAIDisabled);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4EditorGameplaySettingObject, SandbackRole);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4EditorGameplaySettingObject, bSandbackOneHitDie); // #76

	DetailCategoryBuilder.AddProperty(bAIDisabledHandle);
	DetailCategoryBuilder.AddProperty(SandbackRoleHandle);
	DetailCategoryBuilder.AddProperty(bSandbackOneHitDieHandle); // #76

	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4EditorGameplaySettingObject, DieReactionNameIDSelected); // #76

	TSharedRef<FT4EditorViewTargetSelector> ViewTargetSelector = ViewModel->GetViewTargetSelector(); // #57
	TSharedPtr<ST4ReactionDropListWidget> ReactionDropListWidget = SNew(ST4ReactionDropListWidget, ViewTargetSelector)
		.OnSelected(nullptr)
		.PropertyHandle(DieReactionNameIDSelectedHandle);
	ReactionDropListWidget->OnRefresh();

	DetailCategoryBuilder
		.AddCustomRow(LOCTEXT("T4EditorGameplaySettingObjectReactionNameSelector", "ReactionName"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("T4EditorGameplaySettingObjectReactionNameSelectorTitle", "Select a Reaction"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			ReactionDropListWidget.ToSharedRef()
		];
}

void FT4EditorPlaySettingDetailCustomization::CustomizeAttackerDataDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4EditorGameplaySettingObject* InEditorPlaySettings
) // #60
{
	check(nullptr != InEditorPlaySettings);
	{
		AttackerCategoryName = TEXT("Attacker SkillData");

		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4EditorGameplaySettingObject, bOverrideSkillData); // #63

		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(*AttackerCategoryName);
		DetailCategoryBuilder.AddProperty(bOverrideSkillDataHandle);

		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4EditorGameplaySettingObject, SkillContentNameIDSelected);
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4EditorGameplaySettingObject, SkillDataInfo);

		InBuilder.EditCategory(*AttackerCategoryName)
			.AddCustomRow(LOCTEXT("T4EditorGameplaySettingObjectSkillContentNameID", "Skill NameID"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EditorGameplaySettingObjectSkillContentNameIDDTitle", "Select a Skill NameID"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				SNew(ST4ContentNameIDDropListWidget, InEditorPlaySettings, ET4EditorGameDataType::EdData_Skill)
				.OnSelected(this, &FT4EditorPlaySettingDetailCustomization::HandleOnSkillContentNameIDSelected)
				.PropertyHandle(SkillContentNameIDSelectedHandle)
			];

		// #T4_ADD_SKILL_CONTENT_TAG 
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(SkillDataInfoHandle, Name);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(SkillDataInfoHandle, AttackType);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(SkillDataInfoHandle, HitDelayTimeSec);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(SkillDataInfoHandle, DurationSec);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(SkillDataInfoHandle, ProjectileSpeed); // #63
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(SkillDataInfoHandle, bMoveable);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(SkillDataInfoHandle, ResultEffectDataID);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(SkillDataInfoHandle, ContiAsset);
	}
}

void FT4EditorPlaySettingDetailCustomization::CustomizeDefenderDataDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4EditorGameplaySettingObject* InEditorPlaySettings
) // #60
{
	check(nullptr != InEditorPlaySettings);
	{
		DefenderCategoryName = TEXT("Defender EffectData");

		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4EditorGameplaySettingObject, bOverrideEffectData); // #63

		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(*DefenderCategoryName);
		DetailCategoryBuilder.AddProperty(bOverrideEffectDataHandle);

		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4EditorGameplaySettingObject, EffectContentNameIDSelected);
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4EditorGameplaySettingObject, EffectDataInfo);

		InBuilder.EditCategory(*DefenderCategoryName)
			.AddCustomRow(LOCTEXT("T4EditorGameplaySettingObjectEffectContentNameID", "Effect NameID"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EditorGameplaySettingObjectEffectContentNameIDTitle", "Select a Effect NameID"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				SNew(ST4ContentNameIDDropListWidget, InEditorPlaySettings, ET4EditorGameDataType::EdData_Effect)
				.OnSelected(this, &FT4EditorPlaySettingDetailCustomization::HandleOnEffectContentNameIDSelected)
				.PropertyHandle(EffectContentNameIDSelectedHandle)
			];

		// #T4_ADD_EFFECT_CONTENT_TAG
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(EffectDataInfoHandle, Name);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(EffectDataInfoHandle, EffectType);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(EffectDataInfoHandle, HitDelayTimeSec);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(EffectDataInfoHandle, AreaRange);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(EffectDataInfoHandle, DamageEffectDataID);
		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(EffectDataInfoHandle, ContiAsset);
	}
}

FReply FT4EditorPlaySettingDetailCustomization::HandleOnSaveEditorPlaySettings()
{
	ViewModel->DoSaveEditorPlaySettings();
	return FReply::Handled();
}

FReply FT4EditorPlaySettingDetailCustomization::HandleOnButtonNPCShowPreview()
{
	UT4EditorGameplaySettingObject* EditorPlaySettings = ViewModel->GetEditorPlaySettings(); // #60
	check(nullptr != EditorPlaySettings);
	if (EditorPlaySettings->NPCContentNameIDSelected == NAME_None)
	{
#if 0
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			LOCTEXT("T4EditorPlayNPCShowPreviewError", "NPC Content NameID Not Selected."),
			&T4EditorPlayCustomizeErrorTitleText
		);
#endif
		return FReply::Handled();
	}
	IT4EditorGameData* EditorGameData = EditorPlaySettings->GetEditorGameData();
	if (nullptr != EditorGameData)
	{
		UT4EntityAsset* EntityAsset = EditorGameData->GetEntityAsset(
			ET4EditorGameDataType::EdData_NPC,
			EditorPlaySettings->NPCContentNameIDSelected
		);
		ViewModel->SetPreviewViewTarget(EntityAsset);
	}
	return FReply::Handled();
}

FReply FT4EditorPlaySettingDetailCustomization::HandleOnButtonNPCDoSpawn()
{
	UT4EditorGameplaySettingObject* EditorPlaySettings = ViewModel->GetEditorPlaySettings(); // #60
	check(nullptr != EditorPlaySettings);
	if (EditorPlaySettings->NPCContentNameIDSelected == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			LOCTEXT("T4EditorPlayNPCDoSpawnwError", "NPC Content NameID Not Selected."),
			&T4EditorPlayCustomizeErrorTitleText
		);
		return FReply::Handled();
	}
	ViewModel->ServerSpawnObject(EditorPlaySettings->NPCContentNameIDSelected);
	return FReply::Handled();
}

FReply FT4EditorPlaySettingDetailCustomization::HandleOnButtonNPCDoSpawn4()
{
	UT4EditorGameplaySettingObject* EditorPlaySettings = ViewModel->GetEditorPlaySettings(); // #60
	check(nullptr != EditorPlaySettings);
	if (EditorPlaySettings->NPCContentNameIDSelected == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			LOCTEXT("T4EditorPlayNPCDoSpawnwError", "NPC Content NameID Not Selected."),
			&T4EditorPlayCustomizeErrorTitleText
		);
		return FReply::Handled();
	}
	for (uint32 i = 0; i < 4; ++i)
	{
		ViewModel->ServerSpawnObject(EditorPlaySettings->NPCContentNameIDSelected);
	}
	return FReply::Handled();
}

FReply FT4EditorPlaySettingDetailCustomization::HandleOnButtonWeaponShowPreview()
{
	UT4EditorGameplaySettingObject* EditorPlaySettings = ViewModel->GetEditorPlaySettings(); // #60
	check(nullptr != EditorPlaySettings);
	if (EditorPlaySettings->WeaponContentNameIDSelected == NAME_None)
	{
#if 0
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			LOCTEXT("T4EditorPlayWeaponShowPreviewError", "Weapon Content NameID Not Selected."),
			&T4EditorPlayCustomizeErrorTitleText
		);
#endif
		return FReply::Handled();
	}
	IT4EditorGameData* EditorGameData = EditorPlaySettings->GetEditorGameData();
	if (nullptr != EditorGameData)
	{
		UT4EntityAsset* EntityAsset = EditorGameData->GetEntityAsset(
			ET4EditorGameDataType::EdData_Weapon,
			EditorPlaySettings->WeaponContentNameIDSelected
		);
		ViewModel->SetPreviewViewTarget(EntityAsset);
	}
	return FReply::Handled();
}

FReply FT4EditorPlaySettingDetailCustomization::HandleOnButtonWeaponEquip()
{
	UT4EditorGameplaySettingObject* EditorPlaySettings = ViewModel->GetEditorPlaySettings(); // #60
	check(nullptr != EditorPlaySettings);
	if (EditorPlaySettings->WeaponContentNameIDSelected == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			LOCTEXT("T4EditorPlayWeaponEquipError", "Weapon Content NameID Not Selected."),
			&T4EditorPlayCustomizeErrorTitleText
		);
		return FReply::Handled();
	}
	ViewModel->ServerEquipWeapon(EditorPlaySettings->WeaponContentNameIDSelected, false);
	return FReply::Handled();
}

FReply FT4EditorPlaySettingDetailCustomization::HandleOnButtonWeaponUnequip()
{
	UT4EditorGameplaySettingObject* EditorPlaySettings = ViewModel->GetEditorPlaySettings(); // #60
	check(nullptr != EditorPlaySettings);
	if (EditorPlaySettings->WeaponContentNameIDSelected == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			LOCTEXT("T4EditorPlayWeaponUnEquipError", "Weapon Content NameID Not Selected."),
			&T4EditorPlayCustomizeErrorTitleText
		);
		return FReply::Handled();
	}
	ViewModel->ServerEquipWeapon(EditorPlaySettings->WeaponContentNameIDSelected, true);
	return FReply::Handled();
}

FReply FT4EditorPlaySettingDetailCustomization::HandleOnButtonCostumeShowPreview()
{
	UT4EditorGameplaySettingObject* EditorPlaySettings = ViewModel->GetEditorPlaySettings(); // #60
	check(nullptr != EditorPlaySettings);
	if (EditorPlaySettings->CostumeContentNameIDSelected == NAME_None)
	{
#if 0
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			LOCTEXT("FT4EditorPlayCostumeShowPreviewError", "Costume Content NameID Not Selected."),
			&T4EditorPlayCustomizeErrorTitleText
		);
#endif
		return FReply::Handled();
	}
	IT4EditorGameData* EditorGameData = EditorPlaySettings->GetEditorGameData();
	if (nullptr != EditorGameData)
	{
		UT4EntityAsset* EntityAsset = EditorGameData->GetEntityAsset(
			ET4EditorGameDataType::EdData_Costume,
			EditorPlaySettings->CostumeContentNameIDSelected
		);
		ViewModel->SetPreviewViewTarget(EntityAsset);
	}
	return FReply::Handled();
}

FReply FT4EditorPlaySettingDetailCustomization::HandleOnButtonCostumeExchange()
{
	return FReply::Handled();
}

void FT4EditorPlaySettingDetailCustomization::HandleOnNPCContentNameIDSelected(const FName InName)
{
	HandleOnButtonNPCShowPreview();
	if (!ViewModel.IsValid())
	{
		return;
	}
	ViewModel->UpdateEditorPlayNPCEntityAsset();
}

void FT4EditorPlaySettingDetailCustomization::HandleOnWeaponContentNameIDSelected(const FName InName)
{
	HandleOnButtonWeaponShowPreview();
}

void FT4EditorPlaySettingDetailCustomization::HandleOnCostumeContentNameIDSelected(const FName InName)
{
	HandleOnButtonWeaponShowPreview();
}

void FT4EditorPlaySettingDetailCustomization::HandleOnSkillContentNameIDSelected(const FName InName)
{
	if (!ViewModel.IsValid())
	{
		return;
	}
	ViewModel->UpdateEditorPlaySkillDataInfo();
}

void FT4EditorPlaySettingDetailCustomization::HandleOnEffectContentNameIDSelected(const FName InName)
{
	if (!ViewModel.IsValid())
	{
		return;
	}
	ViewModel->UpdateEditorPlayEffectDataInfo();
}

void FT4EditorPlaySettingDetailCustomization::HandleOnRefreshLayout()
{
	if (nullptr != DetailLayoutPtr)
	{
		DetailLayoutPtr->ForceRefreshDetails();
	}
}

#undef LOCTEXT_NAMESPACE
