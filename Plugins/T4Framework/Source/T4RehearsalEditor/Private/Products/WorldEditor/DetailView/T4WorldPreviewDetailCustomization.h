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
class ULevelStreaming;
class ST4WorldPreviewObjectWidget;
class FT4WorldViewModel;
class ST4SubLevelListWidget; // #104
class ST4LevelActorListWidget; // #104
class FT4WorldPreviewDetailCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance(
		TSharedRef<ST4WorldPreviewObjectWidget> InWorldLevelObjectWidget // #85
	);
	
	FT4WorldPreviewDetailCustomization(TSharedPtr<FT4WorldViewModel> InWorldViewModel);

	void CustomizeDetails(IDetailLayoutBuilder& InBuilder);

	void OnRefreshWorld(); // #85

private:
	void UpdateLevelDetailObject(const ULevelStreaming* InLevelStreaming); // #85

	void HandleOnRefreshLayout();

	FReply HandleOnUnloadSelelectedLevel(); // #104

	void HandleOnLoadedLevelMultiSelected(const TArray<FName>& InSubLevelNames);

	void HandleOnLevelActorSelected(const FName InName); // #85
	void HandleOnLevelActorDoubleClicked(const FName InName); // #85

private:
	TSharedPtr<FT4WorldViewModel> ViewModelPtr;
	IDetailLayoutBuilder* DetailLayoutRef;

	TSharedPtr<ST4SubLevelListWidget> LoadedSubLevelListWidgetPtr; // #104
	TSharedPtr<ST4LevelActorListWidget> LevelActorListWidgetPtr; // #104

	TSharedPtr<IDetailsView> LevelActorDetailsViewPtr; // #85
};
