// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Datatable/T4GameTableBase.h"

#include "Public/T4GameTypes.h" // #116

#include "T4GameTableSkillSet.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UT4ActionAsset;

USTRUCT()
struct FT4GameSkillSetTableRow : public FT4GameTableBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4DataTableSkillSetRowDetails::CustomizeDetails

	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

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
	FT4GameSkillSetTableRow()
		: UseType(ET4GameSkillSetUseType::Primary)
	{
	}
};
