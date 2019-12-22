// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/T4EngineTypes.h"
#include "ST4ListViewWidget.h"

/**
  * #73
 */
class UT4AnimSetAsset;
struct FT4EntityOverrideMaterialData;
class ST4OverrideMaterialListWidget : public ST4ListViewWidget
{
public:
	SLATE_BEGIN_ARGS(ST4OverrideMaterialListWidget) {}
		SLATE_EVENT(FT4OnSelected, OnSelected)
		SLATE_EVENT(FT4OnDoubleClicked, OnDoubleClicked)
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs, 
		const FT4EntityOverrideMaterialData* InOverrideMaterialData
	);

	ST4OverrideMaterialListWidget();
	~ST4OverrideMaterialListWidget();

protected:
	void UpdateLists() override;

private:
	const FT4EntityOverrideMaterialData* OverrideMaterialData;
};
