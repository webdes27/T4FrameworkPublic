// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Widgets/SCompoundWidget.h"
#include "Delegates/Delegate.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "PropertyEditorDelegates.h"

/**
  * #60
 */
class IDetailsView;
class FT4ContiViewModel;
class ST4EditorPlaySettingWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(ST4EditorPlaySettingWidget) {}
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs, 
		TSharedPtr<FT4ContiViewModel> InContiViewModel
	);

protected:
	void HandleOnDetailsPropertiesChanged(const FPropertyChangedEvent& InEvent); // #56 : Property 변경을 노티 받는다.

private:
	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<FT4ContiViewModel> ContiViewModel;
};
