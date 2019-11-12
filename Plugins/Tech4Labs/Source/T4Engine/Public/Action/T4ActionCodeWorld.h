// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionCodeBase.h"
#include "T4Asset/Public/T4AssetDefinitions.h" // #73
#include "T4Asset/Public/Entity/T4EntityKey.h"

#include "T4ActionCodeWorld.generated.h"

/**
  *
 */
 // #T4_ADD_ACTION_TAG_CODE

// ET4ActionType::WorldTravel
// ET4ActionType::WorldComposition // #86
// ET4ActionType::SpawnObject
// ET4ActionType::DespawnObject

USTRUCT()
struct T4ENGINE_API FT4WorldTravelAction : public FT4CodeBaseAction
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FSoftObjectPath EntityOrLevelAssetPath; // def MapEntity or LevelAsset

	UPROPERTY(EditAnywhere)
	bool bPreveiwScene; // #87

	UPROPERTY(Transient)
	FVector StartLocation; // #87

public:
	FT4WorldTravelAction()
		: FT4CodeBaseAction(StaticActionType())
		, bPreveiwScene(false) // #87
		, StartLocation(FVector::ZeroVector) // #87
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::WorldTravel; }

	bool Validate(FString& OutMsg) override
	{
		if (!EntityOrLevelAssetPath.IsValid())
		{
			OutMsg = TEXT("Invalid EntityOrLevelAssetPath");
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
struct T4ENGINE_API FT4SpawnObjectAction : public FT4CodeBaseAction
{
	GENERATED_USTRUCT_BODY()

public:
	// DoActionPlaybackStartRecording() // #68

	UPROPERTY(EditAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(EditAnywhere)
	FName Name;

	UPROPERTY(EditAnywhere)
	ET4EntityType EntityType;

	UPROPERTY(EditAnywhere)
	FSoftObjectPath EntityAssetPath;

	UPROPERTY(EditAnywhere)
	FName StanceName; // #73 : 초기 애니메이션 로딩 타임이 있음으로 가급적 스폰시에도 설정하도록 처리한다.
	
	UPROPERTY(EditAnywhere)
	FVector SpawnLocation;

	UPROPERTY(EditAnywhere)
	FRotator SpawnRotation;

	UPROPERTY(EditAnywhere)
	FName GameDataIDName;

	UPROPERTY(EditAnywhere)
	bool bPlayer;
	   
public:
	FT4SpawnObjectAction()
		: FT4CodeBaseAction(StaticActionType())
		, Name(NAME_None)
		, EntityType(ET4EntityType::None)
		, StanceName(NAME_None) // #73
		, SpawnLocation(FVector::ZeroVector)
		, SpawnRotation(FRotator::ZeroRotator)
		, GameDataIDName(NAME_None)
		, bPlayer(false)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::SpawnObject; }

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
		return FString(TEXT("SpawnObjectAction"));
	}
};

USTRUCT()
struct T4ENGINE_API FT4DespawnObjectAction : public FT4CodeBaseAction
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(EditAnywhere)
	float FadeOutTimeSec; // #67, #78

public:
	FT4DespawnObjectAction()
		: FT4CodeBaseAction(StaticActionType())
		, FadeOutTimeSec(T4ObjectWorldLeaveTimeSec) // #67, #78
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::DespawnObject; }

	bool Validate(FString& OutMsg) override
	{
		if (!ObjectID.IsValid())
		{
			OutMsg = TEXT("Invalid ObjectID");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("DespawnObjectAction"));
	}
};