// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "IDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Views/SListView.h" // #83
#include "Input/Reply.h"

/**
  * http://api.unrealengine.com/KOR/Programming/Slate/DetailsCustomization/
 */
class IDetailsView;
class UT4WorldAsset;
class ST4TimeTagListWidget; // #90
class ST4TimeTagDropListWidget; // #90
class ST4MapZoneListWidget; // #92
class ST4MapZoneDropListWidget; // #92
class ST4WorldObjectWidget;
class ST4PointOfInterestListWidget; // #100, #103
class ST4SubLevelListWidget; // #104
class FT4WorldViewModel;
class FT4EnvironmentDetailCustomization; // #94
class FT4WorldDetailCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(
		TSharedRef<ST4WorldObjectWidget> InWorldObjectWidget // #85
	);
	
	FT4WorldDetailCustomization(TSharedPtr<FT4WorldViewModel> InWorldViewModel);

	void CustomizeDetails(IDetailLayoutBuilder& InBuilder);

	void OnRefreshWorld(); // #104

private:
	void CustomizeWorldMapZoneDetails(IDetailLayoutBuilder& InBuilder, UT4WorldAsset* InWorldAsset); // #90
	void CustomizeWorldMapZoneEnvironmentDetails(IDetailLayoutBuilder& InBuilder, UT4WorldAsset* InWorldAsset); // #90

	void HandleOnLevelMultiSelected(const TArray<FName>& InSubLevelNames); // #104
	FReply HandleOnLoadSubLevelInPreviewWorld(); // #104
	FReply HandleOnLoadSubLevelInEditorWorld(); // #104

	void HandleOnAutomationPointOfInterestSelected(int32 InSelectedIndex); // #100, #103
	void HandleOnAutomationPointOfInterestGo(int32 InSelectedIndex); // #100, #103
	FReply HandleOnAutomationPointOfInterestUpdate(); // #100, #103
	FReply HandleOnAutomationPointOfInterestAdd(); // #100, #103
	FReply HandleOnAutomationPointOfInterestRemove(); // #100, #103

	FReply HandleOnRefreshMapZoneLists(); // #92
	FReply HandleOnAddNewMapZoneVolume(); // #92

	void HandleOnMapEntityAssetChanged();

	void HandleOnMapZoneSelected(const FName InName); // #92

	void HandleOnMapZoneDetailChanged(); // #90, #92

	FReply HandleOnEnvironmentAssetSave(); // #90

private:
	TSharedPtr<FT4WorldViewModel> ViewModelPtr;
	IDetailLayoutBuilder* DetailLayoutPtr;

	TSharedPtr<ST4SubLevelListWidget> LoadedSubLevelListWidgetPtr; // #104

	TSharedPtr<ST4MapZoneListWidget> MapZoneListWidgetPtr; // #92
	TSharedPtr<ST4MapZoneDropListWidget> MapZoneDropListWidgetPtr; // #92

	TSharedPtr<ST4PointOfInterestListWidget> PointOfInterestListWidgetPtr; // #100, #103

	TSharedPtr<FT4EnvironmentDetailCustomization> EnvironmentDetailPtr; // #94

	TSharedPtr<IPropertyHandle> EditorTransientDataHandle; // #92
};
