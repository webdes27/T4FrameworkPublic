// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4GameBuiltin_Definitions.h"
#include "Classes/Datatable/T4GameBuiltin_Table_Base.h"

#include "T4Framework/Public/T4FrameworkGameplay.h" // #104

#include "T4GameBuiltin_Table_NPC.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UBehaviorTree;
class UT4ActorEntityAsset;

USTRUCT()
struct FT4GameBuiltin_NPCSpeedData // #108
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
	FT4GameBuiltin_NPCSpeedData()
		: Default(500.0f)
		, Combat(300.0f)
		, Crouch(200.0f)
	{
	}
};

USTRUCT()
struct FT4GameBuiltin_NPCBehaviorData // #108
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameBuiltin_EnemyType EnemyType; // #104

	UPROPERTY(EditAnywhere)
	bool bAggressive; // #50

	UPROPERTY(EditAnywhere)
	float PassiveApproachTimeSec; // #50

	UPROPERTY(EditAnywhere)
	float SensoryRange; // #50

	UPROPERTY(EditAnywhere)
	float RomaingRange; // #50

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float RoamingRateRatio; // #50

public:
	FT4GameBuiltin_NPCBehaviorData()
		: EnemyType(ET4GameBuiltin_EnemyType::NoEnemy) // #104
		, bAggressive(false)
		, PassiveApproachTimeSec(5.0f/*60.0f * 5.0f*/)
		, SensoryRange(1000.0f)
		, RomaingRange(1000.0f)
		, RoamingRateRatio(30.0f)
	{
	}
};

USTRUCT()
struct FT4GameBuiltin_NPCTableRow : public FT4GameBuiltin_TableBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4ContentDatabaseNPCDetails::CustomizeDetails

	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = Common)
	FName RaceName; // #104, #114

	UPROPERTY(EditAnywhere, Category = Common)
	FT4GameBuiltin_NPCSpeedData MoveSpeedData; // #50, #108, #109

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameBuiltin_StatLevel InitializeLevel; // #114 : Stat 레벨

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameBuiltin_GameNPCStatDataID InitializeNPCStatDataID; // #114 : 기본 Stat

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameBuiltin_GameWeaponDataID MainWeaponDataID; // #50

	UPROPERTY(EditAnywhere, Category= ServerOnly)
	TSoftObjectPtr<UBehaviorTree> BehaviorTreePath;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameBuiltin_NPCBehaviorData BehaviorData; // #50

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UT4ActorEntityAsset> EntityAsset;

public:
	FT4GameBuiltin_NPCTableRow()
		: RaceName(T4Const_DefaultNPCRaceName) // #104, #114
		, InitializeLevel(ET4GameBuiltin_StatLevel::Level_1) // #114
	{
	}
};
