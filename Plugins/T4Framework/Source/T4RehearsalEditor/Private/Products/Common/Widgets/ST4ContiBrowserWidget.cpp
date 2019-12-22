// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "ST4ContiBrowserWidget.h"

#include "T4RehearsalEditorModule.h"

#include "T4Asset/Public/T4AssetUtils.h" // #42, #62
#include "T4Asset/Classes/Conti/T4ContiAsset.h"

#include "FrontendFilterBase.h"

#include "Animation/DebugSkelMeshComponent.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "ObjectEditorUtils.h"
#include "EditorStyleSet.h"
#include "FileHelpers.h"

#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "AssetRegistryModule.h"

#include "Toolkits/AssetEditorManager.h"
#include "Toolkits/GlobalEditorCommonCommands.h"
#include "Framework/Application/SlateApplication.h"

#include "T4RehearsalEditorInternal.h"

/**
  * #36 : refer SAnimationSequenceBrowser
 */

#define LOCTEXT_NAMESPACE "T4ContiBrowserWidget"

const FString ST4ContiBrowserWidget::SettingsIniSection = TEXT("T4ContiBrowserWidget");

// #T4_ADD_ENTITY_TAG
class FFrontendFilter_T4ContiAssets : public FFrontendFilter
{
public:
	FFrontendFilter_T4ContiAssets(TSharedPtr<FFrontendFilterCategory> InCategory) : FFrontendFilter(InCategory) {}

	// FFrontendFilter implementation
	virtual FString GetName() const override { return TEXT("ShowContiAssets"); }
	virtual FText GetDisplayName() const override { return LOCTEXT("T4ContiBrowserWidgetFrontendFilter", "Show Conti Assets"); }
	virtual FText GetToolTipText() const override { return LOCTEXT("T4ContiBrowserWidgetFrontendFilter_Tooltip", "Show Conti Assets"); }

	// IFilter implementation
	virtual bool PassesFilter(FAssetFilterType InItem) const override
	{
		return !InItem.GetClass()->IsChildOf(UT4ContiAsset::StaticClass());
	}
};

ST4ContiBrowserWidget::~ST4ContiBrowserWidget()
{
}

void ST4ContiBrowserWidget::HandleOnSelectAsset(const FAssetData& AssetData)
{
	if (UObject* RawAsset = AssetData.GetAsset())
	{
		if (UT4ContiAsset* ContiAsset = Cast<UT4ContiAsset>(RawAsset))
		{
			OnSelectAsset.ExecuteIfBound(ContiAsset);
		}
	}
}

void ST4ContiBrowserWidget::HandleOnDoubleClicked(const FAssetData& AssetData, bool bFromHistory)
{
	if (UObject* RawAsset = AssetData.GetAsset())
	{
		if (UT4ContiAsset* ContiAsset = Cast<UT4ContiAsset>(RawAsset))
		{
			OnDoubleClicked.ExecuteIfBound(ContiAsset);
		}
	}
}

TSharedPtr<SWidget> ST4ContiBrowserWidget::OnGetAssetContextMenu(const TArray<FAssetData>& SelectedAssets)
{
	const bool bMultiSelected = (1 < SelectedAssets.Num()) ? true : false;

	FMenuBuilder MenuBuilder(/*bInShouldCloseWindowAfterMenuSelection=*/ true, Commands);

	MenuBuilder.BeginSection("EntityAssetOptions", LOCTEXT("T4ContiBrowserWidgetAssetContextMenu", "Options") );
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("T4ContiBrowserWidgetOpenSelectedAssets", "Open Contis"),
			LOCTEXT("T4ContiBrowserWidgetOpenSelectedAssets_Tooltip", "Open the selected action contis"),
			FSlateIcon(FEditorStyle::GetStyleSetName(), "Level.SaveIcon16x"),
			FUIAction(
				FExecuteAction::CreateSP(this, &ST4ContiBrowserWidget::HandleOnOpenSelectedAssets, SelectedAssets),
				FCanExecuteAction::CreateSP(this, &ST4ContiBrowserWidget::HandleCanOpenSelectedAssets, SelectedAssets)
			)
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("T4ContiBrowserWidgetPlaySelectedAssets", "Play Contis"),
			LOCTEXT("T4ContiBrowserWidgetPlaySelectedAssets_Tooltip", "Play the selected action contis"),
			FSlateIcon(FEditorStyle::GetStyleSetName(), "Level.SaveIcon16x"),
			FUIAction(
				FExecuteAction::CreateSP(this, &ST4ContiBrowserWidget::HandleOnPlaySelectedAssets, SelectedAssets),
				FCanExecuteAction::CreateSP(this, &ST4ContiBrowserWidget::HandleCanPlaySelectedAssets, SelectedAssets)
			)
		);

		MenuBuilder.AddMenuEntry(FGlobalEditorCommonCommands::Get().FindInContentBrowser);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void ST4ContiBrowserWidget::FindInContentBrowser()
{
	TArray<FAssetData> CurrentSelection = GetCurrentSelectionDelegate.Execute();
	if (CurrentSelection.Num() > 0)
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().SyncBrowserToAssets(CurrentSelection);
	}
}

bool ST4ContiBrowserWidget::CanFindInContentBrowser() const
{
	TArray<FAssetData> CurrentSelection = GetCurrentSelectionDelegate.Execute();
	return CurrentSelection.Num() > 0;
}

void ST4ContiBrowserWidget::GetSelectedPackages(const TArray<FAssetData>& Assets, TArray<UPackage*>& OutPackages) const
{
	for (int32 AssetIdx = 0; AssetIdx < Assets.Num(); ++AssetIdx)
	{
		UPackage* Package = FindPackage(nullptr, *Assets[AssetIdx].PackageName.ToString());

		if ( Package )
		{
			OutPackages.Add(Package);
		}
	}
}

void ST4ContiBrowserWidget::HandleOnOpenSelectedAssets(TArray<FAssetData> InObjects) const
{
	for (FAssetData& AssetData : InObjects)
	{
		if (UObject* RawAsset = AssetData.GetAsset())
		{
			if (UT4ContiAsset* ContiAsset = Cast<UT4ContiAsset>(RawAsset))
			{
				OnDoubleClicked.ExecuteIfBound(ContiAsset);
			}
		}
	}
}

bool ST4ContiBrowserWidget::HandleCanOpenSelectedAssets(TArray<FAssetData> InObjects) const
{
	TArray<UPackage*> Packages;
	GetSelectedPackages(InObjects, Packages);
	// Don't offer save option if none of the packages are loaded
	return Packages.Num() > 0;
}

void ST4ContiBrowserWidget::HandleOnPlaySelectedAssets(TArray<FAssetData> InObjects) const // #71
{
	for (FAssetData& AssetData : InObjects)
	{
		if (UObject* RawAsset = AssetData.GetAsset())
		{
			if (UT4ContiAsset* ContiAsset = Cast<UT4ContiAsset>(RawAsset))
			{
				OnSelectAsset.ExecuteIfBound(ContiAsset);
			}
		}
	}
}

bool ST4ContiBrowserWidget::HandleCanPlaySelectedAssets(TArray<FAssetData> InObjects) const // #71
{
	TArray<UPackage*> Packages;
	GetSelectedPackages(InObjects, Packages);
	// Don't offer save option if none of the packages are loaded
	return Packages.Num() > 0;
}

void ST4ContiBrowserWidget::HandleOnSaveSelectedAssets(TArray<FAssetData> InObjects) const
{
	TArray<UPackage*> PackagesToSave;
	GetSelectedPackages(InObjects, PackagesToSave);

	const bool bCheckDirty = false;
	const bool bPromptToSave = false;
	const FEditorFileUtils::EPromptReturnCode Return = FEditorFileUtils::PromptForCheckoutAndSave(
		PackagesToSave, bCheckDirty, bPromptToSave
	);
}

bool ST4ContiBrowserWidget::HandleCanSaveSelectedAssets(TArray<FAssetData> InObjects) const
{
	TArray<UPackage*> Packages;
	GetSelectedPackages(InObjects, Packages);
	// Don't offer save option if none of the packages are loaded
	return Packages.Num() > 0;
}

bool ST4ContiBrowserWidget::CanShowColumnForAssetRegistryTag(FName AssetType, FName TagName) const
{
	return !AssetRegistryTagsToIgnore.Contains(TagName);
}

void ST4ContiBrowserWidget::Construct(const FArguments& InArgs)
{
	OnSelectAsset = InArgs._OnSelectAsset;
	OnDoubleClicked = InArgs._OnDoubleClicked; // #71
	FilterEntityAsset = InArgs._FilterEntityAsset; // #71

	Commands = MakeShareable(new FUICommandList());
	Commands->MapAction(FGlobalEditorCommonCommands::Get().FindInContentBrowser, FUIAction(
		FExecuteAction::CreateSP(this, &ST4ContiBrowserWidget::FindInContentBrowser),
		FCanExecuteAction::CreateSP(this, &ST4ContiBrowserWidget::CanFindInContentBrowser)
	));

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	// Configure filter for asset picker
	Filter.bRecursiveClasses = true;

	Filter.ClassNames.Add(UT4ContiAsset::StaticClass()->GetFName());

	FAssetPickerConfig Config;
	Config.Filter = Filter;
	Config.SelectionMode = ESelectionMode::Multi;
	Config.InitialAssetViewType = EAssetViewType::Column;
	Config.bAddFilterUI = false; // 필터는 필요없다!
	Config.bShowPathInColumnView = true;
	Config.bSortByPathInColumnView = true;

	// Configure response to click and double-click
	Config.OnAssetSelected = FOnAssetSelected::CreateSP(this, &ST4ContiBrowserWidget::HandleOnSelectAsset);
	Config.OnAssetDoubleClicked = FOnAssetDoubleClicked::CreateSP(this, &ST4ContiBrowserWidget::HandleOnDoubleClicked, false);
	Config.OnGetAssetContextMenu = FOnGetAssetContextMenu::CreateSP(this, &ST4ContiBrowserWidget::OnGetAssetContextMenu);
	Config.OnAssetTagWantsToBeDisplayed = FOnShouldDisplayAssetTag::CreateSP(this, &ST4ContiBrowserWidget::CanShowColumnForAssetRegistryTag);
	Config.SyncToAssetsDelegates.Add(&SyncToAssetsDelegate);
	Config.OnShouldFilterAsset = FOnShouldFilterAsset::CreateSP(this, &ST4ContiBrowserWidget::HandleOnFilterAsset);
	Config.GetCurrentSelectionDelegates.Add(&GetCurrentSelectionDelegate);
	Config.SetFilterDelegates.Add(&SetFilterDelegate);
	Config.bFocusSearchBoxWhenOpened = false;
	//Config.DefaultFilterMenuExpansion = GetT4AssetCategory(); // Conti 는 1개 포멧 뿐이라 제외

	Config.SaveSettingsName = SettingsIniSection;

#if 0
	{
		TSharedPtr<FFrontendFilterCategory> ContiCategory = MakeShareable(
			new FFrontendFilterCategory(
				LOCTEXT("ExtraContiFilters", "Conti Filters"), 
				LOCTEXT("ExtraContiFiltersTooltip", "Filter assets by all filters in this category.")
			)
		);
		Config.ExtraFrontendFilters.Add(MakeShareable(new FFrontendFilter_T4ContiAssets(ContiCategory)));
	}
#endif
	
	Config.OnIsAssetValidForCustomToolTip = FOnIsAssetValidForCustomToolTip::CreateLambda(
		[](const FAssetData& AssetData) 
		{
			return AssetData.IsAssetLoaded(); 
		}
	);

	// hide all asset registry columns by default (we only really want the name and path)
	TArray<UObject::FAssetRegistryTag> AssetRegistryTags;
	{
		UT4ContiAsset::StaticClass()->GetDefaultObject()->GetAssetRegistryTags(AssetRegistryTags);
		for(UObject::FAssetRegistryTag& AssetRegistryTag : AssetRegistryTags)
		{
			Config.HiddenColumnNames.Add(AssetRegistryTag.Name.ToString());
		}
	}

	// Also hide the type column by default (but allow users to enable it, so don't use bShowTypeInColumnView)
	Config.HiddenColumnNames.Add(TEXT("Class"));

	this->ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				ContentBrowserModule.Get().CreateAssetPicker(Config)
			]
		]
	];
}

FReply ST4ContiBrowserWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (Commands->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void ST4ContiBrowserWidget::SelectAsset(UT4ContiAsset* InContiAsset)
{
	FAssetData AssetData(InContiAsset);

	if (AssetData.IsValid())
	{
		TArray<FAssetData> CurrentSelection = GetCurrentSelectionDelegate.Execute();

		if (!CurrentSelection.Contains(AssetData))
		{
			TArray<FAssetData> AssetsToSelect;
			AssetsToSelect.Add(AssetData);

			SyncToAssetsDelegate.Execute(AssetsToSelect);
		}
	}
}

bool ST4ContiBrowserWidget::HandleOnFilterAsset(const FAssetData& InAssetData) const
{
	if (InAssetData.GetClass()->IsChildOf(UT4ContiAsset::StaticClass()))
	{
		if (nullptr != FilterEntityAsset) // #71
		{
			// Conti 에서 PreviewEntityAsset 과 일치하는 Conti List 만 출력
			FString TestEntityString = FAssetData(FilterEntityAsset).ToSoftObjectPath().ToString();
			return (InAssetData.TagsAndValues.FindRef(TEXT("PreviewEntityAsset")) != TestEntityString);
		}
	}

	return false;
}

#undef LOCTEXT_NAMESPACE
