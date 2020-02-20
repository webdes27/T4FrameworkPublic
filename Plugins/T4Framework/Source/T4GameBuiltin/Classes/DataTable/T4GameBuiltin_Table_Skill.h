// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Datatable/T4GameBuiltin_Table_Base.h"

#include "T4Framework/Public/T4FrameworkGameplay.h" // #68
#include "T4Engine/Public/T4EngineTypes.h" // #63

#include "T4GameBuiltin_Table_Skill.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */

class UT4ContiAsset;

USTRUCT()
struct FT4GameBuiltin_SkillShapeData // #108, #114
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Common)
	ET4GameplayAttackType AttackType; // #63

	UPROPERTY(EditAnywhere, Category = Common)
	float HitDelayTimeSec;

	UPROPERTY(EditAnywhere, Category = Common)
	float DurationSec;

	UPROPERTY(EditAnywhere, Category = Common)
	float ProjectileSpeed; // #63

	UPROPERTY(EditAnywhere, Category = Common)
	bool bMoveable;

	UPROPERTY(EditAnywhere, Category = Common)
	bool bLockOn; // #113

	UPROPERTY(EditAnywhere, Category = Common)
	bool bAiming; // #113

	UPROPERTY(EditAnywhere, Category = Common)
	float RotationRateSpeed; // #112, #113 : 캐릭터 RotationYawRate * Speed (1 일 경우 기본값 사용)

	UPROPERTY(EditAnywhere, Category = Common)
	ET4GameplayFindTarget FindTargetType; // #117 : 공객 대상을 찾을 경우에 대한 옵션 (TODO : Tribe or Enemy)

public:
	FT4GameBuiltin_SkillShapeData()
		: AttackType(ET4GameplayAttackType::Melee) // #63
		, HitDelayTimeSec(0.0f)
		, DurationSec(0.0f)
		, ProjectileSpeed(0.0f) // #63
		, bMoveable(false)
		, bLockOn(false)  // #113
		, bAiming(false)  // #113
		, RotationRateSpeed(1.0f) // #112, #113 : 캐릭터 RotationYawRate * Speed (1 일 경우 기본값 사용)
		, FindTargetType(ET4GameplayFindTarget::All)
	{
	}
};

USTRUCT()
struct FT4GameBuiltin_SkillTableRow : public FT4GameBuiltin_TableBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4DataTableSkillRowDetails::CustomizeDetails

	// #T4_ADD_SKILL_CONTENT_TAG 
	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = Common)
	FT4GameBuiltin_SkillShapeData SkillShapeData;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameBuiltin_GameSkillStatDataID SkillStatDataID; // #114 : 기본 Stat

	UPROPERTY(EditAnywhere, Category=ServerOnly)
	FT4GameBuiltin_GameEffectDataID ResultEffectDataID;

	UPROPERTY(EditAnywhere, Category= ClientOnly)
	TSoftObjectPtr<UT4ContiAsset> ContiAsset;

	UPROPERTY(EditAnywhere, Category= ClientOnly, meta = (EditCondition = "bAiming"))
	TSoftObjectPtr<UT4ContiAsset> AimingContiAsset; // #117 : bAiming

	UPROPERTY(EditAnywhere, Category= ClientOnly, meta = (EditCondition = "bAiming"))
	TSoftObjectPtr<UT4ContiAsset> IndicateContiAsset; // #117 : bAiming

public:
	FT4GameBuiltin_SkillTableRow()
	{
	}
};
