// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4EditorPlaySettingWidget.h"
#include "T4EditorPlaySettingDetailCustomization.h"

#include "Products/Common/Helper/T4EditorGameplaySettingObject.h"
#include "Products/ContiEditor/ViewModel/T4ContiViewModel.h"

#include "T4Asset/Classes/Conti/T4ContiAsset.h"

#include "Modules/ModuleManager.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "IDetailsView.h"

#define LOCTEXT_NAMESPACE "T4EditorGameplaySettingObjectDetails"

/**
  * #60
 */

void ST4EditorPlaySettingWidget::Construct(
	const FArguments& InArgs, 
	TSharedPtr<FT4ContiViewModel> InContiViewModel
)
{
	ContiViewModel = InContiViewModel;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, true);
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->OnFinishedChangingProperties().AddRaw(
		this,
		&ST4EditorPlaySettingWidget::HandleOnDetailsPropertiesChanged
	);

	UT4EditorGameplaySettingObject* EditorPlaySettings = InContiViewModel->GetEditorPlaySettings();
	if (nullptr != EditorPlaySettings)
	{
		UStruct* SelectClass = UT4EditorGameplaySettingObject::StaticClass();
		DetailsView->RegisterInstancedCustomPropertyLayout(
			SelectClass,
			FOnGetDetailCustomizationInstance::CreateStatic(
				&FT4EditorPlaySettingDetailCustomization::MakeInstance,
				InContiViewModel
			)
		);
		DetailsView->SetObject((UObject*)EditorPlaySettings);
	}

	ChildSlot
	[
		DetailsView.ToSharedRef()
	];
}

void ST4EditorPlaySettingWidget::HandleOnDetailsPropertiesChanged(
	const FPropertyChangedEvent& InEvent
) // #63
{
	if (!ContiViewModel.IsValid())
	{
		return;
	}
	ContiViewModel->UpdateEditorPlayActionParameters();
}

#undef LOCTEXT_NAMESPACE // "T4EditorGameplaySettingObjectDetails"
