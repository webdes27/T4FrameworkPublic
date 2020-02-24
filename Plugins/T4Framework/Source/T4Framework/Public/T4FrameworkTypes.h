// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/T4EngineTypes.h"

/**
  * #114
 */
enum ET4ControllerType
{
	Controller_Player, // #114 : Server All, Client Player Only
	Controller_NPCAI, // #114 : Server Only

	Controller_Max
};