// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GamePacketTypes.generated.h"

/**
  *
 */

// WARN : Packet 추가 시는 아래 테그를 찾아 추가된 패킷을 모두 구현해주어야 함!!
// #T4_ADD_PACKET_TAG_CS

UENUM()
enum class ET4GamePacketCS : uint32
{
	Move,
	Jump,
	Roll, // #46
	Rotation, // #40

	LockOn, // #40
	LockOff, // #40

	Stance, // #73
	Posture, // #106

	EquipItem, // #22
	UnequipItem, // #22

	ExchangeItem, // #37

	AimSet, // #113, #116
	AimClear, // #113, #116

	SkillTarget, // #116

	// Test
	CmdWorldTravel,
	CmdChangePlayer, // #11, #52

	CmdPCEnter,
	CmdNPCEnter, // #31
	CmdItemEnter, // #41

	CmdLeave, // #68

	CmdTeleport,

	None,
};

// #T4_ADD_PACKET_TAG_SC
UENUM()
enum class ET4GamePacketSC : uint32
{
	WorldTravel,

	MyPCEnter,
	MyPCChange, // #11, #52

	PCEnter,
	PCLeave,

	NPCEnter, // #31
	NPCLeave, // #31

	ItemEnter, // #41
	ItemLeave, // #41

	Move,
	Jump,
	Roll, // #46
	Rotation, // #40
	Teleport,

	MoveStop, // #52
	MoveSpeedSync, // #52

	LockOn, // #40
	LockOff, // #40

	Stance, // #73
	Posture, // #106

	EquipItem,
	UnequipItem,

	ExchangeItem, // #37

	AimSet, // #113, #116
	AimClear, // #113, #116

	SkillTarget, // #116

	EffectDirect,
	EffectArea, // #68

	Die, // #76
	Resurrect, // #76

	None,
};
