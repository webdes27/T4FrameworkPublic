// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/T4EngineTypes.h"
#include "ST4ListViewWidget.h"

/**
  * #71
 */
class UT4AnimSetAsset;
class FT4EditorAnimSetAssetSelector;
class ST4AnimationLayerListWidget : public ST4ListViewWidget
{
public:
	SLATE_BEGIN_ARGS(ST4AnimationLayerListWidget) {}
		SLATE_EVENT(FT4OnSelected, OnSelected)
		SLATE_EVENT(FT4OnDoubleClicked, OnDoubleClicked)
		SLATE_EVENT(FT4OnMoveUpItem, OnMoveUpItem)
		SLATE_EVENT(FT4OnMoveDownItem, OnMoveDownItem)
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs, 
		TSharedRef<FT4EditorAnimSetAssetSelector> InAnimSetAssetSelector,
		ET4AnimLayer InAnimationLayer
	);

	ST4AnimationLayerListWidget();
	~ST4AnimationLayerListWidget();

protected:
	void UpdateLists() override;

	void HandleOnAnimSetChanged(UT4AnimSetAsset* InAnimSetAsset);

private:
	ET4AnimLayer AnimationLayer;
	TWeakPtr<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector;
	FDelegateHandle AnimSetSelectDelegate;
};
