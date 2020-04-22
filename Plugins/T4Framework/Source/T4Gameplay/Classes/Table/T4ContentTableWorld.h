// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Table/T4ContentTableBase.h"

#include "T4ContentTableWorld.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UT4MapEntityAsset;

USTRUCT()
struct FT4ContentWorldTableRow : public FT4ContentTableBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4DataTableWorldRowDetails::CustomizeDetails

	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category= ClientOnly)
	TSoftObjectPtr<UT4MapEntityAsset> EntityAsset;

public:
	FT4ContentWorldTableRow()
	{
	}
};
