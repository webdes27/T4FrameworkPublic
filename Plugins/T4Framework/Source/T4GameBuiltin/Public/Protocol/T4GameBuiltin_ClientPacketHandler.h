// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/T4EngineTypes.h"

/**
  * #114
 */
struct FT4GameBuiltin_PacketSC_Base;
class IT4PlayerController;
class IT4GameBuiltin_ClientPacketHandler
{
public:
	virtual ~IT4GameBuiltin_ClientPacketHandler() {}

#if (WITH_EDITOR || WITH_SERVER_CODE)
	virtual bool RPC_SC_SendPacket(IT4PlayerController* InRecvPC, FT4GameBuiltin_PacketSC_Base* InPacket) = 0;
	virtual bool RPC_SC_BroadcastPacket(FT4GameBuiltin_PacketSC_Base* InPacket) = 0; // #50
#endif

	virtual bool OnRecvPacket(const FT4GameBuiltin_PacketSC_Base* InPacket) = 0;
};

IT4GameBuiltin_ClientPacketHandler* GetClientPacketHandler(ET4LayerType InLayerType); // #49