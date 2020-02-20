// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4GameBuiltin_Definitions.h"
#include "Classes/Datatable/T4GameBuiltin_Table_Base.h"

#include "T4GameBuiltin_Table_Player.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UT4CharacterEntityAsset;

USTRUCT()
struct FT4GameBuiltin_PlayerSpeedData // #108
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	float Default; // #50, #108

	UPROPERTY(EditAnywhere)
	float Combat; // #109

	UPROPERTY(EditAnywhere)
	float Crouch; // #109

public:
	FT4GameBuiltin_PlayerSpeedData()
		: Default(500.0f)
		, Combat(300.0f)
		, Crouch(200.0f)
	{
	}
};

USTRUCT()
struct FT4GameBuiltin_PlayerTableRow : public FT4GameBuiltin_TableBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4DataTablePlayerRowDetails::CustomizeDetails

	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = Common)
	FName RaceName; // #104, #114

	UPROPERTY(EditAnywhere, Category = Common)
	FT4GameBuiltin_PlayerSpeedData MoveSpeedData; // #114

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameBuiltin_StatLevel InitializeLevel; // #114 : Stat 레벨

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameBuiltin_GamePlayerStatDataID InitializePlayerStatDataID; // #114 : 기본 Stat

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UT4CharacterEntityAsset> EntityAsset;

public:
	FT4GameBuiltin_PlayerTableRow()
		: RaceName(T4Const_DefaultPlayerRaceName) // #104, #114
	{
	}
};
