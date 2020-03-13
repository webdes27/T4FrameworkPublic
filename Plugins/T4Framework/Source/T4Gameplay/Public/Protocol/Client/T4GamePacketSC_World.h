// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GamePacketSC.h"
#include "T4Framework/Public/T4FrameworkTypes.h" // #42
#include "T4GamePacketSC_World.generated.h"

/**
  *
 */
 // #T4_ADD_PACKET_TAG_SC

// ET4GamePacketSC::WorldTravel

// ET4GamePacketSC::MyPCEnter
// ET4GamePacketSC::MyPCChange // #11, #52

// ET4GamePacketSC::PCEnter
// ET4GamePacketSC::PCLeave

// ET4GamePacketSC::NPCEnter // #31
// ET4GamePacketSC::NPCLeave // #31

// ET4GamePacketSC::ItemEnter // #41
// ET4GamePacketSC::ItemLeave // #41

USTRUCT()
struct FT4GamePacketSC_WorldTravel : public FT4GamePacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4GameDataID WorldDataID;

public:
	FT4GamePacketSC_WorldTravel()
		: FT4GamePacketSC_Base(ET4GamePacketSC::WorldTravel)
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:WorldTravel"));
	}
};

USTRUCT()
struct FT4GamePacketSC_MyPCEnter : public FT4GamePacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID EnterObjectID;

	UPROPERTY(VisibleAnywhere)
	FT4GameDataID PlayerDataID;

	UPROPERTY(VisibleAnywhere)
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere)
	FRotator SpawnRotation;

public:
	FT4GamePacketSC_MyPCEnter()
		: FT4GamePacketSC_Base(ET4GamePacketSC::MyPCEnter)
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
struct FT4GamePacketSC_MyPCChange : public FT4GamePacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FT4ActorID ChangeActorID;

public:
	FT4GamePacketSC_MyPCChange()
		: FT4GamePacketSC_Base(ET4GamePacketSC::MyPCChange)
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:MyPCChange"));
	}
};

USTRUCT()
struct FT4GamePacketSC_PCEnter : public FT4GamePacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID EnterObjectID;

	UPROPERTY(VisibleAnywhere)
	FT4GameDataID PlayerDataID;

	UPROPERTY(VisibleAnywhere)
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere)
	FRotator SpawnRotation;
	
	UPROPERTY(VisibleAnywhere)
	FName StanceName; // #73

public:
	FT4GamePacketSC_PCEnter()
		: FT4GamePacketSC_Base(ET4GamePacketSC::PCEnter)
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
struct FT4GamePacketSC_PCLeave : public FT4GamePacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;
	
public:
	FT4GamePacketSC_PCLeave()
		: FT4GamePacketSC_Base(ET4GamePacketSC::PCLeave)
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:PCLeave"));
	}
};

// #31
USTRUCT()
struct FT4GamePacketSC_NPCEnter : public FT4GamePacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID EnterObjectID;

	UPROPERTY(VisibleAnywhere)
	FT4GameDataID NPCDataID;

	UPROPERTY(VisibleAnywhere)
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere)
	FRotator SpawnRotation;
	
public:
	FT4GamePacketSC_NPCEnter()
		: FT4GamePacketSC_Base(ET4GamePacketSC::NPCEnter)
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
struct FT4GamePacketSC_NPCLeave : public FT4GamePacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;
	
public:
	FT4GamePacketSC_NPCLeave()
		: FT4GamePacketSC_Base(ET4GamePacketSC::NPCLeave)
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:NPCLeave"));
	}
};

// #41
USTRUCT()
struct FT4GamePacketSC_ItemEnter : public FT4GamePacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID EnterObjectID;

	UPROPERTY(VisibleAnywhere)
	FT4GameDataID ItemDataID;

	UPROPERTY(VisibleAnywhere)
	FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere)
	FRotator SpawnRotation;
	
public:
	FT4GamePacketSC_ItemEnter()
		: FT4GamePacketSC_Base(ET4GamePacketSC::ItemEnter)
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
struct FT4GamePacketSC_ItemLeave : public FT4GamePacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;
	
public:
	FT4GamePacketSC_ItemLeave()
		: FT4GamePacketSC_Base(ET4GamePacketSC::ItemLeave)
	{
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:ItemLeave"));
	}
};
