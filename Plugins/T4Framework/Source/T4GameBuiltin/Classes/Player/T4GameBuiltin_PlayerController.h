// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/Protocol/Server/T4GameBuiltin_PacketCS_Status.h"
#include "Public/Protocol/Server/T4GameBuiltin_PacketCS_Move.h"
#include "Public/Protocol/Server/T4GameBuiltin_PacketCS_Action.h"
#include "Public/Protocol/Server/T4GameBuiltin_PacketCS_Command.h"

#include "Public/Protocol/Client/T4GameBuiltin_PacketSC_Action.h"
#include "Public/Protocol/Client/T4GameBuiltin_PacketSC_Move.h"
#include "Public/Protocol/Client/T4GameBuiltin_PacketSC_Status.h"
#include "Public/Protocol/Client/T4GameBuiltin_PacketSC_World.h"

#include "T4Framework/Classes/Controller/Player/T4PlayerController.h"

#include "T4GameBuiltin_PlayerController.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Framework/Controller/PlayerController/index.html
 */
UCLASS()
class T4GAMEBUILTIN_API AT4GameBuiltin_PlayerController : public AT4PlayerController
{
	GENERATED_UCLASS_BODY()

protected:
	void NotifyAdvance(float InDeltaTime) override; // #49
	void NotifyBeginPlay() override; // #114
	void NotifyPossess(IT4WorldActor* InNewWorldActor) override; // #49
	void NotifyUnPossess(IT4WorldActor* InOldWorldActor) override; // #49

protected:
	// #27 : Protocol
	bool CS_RecvPacket_Validate(const FT4GameBuiltin_PacketCS_Base* InPacket);
	void CS_RecvPacket_Implementation(const FT4GameBuiltin_PacketCS_Base* InPacket);

	void SC_RecvPacket_Implementation(const FT4GameBuiltin_PacketSC_Base* InPacket);

private:
	friend class FT4GameBuiltin_ServerPacketHandler;

	// #T4_ADD_PACKET_TAG_CS
	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_Move(const FT4GameBuiltin_PacketCS_Move& InPacket);

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_Jump(const FT4GameBuiltin_PacketCS_Jump& InPacket);

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_Roll(const FT4GameBuiltin_PacketCS_Roll& InPacket); // #46

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_Turn(const FT4GameBuiltin_PacketCS_Turn& InPacket); // #40

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_LockOn(const FT4GameBuiltin_PacketCS_LockOn& InPacket);

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_LockOff(const FT4GameBuiltin_PacketCS_LockOff& InPacket);

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_Stance(const FT4GameBuiltin_PacketCS_Stance& InPacket); // #73

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_SubStance(const FT4GameBuiltin_PacketCS_SubStance& InPacket); // #106

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_EquipItem(const FT4GameBuiltin_PacketCS_EquipItem& InPacket);

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_UnequipItem(const FT4GameBuiltin_PacketCS_UnequipItem& InPacket);

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_ExchangeItem(const FT4GameBuiltin_PacketCS_ExchangeItem& InPacket); // #37

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_AimSet(const FT4GameBuiltin_PacketCS_AimSet& InPacket); // #113, #116

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_AimClear(const FT4GameBuiltin_PacketCS_AimClear& InPacket); // #113, #116

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_SkillTarget(const FT4GameBuiltin_PacketCS_SkillTarget& InPacket);

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_CmdWorldTravel(const FT4GameBuiltin_PacketCS_CmdWorldTravel& InPacket);

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_CmdChangePlayer(const FT4GameBuiltin_PacketCS_CmdChangePlayer& InPacket); // #11, #52

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_CmdPCEnter(const FT4GameBuiltin_PacketCS_CmdPCEnter& InPacket);

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_CmdNPCEnter(const FT4GameBuiltin_PacketCS_CmdNPCEnter& InPacket); // #31

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_CmdItemEnter(const FT4GameBuiltin_PacketCS_CmdItemEnter& InPacket); // #41

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_CmdLeave(const FT4GameBuiltin_PacketCS_CmdLeave& InPacket); // #68

	UFUNCTION(Reliable, server, WithValidation)
	void CS_RecvPacket_CmdTeleport(const FT4GameBuiltin_PacketCS_CmdTeleport& InPacket);

private:
	friend class FT4GameBuiltin_ClientPacketHandler;

	// #T4_ADD_PACKET_TAG_SC
	UFUNCTION(Reliable, client)
	void SC_RecvPacket_WorldTravel(const FT4GameBuiltin_PacketSC_WorldTravel& InPacket);

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_MyPCEnter(const FT4GameBuiltin_PacketSC_MyPCEnter& InPacket);

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_MyPCChange(const FT4GameBuiltin_PacketSC_MyPCChange& InPacket); // #11, #52

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_PCEnter(const FT4GameBuiltin_PacketSC_PCEnter& InPacket);

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_PCLeave(const FT4GameBuiltin_PacketSC_PCLeave& InPacket);

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_NPCEnter(const FT4GameBuiltin_PacketSC_NPCEnter& InPacket); // #31

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_NPCLeave(const FT4GameBuiltin_PacketSC_NPCLeave& InPacket); // #31

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_ItemEnter(const FT4GameBuiltin_PacketSC_ItemEnter& InPacket); // #41

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_ItemLeave(const FT4GameBuiltin_PacketSC_ItemLeave& InPacket); // #41

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_Move(const FT4GameBuiltin_PacketSC_Move& InPacket);

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_Jump(const FT4GameBuiltin_PacketSC_Jump& InPacket);

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_Roll(const FT4GameBuiltin_PacketSC_Roll& InPacket); // #46

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_Turn(const FT4GameBuiltin_PacketSC_Turn& InPacket); // #40

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_Teleport(const FT4GameBuiltin_PacketSC_Teleport& InPacket);

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_MoveStop(const FT4GameBuiltin_PacketSC_MoveStop& InPacket); // #52

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_MoveSpeedSync(const FT4GameBuiltin_PacketSC_MoveSpeedSync& InPacket); // #52

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_LockOn(const FT4GameBuiltin_PacketSC_LockOn& InPacket);

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_LockOff(const FT4GameBuiltin_PacketSC_LockOff& InPacket);

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_Stance(const FT4GameBuiltin_PacketSC_Stance& InPacket); // #73

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_SubStance(const FT4GameBuiltin_PacketSC_SubStance& InPacket); // #106

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_EquipItem(const FT4GameBuiltin_PacketSC_EquipItem& InPacket);

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_UnequipItem(const FT4GameBuiltin_PacketSC_UnequipItem& InPacket);

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_ExchangeItem(const FT4GameBuiltin_PacketSC_ExchangeItem& InPacket); // #37

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_AimSet(const FT4GameBuiltin_PacketSC_AimSet& InPacket); // #113, #116

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_AimClear(const FT4GameBuiltin_PacketSC_AimClear& InPacket); // #113, #116

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_SkillTarget(const FT4GameBuiltin_PacketSC_SkillTarget& InPacket);

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_EffectDirect(const FT4GameBuiltin_PacketSC_EffectDirect& InPacket);

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_EffectArea(const FT4GameBuiltin_PacketSC_EffectArea& InPacket); // #68

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_Die(const FT4GameBuiltin_PacketSC_Die& InPacket); // #76

	UFUNCTION(Reliable, client)
	void SC_RecvPacket_Resurrect(const FT4GameBuiltin_PacketSC_Resurrect& InPacket); // #76
};
