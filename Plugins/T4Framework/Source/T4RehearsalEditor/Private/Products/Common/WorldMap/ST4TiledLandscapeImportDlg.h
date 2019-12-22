// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4LevelCollectionModel.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Input/SComboBox.h"

struct FAssetData;

/////////////////////////////////////////////////////
// ST4TiledLandcapeImportDlg
// 
class ST4TiledLandcapeImportDlg
	: public SCompoundWidget
{
public:
	/** */
	struct FTileImportConfiguration
	{
		int32 SizeX;
		int32 NumComponents;
		int32 NumSectionsPerComponent;
		int32 NumQuadsPerSection;
	};

	SLATE_BEGIN_ARGS( ST4TiledLandcapeImportDlg )
		{}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<SWindow> InParentWindow);

	/** */
	bool ShouldImport() const;	
	const FT4TiledLandscapeImportSettings& GetImportSettings() const;

private:
	/** */
	TSharedRef<SWidget> HandleTileConfigurationComboBoxGenarateWidget(TSharedPtr<FTileImportConfiguration> InItem) const;
	FText				GetTileConfigurationText() const;

	/** */
	TSharedRef<ITableRow> OnGenerateWidgetForLayerDataListView(TSharedPtr<FT4TiledLandscapeImportSettings::LandscapeLayerSettings> InLayerData, const TSharedRef<STableViewBase>& OwnerTable);

	/** */
	TOptional<float> GetScaleX() const;
	TOptional<float> GetScaleY() const;
	TOptional<float> GetScaleZ() const;
	void OnSetScale(float InValue, ETextCommit::Type CommitType, int32 InAxis);

	/** */
	TOptional<int32> GetTileOffsetX() const;
	TOptional<int32> GetTileOffsetY() const;
	void SetTileOffsetX(int32 InValue);
	void SetTileOffsetY(int32 InValue);

	/** */
	ECheckBoxState GetFlipYAxisState() const;
	void OnFlipYAxisStateChanged(ECheckBoxState NewState);

	/** */
	void OnSetImportConfiguration(TSharedPtr<FTileImportConfiguration> InTileConfig, ESelectInfo::Type SelectInfo);

	/** */
	bool IsImportEnabled() const;

	/** */
	FReply OnClickedSelectHeightmapTiles();
	FReply OnClickedSelectWeightmapTiles(TSharedPtr<FT4TiledLandscapeImportSettings::LandscapeLayerSettings> InLayerData);
	FReply OnClickedImport();
	FReply OnClickedCancel();

	/** */
	FString GetLandscapeMaterialPath() const;
	void OnLandscapeMaterialChanged(const FAssetData& AssetData);

	/** */
	FText GetImportSummaryText() const;
	FText GetWeightmapCountText(TSharedPtr<FT4TiledLandscapeImportSettings::LandscapeLayerSettings> InLayerData) const; 

	/** */
	ECheckBoxState GetLayerBlendState(TSharedPtr<FT4TiledLandscapeImportSettings::LandscapeLayerSettings> InLayerData) const;
	void OnLayerBlendStateChanged(ECheckBoxState NewState, TSharedPtr<FT4TiledLandscapeImportSettings::LandscapeLayerSettings> InLayerData);

	/** */
	int32 SetPossibleConfigurationsForFileWidth(int64 TargetFileWidth);

	/** */
	void GenerateAllPossibleTileConfigurations();

	/** */
	FText GenerateConfigurationText(int32 NumComponents, int32 NumSectionsPerComponent,	int32 NumQuadsPerSection) const;

	/** */
	void UpdateLandscapeLayerList();

private:
	/** */
	bool bShouldImport;

	/** */
	mutable FText StatusMessage;

	/** */
	TSharedPtr<SWindow> ParentWindow;

	/** */
	TSharedPtr<SComboBox<TSharedPtr<FTileImportConfiguration>>> TileConfigurationComboBox;

	TSharedPtr<SListView<TSharedPtr<FT4TiledLandscapeImportSettings::LandscapeLayerSettings>>>	LayerDataListView;
	TArray<TSharedPtr<FT4TiledLandscapeImportSettings::LandscapeLayerSettings>>					LayerDataList;

	/** */
	FT4TiledLandscapeImportSettings ImportSettings;

	/** */
	FIntRect TotalLandscapeRect;

	TArray<FTileImportConfiguration> AllConfigurations;
	TArray<TSharedPtr<FTileImportConfiguration>> ActiveConfigurations;
};

