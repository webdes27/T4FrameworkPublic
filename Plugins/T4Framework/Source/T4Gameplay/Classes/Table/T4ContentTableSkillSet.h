// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Table/T4ContentTableBase.h"

#include "Public/T4GameTypes.h" // #116

#include "T4ContentTableSkillSet.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
USTRUCT()
struct FT4ContentSkillSetTableRow : public FT4ContentTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4DataTableSkillSetRowDetails::CustomizeDetails

	UPROPERTY(EditAnywhere, Category = Common)
	uint32 Version;

	// #T4_ADD_SKILL_TAG_DATA

	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameSkillDataID AttackDataID_A;

	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameSkillDataID AttackDataID_B;

	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameSkillDataID AttackDataID_C;

	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameSkillDataID AttackDataID_D;

	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameSkillDataID AttackDataID_E;

	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameSkillDataID AttackDataID_F;

	UPROPERTY(EditAnywhere, Category = Common)
	FT4GameSkillDataID AttackDataID_Air;

	UPROPERTY(EditAnywhere, Category = Common)
	FT4GameSkillDataID AttackDataID_Dash;


	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameSkillDataID AbilityDataID_A; // Q

	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameSkillDataID AbilityDataID_B; // E

	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameSkillDataID AbilityDataID_C; // R

	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameSkillDataID AbilityDataID_D; // RMB

public:
	FT4ContentSkillSetTableRow()
		: Version(0) // #135
	{
	}
};
