// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameBuiltin_PacketSC.h"
#include "T4Framework/Public/T4FrameworkTypes.h" // #42
#include "T4GameBuiltin_PacketSC_World.generated.h"

/**
  *
 */
 // #T4_ADD_PACKET_TAG_SC

// ET4GameBuiltin_PacketSC::WorldTravel

// ET4GameBuiltin_PacketSC::MyPCEnter
// ET4GameBuiltin_PacketSC::MyPCChange // #11, #52

// ET4GameBuiltin_PacketSC::PCEnter
// ET4GameBuiltin_PacketSC::PCLeave

// ET4GameBuiltin_PacketSC::NPCEnter // #31
// ET4GameBuiltin_PacketSC::NPCLeave // #31

// ET4GameBuiltin_PacketSC::ItemEnter // #41
// ET4GameBuiltin_PacketSC::ItemLeave // #41

USTRUCT()
struct FT4GameBuiltin_PacketSC_WorldTravel : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID WorldDataID;

public:
	FT4GameBuiltin_PacketSC_WorldTravel()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::WorldTravel)
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:WorldTravel"));
	}
};

USTRUCT()
struct FT4GameBuiltin_PacketSC_MyPCEnter : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID EnterObjectID;

	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID PlayerDataID;

	UPROPERTY(VisibleAnywhere)
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere)
	FRotator SpawnRotation;

public:
	FT4GameBuiltin_PacketSC_MyPCEnter()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::MyPCEnter)
		, SpawnLocation(FVector::ZeroVector)
		, SpawnRotation(FRotator::ZeroRotator)
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:MyPCEnter"));
	}
};

// #11, #52
USTRUCT()
struct FT4GameBuiltin_PacketSC_MyPCChange : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FT4ActorID ChangeActorID;

public:
	FT4GameBuiltin_PacketSC_MyPCChange()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::MyPCChange)
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:MyPCChange"));
	}
};

USTRUCT()
struct FT4GameBuiltin_PacketSC_PCEnter : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID EnterObjectID;

	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID PlayerDataID;

	UPROPERTY(VisibleAnywhere)
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere)
	FRotator SpawnRotation;
	
	UPROPERTY(VisibleAnywhere)
	FName StanceName; // #73

public:
	FT4GameBuiltin_PacketSC_PCEnter()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::PCEnter)
		, SpawnLocation(FVector::ZeroVector)
		, SpawnRotation(FRotator::ZeroRotator)
		, StanceName(NAME_None) // #73
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:PCEnter"));
	}
};


USTRUCT()
struct FT4GameBuiltin_PacketSC_PCLeave : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;
	
public:
	FT4GameBuiltin_PacketSC_PCLeave()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::PCLeave)
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:PCLeave"));
	}
};

// #31
USTRUCT()
struct FT4GameBuiltin_PacketSC_NPCEnter : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID EnterObjectID;

	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID NPCDataID;

	UPROPERTY(VisibleAnywhere)
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere)
	FRotator SpawnRotation;
	
public:
	FT4GameBuiltin_PacketSC_NPCEnter()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::NPCEnter)
		, SpawnLocation(FVector::ZeroVector)
		, SpawnRotation(FRotator::ZeroRotator)
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:NPCEnter"));
	}
};

// #31
USTRUCT()
struct FT4GameBuiltin_PacketSC_NPCLeave : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;
	
public:
	FT4GameBuiltin_PacketSC_NPCLeave()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::NPCLeave)
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:NPCLeave"));
	}
};

// #41
USTRUCT()
struct FT4GameBuiltin_PacketSC_ItemEnter : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID EnterObjectID;

	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID ItemDataID;

	UPROPERTY(VisibleAnywhere)
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere)
	FRotator SpawnRotation;
	
public:
	FT4GameBuiltin_PacketSC_ItemEnter()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::ItemEnter)
		, SpawnLocation(FVector::ZeroVector)
		, SpawnRotation(FRotator::ZeroRotator)
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:ItemEnter"));
	}
};

// #41
USTRUCT()
struct FT4GameBuiltin_PacketSC_ItemLeave : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;
	
public:
	FT4GameBuiltin_PacketSC_ItemLeave()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::ItemLeave)
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:ItemLeave"));
	}
};
