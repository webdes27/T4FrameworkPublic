// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GamePacketCS.h"
#include "T4GamePacketCS_Command.generated.h"

/**
  *
 */
 // #T4_ADD_PACKET_TAG_CS

// ET4GamePacketCS::CmdWorldTravel
// ET4GamePacketCS::CmdChangePlayer // #11, #52

// ET4GamePacketCS::CmdPCEnter
// ET4GamePacketCS::CmdNPCEnter // #31
// ET4GamePacketCS::CmdItemEnter // #41

// ET4GamePacketCS::CmdLeave, // #68

// ET4GamePacketCS::CmdTeleport

USTRUCT()
struct FT4GamePacketCS_CmdWorldTravel : public FT4GamePacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4GameDataID WorldDataID;

public:
	FT4GamePacketCS_CmdWorldTravel()
		: FT4GamePacketCS_Base(ET4GamePacketCS::CmdWorldTravel)
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:CmdWorldTravel"));
	}
};

// #11, #52
USTRUCT()
struct FT4GamePacketCS_CmdChangePlayer : public FT4GamePacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FT4ActorID ChangeActorID;

public:
	FT4GamePacketCS_CmdChangePlayer()
		: FT4GamePacketCS_Base(ET4GamePacketCS::CmdChangePlayer)
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
struct FT4GamePacketCS_CmdPCEnter : public FT4GamePacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4GameDataID PlayerDataID;

	UPROPERTY(VisibleAnywhere)
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere)
	FRotator SpawnRotation;

	UPROPERTY(VisibleAnywhere)
	bool bInitializeSpawn;

public:
	FT4GamePacketCS_CmdPCEnter()
		: FT4GamePacketCS_Base(ET4GamePacketCS::CmdPCEnter)
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
struct FT4GamePacketCS_CmdNPCEnter : public FT4GamePacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4GameDataID NPCDataID;

	UPROPERTY(VisibleAnywhere)
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere)
	FRotator SpawnRotation;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere)
	uint32 ReservedObjectID; // #118
#endif

public:
	FT4GamePacketCS_CmdNPCEnter()
		: FT4GamePacketCS_Base(ET4GamePacketCS::CmdNPCEnter)
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
struct FT4GamePacketCS_CmdItemEnter : public FT4GamePacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4GameDataID ItemDataID;

	UPROPERTY(VisibleAnywhere)
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere)
	FRotator SpawnRotation;

public:
	FT4GamePacketCS_CmdItemEnter()
		: FT4GamePacketCS_Base(ET4GamePacketCS::CmdItemEnter)
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
struct FT4GamePacketCS_CmdLeave : public FT4GamePacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID LeaveObjectID;

public:
	FT4GamePacketCS_CmdLeave()
		: FT4GamePacketCS_Base(ET4GamePacketCS::CmdLeave)
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
struct FT4GamePacketCS_CmdTeleport : public FT4GamePacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FVector TargetLocation;

public:
	FT4GamePacketCS_CmdTeleport()
		: FT4GamePacketCS_Base(ET4GamePacketCS::CmdTeleport)
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