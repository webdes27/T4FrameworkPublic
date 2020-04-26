// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/T4EngineTypes.h"

#include "T4FrameworkTypes.generated.h"

/**
  * #114
 */
 // #30
enum ET4FrameworkType
{
	Frame_Client,
	Frame_Server,

	Frame_None
};

enum ET4ControllerType
{
	Controller_Player, // #114 : Server All, Client Player Only
	Controller_NPCAI, // #114 : Server Only

	Controller_Max
};

// #40, #126
UENUM()
enum class ET4ControlModeType : uint8
{
	TPS,
	ShoulderView, // #40
	FPS, // #121

	Free, // #133

	None UMETA(Hidden),
};
