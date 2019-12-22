// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/T4EngineTypes.h"
#include "ST4ListViewWidget.h"

/**
  * #71
 */
class UT4AnimSetAsset;
struct FT4EntityCharacterCompositeMeshData;
class ST4CompositePartListWidget : public ST4ListViewWidget
{
public:
	SLATE_BEGIN_ARGS(ST4CompositePartListWidget) {}
		SLATE_EVENT(FT4OnSelected, OnSelected)
		SLATE_EVENT(FT4OnDoubleClicked, OnDoubleClicked)
	SLATE_END_ARGS();

	void Construct(
		const FArguments& InArgs, 
		const FT4EntityCharacterCompositeMeshData* InCompositeMeshData
	);

	ST4CompositePartListWidget();
	~ST4CompositePartListWidget();

protected:
	void UpdateLists() override;

private:
	const FT4EntityCharacterCompositeMeshData* CompositeMeshData;
};
