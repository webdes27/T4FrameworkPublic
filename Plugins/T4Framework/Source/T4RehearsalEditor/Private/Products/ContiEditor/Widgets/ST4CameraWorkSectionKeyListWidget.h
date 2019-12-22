// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/T4EngineTypes.h"
#include "Products/Common/Widgets/ListView/ST4ListViewWidget.h"

/**
  * #58
 */
struct FT4CameraWorkSectionData;
class ST4CameraWorkSectionKeyListWidget : public ST4ListViewWidget
{
public:
	DECLARE_DELEGATE_OneParam(FT4OnSelectedByIndex, int32);
	DECLARE_DELEGATE_OneParam(FT4OnDoubleClickedByIndex, int32);
	DECLARE_DELEGATE_RetVal(int32, FT4OnGetValueIndexSelected);

public:
	SLATE_BEGIN_ARGS(ST4CameraWorkSectionKeyListWidget) {}
		SLATE_EVENT(FT4OnSelectedByIndex, OnSelectedByIndex)
		SLATE_EVENT(FT4OnDoubleClickedByIndex, OnDoubleClickedByIndex)
		SLATE_EVENT(FT4OnGetValueIndexSelected, OnGetValueIndexSelcted)
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs, 
		const FT4CameraWorkSectionData* InCameraWorkSectionData
	);

	ST4CameraWorkSectionKeyListWidget();
	~ST4CameraWorkSectionKeyListWidget();

protected:
	void UpdateLists() override;

	void OnItemSelected(TSharedPtr<FT4ListViewItem> InSelectedItem) override; // #74
	void OnItemDoubleClicked(TSharedPtr<FT4ListViewItem> InSelectedItem) override; // #81

private:
	int32 SelectedChannelKey;
	FT4OnSelectedByIndex OnSelectedByIndex; // #74, #81
	FT4OnDoubleClickedByIndex OnDoubleClickedByIndex; // #81
	FT4OnGetValueIndexSelected OnGetValueIndexSelcted; // #58
	const FT4CameraWorkSectionData* CameraWorkSectionDataRef;
};
