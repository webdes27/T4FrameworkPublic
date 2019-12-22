// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/T4EngineConstants.h" // #39
#include "ST4DropListViewWidget.h"

/**
  * #39 
 */
class UT4AnimSetAsset;
class FT4EditorAnimSetAssetSelector;
class ST4AnimSectionDropListWidget : public ST4DropListViewWidget
{
public:
	SLATE_BEGIN_ARGS(ST4AnimSectionDropListWidget) {}
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, PropertyHandle) // #88 : DetailView 를 통해 Object Dirty 가 됨으로 사용 유의 (즉, 테스트 했는데 Asset Dirty 가 됨)
		SLATE_EVENT(FT4OnSelected, OnSelected)
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs, 
		TSharedRef<FT4EditorAnimSetAssetSelector> InAnimSetAssetSelector,
		ET4EngineConstantType InConstantTypeType
	);

	ST4AnimSectionDropListWidget();
	~ST4AnimSectionDropListWidget();

protected:
	void UpdateItemLists() override;

	void HandleOnAnimSetChanged(UT4AnimSetAsset* InAnimSetAsset);

private:
	ET4EngineConstantType NameTableType;
	TWeakPtr<FT4EditorAnimSetAssetSelector> AnimSetAssetSelector;
	FDelegateHandle AnimSetSelectDelegate;
};
