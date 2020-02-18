// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameBuiltin_PacketCS.h"
#include "T4GameBuiltin_PacketCS_Command.generated.h"

/**
  *
 */
 // #T4_ADD_PACKET_TAG_CS

// ET4GameBuiltin_PacketCS::CmdWorldTravel
// ET4GameBuiltin_PacketCS::CmdChangePlayer // #11, #52

// ET4GameBuiltin_PacketCS::CmdPCEnter
// ET4GameBuiltin_PacketCS::CmdNPCEnter // #31
// ET4GameBuiltin_PacketCS::CmdItemEnter // #41

// ET4GameBuiltin_PacketCS::CmdLeave, // #68

// ET4GameBuiltin_PacketCS::CmdTeleport

USTRUCT()
struct FT4GameBuiltin_PacketCS_CmdWorldTravel : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID WorldDataID;

public:
	FT4GameBuiltin_PacketCS_CmdWorldTravel()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::CmdWorldTravel)
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:CmdWorldTravel"));
	}
};

// #11, #52
USTRUCT()
struct FT4GameBuiltin_PacketCS_CmdChangePlayer : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FT4ActorID ChangeActorID;

public:
	FT4GameBuiltin_PacketCS_CmdChangePlayer()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::CmdChangePlayer)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!SenderID.IsValid())
		{
			OutMsg = TEXT("Invalid Send ObjectID!");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:CmdChangePlayer"));
	}
};

USTRUCT()
struct FT4GameBuiltin_PacketCS_CmdPCEnter : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID PlayerDataID;

	UPROPERTY(VisibleAnywhere)
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere)
	FRotator SpawnRotation;

	UPROPERTY(VisibleAnywhere)
	bool bInitializeSpawn;

public:
	FT4GameBuiltin_PacketCS_CmdPCEnter()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::CmdPCEnter)
		, SpawnLocation(FVector::ZeroVector)
		, SpawnRotation(FRotator::ZeroRotator)
		, bInitializeSpawn(false)
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:CmdPCEnter"));
	}
};

// #31
USTRUCT()
struct FT4GameBuiltin_PacketCS_CmdNPCEnter : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID NPCDataID;

	UPROPERTY(VisibleAnywhere)
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere)
	FRotator SpawnRotation;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere)
	uint32 ReservedObjectID; // #118
#endif

public:
	FT4GameBuiltin_PacketCS_CmdNPCEnter()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::CmdNPCEnter)
		, SpawnLocation(FVector::ZeroVector)
		, SpawnRotation(FRotator::ZeroRotator)
#if WITH_EDITOR
		, ReservedObjectID(T4Const_EmptyObjectID) // #118
#endif
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:CmdNPCEnter"));
	}
};

// #41
USTRUCT()
struct FT4GameBuiltin_PacketCS_CmdItemEnter : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID ItemDataID;

	UPROPERTY(VisibleAnywhere)
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere)
	FRotator SpawnRotation;

public:
	FT4GameBuiltin_PacketCS_CmdItemEnter()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::CmdItemEnter)
		, SpawnLocation(FVector::ZeroVector)
		, SpawnRotation(FRotator::ZeroRotator)
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:CmdItemEnter"));
	}
};

// #68
USTRUCT()
struct FT4GameBuiltin_PacketCS_CmdLeave : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID LeaveObjectID;

public:
	FT4GameBuiltin_PacketCS_CmdLeave()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::CmdLeave)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!LeaveObjectID.IsValid())
		{
			OutMsg = TEXT("Invalid Leave ObjectID!");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:CmdLeave"));
	}
};

USTRUCT()
struct FT4GameBuiltin_PacketCS_CmdTeleport : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FVector TargetLocation;

public:
	FT4GameBuiltin_PacketCS_CmdTeleport()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::CmdTeleport)
		, TargetLocation(FVector::ZeroVector)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!SenderID.IsValid())
		{
			OutMsg = TEXT("Invalid Send ActorID!");
			return false;
		}
		if (TargetLocation.IsNearlyZero())
		{
			OutMsg = TEXT("Invalid TargetLocation!");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:CmdTeleport"));
	}
};