// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Table/T4ContentTableBase.h"

#include "T4Framework/Public/T4FrameworkGameplay.h" // #68

#include "T4ContentTableStat.generated.h"

/**
  * #114
 */
USTRUCT()
struct FT4ContentStatTableRow : public FT4ContentTableBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4DataTableStatRowDetails::CustomizeDetails

	// #T4_ADD_STAT_CONTENT_TAG
	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameStatCategory StatCategory; // #114 : 사용처를 명시한다.

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "10000"))
	float Health_Point; // #114 : 피, 체력

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "100"))
	float Mana_Point; // #114 : 마력, 기

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "100"))
	float Striking_Power; // #114 : 공격력

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "100"))
	float Defensive_Power; // #114 : 방어력

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Hit_Rate; // #114 : 명중률

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Dodge_Rate; // #114 : 회피률

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "100"))
	float Result_EXP; // #114 : 경험치

public:
	FT4ContentStatTableRow()
		: StatCategory(ET4GameStatCategory::None) // #114 : 사용처를 명시한다.
		, Health_Point(100.0f)
		, Mana_Point(100.0f)
		, Striking_Power(10.0f)
		, Defensive_Power(10.0f)
		, Hit_Rate(0.1f)
		, Dodge_Rate(0.1f)
		, Result_EXP(1.0f)
	{
	}
};
