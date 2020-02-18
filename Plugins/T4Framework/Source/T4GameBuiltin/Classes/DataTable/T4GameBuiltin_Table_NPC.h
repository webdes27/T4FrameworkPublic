// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Datatable/T4GameBuiltin_Table_Base.h"

#include "T4Framework/Public/T4FrameworkGameplay.h" // #104

#include "T4GameBuiltin_Table_NPC.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UBehaviorTree;
class UT4ActorEntityAsset;

USTRUCT()
struct FT4GameBuiltin_NPCTableRow : public FT4GameBuiltin_TableBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4ContentDatabaseNPCDetails::CustomizeDetails

	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameplayStatLevel InitializeLevel; // #114 : Stat 레벨

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameBuiltin_GameNPCStatDataID InitializeNPCStatDataID; // #114 : 기본 Stat

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameplayTribeType TribeType; // #104

	UPROPERTY(EditAnywhere, Category= ServerOnly)
	TSoftObjectPtr<UBehaviorTree> BehaviorTreePath;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameBuiltin_GameWeaponDataID MainWeaponDataID; // #50

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameplayEnemyType EnemyType; // #104

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	bool bAggressive; // #50

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float PassiveApproachTimeSec; // #50

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float DefaultSpeed; // #50, #108

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float CombatSpeed; // #109

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float CrouchSpeed; // #109

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float SensoryRange; // #50

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float RomaingRange; // #50

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float RoamingRateRatio; // #50

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UT4ActorEntityAsset> EntityAsset;

public:
	FT4GameBuiltin_NPCTableRow()
		: InitializeLevel(ET4GameplayStatLevel::Level_1) // #114
		, TribeType(ET4GameplayTribeType::Neutral) // #104
		, EnemyType(ET4GameplayEnemyType::NoEnemy) // #104
		, bAggressive(false)
		, PassiveApproachTimeSec(5.0f/*60.0f * 5.0f*/)
		, DefaultSpeed(500.0f)
		, CombatSpeed(300.0f)
		, CrouchSpeed(200.0f)
		, SensoryRange(1000.0f)
		, RomaingRange(1000.0f)
		, RoamingRateRatio(30.0f)
	{
	}
};
