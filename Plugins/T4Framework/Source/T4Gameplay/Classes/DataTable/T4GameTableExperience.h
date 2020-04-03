// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Datatable/T4GameTableBase.h"

#include "T4Framework/Public/T4FrameworkGameplay.h" // #68

#include "T4GameTableExperience.generated.h"

/**
  * #114
 */
class UT4ActionSetAsset;

USTRUCT()
struct FT4GameExperienceTableRow : public FT4GameTableBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4ContentDatabaseExperienceDetails::CustomizeDetails

	// #T4_ADD_EXPERIENCE_CONTENT_TAG
	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameStatCategory StatCategory; // #114 : 사용처를 명시한다.

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	ET4GameStatLevel StatLevel;

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "1000"))
	float Required_EXP; // #114 : 경험치, 상대적, Required EXP

	UPROPERTY(EditAnywhere, Category = ServerOnly, meta = (ClampMin = "0.0", ClampMax = "1000000"))
	float Total_Required_EXP; // #114 : 경험치, 절대적, Total Required EXP

public:
	FT4GameExperienceTableRow()
		: StatCategory(ET4GameStatCategory::None) // #114 : 사용처를 명시한다.
		, StatLevel(ET4GameStatLevel::Max)
		, Required_EXP(0.0f)
		, Total_Required_EXP(0.0f)
	{
	}
};
