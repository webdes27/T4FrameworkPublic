// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4GameDefinitions.h"
#include "Classes/Table/T4ContentTableBase.h"

#include "T4ContentTablePlayer.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UT4CharacterEntityAsset;

USTRUCT()
struct FT4GamePlayerSpeedData // #108
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	float DefaultSpeed; // #50, #108

	UPROPERTY(EditAnywhere)
	float CombatSpeed; // #109

	UPROPERTY(EditAnywhere)
	float SprintSpeed; // #109

	UPROPERTY(EditAnywhere)
	float JumpMaxHeight; // #140

	UPROPERTY(EditAnywhere)
	float JumpMaxSpeedXY; // #140

	UPROPERTY(EditAnywhere)
	float RotationYawRate; // #135

public:
	FT4GamePlayerSpeedData()
		: DefaultSpeed(500.0f)
		, CombatSpeed(400.0f)
		, SprintSpeed(800.0f)
		, JumpMaxHeight(200.0f)
		, JumpMaxSpeedXY(250.0f)
		, RotationYawRate(300.0f)
	{
	}
};

USTRUCT()
struct FT4ContentPlayerTableRow : public FT4ContentTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4DataTablePlayerRowDetails::CustomizeDetails

	UPROPERTY(EditAnywhere, Category = Common)
	uint32 Version;

	UPROPERTY(EditAnywhere, Category = Common)
	FName RaceName; // #104, #114

	UPROPERTY(EditAnywhere, Category = Common)
	FT4GamePlayerSpeedData MoveSpeedData; // #114

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameStatLevel InitializeLevel; // #114 : Stat 레벨

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GamePlayerStatDataID InitializePlayerStatDataID; // #114 : 기본 Stat

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UT4CharacterEntityAsset> EntityAsset;

public:
	FT4ContentPlayerTableRow()
		: Version(0) // #135
		, RaceName(T4Const_DefaultPlayerRaceName) // #104, #114
		, InitializeLevel(ET4GameStatLevel::Level_1)
	{
	}
};
