// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Datatable/T4GameBuiltin_Table_Base.h"

#include "T4GameBuiltin_Table_World.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UT4MapEntityAsset;

USTRUCT()
struct FT4GameBuiltin_WorldTableRow : public FT4GameBuiltin_TableBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4DataTableWorldRowDetails::CustomizeDetails

	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category= ClientOnly)
	TSoftObjectPtr<UT4MapEntityAsset> EntityAsset;

public:
	FT4GameBuiltin_WorldTableRow()
	{
	}
};
