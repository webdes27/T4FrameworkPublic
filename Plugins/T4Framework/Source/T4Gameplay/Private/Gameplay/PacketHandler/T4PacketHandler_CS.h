// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameplayInternal.h"

#include "T4Engine/Public/T4EngineTypes.h"

/**
  *
 */
struct FT4PacketCtoS;
class IT4GameObject;
class IT4PlayerController;
class AT4GameplayPlayerController;
class FT4PacketHandlerCS : public IT4PacketHandlerCS
{
public:
	explicit FT4PacketHandlerCS(ET4LayerType InLayerType);
	~FT4PacketHandlerCS();

	bool DoSendPacket(FT4PacketCtoS* InPacket) override; // Client, Reliable

	bool OnRecvPacket_Validation(const FT4PacketCtoS* InPacket) override;
	bool OnRecvPacket(const FT4PacketCtoS* InPacket, IT4PlayerController* InSenderPC) override;

public:
	ET4LayerType GetLayerType() const { return LayerType; }

	void Reset();

private:
	// #31
	// #T4_ADD_PACKET_TAG_CS
	void HandleCS_Move(const struct FT4PacketMoveCS* InPacket);
	void HandleCS_Jump(const struct FT4PacketJumpCS* InPacket);
	void HandleCS_Roll(const struct FT4PacketRollCS* InPacket); // #46
	void HandleCS_Turn(const struct FT4PacketTurnCS* InPacket); // #40
	void HandleCS_LockOn(const struct FT4PacketLockOnCS* InPacket);
	void HandleCS_LockOff(const struct FT4PacketLockOffCS* InPacket);
	void HandleCS_Stance(const struct FT4PacketStanceCS* InPacket); // #73
	void HandleCS_Equip(const struct FT4PacketEquipCS* InPacket);
	void HandleCS_UnEquip(const struct FT4PacketUnEquipCS* InPacket);
	void HandleCS_Exchange(const struct FT4PacketExchangeCS* InPacket); // #37
	void HandleCS_Attack(const struct FT4PacketAttackCS* InPacket);
	void HandleCS_CmdWorldTravel(const struct FT4PacketCmdWorldTravelCS* InPacket);
	void HandleCS_CmdChangePlayer(const struct FT4PacketCmdChangePlayerCS* InPacket, IT4PlayerController* InSenderPC); // #11, #52
	void HandleCS_CmdPCEnter(const struct FT4PacketCmdPCEnterCS* InPacket, IT4PlayerController* InSenderPC);
	void HandleCS_CmdNPCEnter(const struct FT4PacketCmdNPCEnterCS* InPacket);
	void HandleCS_CmdFOEnter(const struct FT4PacketCmdFOEnterCS* InPacket);
	void HandleCS_CmdItemEnter(const struct FT4PacketCmdItemEnterCS* InPacket); // #41
	void HandleCS_CmdLeave(const struct FT4PacketCmdLeaveCS* InPacket, IT4PlayerController* InSenderPC); // #68
	void HandleCS_CmdTeleport(const struct FT4PacketCmdTeleportCS* InPacket);

	UWorld* GetWorld() const;
	IT4GameWorld* GetGameWorld() const; // #52
	IT4PlayerController* GetPlayerController() const;
	IT4GameObject* GetGameObjectForServer(const FT4ObjectID& InObjectID) const;

	AT4GameplayPlayerController* CastGameplayPlayerController(IT4GameObject* InGameObject); // #52

	IT4PacketHandlerSC* GetPacketHandlerSC() const;

private:
	ET4LayerType LayerType;
};
