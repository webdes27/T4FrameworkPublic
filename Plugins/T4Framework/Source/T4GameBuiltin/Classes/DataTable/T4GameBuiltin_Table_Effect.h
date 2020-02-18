// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Datatable/T4GameBuiltin_Table_Base.h"

#include "T4Framework/Public/T4FrameworkGameplay.h" // #68

#include "T4GameBuiltin_Table_Effect.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UT4ContiAsset;

USTRUCT()
struct FT4GameBuiltin_EffectTableRow : public FT4GameBuiltin_TableBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4ContentDatabaseEffectDetails::CustomizeDetails

	// #T4_ADD_EFFECT_CONTENT_TAG
	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameBuiltin_GameEffectStatDataID EffectStatDataID; // #114 : 기본 Stat

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameplayEffectType EffectType; // #68

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float HitDelayTimeSec;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float AreaRange;

	UPROPERTY(EditAnywhere, Category=ServerOnly)
	FT4GameBuiltin_GameEffectDataID DamageEffectDataID; // #68

	UPROPERTY(EditAnywhere, Category= ClientOnly)
	TSoftObjectPtr<UT4ContiAsset> ContiAsset;

public:
	FT4GameBuiltin_EffectTableRow()
		: EffectType(ET4GameplayEffectType::Direct)
		, HitDelayTimeSec(0.0f)
		, AreaRange(0.0f)
	{
	}
};
