// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Datatable/T4GameTableBase.h"

#include "T4Framework/Public/T4FrameworkGameplay.h" // #68

#include "T4GameTableEffect.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UT4ActionAsset;

USTRUCT()
struct FT4GameEffectTableRow : public FT4GameTableBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4DataTableEffectRowDetails::CustomizeDetails

	// #T4_ADD_EFFECT_CONTENT_TAG
	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameEffectStatDataID EffectStatDataID; // #114 : 기본 Stat

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameplayEffectType EffectType; // #68

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float HitDelayTimeSec;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	float AreaRange;

	UPROPERTY(EditAnywhere, Category=ServerOnly)
	FT4GameEffectDataID ChainEffectDataID; // #68

	UPROPERTY(EditAnywhere, Category= ClientOnly)
	TSoftObjectPtr<UT4ActionAsset> DefaultActionAsset;

public:
	FT4GameEffectTableRow()
		: EffectType(ET4GameplayEffectType::Direct)
		, HitDelayTimeSec(0.0f)
		, AreaRange(0.0f)
	{
	}
};
