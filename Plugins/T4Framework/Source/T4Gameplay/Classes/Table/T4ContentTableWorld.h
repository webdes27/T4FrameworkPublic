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
struct FT4ContentWorldTableRow : public FT4ContentTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4DataTableWorldRowDetails::CustomizeDetails

	UPROPERTY(EditAnywhere, Category = Common)
	uint32 Version;

	UPROPERTY(EditAnywhere, Category= ClientOnly)
	TSoftObjectPtr<UT4MapEntityAsset> EntityAsset;

public:
	FT4ContentWorldTableRow()
		: Version(0) // #135
	{
	}
};
