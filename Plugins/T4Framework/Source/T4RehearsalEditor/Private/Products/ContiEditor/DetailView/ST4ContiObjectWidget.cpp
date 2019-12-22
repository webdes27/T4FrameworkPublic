// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4ContiObjectWidget.h"
#include "T4ContiDetailCustomization.h"

#include "T4Asset/Classes/Conti/T4ContiAsset.h"

#include "Modules/ModuleManager.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "IDetailsView.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "T4ContiObjectsDetails"

/**
  *
 */
void ST4ContiObjectWidget::Construct(
	const FArguments& InArgs, 
	TSharedPtr<FT4ContiViewModel> InContiViewModel
)
{
	bUpdatingChangingPropertiesFromDetailView = false; // #54 : 이벤트에 의한 루핑 방지
	
	ContiViewModelPtr = InContiViewModel;

	// #56 : 뷰모델의 변경사항을 노티 받는다.
	ContiViewModelPtr->GetOnViewModelChanged().AddRaw(
		this, 
		&ST4ContiObjectWidget::HandleOnViewModelChanged
	);

	// #56 : Property 변경을 ViewModel 에 노티한다.
	// #100 : 전체에서 일부 Property 만 전체 변경하는 것으로 처리했느나, 현 시점부로 개별 업데이트 처리한다.
#if 0
	GetOnDetailsPropertiesChanged().AddRaw(
		ContiViewModel.Get(),
		&FT4ContiViewModel::HandleOnDetailsPropertiesChanged
	);
#endif

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(false, false, true, FDetailsViewArgs::HideNameArea, true);
	DetailsViewPtr = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	// #56 : Property 변경을 노티 받는다.
	DetailsViewPtr->OnFinishedChangingProperties().AddRaw(
		this,
		&ST4ContiObjectWidget::HandleOnDetailsPropertiesChanged
	);

	DetailsViewPtr->RegisterInstancedCustomPropertyLayout(
		UT4ContiAsset::StaticClass(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FT4ContiDetailCustomization::MakeInstance, 
			SharedThis(this)
		)
	);

	UT4ContiAsset* ContiAsset = ContiViewModelPtr->GetContiAsset();
	if (nullptr != ContiAsset)
	{
		DetailsViewPtr->SetObject((UObject*)ContiAsset);
	}

	ChildSlot
	[
		DetailsViewPtr.ToSharedRef()
	];
}

void ST4ContiObjectWidget::HandleOnDetailsPropertiesChanged(
	const FPropertyChangedEvent& InEvent
) // #56
{
	if (bUpdatingChangingPropertiesFromDetailView)
	{
		return;
	}
	TGuardValue<bool> UpdateGuard(bUpdatingChangingPropertiesFromDetailView, true);
	GetOnDetailsPropertiesChanged().Broadcast(InEvent.GetPropertyName());
}

void ST4ContiObjectWidget::HandleOnViewModelChanged()
{
	if (bUpdatingChangingPropertiesFromDetailView)
	{
		return;
	}
	UT4ContiAsset* ContiAsset = ContiViewModelPtr->GetContiAsset();
	if (nullptr != ContiAsset)
	{
		// #56, #58 : 디테일 뷰 업데이트는 편집 이슈가 있어 필요할 때만 최소한으로 처리한다.
		const TArray<FT4ContiActionInfo>& SelectedActionInfos = ContiViewModelPtr->GetSelectedActionInfos();
		bool bFullUpdating = (DisplayContiActionInfos.Num() != SelectedActionInfos.Num()) ? true : false;
		if (!bFullUpdating)
		{
			for (int32 Idx = 0; Idx < DisplayContiActionInfos.Num(); ++Idx)
			{
				if (DisplayContiActionInfos[Idx] != SelectedActionInfos[Idx])
				{
					bFullUpdating = true;
					break;
				}
			}
		}
		if (bFullUpdating)
		{
			DetailsViewPtr->SetObject((UObject*)ContiAsset, true);
			DisplayContiActionInfos = SelectedActionInfos;
		}
		else
		{
			if (ContiDetailCustomizationPtr.IsValid())
			{
				ContiDetailCustomizationPtr->RefreshWidgets();
			}
		}
	}
	else
	{
		DetailsViewPtr->SetObject(nullptr);
		DisplayContiActionInfos.Empty(); // #56
	}
}

void ST4ContiObjectWidget::SetDetailCustomization(
	TSharedPtr<FT4ContiDetailCustomization> InContiDetailCustomization
)
{
	ContiDetailCustomizationPtr = InContiDetailCustomization;
}

#undef LOCTEXT_NAMESPACE // "T4ContiObjectsDetails"
