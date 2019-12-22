// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Widgets/SCompoundWidget.h"
#include "Delegates/Delegate.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "PropertyEditorDelegates.h"

/**
  * #85
 */
class IDetailsView;
class FT4WorldViewModel;
class FT4WorldDetailCustomization;
class ST4WorldObjectWidget : public SCompoundWidget
{
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FT4OnDetailsPropertiesChanged, const FName&); // #77 : Property 변경을 노티를 보낸다.

public:
	SLATE_BEGIN_ARGS(ST4WorldObjectWidget) {}
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs, 
		TSharedPtr<FT4WorldViewModel> InWorldViewModel
	);

	TSharedPtr<FT4WorldViewModel> GetWorldViewModel() { return WorldViewModelPtr; }

	FT4OnDetailsPropertiesChanged& GetOnDetailsPropertiesChanged() { return OnDetailsPropertiesChanged; }

	void SetDetailCustomization(TSharedPtr<FT4WorldDetailCustomization> InWorldDetailCustomization);

	void OnRefreshWorld(); // #104

private:
	void HandleOnDetailsPropertiesChanged(const FPropertyChangedEvent& InEvent); // #77 : Property 변경을 노티 받는다.
	void HandleOnWorldEnvironmentPropertyChanged(); // #90

private:
	TSharedPtr<IDetailsView> DetailsViewPtr;
	TSharedPtr<FT4WorldViewModel> WorldViewModelPtr;
	TSharedPtr<FT4WorldDetailCustomization> WorldDetailCustomizationPtr;

	FT4OnDetailsPropertiesChanged OnDetailsPropertiesChanged; // #77

	bool bUpdatingChangingPropertiesFromDetailView; // #77 : 이벤트에 의한 루핑 방지
};
