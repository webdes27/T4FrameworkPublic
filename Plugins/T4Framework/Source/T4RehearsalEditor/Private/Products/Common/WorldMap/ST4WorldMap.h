// Copyright 2019 SoonBo Noh. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "Layout/Visibility.h"
#include "Input/Reply.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"
#include "Widgets/SCompoundWidget.h"
#include "Framework/Application/IMenu.h"

#include "T4WorldTileCollectionModel.h"

class SBorder;
class SButton;
class ST4WorldMapGrid;
class SWrapBox;
class FT4WorldMapViewModel;
class ST4WorldMap 
	: public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(ST4WorldMap) {}
	SLATE_END_ARGS();

	ST4WorldMap();
	~ST4WorldMap();

	void Construct(const FArguments& InArgs);

	void OnInitialize(FT4WorldMapViewModel* InWorldViewModel); // #90

	void OnRefresh(); // #90
	void OnRefreshSelection(); // #86

	void OnUpdateThumbnails(); // #93

	void OnRequestScrollTo(const FVector& InLocation, const FBox& InArea); // #90

	void OnSubLevelSelected(const TArray<FName>& InSubLevelNames); // #104

	void OnEditorSubLevelLoad(const TArray<FName>& InSubLevelNames); // #104
	void OnPreviewSubLevelLoad(const TArray<FName>& InSubLevelNames); // #104
	void OnPreviewSubLevelUnload(const TArray<FName>& InSubLevelNames); // #104

	void OnActorSelected(const FVector& InLocation, const FBox& InBoundingBox); // #104
	void OnMapZoneSelected(FName InMapZoneName); // #92

	FVector2D GetMouseDownLocationInWorldSpace() const; // #90

private:
	/**  */
	void BrowseWorld();

	/**  */
	TSharedRef<SWidget> ConstructContentWidget();

	/** Populate current FWorldTileLayer list to UI */
	void PopulateLayersList();

	/** Creates a popup window with New layer parameters */
	FReply NewLayer_Clicked();
	
	/** Creates a new managed layer */
	FReply CreateNewLayer(const FWorldTileLayer& NewLayer);

	/** Top status bar details */
	FText GetZoomText() const;
	FText GetCurrentOriginText() const;
	FText GetCurrentLevelText() const;

	/** Bottom status bar details */
	FText GetMouseLocationText() const;
	FText GetMarqueeSelectionSizeText() const;
	FText GetWorldSizeText() const;

	/** @return whether SIMULATION sign should be visible */
	EVisibility IsSimulationVisible() const;
	
	void HandleOnSelectLayerClicked(); // #86
	void HandleOnEditLayerClicked(
		const FWorldTileLayer& InOldWorldTileLayer,
		const FWorldTileLayer& InNewWorldTileLayer
	); // #86

private:
	void Reset(); // #83

private:
	FT4WorldMapViewModel*						WorldMapViewModelRef; // #83

	TSharedPtr<FT4WorldTileCollectionModel>		TileWorldModel;
	
	TSharedPtr<SBorder>							ContentParent;
	TSharedPtr<SWrapBox>						LayersListWrapBox;
	TSharedPtr<SButton>							NewLayerButton;
	TWeakPtr<class IMenu>						NewLayerMenu;
	TSharedPtr<class ST4WorldMapGrid>			GridView;
};
