// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/GameTable/T4GameTableDataTypes.h" // #48
#include "T4Frame/Public/T4FrameGameplayTypes.h" // #104

#include "Classes/Engine/DataTable.h"

#include "T4GameTable_NPC.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UBehaviorTree;
class UT4CharacterEntityAsset;

USTRUCT()
struct FT4GameNPCTableRow : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, Category= Common)
	FName Name;

	UPROPERTY(EditAnywhere, Category= Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameTribeType TribeType; // #104

	UPROPERTY(EditAnywhere, Category= ServerOnly)
	TSoftObjectPtr<UBehaviorTree> BehaviorTreePath;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UT4CharacterEntityAsset> EntityAsset;

	UPROPERTY(EditAnywhere, Category = All)
	FT4GameItemWeaponDataID MainWeaponDataID; // #50

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameEnemyType EnemyType; // #104

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	bool bAggressive; // #50

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float PassiveApproachTimeSec; // #50

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float RunSpeed; // #50

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float WalkSpeed; // #50

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float FastRunSpeed; // #50

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float SensoryRange; // #50

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float RomaingRange; // #50

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float RoamingRateRatio; // #50

public:
	FT4GameNPCTableRow()
		: Name(NAME_None)
		, TribeType(ET4GameTribeType::Neutral) // #104
		, EnemyType(ET4GameEnemyType::NoEnemy) // #104
		, bAggressive(false)
		, PassiveApproachTimeSec(5.0f/*60.0f * 5.0f*/)
		, RunSpeed(600.0f)
		, WalkSpeed(200.0f)
		, FastRunSpeed(0.0f)
		, SensoryRange(1000.0f)
		, RomaingRange(1000.0f)
		, RoamingRateRatio(30.0f)
	{
	}
};
