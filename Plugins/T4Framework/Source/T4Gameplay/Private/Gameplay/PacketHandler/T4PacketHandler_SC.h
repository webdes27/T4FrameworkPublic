// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameplayInternal.h"

#include "T4Engine/Public/T4EngineTypes.h"

/**
  *
 */
struct FT4PacketStoC;
class IT4GameObject;
class AT4GameplayPlayerController;
class FT4PacketHandlerSC : public IT4PacketHandlerSC
{
public:
	explicit FT4PacketHandlerSC(ET4LayerType InLayerType);
	~FT4PacketHandlerSC();

#if (WITH_EDITOR || WITH_SERVER_CODE)
	bool DoSendPacketForServer(FT4PacketStoC* InPacket, IT4PlayerController* InRecvPC) override;
	bool DoBroadcastPacketForServer(FT4PacketStoC* InPacket, bool bProcessServerPacket) override; // #50

	bool DoProcessPacketOnlyServer(FT4PacketStoC* InPacket, bool bCheckValidate) override; // #52
#endif

	bool OnRecvPacket(const FT4PacketStoC* InPacket) override;

public:
	ET4LayerType GetLayerType() const { return LayerType; }

	void Reset();

private:
	bool SendPacketInternal(FT4PacketStoC* InPacket, AT4GameplayPlayerController* InRecvPC);

	// #31
	// #T4_ADD_PACKET_TAG_SC
	void HandleSC_WorldTravel(const struct FT4PacketWorldTravelSC* InPacket);
	void HandleSC_MyPCEnter(const struct FT4PacketMyPCEnterSC* InPacket);
	void HandleSC_MyPCChange(const struct FT4PacketMyPCChangeSC* InPacket); // #11, #52
	void HandleSC_PCEnter(const struct FT4PacketPCEnterSC* InPacket);
	void HandleSC_PCLeave(const struct FT4PacketPCLeaveSC* InPacket);
	void HandleSC_NPCEnter(const struct FT4PacketNPCEnterSC* InPacket);
	void HandleSC_NPCLeave(const struct FT4PacketNPCLeaveSC* InPacket);
	void HandleSC_FOEnter(const struct FT4PacketFOEnterSC* InPacket);
	void HandleSC_FOLeave(const struct FT4PacketFOLeaveSC* InPacket);
	void HandleSC_ItemEnter(const struct FT4PacketItemEnterSC* InPacket); // #41
	void HandleSC_ItemLeave(const struct FT4PacketItemLeaveSC* InPacket); // #41
	void HandleSC_MoveTo(const struct FT4PacketMoveToSC* InPacket);
	void HandleSC_JumpTo(const struct FT4PacketJumpToSC* InPacket);
	void HandleSC_RollTo(const struct FT4PacketRollToSC* InPacket); // #46
	void HandleSC_TurnTo(const struct FT4PacketTurnToSC* InPacket); // #40
	void HandleSC_TeleportTo(const struct FT4PacketTeleportToSC* InPacket);
	void HandleSC_MoveStop(const struct FT4PacketMoveStopSC* InPacket); // #52
	void HandleSC_MoveSpeedSync(const struct FT4PacketMoveSpeedSyncSC* InPacket); // #52
	void HandleSC_LockOn(const struct FT4PacketLockOnSC* InPacket);
	void HandleSC_LockOff(const struct FT4PacketLockOffSC* InPacket);
	void HandleSC_Stance(const struct FT4PacketStanceSC* InPacket); // #73
	void HandleSC_Equip(const struct FT4PacketEquipSC* InPacket);
	void HandleSC_UnEquip(const struct FT4PacketUnEquipSC* InPacket);
	void HandleSC_Exchange(const struct FT4PacketExchangeSC* InPacket); // #37
	void HandleSC_Attack(const struct FT4PacketAttackSC* InPacket);
	void HandleSC_EffectDirect(const struct FT4PacketEffectDirectSC* InPacket);
	void HandleSC_EffectArea(const struct FT4PacketEffectAreaSC* InPacket); // #68
	void HandleSC_Die(const struct FT4PacketDieSC* InPacket); // #76
	void HandleSC_Resurrect(const struct FT4PacketResurrectSC* InPacket); // #76

	UWorld* GetWorld() const;
	IT4GameWorld* GetGameWorld() const; // #52
	IT4PlayerController* GetPlayerController() const;
	IT4GameObject* GetGameObjectForClient(const FT4ObjectID& InObjectID) const;

private:
	ET4LayerType LayerType;
};
