// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Table/T4ContentTableBase.h"

#include "T4Asset/Public/T4AssetCommonTypes.h" // #135
#include "T4Framework/Public/T4FrameworkGameplay.h" // #68

#include "T4ContentTableEffect.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UT4ActionSetAsset;

USTRUCT()
struct FT4GameEffectShapeData // #135
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameplayEffectType EffectType; // #68

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float HitDelayTimeSec;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float DurationSec;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4MoveAngleType MoveAngleType; // #135

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float MovementSpeed; // #135

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float MinMoveDistance; // #135 : Area or Knockback

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float MaxMoveDistance; // #135 : Area or Knockback

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float MaxMoveHeight; // #135 : Area or Knockback, Airborne

public:
	FT4GameEffectShapeData()
		: EffectType(ET4GameplayEffectType::None)
		, HitDelayTimeSec(0.0f)
		, DurationSec(0.0f)
		, MoveAngleType(ET4MoveAngleType::InPlace) // #135
		, MovementSpeed(0.0f) // #135
		, MinMoveDistance(0.0f) // #135
		, MaxMoveDistance(0.0f) // #135
		, MaxMoveHeight(0.0f) // #135
	{
	}
};

USTRUCT()
struct FT4ContentEffectTableRow : public FT4ContentTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4DataTableEffectRowDetails::CustomizeDetails

	// #T4_ADD_EFFECT_TAG_DATA
	UPROPERTY(EditAnywhere, Category = Common)
	uint32 Version;

	UPROPERTY(EditAnywhere, Category = Common)
	FT4GameEffectShapeData EffectShapeData; // #135

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameEffectStatDataID EffectStatDataID; // #114 : 기본 Stat

	UPROPERTY(EditAnywhere, Category=ServerOnly)
	FT4GameEffectDataID ChainEffectDataID; // #68

	UPROPERTY(EditAnywhere, Category= ClientOnly)
	TSoftObjectPtr<UT4ActionSetAsset> DefaultActionSetAsset;

public:
	FT4ContentEffectTableRow()
		: Version(0) // #135

	{
	}
};
