// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ST4DropListViewWidget.h"

/**
  * #73
 */
class IT4GameObject;
class FT4EditorViewTargetSelector;
class ST4ReactionDropListWidget : public ST4DropListViewWidget
{
public:
	SLATE_BEGIN_ARGS(ST4ReactionDropListWidget) {}
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, PropertyHandle) // #88 : DetailView 를 통해 Object Dirty 가 됨으로 사용 유의 (즉, 테스트 했는데 Asset Dirty 가 됨)
		SLATE_EVENT(FT4OnSelected, OnSelected)
	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs, TSharedRef<FT4EditorViewTargetSelector> InViewTargetSelector);
	void Construct(
		const FArguments& InArgs, 
		const TSet<FName>& InReactionNamelist,
		TSharedRef<FT4EditorViewTargetSelector> InViewTargetSelector
	);

	ST4ReactionDropListWidget();
	~ST4ReactionDropListWidget();

protected:
	void UpdateItemLists() override;

	void HandleOnViewTargetChanged(IT4GameObject* InViewTarget);

private:
	bool bCheckValidReactionList;
	TSet<FName> ValidReactionNamelist;
	TWeakPtr<FT4EditorViewTargetSelector> ViewTargetSelector;
};
