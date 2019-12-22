// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "Input/Reply.h"

/**
  * #58
 */

class FT4ContiViewModel;
class FT4CameraWorkSectionKeyDetailCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(
		TSharedPtr<FT4ContiViewModel> InContiViewModel
	); // #58
	
	FT4CameraWorkSectionKeyDetailCustomization(TSharedPtr<FT4ContiViewModel> InContiViewModel);

	void CustomizeDetails(IDetailLayoutBuilder& InBuilder);

	void HandleOnRefresh();

private:
	TSharedPtr<FT4ContiViewModel> ViewModelPtr;
	IDetailLayoutBuilder* DetailLayoutPtr;
};
