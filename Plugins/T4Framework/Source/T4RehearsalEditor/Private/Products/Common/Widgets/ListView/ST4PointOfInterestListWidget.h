// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/T4EngineTypes.h"
#include "ST4ListViewWidget.h"

/**
  * #100
 */
struct FT4EditorTestAutomation;
class ST4PointOfInterestListWidget : public ST4ListViewWidget
{
public:
	DECLARE_DELEGATE_OneParam(FT4OnSelectedByIndex, int32);
	DECLARE_DELEGATE_OneParam(FT4OnDoubleClickedByIndex, int32);

public:
	SLATE_BEGIN_ARGS(ST4PointOfInterestListWidget) {}
		SLATE_EVENT(FT4OnSelectedByIndex, OnSelectedByIndex)
		SLATE_EVENT(FT4OnDoubleClickedByIndex, OnDoubleClickedByIndex)
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs, 
		const FT4EditorTestAutomation* InTestAutomation
	);

	ST4PointOfInterestListWidget();
	~ST4PointOfInterestListWidget();

	void SetInitializeIndex(int32 InIndex) { SelectedIndex = InIndex; } // #103

protected:
	void UpdateLists() override;

	void OnItemSelected(TSharedPtr<FT4ListViewItem> InSelectedItem) override; // #74
	void OnItemDoubleClicked(TSharedPtr<FT4ListViewItem> InSelectedItem) override; // #81

private:
	int32 SelectedIndex;
	FT4OnSelectedByIndex OnSelectedByIndex; // #74, #81
	FT4OnDoubleClickedByIndex OnDoubleClickedByIndex; // #81
	const FT4EditorTestAutomation* TestAutomatioRef;
};
