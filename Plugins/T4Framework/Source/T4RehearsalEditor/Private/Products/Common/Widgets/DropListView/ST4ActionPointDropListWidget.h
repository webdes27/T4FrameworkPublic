// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ST4DropListViewWidget.h"

/**
  * #57
 */
class IT4GameObject;
class FT4EditorViewTargetSelector;
class ST4ActionPointDropListWidget : public ST4DropListViewWidget
{
public:
	SLATE_BEGIN_ARGS(ST4ActionPointDropListWidget) {}
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, PropertyHandle) // #88 : DetailView 를 통해 Object Dirty 가 됨으로 사용 유의 (즉, 테스트 했는데 Asset Dirty 가 됨)
		SLATE_EVENT(FT4OnSelected, OnSelected)
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs, 
		TSharedRef<FT4EditorViewTargetSelector> InViewTargetSelector
	);

	ST4ActionPointDropListWidget();
	~ST4ActionPointDropListWidget();

protected:
	void UpdateItemLists() override;

	void HandleOnViewTargetChanged(IT4GameObject* InViewTarget);

private:
	TWeakPtr<FT4EditorViewTargetSelector> ViewTargetSelector;
	FDelegateHandle ViewTargetSelectDelegate;
};
