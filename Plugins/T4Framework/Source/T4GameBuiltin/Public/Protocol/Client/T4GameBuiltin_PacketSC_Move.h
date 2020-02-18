// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameBuiltin_PacketSC.h"
#include "T4GameBuiltin_PacketSC_Move.generated.h"

/**
  *
 */
 // #T4_ADD_PACKET_TAG_SC

// ET4GameBuiltin_PacketSC::Move
// ET4GameBuiltin_PacketSC::Jump
// ET4GameBuiltin_PacketSC::Roll // #46
// ET4GameBuiltin_PacketSC::Turn // #40
// ET4GameBuiltin_PacketSC::Teleport

// ET4GameBuiltin_PacketSC::MoveStop // #52
// ET4GameBuiltin_PacketSC::MoveSpeedSync // #52

// ET4GameBuiltin_PacketSC::LockOn
// ET4GameBuiltin_PacketSC::LockOff

// WARN : #46
//
// StoC 패킷은 클라에서 넘어온 Normalize 된 Direction 에 Speed 를 얻어 Velocity 를 넘겨준다. 
//

USTRUCT()
struct FT4GameBuiltin_PacketSC_Move : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FVector MoveToLocation; // #52 : Velocity * (1.0f / GameplayDefaultNetworkLatencySec) / 레이턴시 감안 복원

	UPROPERTY(VisibleAnywhere)
	float HeadYawAngle; // #40 : 필요할 때 3D 로 확장. #50 : 이동 방향과 Head 방향이 다를 경우를 대비해 존재

	UPROPERTY(VisibleAnywhere)
	bool bForceMaxSpeed; // #52 : MovementComponet::MaxSpeed 를 사용할지에 대한 Flag, 기본값이 false 로 Velocity 에서 Speed 를 얻는다. 동기화 이슈!!

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere)
	FVector ServerNavPoint; // #52

	UPROPERTY(EditAnywhere)
	FVector ServerDirection; // #52
#endif

public:
	FT4GameBuiltin_PacketSC_Move()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::Move)
		, MoveToLocation(FVector::ZeroVector)
		, HeadYawAngle(TNumericLimits<float>::Max())
		, bForceMaxSpeed(false) // #52
#if WITH_EDITORONLY_DATA
		, ServerNavPoint(FVector::ZeroVector) // #52
		, ServerDirection(FVector::ForwardVector) // #52
#endif
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!ObjectID.IsValid())
		{
			OutMsg = TEXT("Invalid ObjectID");
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
		return FString(TEXT("SC_Packet:MoveTo"));
	}
};

USTRUCT()
struct FT4GameBuiltin_PacketSC_Jump : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FVector JumpVelocity;

public:
	FT4GameBuiltin_PacketSC_Jump()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::Jump)
		, JumpVelocity(FVector::ZeroVector)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!ObjectID.IsValid())
		{
			OutMsg = TEXT("Invalid ObjectID");
			return false;
		}
		if (JumpVelocity.IsNearlyZero())
		{
			OutMsg = TEXT("Invalid JumpVelocity!");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:JumpTo"));
	}
};

// #46
USTRUCT()
struct FT4GameBuiltin_PacketSC_Roll : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FVector RollVelocity;

public:
	FT4GameBuiltin_PacketSC_Roll()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::Roll)
		, RollVelocity(FVector::ZeroVector)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!ObjectID.IsValid())
		{
			OutMsg = TEXT("Invalid ObjectID");
			return false;
		}
		if (RollVelocity.IsNearlyZero())
		{
			OutMsg = TEXT("Invalid RollVelocity!");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:RollTo"));
	}
};

USTRUCT()
struct FT4GameBuiltin_PacketSC_Turn : public FT4GameBuiltin_PacketSC_Base // #40
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	float TargetYawAngle; // #40 : 필요할 때 3D 로 확장.

public:
	FT4GameBuiltin_PacketSC_Turn()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::Turn)
		, TargetYawAngle(TNumericLimits<float>::Max())
	{
	}

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
		return FString(TEXT("SC_Packet:TurnTo"));
	}
};

USTRUCT()
struct FT4GameBuiltin_PacketSC_Teleport : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FVector TargetLocation;

public:
	FT4GameBuiltin_PacketSC_Teleport()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::Teleport)
		, TargetLocation(FVector::ZeroVector)
	{
	}

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
		return FString(TEXT("SC_Packet:TeleportTo"));
	}
};

// #52
USTRUCT()
struct FT4GameBuiltin_PacketSC_MoveStop : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FVector StopLocation;

	UPROPERTY(VisibleAnywhere)
	float HeadYawAngle; // #40 : 필요할 때 3D 로 확장. #50 : 이동 방향과 Head 방향이 다를 경우를 대비해 존재

	UPROPERTY(VisibleAnywhere)
	bool bSyncLocation;

public:
	FT4GameBuiltin_PacketSC_MoveStop()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::MoveStop)
		, StopLocation(FVector::ZeroVector)
		, HeadYawAngle(TNumericLimits<float>::Max())
		, bSyncLocation(false)
	{
	}

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
		return FString(TEXT("SC_Packet:MoveStop"));
	}
};

// #52
USTRUCT()
struct FT4GameBuiltin_PacketSC_MoveSpeedSync : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	float MoveSpeed;

public:
	FT4GameBuiltin_PacketSC_MoveSpeedSync()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::MoveSpeedSync)
		, MoveSpeed(0.0f)
	{
	}

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
		return FString(TEXT("SC_Packet:MoveSpeedSync"));
	}
};

USTRUCT()
struct FT4GameBuiltin_PacketSC_LockOn : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	float HeadYawAngle; // #40 : 필요할 때 3D 로 확장.

public:
	FT4GameBuiltin_PacketSC_LockOn()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::LockOn)
		, HeadYawAngle(TNumericLimits<float>::Max())
	{
	}

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
		return FString(TEXT("SC_Packet:LockOn"));
	}
};

USTRUCT()
struct FT4GameBuiltin_PacketSC_LockOff : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	float HeadYawAngle; // #40 : 필요할 때 3D 로 확장.

public:
	FT4GameBuiltin_PacketSC_LockOff()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::LockOff)
		, HeadYawAngle(TNumericLimits<float>::Max())
	{
	}

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
		return FString(TEXT("SC_Packet:LockOff"));
	}
};
