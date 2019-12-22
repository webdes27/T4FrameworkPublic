// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4CameraWorkSectionKeyDetailCustomization.h"

#include "Products/Common/DetailView/T4DetailCustomizationMacros.h"

#include "Products/Common/Widgets/DropListView/ST4ActionPointDropListWidget.h"

#include "Products/ContiEditor/ViewModel/T4ContiViewModel.h"
#include "Products/ContiEditor/Helper/T4CameraWorkSectionKeyObject.h" // #58

#include "Widgets/Input/SButton.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4CameraWorkSectionKeyDetailCustomization"

/**
  * #58
 */

TSharedRef<IDetailCustomization> FT4CameraWorkSectionKeyDetailCustomization::MakeInstance(
	TSharedPtr<FT4ContiViewModel> InContiViewModel
)
{
	return MakeShared<FT4CameraWorkSectionKeyDetailCustomization>(InContiViewModel);
}

FT4CameraWorkSectionKeyDetailCustomization::FT4CameraWorkSectionKeyDetailCustomization(
	TSharedPtr<FT4ContiViewModel> InContiViewModel
)	: ViewModelPtr(InContiViewModel)
	, DetailLayoutPtr(nullptr)
{
}

void FT4CameraWorkSectionKeyDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& InBuilder)
{
	if (!ViewModelPtr.IsValid())
	{
		return;
	}
	DetailLayoutPtr = &InBuilder;

	static const FName KeyDataCategoryName = TEXT("Key Data");

	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CameraWorkSectionKeyObject, ChannelKey);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CameraWorkSectionKeyObject, StartTimeSec);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CameraWorkSectionKeyObject, EasingCurve);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CameraWorkSectionKeyObject, LookAtPoint);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CameraWorkSectionKeyObject, bInverse);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CameraWorkSectionKeyObject, ViewDirection);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CameraWorkSectionKeyObject, Distance);
	DEFINE_DETAIL_HIDE_CLASS_PROPERTY_MACRO(UT4CameraWorkSectionKeyObject, FOVDegree);

	IDetailCategoryBuilder& DetailCategoryBuilder = InBuilder.EditCategory(KeyDataCategoryName);

	DetailCategoryBuilder.AddProperty(ChannelKeyHandle);
	DetailCategoryBuilder.AddProperty(StartTimeSecHandle);
	DetailCategoryBuilder.AddProperty(EasingCurveHandle);

	TSharedRef<FT4EditorViewTargetSelector> ViewTargetSelector = ViewModelPtr->GetViewTargetSelector(); // #57
	TSharedPtr<ST4ActionPointDropListWidget> ActionPointDropListWidget = SNew(ST4ActionPointDropListWidget, ViewTargetSelector)
		.PropertyHandle(LookAtPointHandle);
	ActionPointDropListWidget->SetNoNameDescription(TEXT("ActiveCamera"));
	ActionPointDropListWidget->OnRefresh();

	{
		DetailCategoryBuilder
		.AddCustomRow(LOCTEXT("CameraSectionKeyDetailLookAtPoint", "LookAtPoint"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(LOCTEXT("LookAtPointTitle", "LookAtPoint"))
		]
		.ValueContent()
		.HAlign(HAlign_Fill)
		.MaxDesiredWidth(400.0f)
		[
			ActionPointDropListWidget.ToSharedRef()
		];
	}

	DetailCategoryBuilder.AddProperty(bInverseHandle);
	DetailCategoryBuilder.AddProperty(ViewDirectionHandle);
	DetailCategoryBuilder.AddProperty(DistanceHandle);
	DetailCategoryBuilder.AddProperty(FOVDegreeHandle);
}

void FT4CameraWorkSectionKeyDetailCustomization::HandleOnRefresh()
{
	if (nullptr != DetailLayoutPtr)
	{
		DetailLayoutPtr->ForceRefreshDetails();
	}
}

#undef LOCTEXT_NAMESPACE
