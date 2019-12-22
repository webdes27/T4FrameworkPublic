// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "Input/Reply.h"

/**
  * #94
 */
class IDetailsView;
class ST4TimeTagListWidget; // #90
class ST4TimeTagDropListWidget; // #90
class IT4RehearsalViewModel;
class UT4EnvironmentAsset;
class FT4EnvironmentDetailCustomization : public TSharedFromThis<FT4EnvironmentDetailCustomization>
{
public:
	FT4EnvironmentDetailCustomization(IT4RehearsalViewModel* InViewModel);
	~FT4EnvironmentDetailCustomization();

	void CustomizeEnvironmentDetails(
		IDetailCategoryBuilder& InDetailCategoryBuilder,
		UT4EnvironmentAsset* InEnvironmentAsset
	); // #94

	void OnApplyEnvironmentAsset(UT4EnvironmentAsset* InEnvironmentAsset, bool bAutoSelect); // #94

	void OnApplyTimeTag(); // #95

private:
	void RefreshWidgets();
	void RefreshWithAutoSelected();

	void HandleOnEnvironmentDetailsPropertiesChanged(); // #95

	void HandleOnTimeTagSelected(const FName InName); // #90
	FReply HandleOnTimeTagAdd(); // #90
	FReply HandleOnTimeTagRemoveSelected(); // #90
	FReply HandleOnTimeTagCopyClipboardSelected(); // #104
	FReply HandleOnTimeTagPasteClipboardSelected(); // #104
	FReply HandleOnTimeTagGetEnvronmentSettings(); // #90

private:
	IT4RehearsalViewModel* ViewModelRef;
	TWeakObjectPtr<UT4EnvironmentAsset> EnvironmentAssetPtr;

	TSharedPtr<ST4TimeTagListWidget> TimeTagListWidgetPtr; // #90
	TSharedPtr<ST4TimeTagDropListWidget> TimeTagDropListWidgetPtr; // #90

	TSharedPtr<IDetailsView> EnvironmentDetailsViewPtr; // #90
};
