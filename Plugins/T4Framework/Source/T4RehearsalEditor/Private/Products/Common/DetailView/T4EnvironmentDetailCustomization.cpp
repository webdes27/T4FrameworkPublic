// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4EnvironmentDetailCustomization.h"

#include "Products/Common/Widgets/DropListView/ST4TimeTagDropListWidget.h" // #90
#include "Products/Common/Widgets/ListView/ST4TimeTagListWidget.h" // #90

#include "Products/Common/DetailView/T4EnvironmentDetailObject.h" // #90
#include "Products/Common/ViewModel/T4WorldMapViewModel.h"

#include "Products/T4RehearsalEditorUtils.h" // #104
#include "Products/Common/Utility/T4AssetEnvironmentUtils.h" // #94

#include "T4Asset/Classes/World/T4EnvironmentAsset.h"

#include "T4Engine/Public/T4Engine.h"

#include "HAL/PlatformApplicationMisc.h" // #104

#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SScrollBox.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"

#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4EnvironmentDetailCustomization"

static const FText T4T4EnvironmentDetailsErrorTitleText = LOCTEXT("T4EnvironmentDetailsError", "Environment CustomDetail Error"); // #94

/**
  * #94
 */
FT4EnvironmentDetailCustomization::FT4EnvironmentDetailCustomization(
	IT4RehearsalViewModel* InViewModel
)	: ViewModelRef(InViewModel)
{
}

FT4EnvironmentDetailCustomization::~FT4EnvironmentDetailCustomization()
{
}

void FT4EnvironmentDetailCustomization::CustomizeEnvironmentDetails(
	IDetailCategoryBuilder& InDetailCategoryBuilder,
	UT4EnvironmentAsset* InEnvironmentAsset
) // #94
{
	check(nullptr != ViewModelRef);
	EnvironmentAssetPtr = InEnvironmentAsset;

	TimeTagListWidgetPtr = SNew(ST4TimeTagListWidget, InEnvironmentAsset)
		.OnSelected(this, &FT4EnvironmentDetailCustomization::HandleOnTimeTagSelected)
		.OnDoubleClicked(nullptr);

	InDetailCategoryBuilder
		.AddCustomRow(LOCTEXT("T4EnvironmentDetailTimeTagList", "TimeTags"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("T4EnvironmentDetailTimeTagListEntryBoxTitle", "Tags"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			TimeTagListWidgetPtr.ToSharedRef()
		];

	{
		TimeTagDropListWidgetPtr = SNew(ST4TimeTagDropListWidget)
			.OnSelected(nullptr)
			.PropertyHandle(nullptr);
		TimeTagDropListWidgetPtr->OnRefresh();

		InDetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EnvironmentDetailTimeTagNameSelector", "TimeTagName"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(LOCTEXT("T4EnvironmentDetailTimeTagNameSelectorTitle", "TimeTag Name"))
			]
			.ValueContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				TimeTagDropListWidgetPtr.ToSharedRef()
			];

		UT4EnvironmentDetailObject* EnvironmentDetailObject = ViewModelRef->GetEnvironmentDetailObject();
		check(nullptr != EnvironmentDetailObject);

		EnvironmentDetailObject->OnPropertiesChanged().AddRaw(
			this,
			&FT4EnvironmentDetailCustomization::HandleOnEnvironmentDetailsPropertiesChanged
		); // #95, #98

		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, true);

		EnvironmentDetailsViewPtr = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
		EnvironmentDetailsViewPtr->SetObject(Cast<UObject>(EnvironmentDetailObject));

		InDetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EnvironmentDetailMapEnvironemntCustomizationDetail", "MapEnvironemntDetail"))
			.WholeRowContent()
			.HAlign(HAlign_Fill)
			.MaxDesiredWidth(400.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				.Padding(0.0f)
				[
					SNew(SBorder)
					//.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
					.Padding(0)
					[
						SNew(SBox)
						//.WidthOverride(175)
						[
							SNew(SVerticalBox)

							+ SVerticalBox::Slot()
							.MaxHeight(600)
							.Padding(4.0f)
							[
								EnvironmentDetailsViewPtr.ToSharedRef()
							]
						]
					]
				]
			];

		InDetailCategoryBuilder
			.AddCustomRow(LOCTEXT("T4EnvironmentDetailTimeTagSelector", "TimeTag Selector"))
			.WholeRowContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EnvironmentDetailTimeTagAddBtn", "Add"))
					.ToolTipText(LOCTEXT("T4EnvironmentDetailTimeTagAddBtn_Tooltip", "Add TimeTag"))
					.OnClicked(this, &FT4EnvironmentDetailCustomization::HandleOnTimeTagAdd)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EnvironmentDetailTimeTagRemoveBtn", "Remove"))
					.ToolTipText(LOCTEXT("T4EnvironmentDetailTimeTagRemoveBtn_Tooltip", "Remove Selected TimeTag"))
					.OnClicked(this, &FT4EnvironmentDetailCustomization::HandleOnTimeTagRemoveSelected)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EnvironmentDetailTimeTagCopyClipboardBtn", "Copy to Clipboard"))
					.ToolTipText(LOCTEXT("T4EnvironmentDetailTimeTagCopyClipboardBtn_Tooltip", "Copy To Clipboard"))
					.OnClicked(this, &FT4EnvironmentDetailCustomization::HandleOnTimeTagCopyClipboardSelected)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EnvironmentDetailTimeTagPasteClipboardBtn", "Paste from Clipboard"))
					.ToolTipText(LOCTEXT("T4EnvironmentDetailTimeTagPasteClipboardBtn_Tooltip", "Paste from Clipboard"))
					.OnClicked(this, &FT4EnvironmentDetailCustomization::HandleOnTimeTagPasteClipboardSelected)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(2.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("T4EnvironmentDetailTimeTagGetEnvironmentSettingsBtn", "Get from World Settings"))
					.ToolTipText(LOCTEXT("T4EnvironmentDetailTimeTagGetEnvironmentSettingsBtn_Tooltip", "Get Environment Settings from World"))
					.OnClicked(this, &FT4EnvironmentDetailCustomization::HandleOnTimeTagGetEnvronmentSettings)
				]
			];
	}

	TimeTagListWidgetPtr->OnRefresh(true);
}

void FT4EnvironmentDetailCustomization::OnApplyEnvironmentAsset(
	UT4EnvironmentAsset* InEnvironmentAsset,
	bool bAutoSelect
) // #94
{
	if (EnvironmentAssetPtr != InEnvironmentAsset)
	{
		EnvironmentAssetPtr = InEnvironmentAsset;
		if (TimeTagListWidgetPtr.IsValid())
		{
			TimeTagListWidgetPtr->SetEnvironmentAsset(InEnvironmentAsset);
		}
		if (bAutoSelect)
		{
			RefreshWithAutoSelected();
		}
		else
		{
			RefreshWidgets();
		}
	}
}

void FT4EnvironmentDetailCustomization::OnApplyTimeTag() // #95
{
	if (TimeTagListWidgetPtr.IsValid())
	{
		check(nullptr != ViewModelRef);
		ViewModelRef->ChangeWorldEnvironment(TimeTagListWidgetPtr->GetItemValueSelected());
	}
}

void FT4EnvironmentDetailCustomization::RefreshWidgets()
{
	FName TimeTagSelected = NAME_None;
	if (TimeTagListWidgetPtr.IsValid())
	{
		TimeTagListWidgetPtr->OnRefresh(false);
		TimeTagSelected = TimeTagListWidgetPtr->GetItemValueSelected();
	}
	if (TimeTagDropListWidgetPtr.IsValid())
	{
		TimeTagDropListWidgetPtr->SetInitializeValue(TimeTagSelected);
		TimeTagDropListWidgetPtr->OnRefresh();
	}
}

void FT4EnvironmentDetailCustomization::RefreshWithAutoSelected()
{
	FName TimeTagSelected = NAME_None;
	if (TimeTagListWidgetPtr.IsValid())
	{
		TimeTagListWidgetPtr->SetInitializeValue(NAME_None);
		TimeTagListWidgetPtr->OnRefresh(false);

		TimeTagSelected = TimeTagListWidgetPtr->GetItemValueSelected();
		if (TimeTagSelected != NAME_None)
		{
			HandleOnTimeTagSelected(TimeTagSelected);
		}
		else
		{
			check(nullptr != ViewModelRef);
			ViewModelRef->ChangeWorldEnvironment(NAME_None);
		}
	}
	if (TimeTagDropListWidgetPtr.IsValid())
	{
		TimeTagDropListWidgetPtr->SetInitializeValue(TimeTagSelected);
		TimeTagDropListWidgetPtr->OnRefresh();
	}
}

void FT4EnvironmentDetailCustomization::HandleOnEnvironmentDetailsPropertiesChanged() // #95
{
	FName TimeTagSelected = TimeTagDropListWidgetPtr->GetItemValueSelected();
	if (TimeTagSelected == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(TEXT("No TimeTag Selected")),
			&T4T4EnvironmentDetailsErrorTitleText
		);
		return;
	}
	if (!EnvironmentAssetPtr.IsValid())
	{
		return;
	}

	check(nullptr != ViewModelRef);
	UT4EnvironmentDetailObject* EnvironmentDetailObject = ViewModelRef->GetEnvironmentDetailObject();
	if (nullptr == EnvironmentDetailObject)
	{
		return;
	}

	FT4EnvTimeTagData TimeTagData;
	EnvironmentDetailObject->CopyTo(TimeTagData);
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EnviromentTimeTagUpdate(
		EnvironmentAssetPtr.Get(),
		TimeTagSelected,
		&TimeTagData,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4T4EnvironmentDetailsErrorTitleText
		);
		return;
	}
	if (TimeTagListWidgetPtr.IsValid())
	{
		TimeTagListWidgetPtr->SetInitializeValue(TimeTagSelected);
		TimeTagListWidgetPtr->OnRefresh(false);

		// OnRefresh 후 자동 선택 Notify 는 가지 않음으로 별도로 호출해준다.
		HandleOnTimeTagSelected(TimeTagSelected);
	}
}

void FT4EnvironmentDetailCustomization::HandleOnTimeTagSelected(const FName InName) // #90
{
	if (!EnvironmentAssetPtr.IsValid())
	{
		return;
	}
	FT4EnvTimeTagSetData& TimeTagSetData = EnvironmentAssetPtr->TimeTagSetData;
	if (!TimeTagSetData.TimeTagMap.Contains(InName))
	{
		return;
	}
	check(nullptr != ViewModelRef);
	const FT4EnvTimeTagData& TimeTagData = TimeTagSetData.TimeTagMap[InName];
	UT4EnvironmentDetailObject* EnvironmentDetailObject = ViewModelRef->GetEnvironmentDetailObject();
	check(nullptr != EnvironmentDetailObject);
	EnvironmentDetailObject->CopyFrom(TimeTagData);
	if (TimeTagDropListWidgetPtr.IsValid())
	{
		TimeTagDropListWidgetPtr->SetInitializeValue(InName);
		TimeTagDropListWidgetPtr->OnRefresh();
	}
	ViewModelRef->ChangeWorldEnvironment(InName);
}

FReply FT4EnvironmentDetailCustomization::HandleOnTimeTagAdd() // #90
{
	if (!TimeTagDropListWidgetPtr.IsValid())
	{
		return FReply::Handled();
	}
	FName TimeTagSelected = TimeTagDropListWidgetPtr->GetItemValueSelected();
	if (TimeTagSelected == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(TEXT("No TimeTag Selected")),
			&T4T4EnvironmentDetailsErrorTitleText
		);
		return FReply::Handled();
	}
	if (!EnvironmentAssetPtr.IsValid())
	{
		return FReply::Handled();
	}

	check(nullptr != ViewModelRef);
	UT4EnvironmentDetailObject* EnvironmentDetailObject = ViewModelRef->GetEnvironmentDetailObject();
	check(nullptr != EnvironmentDetailObject);

	FT4EnvTimeTagData TimeTagData;
	EnvironmentDetailObject->CopyTo(TimeTagData);
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EnviromentTimeTagAdd(
		EnvironmentAssetPtr.Get(),
		TimeTagSelected,
		&TimeTagData,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4T4EnvironmentDetailsErrorTitleText
		);
		return FReply::Handled();
	}
	if (TimeTagListWidgetPtr.IsValid())
	{
		TimeTagListWidgetPtr->SetInitializeValue(TimeTagSelected);
		TimeTagListWidgetPtr->OnRefresh(false);

		// OnRefresh 후 자동 선택 Notify 는 가지 않음으로 별도로 호출해준다.
		HandleOnTimeTagSelected(TimeTagSelected);
	}
	return FReply::Handled();
}

FReply FT4EnvironmentDetailCustomization::HandleOnTimeTagRemoveSelected() // #90
{
	if (!TimeTagListWidgetPtr.IsValid())
	{
		return FReply::Handled();
	}
	FName TimeTagSelected = TimeTagListWidgetPtr->GetItemValueSelected();
	if (TimeTagSelected == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(TEXT("No TimeTag Selected")),
			&T4T4EnvironmentDetailsErrorTitleText
		);
		return FReply::Handled();
	}
	if (!EnvironmentAssetPtr.IsValid())
	{
		return FReply::Handled();
	}
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EnviromentTimeTagRemove(
		EnvironmentAssetPtr.Get(),
		TimeTagSelected,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4T4EnvironmentDetailsErrorTitleText
		);
		return FReply::Handled();
	}
	if (TimeTagListWidgetPtr.IsValid())
	{
		TimeTagListWidgetPtr->OnRefresh(false);
	}
	return FReply::Handled();
}

FReply FT4EnvironmentDetailCustomization::HandleOnTimeTagCopyClipboardSelected() // #104
{
	if (!TimeTagListWidgetPtr.IsValid())
	{
		return FReply::Handled();
	}
	FName TimeTagSelected = TimeTagListWidgetPtr->GetItemValueSelected();
	if (TimeTagSelected == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(TEXT("No TimeTag Selected")),
			&T4T4EnvironmentDetailsErrorTitleText
		);
		return FReply::Handled();
	}
	if (!EnvironmentAssetPtr.IsValid())
	{
		return FReply::Handled();
	}
	FString ExportedText;
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EnviromentTimeTagCopyToClipboard(
		EnvironmentAssetPtr.Get(),
		TimeTagSelected,
		ExportedText,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4T4EnvironmentDetailsErrorTitleText
		);
		return FReply::Handled();
	}
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
	return FReply::Handled();
}

FReply FT4EnvironmentDetailCustomization::HandleOnTimeTagPasteClipboardSelected() // #104
{
	if (!TimeTagListWidgetPtr.IsValid())
	{
		return FReply::Handled();
	}
	FString ImportedText;
	FPlatformApplicationMisc::ClipboardPaste(ImportedText);
	if (ImportedText.IsEmpty())
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(TEXT("Clipboard test is Empty")),
			&T4T4EnvironmentDetailsErrorTitleText
		);
		return FReply::Handled();
	}
	FName TimeTagSelected = TimeTagListWidgetPtr->GetItemValueSelected();
	if (TimeTagSelected == NAME_None)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(TEXT("No TimeTag Selected")),
			&T4T4EnvironmentDetailsErrorTitleText
		);
		return FReply::Handled();
	}
	if (!EnvironmentAssetPtr.IsValid())
	{
		return FReply::Handled();
	}
	FString ErrorMessage;
	bool bResult = T4AssetUtil::EnviromentTimeTagPastToClipboard(
		EnvironmentAssetPtr.Get(),
		TimeTagSelected,
		ImportedText,
		ErrorMessage
	);
	if (!bResult)
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			FText::FromString(ErrorMessage),
			&T4T4EnvironmentDetailsErrorTitleText
		);
		return FReply::Handled();
	}
	const FT4EnvTimeTagSetData& TimeTagSetData = EnvironmentAssetPtr->TimeTagSetData;
	if (!TimeTagSetData.TimeTagMap.Contains(TimeTagSelected))
	{
		return FReply::Handled();
	}
	check(nullptr != ViewModelRef);
	const FT4EnvTimeTagData& TimeTagData = TimeTagSetData.TimeTagMap[TimeTagSelected];
	UT4EnvironmentDetailObject* EnvironmentDetailObject = ViewModelRef->GetEnvironmentDetailObject();
	check(nullptr != EnvironmentDetailObject);
	EnvironmentDetailObject->CopyFrom(TimeTagData);
	ViewModelRef->ChangeWorldEnvironment(TimeTagSelected);
	return FReply::Handled();
}

FReply FT4EnvironmentDetailCustomization::HandleOnTimeTagGetEnvronmentSettings() // #90
{
	if (!EnvironmentAssetPtr.IsValid())
	{
		return FReply::Handled();
	}
	check(nullptr != ViewModelRef);
	UT4EnvironmentDetailObject* EnvironmentDetailObject = ViewModelRef->GetEnvironmentDetailObject();
	check(nullptr != EnvironmentDetailObject);
	UWorld* EditorWorld = T4EditorUtil::GetWorld(ET4LayerType::LevelEditor);
	if (nullptr != EditorWorld)
	{
		EnvironmentDetailObject->SyncFrom(EditorWorld);
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE