// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4GameDefinitions.h"
#include "Classes/Datatable/T4GameTableBase.h"

#include "T4Framework/Public/T4FrameworkGameplay.h" // #104

#include "T4GameTableNPC.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UBehaviorTree;
class UT4ActorEntityAsset;

USTRUCT()
struct FT4GameNPCSpeedData // #108
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
	FT4GameNPCSpeedData()
		: Default(500.0f)
		, Combat(300.0f)
		, Crouch(200.0f)
	{
	}
};

USTRUCT()
struct FT4GameNPCBehaviorData // #108
{
	GENERATED_USTRUCT_BODY()

public:
	// #114
	// FT4GameAIBehaviorData : AIController (Runtime)
	// FT4GameNPCBehaviorData : NPC DataTable (Original Set)
	// FT4GameOverrideNPCBehaviorData : Spawn Asset (Instance Set)

	UPROPERTY(EditAnywhere)
	ET4GameEnemyType EnemyType; // #104

	UPROPERTY(EditAnywhere)
	bool bAggressive; // #50

	UPROPERTY(EditAnywhere)
	float ActiveOrKeepAggroTimeSec; // #50

	UPROPERTY(EditAnywhere)
	float AgentRadius; // #114 : Agent 크기 및 Attack/Stop Distance 에서 겹치지 않기 위한 값으로 사용. WorldActor 의 CapsuleRadius 보다 커야함

	UPROPERTY(EditAnywhere)
	float SensoryRange; // #50

	UPROPERTY(EditAnywhere)
	float RomaingRange; // #50

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RoamingRate; // #50

public:
	FT4GameNPCBehaviorData()
		: EnemyType(ET4GameEnemyType::NoEnemy) // #104
		, bAggressive(false)
		, ActiveOrKeepAggroTimeSec(5.0f/*60.0f * 5.0f*/)
		, AgentRadius(50.0f) // #114 : Agent 크기 및 Attack/Stop Distance 에서 겹치지 않기 위한 값으로 사용. WorldActor 의 CapsuleRadius 보다 커야함
		, SensoryRange(1000.0f)
		, RomaingRange(1000.0f)
		, RoamingRate(0.5f)
	{
	}
};

USTRUCT()
struct FT4GameNPCTableRow : public FT4GameTableBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4DataTableNPCRowDetails::CustomizeDetails

	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = Common)
	FName RaceName; // #104, #114

	UPROPERTY(EditAnywhere, Category = Common)
	FT4GameNPCSpeedData MoveSpeedData; // #50, #108, #109

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameStatLevel InitializeLevel; // #114 : Stat 레벨

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameNPCStatDataID InitializeNPCStatDataID; // #114 : 기본 Stat

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameWeaponDataID MainWeaponDataID; // #50

	UPROPERTY(EditAnywhere, Category= ServerOnly)
	TSoftObjectPtr<UBehaviorTree> BehaviorTreePath;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameNPCBehaviorData BehaviorData; // #50

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UT4ActorEntityAsset> EntityAsset;

public:
	FT4GameNPCTableRow()
		: RaceName(T4Const_DefaultNPCRaceName) // #104, #114
		, InitializeLevel(ET4GameStatLevel::Level_1) // #114
	{
	}
};
