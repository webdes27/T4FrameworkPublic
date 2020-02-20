// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameBuiltin_Table_ItemBase.h" // #48
#include "T4GameBuiltin_Table_Weapon.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UT4WeaponEntityAsset;

USTRUCT()
struct FT4GameBuiltin_WeaponEntityData // #108
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UT4WeaponEntityAsset> EntityAsset;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	FName OverrideEquipPoint;

public:
	FT4GameBuiltin_WeaponEntityData()
		: OverrideEquipPoint(NAME_None)
	{
	}
};

USTRUCT()
struct FT4GameBuiltin_WeaponTableRow : public FT4GameBuiltin_ItemTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	// FT4DataTableWeaponRowDetails::CustomizeDetails

	UPROPERTY(VisibleAnywhere, Category = Common)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = Common)
	float AttackRange; // #50

	UPROPERTY(EditAnywhere, Category = Common)
	FT4GameBuiltin_GameSkillSetDataID DefaultSkillSetDataID; // #50, #106 : Key = SubStance

	UPROPERTY(EditAnywhere, Category = Common)
	FT4GameBuiltin_GameSkillSetDataID CombatSkillSetDataID; // #50, #106 : Key = SubStance

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameBuiltin_GameItemStatDataID ItemStatDataID; // #114 : 기본 Stat

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	FT4GameBuiltin_WeaponEntityData MainEntityData;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TArray<FT4GameBuiltin_WeaponEntityData> SubEntityDatas;

public:
	FT4GameBuiltin_WeaponTableRow()
		: AttackRange(0.0f)
	{
	}
};
