// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4WorldPreviewObjectWidget.h"
#include "T4WorldPreviewDetailCustomization.h"

#include "Products/WorldEditor/ViewModel/T4WorldViewModel.h"
#include "Products/WorldEditor/DetailView/T4WorldPreviewLevelDetailObject.h" // #85

#include "T4Asset/Classes/World/T4WorldAsset.h"

#include "Modules/ModuleManager.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "IDetailsView.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4WorldPreviewObjectWidget"

/**
  * #85
 */
void ST4WorldPreviewObjectWidget::Construct(
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

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, true);
	DetailsViewPtr = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	// #77 : Property 변경을 노티 받는다.
	DetailsViewPtr->OnFinishedChangingProperties().AddRaw(
		this,
		&ST4WorldPreviewObjectWidget::HandleOnDetailsPropertiesChanged
	);

	UT4WorldPreviewLevelDetailObject* SubLevelDetailObject = InWorldViewModel->GetWorldLevelDetailObject();
	if (nullptr != SubLevelDetailObject)
	{
		DetailsViewPtr->RegisterInstancedCustomPropertyLayout(
			UT4WorldPreviewLevelDetailObject::StaticClass(),
			FOnGetDetailCustomizationInstance::CreateStatic(
				&FT4WorldPreviewDetailCustomization::MakeInstance,
				SharedThis(this)
			)
		);
		DetailsViewPtr->SetObject((UObject*)SubLevelDetailObject);
	}

	ChildSlot
	[
		DetailsViewPtr.ToSharedRef()
	];
}

void ST4WorldPreviewObjectWidget::HandleOnDetailsPropertiesChanged(
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

void ST4WorldPreviewObjectWidget::OnRefreshWorld() // #85 : 서브레벨 정보를 업데이트 한다.
{
	if (bUpdatingChangingPropertiesFromDetailView)
	{
		return;
	}
	if (!WorldPreviewDetailCustomizationPtr.IsValid())
	{
		return;
	}
	WorldPreviewDetailCustomizationPtr->OnRefreshWorld();
}

void ST4WorldPreviewObjectWidget::SetDetailCustomization(
	TSharedPtr<FT4WorldPreviewDetailCustomization> InWorldPreviewDetailCustomization
)
{
	WorldPreviewDetailCustomizationPtr = InWorldPreviewDetailCustomization;
}

#undef LOCTEXT_NAMESPACE // "T4WorldObjectsDetails"
