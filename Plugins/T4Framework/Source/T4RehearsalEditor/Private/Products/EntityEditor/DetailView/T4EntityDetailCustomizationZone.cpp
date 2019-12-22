// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4EntityDetailCustomization.h"
#include "Products/Common/DetailView/T4DetailCustomizationMacros.h"

#include "Products/EntityEditor/ViewModel/T4EntityViewModel.h"

#include "Products/Common/DetailView/T4EnvironmentDetailCustomization.h" // #94

#include "Products/Common/Utility/T4AssetEnvironmentUtils.h" // #94
#include "Products/EntityEditor/Utility/T4AssetEntityUtils.h" // #88

#include "T4Asset/Classes/Entity/T4EntityAsset.h"
#include "T4Asset/Classes/Entity/T4ZoneEntityAsset.h" // #94

#include "Widgets/Input/SButton.h"

#include "DetailLayoutBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"

#include "Misc/MessageDialog.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4EntityCustomizeItemDetails"

static const FText T4ZoneEntityDetailErrorTitleText = LOCTEXT("T4ZoneEntityDetailsError", "Zone CustomDetail Error"); // #94

/**
  * http://api.unrealengine.com/KOR/Programming/Slate/DetailsCustomization/
 */
void FT4EntityDetailCustomization::CustomizeZoneEntityDetails(
	IDetailLayoutBuilder& InBuilder,
	UT4EntityAsset* InEntityAsset
) // #94
{
	check(nullptr != InEntityAsset);

	EnvironmentDetailPtr = MakeShareable(new FT4EnvironmentDetailCustomization(ViewModelPtr.Get())); // #94

	UT4ZoneEntityAsset* ZoneEntityAsset = Cast<UT4ZoneEntityAsset>(InEntityAsset);
	if (nullptr == ZoneEntityAsset)
	{
		return;
	}
	{
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4ZoneEntityAsset, ZoneData);

		static const TCHAR* T4DZoneDefaultCategoryName = TEXT("Default");
		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(T4DZoneDefaultCategoryName);

		DetailCategoryBuilder.AddProperty(ZoneDataHandle);
	}
	{
		DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4ZoneEntityAsset, ZoneEnvironmentData);

		static const TCHAR* T4EnvironmentCategoryName = TEXT("Environment");
		IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(T4EnvironmentCategoryName);

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
				.Text(LOCTEXT("T4EntityZoneDetailEnvironmentAssetSaveBtn", "Save the Modified Properties"))
				.ToolTipText(LOCTEXT("T4EntityZoneDetailEnvironmentAssetSaveBtn_Tooltip", "Save the Modified Properties"))
				.OnClicked(this, &FT4EntityDetailCustomization::HandleOnZoneEnvironmentAssetSave)
			];

		DetailCategoryBuilder.HeaderContent(EditorSettingsHeaderWidget);

		DEFINE_DETAIL_ADD_CATEGORY_CHILD_PROPERTY_MACRO(ZoneEnvironmentDataHandle, EnvironmentAsset); // #94

		EnvironmentAssetHandle->SetOnPropertyValueChanged(
			FSimpleDelegate::CreateSP(this, &FT4EntityDetailCustomization::HandleOnZoneEnvironmentChange)
		);

		// #94
		DetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EntityZoneSimulationDisabled", "Toggle Simulation Disabled"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EntityZoneWorldSimulationDisabledTitle", "Simulation Disabled"))
			]
			.ValueContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SCheckBox)
					.ToolTipText(LOCTEXT("T4EntityZoneWorldTimeStopEnabledBtn", "Toggle World Time Stop"))
					.IsChecked(this, &FT4EntityDetailCustomization::HandleOnGetTimeStopEnabledCheckState)
					.OnCheckStateChanged(this, &FT4EntityDetailCustomization::HandleOnTimeStopEnabledCheckStateChanged)
				]
			];

		if (EnvironmentDetailPtr.IsValid())
		{
			EnvironmentDetailPtr->CustomizeEnvironmentDetails(
				DetailCategoryBuilder,
				ZoneEntityAsset->ZoneEnvironmentData.EnvironmentAsset.LoadSynchronous()
			); // #94
		}
	}
}

void FT4EntityDetailCustomization::HandleOnZoneEnvironmentChange() // #94
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	UT4ZoneEntityAsset* ZoneEntityAsset = Cast<UT4ZoneEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == ZoneEntityAsset)
	{
		return;
	}
	if (EnvironmentDetailPtr.IsValid())
	{
		EnvironmentDetailPtr->OnApplyEnvironmentAsset(
			ZoneEntityAsset->ZoneEnvironmentData.EnvironmentAsset.LoadSynchronous(),
			true
		);
	}
}

FReply FT4EntityDetailCustomization::HandleOnZoneEnvironmentAssetSave() // #94
{
	if (!ViewModelPtr.IsValid())
	{
		return FReply::Handled();
	}
	UT4ZoneEntityAsset* ZoneEntityAsset = Cast<UT4ZoneEntityAsset>(ViewModelPtr->GetEntityAsset());
	if (nullptr == ZoneEntityAsset)
	{
		return FReply::Handled();
	}
	const FT4EntityZoneEnvironmentData& ZoneEnvironmentData = ZoneEntityAsset->ZoneEnvironmentData;
	if (!ZoneEnvironmentData.EnvironmentAsset.IsValid())
	{
		return FReply::Handled();
	}
	UT4EnvironmentAsset* EnvironmentAsset = ZoneEnvironmentData.EnvironmentAsset.LoadSynchronous();
	if (nullptr == EnvironmentAsset)
	{
		return FReply::Handled();
	}
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EnviromentAssetSave(EnvironmentAsset, ErrorMessage);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4ZoneEntityDetailErrorTitleText
		);
	}
	return FReply::Handled();
}

ECheckBoxState FT4EntityDetailCustomization::HandleOnGetTimeStopEnabledCheckState() const // #94
{
	if (!ViewModelPtr.IsValid())
	{
		return ECheckBoxState::Unchecked;
	}
	if (ViewModelPtr->IsGameWorldTimeStopped())
	{
		return ECheckBoxState::Checked;
	}
	return ECheckBoxState::Unchecked;
}

void FT4EntityDetailCustomization::HandleOnTimeStopEnabledCheckStateChanged(ECheckBoxState InCheckState) // #94
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	if (ECheckBoxState::Checked == InCheckState)
	{
		ViewModelPtr->SetGameWorldTimeStop(true);
		if (EnvironmentDetailPtr.IsValid())
		{
			EnvironmentDetailPtr->OnApplyTimeTag();
		}
	}
	else
	{
		ViewModelPtr->SetGameWorldTimeStop(false);
	}
}

#undef LOCTEXT_NAMESPACE
