// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Datatable/T4GameBuiltin_Table_Base.h"

#include "T4GameBuiltin_Table_Master.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
USTRUCT()
struct FT4GameBuiltin_MasterTableRow : public FT4GameBuiltin_TableBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4DataTableMasterRowDetails::CustomizeDetails

	UPROPERTY(EditAnywhere, Category = Common)
	ET4GameBuiltin_GameDataType Type;

	UPROPERTY(EditAnywhere, Category = Common)
	TSoftObjectPtr<UDataTable> Table;

	UPROPERTY(Transient)
	FGuid Guid; // #118 : Master 는 필요없다.

public:
	FT4GameBuiltin_MasterTableRow()
		: Type(ET4GameBuiltin_GameDataType::Master)
	{
	}
};
