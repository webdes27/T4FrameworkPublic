// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GamePacketCS.h"
#include "T4GamePacketCS_Move.generated.h"

/**
  *
 */
 // #T4_ADD_PACKET_TAG_CS

// ET4GamePacketCS::Move
// ET4GamePacketCS::Jump
// ET4GamePacketCS::Rotation // #40
// ET4GamePacketCS::LockOn
// ET4GamePacketCS::LockOff

// WARN : #46
//
// CtoS 패킷의 Vector 는 Normalize 된 방향 벡터를 담아야 한다. Speed 및 Velocity 는 
// 서버측에서 구해 넘겨주는 것으로 처리한다.
//

USTRUCT()
struct FT4GamePacketCS_Move : public FT4GamePacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FVector MoveToLocation; // #52 : Normal + (MoveSpeed * DefaultNetworkLatencySec) / 레이턴시 감안 거리

	UPROPERTY(VisibleAnywhere)
	float HeadYawAngle; // #40 : 필요하다면 3D 로 확장. 이동 패킷은 량이 많음을 고려.

public:
	FT4GamePacketCS_Move()
		: FT4GamePacketCS_Base(ET4GamePacketCS::Move)
		, MoveToLocation(FVector::ZeroVector)
		, HeadYawAngle(TNumericLimits<float>::Max()) // #40
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!SenderID.IsValid())
		{
			OutMsg = TEXT("Invalid Send ObjectID!");
			return false;
		}
		if (MoveToLocation.IsNearlyZero())
		{
			OutMsg = TEXT("Invalid MoveToLocation!");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:Move"));
	}
};

USTRUCT()
struct FT4GamePacketCS_Jump : public FT4GamePacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FVector GoalLocation; // #140

public:
	FT4GamePacketCS_Jump()
		: FT4GamePacketCS_Base(ET4GamePacketCS::Jump)
		, GoalLocation(FVector::ZeroVector)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!SenderID.IsValid())
		{
			OutMsg = TEXT("Invalid Send ObjectID!");
			return false;
		}
		if (GoalLocation.IsNearlyZero())
		{
			OutMsg = TEXT("Goal Location is Zero!");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:Jump"));
	}
};

USTRUCT()
struct FT4GamePacketCS_Rotation : public FT4GamePacketCS_Base // #40
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	float TargetYawAngle; // #40 : 필요하다면 3D 로 확장.

public:
	FT4GamePacketCS_Rotation()
		: FT4GamePacketCS_Base(ET4GamePacketCS::Rotation)
		, TargetYawAngle(TNumericLimits<float>::Max())
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
		return FString(TEXT("CS_Packet:Turn"));
	}
};

USTRUCT()
struct FT4GamePacketCS_LockOn : public FT4GamePacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	float HeadYawAngle; // #40 : 필요할 때 3D 로 확장.

public:
	FT4GamePacketCS_LockOn()
		: FT4GamePacketCS_Base(ET4GamePacketCS::LockOn)
		, HeadYawAngle(TNumericLimits<float>::Max())
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
		return FString(TEXT("CS_Packet:LockOn"));
	}
};

USTRUCT()
struct FT4GamePacketCS_LockOff : public FT4GamePacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	float HeadYawAngle; // #40 : 필요할 때 3D 로 확장.

public:
	FT4GamePacketCS_LockOff()
		: FT4GamePacketCS_Base(ET4GamePacketCS::LockOff)
		, HeadYawAngle(TNumericLimits<float>::Max())
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
		return FString(TEXT("CS_Packet:LockOff"));
	}
};
