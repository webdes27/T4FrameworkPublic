// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4GameTypes.generated.h"

/**
  * #40
 */
enum ET4GameHotKeyType // #116
{
	HotKey1_Pressed,
	HotKey2_Pressed,
	HotKey3_Pressed,
	HotKey4_Pressed,

	None,
};

UENUM()
enum class ET4GameSkillSequence : uint8
{
	Ready, // #48

	Sequence_A,
	Sequence_B,
	Sequence_C,
	Sequence_D,
	Sequence_E,
	Sequence_F,

	Nums,
};

UENUM()
enum class ET4GameTargetParamType : uint8 // #112
{
	ObjectID,
	ObjectIDAndLocation, // #126
	ObjectIDAndDirection, // #135

	Location,
	Direction,

	None,
};
