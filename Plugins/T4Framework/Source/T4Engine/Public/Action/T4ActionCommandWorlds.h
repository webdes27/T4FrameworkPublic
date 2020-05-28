// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionCommandBase.h"
#include "T4Asset/Public/T4AssetDefinitions.h" // #73
#include "T4Asset/Public/Entity/T4EntityKey.h"

#include "T4ActionCommandWorlds.generated.h"

/**
  *
 */
 // #T4_ADD_ACTION_TAG_CMD

// ET4ActionCommandType::WorldTravel
// ET4ActionCommandType::WorldSpawn
// ET4ActionCommandType::WorldDespawn

USTRUCT()
struct T4ENGINE_API FT4WorldTravelActionCommand : public FT4ActionCommandBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FSoftObjectPath MapEntityOrLevelObjectPath; // def MapEntity or LevelAsset

	UPROPERTY(EditAnywhere)
	bool bPreveiwScene; // #87

	UPROPERTY(Transient)
	FVector StartLocation; // #87

public:
	FT4WorldTravelActionCommand()
		: FT4ActionCommandBase(StaticActionType())
		, bPreveiwScene(false) // #87
		, StartLocation(FVector::ZeroVector) // #87
	{
	}

	static ET4ActionCommandType StaticActionType() { return ET4ActionCommandType::WorldTravel; }

	bool Validate(FString& OutMsg) override
	{
		if (!MapEntityOrLevelObjectPath.IsValid())
		{
			OutMsg = TEXT("Invalid MapEntityOrLevelObjectPath");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("WorldTravelAction"));
	}
};

USTRUCT()
struct T4ENGINE_API FT4WorldSpawnActionCommand : public FT4ActionCommandBase
{
	GENERATED_USTRUCT_BODY()

public:
	// SaveActionReplaySnapshot() // #68

	UPROPERTY(EditAnywhere)
	FT4ActorID ActorID;

	UPROPERTY(EditAnywhere)
	FT4ObjectID OwnerObjectID; // #114 : GameObject ID

	UPROPERTY(EditAnywhere)
	FName Name;

	UPROPERTY(EditAnywhere)
	ET4EntityType EntityType;

	UPROPERTY(EditAnywhere)
	FSoftObjectPath EntityAssetPath;

	UPROPERTY(EditAnywhere)
	FName SkinName; // #135 : 기본 스킨

	UPROPERTY(EditAnywhere)
	FName StanceName; // #73 : 초기 애니메이션 로딩 타임이 있음으로 가급적 스폰시에도 설정하도록 처리한다.
	
	UPROPERTY(EditAnywhere)
	FName PostureName; // #73, #111

	UPROPERTY(EditAnywhere)
	FVector SpawnLocation;

	UPROPERTY(EditAnywhere)
	FRotator SpawnRotation;

	UPROPERTY(EditAnywhere)
	float MoveSpeed; // #140

	UPROPERTY(EditAnywhere)
	FName GameDataIDName;

	UPROPERTY(EditAnywhere)
	bool bPlayer;
	   
public:
	FT4WorldSpawnActionCommand()
		: FT4ActionCommandBase(StaticActionType())
		, Name(NAME_None)
		, EntityType(ET4EntityType::None)
		, SkinName(NAME_None) // #135
		, StanceName(NAME_None) // #73
		, PostureName(NAME_None) // #73, #111
		, SpawnLocation(FVector::ZeroVector)
		, SpawnRotation(FRotator::ZeroRotator)
		, MoveSpeed(0.0f) // #140
		, GameDataIDName(NAME_None)
		, bPlayer(false)
	{
	}

	static ET4ActionCommandType StaticActionType() { return ET4ActionCommandType::WorldSpawn; }

	bool Validate(FString& OutMsg) override
	{
		if (ET4EntityType::None == EntityType)
		{
			OutMsg = TEXT("Not set EntityType");
			return false;
		}
		if (!EntityAssetPath.IsValid())
		{
			OutMsg = TEXT("Invalid EntityAssetPath");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("WorldSpawnAction"));
	}
};

USTRUCT()
struct T4ENGINE_API FT4WorldDespawnActionCommand : public FT4ActionCommandBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FT4ActorID ActorID;

	UPROPERTY(EditAnywhere)
	float FadeOutTimeSec; // #67, #78

public:
	FT4WorldDespawnActionCommand()
		: FT4ActionCommandBase(StaticActionType())
		, FadeOutTimeSec(T4Const_ObjectWorldLeaveTimeSec) // #67, #78
	{
	}

	static ET4ActionCommandType StaticActionType() { return ET4ActionCommandType::WorldDespawn; }

	bool Validate(FString& OutMsg) override
	{
		if (!ActorID.IsValid())
		{
			OutMsg = TEXT("Invalid ActorID");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("WorldDespawnAction"));
	}
};