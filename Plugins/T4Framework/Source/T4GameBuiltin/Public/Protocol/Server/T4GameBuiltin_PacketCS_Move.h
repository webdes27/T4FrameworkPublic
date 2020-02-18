// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameBuiltin_PacketCS.h"
#include "T4GameBuiltin_PacketCS_Move.generated.h"

/**
  *
 */
 // #T4_ADD_PACKET_TAG_CS

// ET4GameBuiltin_PacketCS::Move
// ET4GameBuiltin_PacketCS::Jump
// ET4GameBuiltin_PacketCS::Roll // 46
// ET4GameBuiltin_PacketCS::Turn // #40
// ET4GameBuiltin_PacketCS::LockOn
// ET4GameBuiltin_PacketCS::LockOff

// WARN : #46
//
// CtoS 패킷의 Vector 는 Normalize 된 방향 벡터를 담아야 한다. Speed 및 Velocity 는 
// 서버측에서 구해 넘겨주는 것으로 처리한다.
//

USTRUCT()
struct FT4GameBuiltin_PacketCS_Move : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FVector MoveToLocation; // #52 : Normal + (MoveSpeed * GameplayDefaultNetworkLatencySec) / 레이턴시 감안 거리

	UPROPERTY(VisibleAnywhere)
	float HeadYawAngle; // #40 : 필요하다면 3D 로 확장. 이동 패킷은 량이 많음을 고려.

public:
	FT4GameBuiltin_PacketCS_Move()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::Move)
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
struct FT4GameBuiltin_PacketCS_Jump : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FVector JumpDirection;

public:
	FT4GameBuiltin_PacketCS_Jump()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::Jump)
		, JumpDirection(FVector::ZeroVector)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!SenderID.IsValid())
		{
			OutMsg = TEXT("Invalid Send ObjectID!");
			return false;
		}
		if (JumpDirection.IsNearlyZero())
		{
			OutMsg = TEXT("Invalid JumpDirection!");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:Jump"));
	}
};

// #46
USTRUCT()
struct FT4GameBuiltin_PacketCS_Roll : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FVector RollDirection;

public:
	FT4GameBuiltin_PacketCS_Roll()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::Roll)
		, RollDirection(FVector::ZeroVector)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!SenderID.IsValid())
		{
			OutMsg = TEXT("Invalid Send ObjectID!");
			return false;
		}
		if (RollDirection.IsNearlyZero())
		{
			OutMsg = TEXT("Invalid RollDirection!");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:Roll"));
	}
};

USTRUCT()
struct FT4GameBuiltin_PacketCS_Turn : public FT4GameBuiltin_PacketCS_Base // #40
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	float TargetYawAngle; // #40 : 필요하다면 3D 로 확장.

public:
	FT4GameBuiltin_PacketCS_Turn()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::Turn)
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
struct FT4GameBuiltin_PacketCS_LockOn : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	float HeadYawAngle; // #40 : 필요할 때 3D 로 확장.

public:
	FT4GameBuiltin_PacketCS_LockOn()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::LockOn)
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
struct FT4GameBuiltin_PacketCS_LockOff : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	float HeadYawAngle; // #40 : 필요할 때 3D 로 확장.

public:
	FT4GameBuiltin_PacketCS_LockOff()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::LockOff)
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
