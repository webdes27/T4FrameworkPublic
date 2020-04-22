// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ContentTableItemBase.h" // #48
#include "T4ContentTableCostume.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UT4CostumeEntityAsset;

USTRUCT()
struct FT4ContentCostumeTableRow : public FT4ContentItemTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4DataTableCostumeRowDetails::CustomizeDetails

	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameItemStatDataID ItemStatDataID; // #114 : 기본 Stat

	UPROPERTY(EditAnywhere, Category= ClientOnly)
	TSoftObjectPtr<UT4CostumeEntityAsset> EntityAsset;

	UPROPERTY(EditAnywhere, Category= ClientOnly)
	FName ExchangePartName;

public:
	FT4ContentCostumeTableRow()
	{
	}
};
