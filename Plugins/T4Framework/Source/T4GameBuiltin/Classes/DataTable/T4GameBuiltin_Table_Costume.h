// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameBuiltin_Table_ItemBase.h" // #48
#include "T4GameBuiltin_Table_Costume.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UT4CostumeEntityAsset;

USTRUCT()
struct FT4GameBuiltin_CostumeTableRow : public FT4GameBuiltin_ItemTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4ContentDatabaseCostumeDetails::CustomizeDetails

	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameBuiltin_GameItemStatDataID ItemStatDataID; // #114 : 기본 Stat

	UPROPERTY(EditAnywhere, Category= ClientOnly)
	TSoftObjectPtr<UT4CostumeEntityAsset> EntityAsset;

	UPROPERTY(EditAnywhere, Category= ClientOnly)
	FName ExchangePartName;

public:
	FT4GameBuiltin_CostumeTableRow()
	{
	}
};
