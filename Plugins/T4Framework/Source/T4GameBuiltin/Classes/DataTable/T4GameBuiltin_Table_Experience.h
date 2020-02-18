// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Datatable/T4GameBuiltin_Table_Base.h"

#include "T4Framework/Public/T4FrameworkGameplay.h" // #68

#include "T4GameBuiltin_Table_Experience.generated.h"

/**
  * #114
 */
class UT4ContiAsset;

USTRUCT()
struct FT4GameBuiltin_ExperienceTableRow : public FT4GameBuiltin_TableBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4ContentDatabaseExperienceDetails::CustomizeDetails

	// #T4_ADD_EXPERIENCE_CONTENT_TAG
	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameplayStatCategory StatCategory; // #114 : 사용처를 명시한다.

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameplayStatLevel StatLevel;

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "1000"))
	float Required_EXP; // #114 : 경험치, 상대적, Required EXP

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "1000000"))
	float Total_Required_EXP; // #114 : 경험치, 절대적, Total Required EXP

public:
	FT4GameBuiltin_ExperienceTableRow()
		: StatCategory(ET4GameplayStatCategory::None) // #114 : 사용처를 명시한다.
		, StatLevel(ET4GameplayStatLevel::Max)
		, Required_EXP(0.0f)
		, Total_Required_EXP(0.0f)
	{
	}
};
