// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionTypes.generated.h"

/**
  * #134
 */
UENUM()
enum class ET4ActionCommandType : uint32
{
	// #62 : Start Code Format

	// begin World

	WorldTravel,

	WorldSpawn,
	WorldDespawn,

	// begin Object / #68 : #T4_ADD_ACTION_TAG_CMD

	MoveAsync, // #40
	MoveSync, // #40

	Jump,
	Roll, // #46
	Teleport,
	Rotation,
	Turn, // #131

	MoveStop, // #52
	MoveSpeedSync, // #52

	Launch, // #63 : Only Projectile

	Aim, // #113
	LockOn,

	Stance, // #73
	Posture, // #106

	EquipWeapon, // #22
	UnequipWeapon, // #48
	Costume, // #37
	Skin, // #130 : Fullbody Skin

	Hit, // #76

	CrowdControl, // #131
	Die, // #76
	Resurrect, // #76
	ReactionStop, // #132

	ActionSet, // #24, #127 : ActionSetAsset

	Stop,

	// begin Editor
	Editor, // #37

	// #62 : End Code Format

	None,
};
