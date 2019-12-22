// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ST4DropListViewWidget.h"

/**
  * #73
 */
class IT4GameObject;
class FT4EditorViewTargetSelector;
class ST4StanceDropListWidget : public ST4DropListViewWidget
{
public:
	SLATE_BEGIN_ARGS(ST4StanceDropListWidget) {}
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, PropertyHandle) // #88 : DetailView 를 통해 Object Dirty 가 됨으로 사용 유의 (즉, 테스트 했는데 Asset Dirty 가 됨)
		SLATE_EVENT(FT4OnSelected, OnSelected)
	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs);
	void Construct(const FArguments& InArgs, const TSet<FName>& InStanceNamelist);

	ST4StanceDropListWidget();
	~ST4StanceDropListWidget();

protected:
	void UpdateItemLists() override;

private:
	bool bCheckValidStanceList;
	TSet<FName> ValidStanceNamelist;
};
