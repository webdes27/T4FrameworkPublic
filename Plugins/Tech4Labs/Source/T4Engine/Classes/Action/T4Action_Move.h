// Copyright 2019 Tech4 Labs, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Action_Base.h"
#include "T4Action_Move.generated.h"

/**
  *
 */
 // #T4_ADD_ACTION_TAG

// ET4ActionType::MoveAsyncTo
// ET4ActionType::MoveSyncTo
// ET4ActionType::TeleportTo
// ET4ActionType::JumpTo
// ET4ActionType::RollTo

// ET4ActionType::MoveSpeed

// ET4ActionType::Rotation

// #40
USTRUCT()
struct T4ENGINE_API FT4MoveAsyncToAction : public FT4ObjectAction
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FVector MoveDirection;

	UPROPERTY(EditAnywhere)
	float HeadYawAngle; // #44 : degree, LockOn 일 경우 이동 방향과 달라진다.

public:
	FT4MoveAsyncToAction()
		: FT4ObjectAction(StaticActionType())
		, MoveDirection(FVector::ZeroVector)
		, HeadYawAngle(TNumericLimits<float>::Max())
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::MoveAsyncTo; }

	bool Validate(FString& OutMsg) override
	{
		if (MoveDirection.IsNearlyZero())
		{
			OutMsg = TEXT("Invalid MoveDirection");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("OAction:MoveAsyncTo"));
	}
};

// #33, #40
USTRUCT()
struct T4ENGINE_API FT4MoveSyncToAction : public FT4ObjectAction
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FVector MoveVelocity;

	UPROPERTY(EditAnywhere)
	float HeadYawAngle; // #40 : degree, LockOn 일 경우 이동 방향과 달라진다.

public:
	FT4MoveSyncToAction()
		: FT4ObjectAction(StaticActionType())
		, MoveVelocity(FVector::ZeroVector)
		, HeadYawAngle(TNumericLimits<float>::Max())
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::MoveSyncTo; }

	bool Validate(FString& OutMsg) override
	{
		if (MoveVelocity.IsNearlyZero())
		{
			OutMsg = TEXT("Invalid MoveVelocity");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("OAction:MoveSyncTo"));
	}
};

USTRUCT()
struct T4ENGINE_API FT4JumpToAction : public FT4ObjectAction
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FVector JumpVelocity;

public:
	FT4JumpToAction()
		: FT4ObjectAction(StaticActionType())
		, JumpVelocity(FVector::ZeroVector)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::JumpTo; }

	bool Validate(FString& OutMsg) override
	{
		if (JumpVelocity.IsNearlyZero())
		{
			OutMsg = TEXT("Invalid JumpVelocity");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("OAction:JumpTo"));
	}
};

// #46
USTRUCT()
struct T4ENGINE_API FT4RollToAction : public FT4ObjectAction
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FVector RollVelocity;

public:
	FT4RollToAction()
		: FT4ObjectAction(StaticActionType())
		, RollVelocity(FVector::ZeroVector)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::RollTo; }

	bool Validate(FString& OutMsg) override
	{
		if (RollVelocity.IsNearlyZero())
		{
			OutMsg = TEXT("Invalid RollVelocity");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("OAction:RollTo"));
	}
};

// #34
USTRUCT()
struct T4ENGINE_API FT4TeleportToAction : public FT4ObjectAction
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FVector TargetLocation;

public:
	FT4TeleportToAction()
		: FT4ObjectAction(StaticActionType())
		, TargetLocation(FVector::ZeroVector)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::TeleportTo; }

	bool Validate(FString& OutMsg) override
	{
		if (TargetLocation.IsNearlyZero())
		{
			OutMsg = TEXT("Invalid TargetLocation");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("OAction:TeleportTo"));
	}
};

USTRUCT()
struct T4ENGINE_API FT4MoveSpeedAction : public FT4ObjectAction
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	ET4MoveType MoveType;

	UPROPERTY(EditAnywhere)
	float MoveSpeed;

public:
	FT4MoveSpeedAction()
		: FT4ObjectAction(StaticActionType())
		, MoveType(ET4MoveType::Sync)
		, MoveSpeed(0.0f)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::MoveSpeed; }

	FString ToString() const override
	{
		return FString(TEXT("OAction:MoveSpeed"));
	}
};

USTRUCT()
struct T4ENGINE_API FT4RotationAction : public FT4ObjectAction
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ContiCustomizeDetails::CustomizeRotationActionDetails

	UPROPERTY(EditAnywhere)
	ET4RotationType RotationType;

	UPROPERTY(EditAnywhere)
	float TargetYawAngle; // #40 : LockOn 에서 방향을 맞출 경우 사용 (ET4RotationType::TargetYawAngle)

	UPROPERTY(EditAnywhere)
	float RotationYawRate; // #44 : 초당 회전 단위, Yaw

public:
	FT4RotationAction()
		: FT4ObjectAction(StaticActionType())
		, RotationType(ET4RotationType::Default)
		, TargetYawAngle(TNumericLimits<float>::Max())
		, RotationYawRate(0.0f)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::Rotation; }

	FString ToString() const override
	{
		return FString(TEXT("OAction:Rotation"));
	}
};