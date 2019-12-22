// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ST4DropListViewWidget.h"

/**
  * #39
 */
class UT4AnimSetAsset;
class FT4EditorAnimSetAssetSelector;
class ST4BlendSpaceDropListWidget : public ST4DropListViewWidget
{
public:
	SLATE_BEGIN_ARGS(ST4BlendSpaceDropListWidget) {}
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, PropertyHandle) // #88 : DetailView 를 통해 Object Dirty 가 됨으로 사용 유의 (즉, 테스트 했는데 Asset Dirty 가 됨)
		SLATE_EVENT(FT4OnSelected, OnSelected)
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs, 
		TSharedRef<FT4EditorAnimSetAssetSelector> InAnimSetAssetSelector
	);

	ST4BlendSpaceDropListWidget();
	~ST4BlendSpaceDropListWidget();

protected:
	void UpdateItemLists() override;

	void HandleOnAnimSetChanged(UT4AnimSetAsset* InAnimSetAsset);

private:
	TWeakPtr<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector;
	FDelegateHandle AnimSetSelectDelegate;
};
