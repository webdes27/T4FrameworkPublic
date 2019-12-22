// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Widgets/SCompoundWidget.h"
#include "Delegates/Delegate.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "PropertyEditorDelegates.h"

/**
  * #39
 */
class IDetailsView;
class FT4EntityViewModel;
class ST4AnimSetObjectsWidget : public SCompoundWidget
{
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FT4OnDetailsPropertiesChanged, const FName&); // #77 : Property 변경을 노티를 보낸다.

public:
	SLATE_BEGIN_ARGS(ST4AnimSetObjectsWidget) {}
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs, 
		TSharedPtr<FT4EntityViewModel> InEntityViewModel
	);

	FT4OnDetailsPropertiesChanged& GetOnDetailsPropertiesChanged() { return OnDetailsPropertiesChanged; }

private:
	void HandleOnUpdateDetails();
	void HandleOnDetailsPropertiesChanged(const FPropertyChangedEvent& InEvent); // #77

private:
	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<FT4EntityViewModel> EntityViewModel;

	FT4OnDetailsPropertiesChanged OnDetailsPropertiesChanged; // #77

	bool bUpdatingChangingPropertiesFromDetailView; // #77 : 이벤트에 의한 루핑 방지
};
