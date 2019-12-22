// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/WorldCompositionUtility.h"
#include "Input/Reply.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SWidget.h"
#include "Widgets/SCompoundWidget.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Layout/SBorder.h"

class FT4WorldTileCollectionModel;

//----------------------------------------------------------------
//
//
//----------------------------------------------------------------
class ST4WorldLayerButton 
	: public SCompoundWidget
{
public:
	DECLARE_DELEGATE(FOnSelectLayer) // #86
	DECLARE_DELEGATE_TwoParams(FOnEditLayer, const FWorldTileLayer&, const FWorldTileLayer&) // #86

	SLATE_BEGIN_ARGS(ST4WorldLayerButton)
	{}
		/** Data for the asset this item represents */
		SLATE_ARGUMENT(FWorldTileLayer, WorldLayer)
		SLATE_ARGUMENT(TSharedPtr<FT4WorldTileCollectionModel>, InWorldModel)
		SLATE_EVENT(FOnSelectLayer, OnSelectLayer) // #86
		SLATE_EVENT(FOnEditLayer, OnEditLayer) // #86
	SLATE_END_ARGS()

	~ST4WorldLayerButton();
	void Construct(const FArguments& InArgs);
	void OnCheckStateChanged(ECheckBoxState NewState);
	ECheckBoxState IsChecked() const;
	FReply OnDoubleClicked();
	FReply OnCtrlClicked();
	TSharedRef<SWidget> GetRightClickMenu();
	FText GetToolTipText() const;
			
	FReply HandleOnEditLayerClicked(const FWorldTileLayer& InWorldTileLayer); // #86

private:
	/** The data for this item */
	TSharedPtr<FT4WorldTileCollectionModel>	WorldModel;
	FWorldTileLayer							WorldLayer;
	FOnSelectLayer							OnSelectLayer; // #86
	FOnEditLayer							OnEditLayer; // #86
};


class ST4NewWorldLayerPopup 
	: public SBorder
{
public:
	DECLARE_DELEGATE_RetVal_OneParam(FReply, FOnCreateLayer, const FWorldTileLayer&)
	
	SLATE_BEGIN_ARGS(ST4NewWorldLayerPopup)
	{}
	SLATE_EVENT(FOnCreateLayer, OnCreateLayer)
	SLATE_ARGUMENT(FString, DefaultName)
	SLATE_ARGUMENT(TSharedPtr<FT4WorldTileCollectionModel>, InWorldModel)
	SLATE_END_ARGS()
		
	void Construct(const FArguments& InArgs);

	TOptional<int32> GetStreamingDistance() const
	{
		return LayerData.StreamingDistance;
	}
	
	ECheckBoxState GetDistanceStreamingState() const
	{
		return IsDistanceStreamingEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	bool IsDistanceStreamingEnabled() const
	{
		return LayerData.DistanceStreamingEnabled;
	}
	
	void OnDistanceStreamingStateChanged(ECheckBoxState NewState)
	{
		SetDistanceStreamingState(NewState == ECheckBoxState::Checked);
	}

	FText GetLayerName() const
	{
		return FText::FromString(LayerData.Name);
	}
	
private:
	FReply OnClickedCreate();
	bool CanCreateLayer() const;

	void SetLayerName(const FText& InText)
	{
		LayerData.Name = InText.ToString();
	}

	void SetStreamingDistance(int32 InValue)
	{
		LayerData.StreamingDistance = InValue;
	}

	void SetDistanceStreamingState(bool bIsEnabled)
	{
		LayerData.DistanceStreamingEnabled = bIsEnabled;
	}

private:
	/** The delegate to execute when the create button is clicked */
	FOnCreateLayer							OnCreateLayer;
	FWorldTileLayer							LayerData;
	TSet<FString>							ExistingLayerNames;
};

// #86
class ST4EditWorldLayerPopup
	: public SBorder
{
public:
	DECLARE_DELEGATE_RetVal_OneParam(FReply, FOnEditLayer, const FWorldTileLayer&)

	SLATE_BEGIN_ARGS(ST4EditWorldLayerPopup)
	{}
		SLATE_EVENT(FOnEditLayer, OnEditLayer)
		SLATE_ARGUMENT(FWorldTileLayer, TileLayerData)
		SLATE_ARGUMENT(TSharedPtr<FT4WorldTileCollectionModel>, InWorldModel)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	TOptional<int32> GetStreamingDistance() const
	{
		return LayerData.StreamingDistance;
	}

	ECheckBoxState GetDistanceStreamingState() const
	{
		return IsDistanceStreamingEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	bool IsDistanceStreamingEnabled() const
	{
		return LayerData.DistanceStreamingEnabled;
	}

	void OnDistanceStreamingStateChanged(ECheckBoxState NewState)
	{
		SetDistanceStreamingState(NewState == ECheckBoxState::Checked);
	}

	FText GetLayerName() const
	{
		return FText::FromString(LayerData.Name);
	}

private:
	FReply OnClickedUpdate();
	bool CanUpdateLayer() const;

	void SetLayerName(const FText& InText)
	{
		LayerData.Name = InText.ToString();
	}

	void SetStreamingDistance(int32 InValue)
	{
		LayerData.StreamingDistance = InValue;
	}

	void SetDistanceStreamingState(bool bIsEnabled)
	{
		LayerData.DistanceStreamingEnabled = bIsEnabled;
	}

private:
	/** The delegate to execute when the create button is clicked */
	TSharedPtr<FT4WorldTileCollectionModel>	WorldModel; // #86
	FOnEditLayer							OnEditLayer;
	FWorldTileLayer							LayerData;
	TArray<FWorldTileLayer>					ExistingLayers;
};
