// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/T4EngineTypes.h"

/**
  * #114
 */
struct FT4GameBuiltin_PacketCS_Base;
class IT4PlayerController;
class IT4GameBuiltin_ServerPacketHandler
{
public:
	virtual ~IT4GameBuiltin_ServerPacketHandler() {}

	virtual bool DoSendPacket(FT4GameBuiltin_PacketCS_Base* InPacket) = 0; // Client, Reliable

	virtual bool OnRecvPacket_Validation(const FT4GameBuiltin_PacketCS_Base* InPacket) = 0;
	virtual bool OnRecvPacket(const FT4GameBuiltin_PacketCS_Base* InPacket, IT4PlayerController* InSenderPC) = 0;
};

IT4GameBuiltin_ServerPacketHandler* GetServerPacketHandler(ET4LayerType InLayerType); // #114