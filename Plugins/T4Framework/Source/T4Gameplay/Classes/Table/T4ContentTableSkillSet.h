// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Table/T4ContentTableBase.h"

#include "Public/T4GameTypes.h" // #116

#include "T4ContentTableSkillSet.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UT4ActionSetAsset;

USTRUCT()
struct FT4ContentSkillSetTableRow : public FT4ContentTableBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4DataTableSkillSetRowDetails::CustomizeDetails

	UPROPERTY(EditAnywhere, Category = Common)
	uint32 Version;

	UPROPERTY(EditAnywhere, Category= Common)
	ET4GameSkillSetUseType UseType; // #113, #116 : false = PrimarySkillNameID

	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameSkillDataID PrimarySkillDataID;

	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameSkillDataID SecondarySkillDataID;

	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameSkillDataID TertiarySkillDataID;

	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameSkillDataID FinishSkillDataID;

public:
	FT4ContentSkillSetTableRow()
		: Version(0) // #135
		, UseType(ET4GameSkillSetUseType::Primary)
	{
	}
};
