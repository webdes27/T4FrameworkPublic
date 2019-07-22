// Copyright 2019 Tech4 Labs, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionBase.h"
#include "Public/Entity/T4EngineEntityKey.h"
#include "T4ActionWorld.generated.h"

/**
  *
 */
 // #T4_ADD_ACTION_TAG
USTRUCT()
struct T4ENGINE_API FT4ChangeWorldAction : public FT4BaseAction
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FSoftObjectPath EntityAssetPath;

public:
	FT4ChangeWorldAction()
		: FT4BaseAction(StaticActionType())
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::ChangeWorld; }

	bool Validate(FString& OutMsg) override
	{
		if (!EntityAssetPath.IsValid())
		{
			OutMsg = TEXT("Invalid EntityAssetPath");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("Action:ChangeWorld"));
	}
};

USTRUCT()
struct T4ENGINE_API FT4ObjectEnterAction : public FT4BaseAction
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(EditAnywhere)
	FName Name;

	UPROPERTY(EditAnywhere)
	ET4EntityType EntityType;

	UPROPERTY(EditAnywhere)
	FSoftObjectPath EntityAssetPath;

	UPROPERTY(EditAnywhere)
	FName GameDataIDName;

	UPROPERTY(EditAnywhere)
	FVector SpawnLocation;

	UPROPERTY(EditAnywhere)
	FRotator SpawnRotation;

public:
	FT4ObjectEnterAction()
		: FT4BaseAction(StaticActionType())
		, Name(NAME_None)
		, EntityType(ET4EntityType::None)
		, GameDataIDName(NAME_None)
		, SpawnLocation(FVector::ZeroVector)
		, SpawnRotation(FRotator::ZeroRotator)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::ObjectEnter; }

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
		return FString(TEXT("Action:ObjectEnter"));
	}
};

USTRUCT()
struct T4ENGINE_API FT4ObjectLeaveAction : public FT4BaseAction
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FT4ObjectID ObjectID;

public:
	FT4ObjectLeaveAction()
		: FT4BaseAction(StaticActionType())
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::ObjectLeave; }

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
		return FString(TEXT("Action:ObjectLeave"));
	}
};