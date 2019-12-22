// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/T4EngineTypes.h"
#include "Products/Common/Widgets/ListView/ST4ListViewWidget.h"

/**
  * #104
 */
enum ET4SubLevelListType
{
	SubLevelList_Tile,
	SubLevelList_Loaded
};

class UWorld;
class ST4SubLevelListWidget : public ST4ListViewWidget
{
public:
	SLATE_BEGIN_ARGS(ST4SubLevelListWidget) {}
		SLATE_EVENT(FT4OnMultiSelected, OnMultiSelected)
		SLATE_EVENT(FT4OnDoubleClicked, OnDoubleClicked)
	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs, UWorld* InWorld, ET4SubLevelListType InSubLevelListType);

	ST4SubLevelListWidget();
	~ST4SubLevelListWidget();

	void RefreshWorld(UWorld* InWorld) { WorldRef = InWorld; } // #104

	void SetLoadedLevels(const TSet<FName>& InLoadedLevels); // #104

protected:
	void UpdateLists() override;

private:
	UWorld* WorldRef;
	ET4SubLevelListType SubLevelListType;

	TSet<FName> LoadedLevels; // #104 : SubLevelList_Tile
};
