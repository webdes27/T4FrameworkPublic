// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Asset/Public/Action/T4ActionTypes.h" // #81
#include "T4Engine/Public/T4EngineTypes.h"
#include "ST4ListViewWidget.h"

/**
  * #74
 */
struct FT4EntityLayerTagData;
class ST4EntityLayerTagListWidget : public ST4ListViewWidget
{
public:
	DECLARE_DELEGATE_TwoParams(FT4OnSelectedByIndex, ET4LayerTagType, int32); // #74, #81
	DECLARE_DELEGATE_TwoParams(FT4OnDoubleClickedByIndex, ET4LayerTagType, int32); // #81

public:
	SLATE_BEGIN_ARGS(ST4EntityLayerTagListWidget) {}
		SLATE_EVENT(FT4OnSelectedByIndex, OnSelectedByIndex)
		SLATE_EVENT(FT4OnDoubleClickedByIndex, OnDoubleClickedByIndex)
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs, 
		const FT4EntityLayerTagData* InLayerTagData,
		const ET4LayerTagType InLayerTagType
	);

	ST4EntityLayerTagListWidget();
	~ST4EntityLayerTagListWidget();

	void SetInitializeIndex(int32 InIndex) { SelectedIndex = InIndex; } // #104

	int32 GetSelectedIndex() const { return SelectedIndex; }
	void ClearSelectedIndex() { SelectedIndex = -1; }

protected:
	void UpdateLists() override;

	void OnItemSelected(TSharedPtr<FT4ListViewItem> InSelectedItem) override; // #74
	void OnItemDoubleClicked(TSharedPtr<FT4ListViewItem> InSelectedItem) override; // #81

private:
	int32 SelectedIndex;
	FT4OnSelectedByIndex OnSelectedByIndex; // #74, #81
	FT4OnDoubleClickedByIndex OnDoubleClickedByIndex; // #81
	ET4LayerTagType LayerTagType;
	const FT4EntityLayerTagData* LayerTagData;
};
