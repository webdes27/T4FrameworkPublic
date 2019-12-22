// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4WorldObjectWidget.h"
#include "T4WorldDetailCustomization.h"

#include "Products/Common/DetailView/T4EnvironmentDetailObject.h" // #90
#include "Products/WorldEditor/ViewModel/T4WorldViewModel.h"

#include "T4Asset/Classes/World/T4WorldAsset.h"

#include "Modules/ModuleManager.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "IDetailsView.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4WorldObjectWidget"

/**
  * #85
 */
void ST4WorldObjectWidget::Construct(
	const FArguments& InArgs, 
	TSharedPtr<FT4WorldViewModel> InWorldViewModel
)
{
	bUpdatingChangingPropertiesFromDetailView = false; // #54 : 이벤트에 의한 루핑 방지

	WorldViewModelPtr = InWorldViewModel;

	// #77 : Property 변경을 ViewModel 에 노티한다.
	GetOnDetailsPropertiesChanged().AddRaw(
		WorldViewModelPtr.Get(),
		&FT4WorldViewModel::HandleOnDetailsPropertiesChanged
	);

	UT4EnvironmentDetailObject* EnvironmentDetailObject = WorldViewModelPtr->GetEnvironmentDetailObject();
	check(nullptr != EnvironmentDetailObject);

	EnvironmentDetailObject->OnPropertiesChanged().AddRaw(
		this,
		&ST4WorldObjectWidget::HandleOnWorldEnvironmentPropertyChanged
	);

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, true);
	DetailsViewPtr = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	// #77 : Property 변경을 노티 받는다.
	DetailsViewPtr->OnFinishedChangingProperties().AddRaw(
		this,
		&ST4WorldObjectWidget::HandleOnDetailsPropertiesChanged
	);

	UT4WorldAsset* WorldAsset = WorldViewModelPtr->GetWorldAsset();
	if (nullptr != WorldAsset)
	{
		DetailsViewPtr->RegisterInstancedCustomPropertyLayout(
			UT4WorldAsset::StaticClass(),
			FOnGetDetailCustomizationInstance::CreateStatic(
				&FT4WorldDetailCustomization::MakeInstance, 
				SharedThis(this)
			)
		);
		DetailsViewPtr->SetObject((UObject*)WorldAsset);
	}

	ChildSlot
	[
		DetailsViewPtr.ToSharedRef()
	];
}

void ST4WorldObjectWidget::HandleOnDetailsPropertiesChanged(
	const FPropertyChangedEvent& InEvent
) // #77
{
	if (bUpdatingChangingPropertiesFromDetailView)
	{
		return;
	}
	TGuardValue<bool> UpdateGuard(bUpdatingChangingPropertiesFromDetailView, true);
	GetOnDetailsPropertiesChanged().Broadcast(InEvent.GetPropertyName());
}

void ST4WorldObjectWidget::HandleOnWorldEnvironmentPropertyChanged() // #90
{
	if (WorldViewModelPtr.IsValid())
	{
		WorldViewModelPtr->UpdateWorldEnvironment();
	}
}

void ST4WorldObjectWidget::SetDetailCustomization(
	TSharedPtr<FT4WorldDetailCustomization> InWorldDetailCustomization
)
{
	WorldDetailCustomizationPtr = InWorldDetailCustomization;
}

void ST4WorldObjectWidget::OnRefreshWorld() // #104
{
	if (!WorldDetailCustomizationPtr.IsValid())
	{
		return;
	}
	WorldDetailCustomizationPtr->OnRefreshWorld();
}

#undef LOCTEXT_NAMESPACE // "T4WorldObjectsDetails"
