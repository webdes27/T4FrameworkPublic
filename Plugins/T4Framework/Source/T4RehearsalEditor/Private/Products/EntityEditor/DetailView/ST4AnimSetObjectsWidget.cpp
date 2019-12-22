// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4AnimSetObjectsWidget.h"
#include "T4AnimSetDetailCustomization.h"

#include "Products/EntityEditor/ViewModel/T4EntityViewModel.h"

#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h"

#include "Modules/ModuleManager.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "IDetailsView.h"

#define LOCTEXT_NAMESPACE "T4AnimSetObjectsDetails"

/**
  * #39
 */

void ST4AnimSetObjectsWidget::Construct(
	const FArguments& InArgs, 
	TSharedPtr<FT4EntityViewModel> InEntityViewModel
)
{
	bUpdatingChangingPropertiesFromDetailView = false;

	EntityViewModel = InEntityViewModel;

	// #77 : Entity 뷰모델의 Detail 이 변경되면 함께 업데이트 해준다.
	EntityViewModel->GetOnViewModelDetailPropertyChanged().AddRaw(
		this,
		&ST4AnimSetObjectsWidget::HandleOnUpdateDetails
	);

	// #77 : Property 변경을 ViewModel 에 노티한다.
	GetOnDetailsPropertiesChanged().AddRaw(
		EntityViewModel.Get(),
		&FT4EntityViewModel::HandleOnAnimSetDetailsPropertiesChanged
	);

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, true);
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	DetailsView->OnFinishedChangingProperties().AddRaw(
		this,
		&ST4AnimSetObjectsWidget::HandleOnDetailsPropertiesChanged
	);

	DetailsView->RegisterInstancedCustomPropertyLayout(
		UT4AnimSetAsset::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FT4AnimSetDetailCustomization::MakeInstance,
			EntityViewModel
		)
	);

	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = EntityViewModel->GetAnimSetAssetSelector();
	if (!AnimSetAssetSelector->IsNull())
	{
		UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector->GetAnimSetAsset();
		check(nullptr != AnimSetAsset)
		DetailsView->SetObject((UObject*)AnimSetAsset);
	}

	ChildSlot
	[
		DetailsView.ToSharedRef()
	];
}

void ST4AnimSetObjectsWidget::HandleOnUpdateDetails()
{
	if (bUpdatingChangingPropertiesFromDetailView)
	{
		return;
	}
	TSharedRef<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector = EntityViewModel->GetAnimSetAssetSelector();
	if (!AnimSetAssetSelector->IsNull())
	{
		UT4AnimSetAsset* AnimSetAsset = AnimSetAssetSelector->GetAnimSetAsset();
		check(nullptr != AnimSetAsset)
		DetailsView->SetObject((UObject*)AnimSetAsset);
	}
	else
	{
		DetailsView->SetObject(nullptr);
	}
}

void ST4AnimSetObjectsWidget::HandleOnDetailsPropertiesChanged(
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

#undef LOCTEXT_NAMESPACE // "T4AnimSetObjectsDetails"
