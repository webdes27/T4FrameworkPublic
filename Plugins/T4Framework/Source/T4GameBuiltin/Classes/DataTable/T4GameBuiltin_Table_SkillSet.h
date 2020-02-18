// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Datatable/T4GameBuiltin_Table_Base.h"

#include "Public/T4GameBuiltin_Types.h" // #116

#include "T4GameBuiltin_Table_SkillSet.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UT4ContiAsset;

USTRUCT()
struct FT4GameBuiltin_SkillSetTableRow : public FT4GameBuiltin_TableBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4ContentDatabaseSkillSetDetails::CustomizeDetails

	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category= Common)
	ET4GameBuiltin_SkillSetUseType UseType; // #113, #116 : false = PrimarySkillNameID

	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameBuiltin_GameSkillDataID PrimarySkillDataID;

	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameBuiltin_GameSkillDataID SecondarySkillDataID;

	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameBuiltin_GameSkillDataID TertiarySkillDataID;

	UPROPERTY(EditAnywhere, Category= Common)
	FT4GameBuiltin_GameSkillDataID FinishSkillDataID;

public:
	FT4GameBuiltin_SkillSetTableRow()
		: UseType(ET4GameBuiltin_SkillSetUseType::Primary)
	{
	}
};
