// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4GameTypes.generated.h"

/**
  * #40
 */
UENUM()
enum class ET4GameplayModeType : uint8
{
	TPS,
	ShoulderView, // #40

	None UMETA(Hidden),
};

enum ET4GameHotKeyType // #116
{
	HotKey1_Pressed,
	HotKey2_Pressed,
	HotKey3_Pressed,
	HotKey4_Pressed,
};

UENUM()
enum class ET4GameSkillSequence : uint8
{
	Ready, // #48
	Primary,
	Secondary,
	Tertiary,
	Finish, // #48

	Nums,
};

UENUM()
enum class ET4GameAttackTarget : uint8 // #112
{
	ObjectID,
	Location,
	Direction,

	None,
};
