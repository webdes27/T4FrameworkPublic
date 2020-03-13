// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GamePacketTypes.h"

#include "Public/T4GameTypes.h"
#include "Public/T4GameDataTypes.h" // #48

#include "T4Engine/Public/T4EngineTypes.h"
#include "T4Framework/Public/T4FrameworkTypes.h" // #114
#include "T4Framework/Public/T4FrameworkGameplay.h" // #112

#include "T4GamePacketCS.generated.h"

/**
  *
 */
USTRUCT()
struct FT4GamePacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	ET4GamePacketCS PacketCS;

public:
	FT4GamePacketCS_Base()
		: PacketCS(ET4GamePacketCS::None)
	{
	}

	FT4GamePacketCS_Base(ET4GamePacketCS InPacketCS)
		: PacketCS(InPacketCS)
	{
	}

	virtual ~FT4GamePacketCS_Base() {}

	virtual bool Validate(FString& OutMsg)
	{
		return true;
	}

	virtual FString ToString() const
	{
		return FString(TEXT("CS_Packet:None"));
	}
};