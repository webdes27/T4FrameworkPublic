// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Datatable/T4GameBuiltin_Table_Base.h"

#include "T4Framework/Public/T4FrameworkGameplay.h" // #68

#include "T4GameBuiltin_Table_Stat.generated.h"

/**
  * #114
 */
USTRUCT()
struct FT4GameBuiltin_StatTableRow : public FT4GameBuiltin_TableBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4ContentDatabaseStatDetails::CustomizeDetails

	// #T4_ADD_STAT_CONTENT_TAG
	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameplayStatCategory StatCategory; // #114 : 사용처를 명시한다.

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "100"))
	float Health_Point; // #114 : 피, 체력

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "100"))
	float Mana_Point; // #114 : 마력, 기

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "100"))
	float Striking_Power; // #114 : 공격력

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "100"))
	float Defensive_Power; // #114 : 방어력

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Hit_Ratio; // #114 : 명중률

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Dodge_Ratio; // #114 : 회피률

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "100"))
	float Result_EXP; // #114 : 경험치

public:
	FT4GameBuiltin_StatTableRow()
		: StatCategory(ET4GameplayStatCategory::None) // #114 : 사용처를 명시한다.
		, Health_Point(0.0f)
		, Mana_Point(0.0f)
		, Striking_Power(0.0f)
		, Defensive_Power(0.0f)
		, Hit_Ratio(0.0f)
		, Dodge_Ratio(0.0f)
		, Result_EXP(0.0f)
	{
	}
};
