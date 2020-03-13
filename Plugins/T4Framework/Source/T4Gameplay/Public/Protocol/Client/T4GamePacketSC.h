// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GamePacketTypes.h"

#include "Public/T4GameTypes.h" // #114
#include "Public/T4GameDataTypes.h" // #48

#include "T4Engine/Public/T4EngineTypes.h"
#include "T4Framework/Public/T4FrameworkTypes.h" // #114
#include "T4Framework/Public/T4FrameworkGameplay.h" // #112

#include "T4GamePacketSC.generated.h"

/**
  *
 */
USTRUCT()
struct FT4GamePacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	ET4GamePacketSC PacketSC;

public:
	FT4GamePacketSC_Base()
		: PacketSC(ET4GamePacketSC::None)
	{
	}

	FT4GamePacketSC_Base(ET4GamePacketSC InPacketSC)
		: PacketSC(InPacketSC)
	{
	}

	virtual ~FT4GamePacketSC_Base() {}

	virtual bool Validate(FString& OutMsg)
	{
		return true;
	}

	virtual FString ToString() const
	{
		return FString(TEXT("SC_Packet:None"));
	}
};
