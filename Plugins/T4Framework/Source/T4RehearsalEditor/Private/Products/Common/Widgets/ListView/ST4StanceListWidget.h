// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/T4EngineTypes.h"
#include "ST4ListViewWidget.h"

/**
  * #73
 */
class UT4AnimSetAsset;
struct FT4EntityCharacterStanceSetData;
class ST4StanceListWidget : public ST4ListViewWidget
{
public:
	SLATE_BEGIN_ARGS(ST4StanceListWidget) {}
		SLATE_EVENT(FT4OnSelected, OnSelected)
		SLATE_EVENT(FT4OnDoubleClicked, OnDoubleClicked)
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs, 
		const FT4EntityCharacterStanceSetData* InStanceSetData
	);

	ST4StanceListWidget();
	~ST4StanceListWidget();

protected:
	void UpdateLists() override;

private:
	const FT4EntityCharacterStanceSetData* StanceSetData;
};
