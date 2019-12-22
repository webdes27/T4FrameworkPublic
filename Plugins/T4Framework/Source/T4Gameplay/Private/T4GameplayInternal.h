// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
  * 
 */
struct FT4PacketStoC;
struct FT4PacketCtoS;
class IT4PlayerController;
class IT4PacketHandlerSC
{
public:
	virtual ~IT4PacketHandlerSC() {}

#if (WITH_EDITOR || WITH_SERVER_CODE)
	virtual bool DoSendPacketForServer(FT4PacketStoC* InPacket, IT4PlayerController* InRecvPC) = 0;
	virtual bool DoBroadcastPacketForServer(FT4PacketStoC* InPacket, bool bProcessServerPacket) = 0; // #50

	virtual bool DoProcessPacketOnlyServer(FT4PacketStoC* InPacket, bool bCheckValidate) = 0; // #52
#endif

	virtual bool OnRecvPacket(const FT4PacketStoC* InPacket) = 0;
};

class IT4PacketHandlerCS
{
public:
	virtual ~IT4PacketHandlerCS() {}

	virtual bool DoSendPacket(FT4PacketCtoS* InPacket) = 0; // Client, Reliable

	virtual bool OnRecvPacket_Validation(const FT4PacketCtoS* InPacket) = 0;
	virtual bool OnRecvPacket(const FT4PacketCtoS* InPacket, IT4PlayerController* InSenderPC) = 0;
};

IT4PacketHandlerSC* GetPacketHandlerSC(ET4LayerType InLayerType); // #49

DECLARE_LOG_CATEGORY_EXTERN(LogT4Gameplay, Log, All)
