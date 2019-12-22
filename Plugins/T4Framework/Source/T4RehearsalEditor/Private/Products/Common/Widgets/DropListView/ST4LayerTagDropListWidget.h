// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Asset/Public/Action/T4ActionTypes.h" // #81
#include "ST4DropListViewWidget.h"

/**
  * #74
 */
class IT4GameObject;
class FT4EditorViewTargetSelector;
class ST4LayerTagDropListWidget : public ST4DropListViewWidget
{
public:
	SLATE_BEGIN_ARGS(ST4LayerTagDropListWidget) {}
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, PropertyHandle) // #88 : DetailView 를 통해 Object Dirty 가 됨으로 사용 유의 (즉, 테스트 했는데 Asset Dirty 가 됨)
		SLATE_EVENT(FT4OnSelected, OnSelected)
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs,
		const ET4LayerTagType InLayerTagType, // #81
		TSharedRef<FT4EditorViewTargetSelector> InViewTargetSelector
	);

	ST4LayerTagDropListWidget();
	~ST4LayerTagDropListWidget();

protected:
	void UpdateItemLists() override;

	void HandleOnViewTargetChanged(IT4GameObject* InViewTarget);

private:
	ET4LayerTagType LayerTagType; // #81
	TWeakPtr<FT4EditorViewTargetSelector> ViewTargetSelector;
	FDelegateHandle ViewTargetSelectDelegate;
};
