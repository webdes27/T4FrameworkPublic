// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Products/ContiEditor/ViewModel/T4ContiViewModel.h"

#include "Widgets/SCompoundWidget.h"
#include "Delegates/Delegate.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "PropertyEditorDelegates.h"

/**
  *
 */
class IDetailsView;
class FT4ContiViewModel;
class FT4ContiDetailCustomization;
class ST4ContiObjectWidget : public SCompoundWidget
{
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FT4OnDetailsPropertiesChanged, const FName&); // #56 : Property 변경을 노티를 보낸다.

public:
	SLATE_BEGIN_ARGS(ST4ContiObjectWidget) {}
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs, 
		TSharedPtr<FT4ContiViewModel> InContiViewModel
	);

	TSharedPtr<FT4ContiViewModel> GetContiViewModel() { return ContiViewModelPtr; }

	FT4OnDetailsPropertiesChanged& GetOnDetailsPropertiesChanged() { return OnDetailsPropertiesChanged; }

	void SetDetailCustomization(TSharedPtr<FT4ContiDetailCustomization> InContiDetailCustomization); // #58

private:
	void HandleOnDetailsPropertiesChanged(const FPropertyChangedEvent& InEvent); // #56 : Property 변경을 노티 받는다.
	void HandleOnViewModelChanged(); // #56 : 뷰모델의 변경사항을 노티 받는다.

private:
	TSharedPtr<IDetailsView> DetailsViewPtr;
	TSharedPtr<FT4ContiViewModel> ContiViewModelPtr;
	TSharedPtr<FT4ContiDetailCustomization> ContiDetailCustomizationPtr;

	TArray<FT4ContiActionInfo> DisplayContiActionInfos; // #56 : 동일한지 체크 후 변경이 있을 때만 업데이트

	FT4OnDetailsPropertiesChanged OnDetailsPropertiesChanged; // #56

	bool bUpdatingChangingPropertiesFromDetailView; // #54 : 이벤트에 의한 루핑 방지
};
