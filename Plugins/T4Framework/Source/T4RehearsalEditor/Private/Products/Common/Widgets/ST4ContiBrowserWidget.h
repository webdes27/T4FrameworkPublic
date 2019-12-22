// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Layout/Visibility.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "AssetData.h"
#include "Widgets/SToolTip.h"
#include "Widgets/SCompoundWidget.h"
#include "Editor/ContentBrowser/Public/ContentBrowserDelegates.h"
#include "ARFilter.h"

/**
  * #36 : refer SAnimationSequenceBrowser
 */

class FUICommandList;
class SMenuAnchor;
class UT4ContiAsset;
class FFrontendFilter_Folder;
class UT4EntityAsset;
class ST4ContiBrowserWidget : public SCompoundWidget
{
public:
	DECLARE_DELEGATE_OneParam(FT4OnSelectAsset, UObject* /* InAsset */);
	DECLARE_DELEGATE_OneParam(FT4OnDoubleClicked, UObject* /* InAsset */);

public:
	SLATE_BEGIN_ARGS(ST4ContiBrowserWidget) {}
		SLATE_ARGUMENT(FT4OnSelectAsset, OnSelectAsset)
		SLATE_ARGUMENT(FT4OnDoubleClicked, OnDoubleClicked) // #71
		SLATE_ARGUMENT(UT4EntityAsset*, FilterEntityAsset) // #71
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

	virtual ~ST4ContiBrowserWidget();

	void HandleOnSelectAsset(const FAssetData& AssetData);
	void HandleOnDoubleClicked(const FAssetData& AssetData, bool bFromHistory);

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);

	virtual void SelectAsset(UT4ContiAsset* InContiAsset);

	TSharedPtr<SWidget> OnGetAssetContextMenu(const TArray<FAssetData>& SelectedAssets);

	void FindInContentBrowser();

	bool CanFindInContentBrowser() const;

	void HandleOnOpenSelectedAssets(TArray<FAssetData> InObjects) const;
	bool HandleCanOpenSelectedAssets(TArray<FAssetData> InObjects) const;

	void HandleOnPlaySelectedAssets(TArray<FAssetData> InObjects) const; // #71
	bool HandleCanPlaySelectedAssets(TArray<FAssetData> InObjects) const; // #71

	void HandleOnSaveSelectedAssets(TArray<FAssetData> InObjects) const;
	bool HandleCanSaveSelectedAssets(TArray<FAssetData> InObjects) const;

protected:
	bool CanShowColumnForAssetRegistryTag(FName AssetType, FName TagName) const;
	
	void GetSelectedPackages(const TArray<FAssetData>& Assets, TArray<UPackage*>& OutPackages) const;

	bool HandleOnFilterAsset(const FAssetData& InAssetData) const;

protected:
	TSharedPtr<FUICommandList> Commands;

	TSet<FName> AssetRegistryTagsToIgnore;

	FT4OnSelectAsset OnSelectAsset;
	FT4OnDoubleClicked OnDoubleClicked;

	FSyncToAssetsDelegate SyncToAssetsDelegate;
	FGetCurrentSelectionDelegate GetCurrentSelectionDelegate;

	FSetARFilterDelegate SetFilterDelegate;
	FARFilter Filter;
	TArray<TSharedPtr<FFrontendFilter_Folder>> FolderFilters;

	UT4EntityAsset* FilterEntityAsset; // #71

public:
	static const FString SettingsIniSection;
};
