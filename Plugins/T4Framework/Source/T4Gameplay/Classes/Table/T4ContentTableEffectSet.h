// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Table/T4ContentTableBase.h"

#include "Public/T4GameTypes.h" // #116

#include "T4ContentTableEffectSet.generated.h"

/**
  * #135
 */
USTRUCT()
struct FT4ContentEffectSetTableRow : public FT4ContentTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4DataTableEffectSetRowDetails::CustomizeDetails

	UPROPERTY(EditAnywhere, Category = Common)
	uint32 Version;

	// #T4_ADD_EFFECT_TAG_DATA

	UPROPERTY(EditAnywhere, Category= ServerOnly)
	FT4GameEffectDataID HitDataID_Normal;

	UPROPERTY(EditAnywhere, Category= ServerOnly)
	FT4GameEffectDataID KnockbackDataID_Normal;

	UPROPERTY(EditAnywhere, Category= ServerOnly)
	FT4GameEffectDataID AirborneDataID_Normal;

	UPROPERTY(EditAnywhere, Category= ServerOnly)
	FT4GameEffectDataID StunDataID_Normal;

	UPROPERTY(EditAnywhere, Category= ServerOnly)
	FT4GameEffectDataID MissDataID_Normal;

public:
	FT4ContentEffectSetTableRow()
		: Version(0) // #135
	{
	}
};
