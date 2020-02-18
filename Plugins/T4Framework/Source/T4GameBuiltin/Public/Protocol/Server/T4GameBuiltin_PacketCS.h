// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameBuiltin_PacketTypes.h"

#include "Public/T4GameBuiltin_GameDataTypes.h" // #48

#include "T4Engine/Public/T4EngineTypes.h"
#include "T4Framework/Public/T4FrameworkTypes.h" // #114
#include "T4Framework/Public/T4FrameworkGameplay.h" // #112

#include "T4GameBuiltin_PacketCS.generated.h"

/**
  *
 */
USTRUCT()
struct FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	ET4GameBuiltin_PacketCS PacketCS;

public:
	FT4GameBuiltin_PacketCS_Base()
		: PacketCS(ET4GameBuiltin_PacketCS::None)
	{
	}

	FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS InPacketCS)
		: PacketCS(InPacketCS)
	{
	}

	virtual ~FT4GameBuiltin_PacketCS_Base() {}

	virtual bool Validate(FString& OutMsg)
	{
		return true;
	}

	virtual FString ToString() const
	{
		return FString(TEXT("CS_Packet:None"));
	}
};