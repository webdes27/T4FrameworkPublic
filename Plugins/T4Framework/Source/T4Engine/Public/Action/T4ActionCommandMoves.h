// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionCommandBase.h"
#include "T4ActionCommandMoves.generated.h"

/**
  *
 */
 // #T4_ADD_ACTION_TAG_CMD

// ET4ActionCommandType::MoveAsync
// ET4ActionCommandType::MoveSync
// ET4ActionCommandType::Teleport
// ET4ActionCommandType::Jump
// ET4ActionCommandType::Rotation
// ET4ActionCommandType::Turn // #131

// ET4ActionCommandType::MoveStop // #52
// ET4ActionCommandType::MoveSpeedSync // #52

// ET4ActionCommandType::Launch // #63 : Only Projectile & Movement

// #40
USTRUCT()
struct T4ENGINE_API FT4MoveAsyncActionCommand : public FT4ActionCommandBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FVector MoveDirection;

	UPROPERTY(EditAnywhere)
	float HeadYawAngle; // #44 : degree, LockOn 일 경우 이동 방향과 달라진다.

public:
	FT4MoveAsyncActionCommand()
		: FT4ActionCommandBase(StaticActionType())
		, MoveDirection(FVector::ZeroVector)
		, HeadYawAngle(T4Const_EmptyYawAngle)
	{
	}

	static ET4ActionCommandType StaticActionType() { return ET4ActionCommandType::MoveAsync; }

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
		return FString(TEXT("MoveAsyncToAction"));
	}
};

// #33, #40
USTRUCT()
struct T4ENGINE_API FT4MoveSyncActionCommand : public FT4ActionCommandBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FVector MoveVelocity; // #50

	UPROPERTY(EditAnywhere)
	float HeadYawAngle; // #40 : degree, LockOn 일 경우 이동 방향과 달라진다.

	UPROPERTY(EditAnywhere)
	bool bForceMaxSpeed; // #52 : MovementComponet::MaxSpeed 를 사용할지에 대한 Flag, 기본값이 false 로 Velocity 에서 Speed 를 얻는다. 동기화 이슈!!

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	FVector ServerNavPoint; // #52

	UPROPERTY(EditAnywhere)
	FVector ServerDirection; // #52
#endif

public:
	FT4MoveSyncActionCommand()
		: FT4ActionCommandBase(StaticActionType())
		, MoveVelocity(FVector::ZeroVector)
		, HeadYawAngle(T4Const_EmptyYawAngle)
		, bForceMaxSpeed(false) // #52
#if WITH_EDITORONLY_DATA
		, ServerNavPoint(FVector::ZeroVector) // #52
		, ServerDirection(FVector::ForwardVector) // #52
#endif
	{
	}

	static ET4ActionCommandType StaticActionType() { return ET4ActionCommandType::MoveSync; }

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
		return FString(TEXT("MoveSyncToAction"));
	}
};

USTRUCT()
struct T4ENGINE_API FT4JumpActionCommand : public FT4ActionCommandBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FVector GoalLocation; // #140 : Musb be FT4ActionBase::DurationSec, Ref FT4ActionCommandBase::OffsetTimeSec

	UPROPERTY(EditAnywhere)
	FVector CollideLocation; // #140 : 점프시 첫번째 부딪히는 지점이 있을 경우. 없으면 Zero

	UPROPERTY(EditAnywhere)
	float CollideTimeSec; // #140 : 점프시 첫번째 부딪히는 지점까지의 시간. 없으면 Zero

	UPROPERTY(EditAnywhere)
	float MaxSpeedXY; // #140

	UPROPERTY(EditAnywhere)
	float MaxHeight; // #140

public:
	FT4JumpActionCommand()
		: FT4ActionCommandBase(StaticActionType())
		, GoalLocation(FVector::ZeroVector)
		, CollideLocation(FVector::ZeroVector) // #140
		, CollideTimeSec(0.0f) // #140
		, MaxSpeedXY(0.0f)
		, MaxHeight(0.0f)
	{
	}

	static ET4ActionCommandType StaticActionType() { return ET4ActionCommandType::Jump; }

	bool Validate(FString& OutMsg) override
	{
		if (GoalLocation.IsNearlyZero())
		{
			OutMsg = TEXT("Invalid Jump GoalLocation");
			return false;
		}
		if (0.0f >= MaxSpeedXY)
		{
			OutMsg = TEXT("MaxSpeedXY is zero");
			return false;
		}
		if (0.0f >= MaxHeight)
		{
			OutMsg = TEXT("MaxHeight is zero");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("JumpToAction"));
	}
};

// #34
USTRUCT()
struct T4ENGINE_API FT4TeleportActionCommand : public FT4ActionCommandBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FVector TargetLocation;

public:
	FT4TeleportActionCommand()
		: FT4ActionCommandBase(StaticActionType())
		, TargetLocation(FVector::ZeroVector)
	{
	}

	static ET4ActionCommandType StaticActionType() { return ET4ActionCommandType::Teleport; }

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
		return FString(TEXT("TeleportToAction"));
	}
};

USTRUCT()
struct T4ENGINE_API FT4RotationActionCommand : public FT4ActionCommandBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	ET4TargetType TargetType;

	UPROPERTY(EditAnywhere)
	float RotationYawRate; // #44 : 초당 회전 단위, Yaw

	UPROPERTY(EditAnywhere)
	float TargetYawAngle; // #40 : LockOn 에서 방향을 맞출 경우 사용 (only ET4TargetType::TargetCustom)

public:
	FT4RotationActionCommand()
		: FT4ActionCommandBase(StaticActionType())
		, TargetType(ET4TargetType::Default)
		, RotationYawRate(0.0f)
		, TargetYawAngle(0.0f)
	{
	}

	static ET4ActionCommandType StaticActionType() { return ET4ActionCommandType::Rotation; }

	FString ToString() const override
	{
		return FString(TEXT("RotationAction"));
	}
};

// #131
USTRUCT()
struct T4ENGINE_API FT4TurnActionCommand : public FT4ActionCommandBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	bool bTurnRight;

	UPROPERTY(EditAnywhere)
	float TurnRate;

public:
	FT4TurnActionCommand()
		: FT4ActionCommandBase(StaticActionType())
		, bTurnRight(false)
		, TurnRate(500.0f)
	{
	}

	static ET4ActionCommandType StaticActionType() { return ET4ActionCommandType::Turn; }

	FString ToString() const override
	{
		return FString(TEXT("TurnAction"));
	}
};

// #52
USTRUCT()
struct T4ENGINE_API FT4MoveStopActionCommand : public FT4ActionCommandBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FVector StopLocation;

	UPROPERTY(EditAnywhere)
	float HeadYawAngle; // #40 : degree, LockOn 일 경우 이동 방향과 달라진다.

	UPROPERTY(EditAnywhere)
	bool bSyncLocation;

public:
	FT4MoveStopActionCommand()
		: FT4ActionCommandBase(StaticActionType())
		, StopLocation(FVector::ZeroVector)
		, HeadYawAngle(T4Const_EmptyYawAngle)
		, bSyncLocation(false)
	{
	}

	static ET4ActionCommandType StaticActionType() { return ET4ActionCommandType::MoveStop; }

	FString ToString() const override
	{
		return FString(TEXT("MoveStopAction"));
	}
};

// #52
USTRUCT()
struct T4ENGINE_API FT4MoveSpeedSyncActionCommand : public FT4ActionCommandBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	float MoveSpeed;

public:
	FT4MoveSpeedSyncActionCommand()
		: FT4ActionCommandBase(StaticActionType())
		, MoveSpeed(0.0f)
	{
	}

	static ET4ActionCommandType StaticActionType() { return ET4ActionCommandType::MoveSpeedSync; }

	FString ToString() const override
	{
		return FString(TEXT("MoveSpeedSyncAction"));
	}
};

// #63
class UT4ActionSetAsset;
USTRUCT()
struct T4ENGINE_API FT4LaunchActionCommand : public FT4ActionCommandBase
{
	GENERATED_USTRUCT_BODY()

public:
	// Common Properties
	//
	UPROPERTY(EditAnywhere)
	ET4MovementType MovementType; // #127

	UPROPERTY(EditAnywhere)
	ET4MovementPathType MovementPathType; // #135

	UPROPERTY(EditAnywhere)
	float InitializeSpeed; // #135 : ProjecitleSpeed or MovementSpeed (수평)

	UPROPERTY(EditAnywhere)
	float MaxHeight; // #132, #140 : 최대 높이 (MaxHeight 로 수직 속도를 얻는다.)

	UPROPERTY(EditAnywhere)
	FT4ActorID TargetActorID; // #135

	UPROPERTY(EditAnywhere)
	FVector ShootDirection; // #135

	UPROPERTY(EditAnywhere)
	FVector GoalLocation; // #135

	UPROPERTY(EditAnywhere)
	ET4AcceleratedMotion AcceleratedMotion; // #127

	UPROPERTY(EditAnywhere)
	float BoundLength; // #112 : Projectile 의 길이, 캐릭터 BoundHeight, 충돌 계산에서 Offset 으로 사용. (원점 에서의 길이)


	// Movement Properties
	//
	UPROPERTY(EditAnywhere)
	FVector CollideLocation; // #140 : 점프시 첫번째 부딪히는 지점이 있을 경우. 없으면 Zero

	UPROPERTY(EditAnywhere)
	float CollideTimeSec; // #140 : 점프시 첫번째 부딪히는 지점까지의 시간. 없으면 Zero

	UPROPERTY(EditAnywhere)
	float AirborneFlightTimeRatio; // #132 : 정점에서 유지할 체공시간 비율


	// Projectile Properties
	//
	UPROPERTY(EditAnywhere)
	FT4ActorID OwnerActorID; // #112

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UT4ActionSetAsset> HeadActionSetAsset;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UT4ActionSetAsset> EndActionSetAsset;

	UPROPERTY(EditAnywhere)
	ET4LoadingPolicy LoadingPolicy;

	UPROPERTY(EditAnywhere)
	bool bRandomRollAngle; // #127

	UPROPERTY(EditAnywhere)
	float InitialRollAngle; // #127

	UPROPERTY(EditAnywhere)
	bool bEnableHitAttached; // #112 : 충돌 지점에 잔상을 남길지 여부 (Arrow : true, Fireball : false)

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnableHitAttached"))
	float HitAttachedTimeSec; // #112 : 충돌 지점에 잔상 시간

	UPROPERTY(EditAnywhere)
	FName HitTargetBoneName; // #135 : Actor 충돌이라면 본 위치를 참조

	UPROPERTY(EditAnywhere)
	bool bEnableBounceOut; // #127 : 명확한 타겟없이 무한대로 발사될 경우 부딪히는 효과 처리 사용 여부

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnableBounceOut"))
	TSoftObjectPtr<UT4ActionSetAsset> BounceOutActionSetAsset;

	UPROPERTY(EditAnywhere)
	bool bUseOscillate; // #127 : 흔들림 여부

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bUseOscillate"))
	float OscillateRange; // #127 : 흔들림 크기

public:
	FT4LaunchActionCommand()
		: FT4ActionCommandBase(StaticActionType())
		, MovementType(ET4MovementType::Straight) // #127
		, MovementPathType(ET4MovementPathType::InPlace) // #135
		, InitializeSpeed(0.0f) // #135 : ProjecitleSpeed or MovementSpeed
		, MaxHeight(0.0f) // #127, #140 : 포물선(Parabola) 에서 사용될 최대 높이 (MaxHeight 로 수직 속도를 얻는다.)
		, ShootDirection(FVector::ZeroVector) // #135
		, GoalLocation(FVector::ZeroVector) // #135
		, AcceleratedMotion(ET4AcceleratedMotion::Uniform) // #127
		, BoundLength(80.0f) // #112
		, CollideLocation(FVector::ZeroVector) // #140 : 점프시 첫번째 부딪히는 지점이 있을 경우. 없으면 Zero
		, CollideTimeSec(0.0f) // #140 : 점프시 첫번째 부딪히는 지점까지의 시간. 없으면 Zero
		, AirborneFlightTimeRatio(0.0f) // #132 : 정점에서 유지할 체공시간 비율
		, LoadingPolicy(ET4LoadingPolicy::Default)
		, bRandomRollAngle(false) // #127
		, InitialRollAngle(0.0f) // #127
		, bEnableHitAttached(false)// #112
		, HitAttachedTimeSec(1.0f) // #112
		, HitTargetBoneName(NAME_None) // #135
		, bEnableBounceOut(false) // #127 : 명확한 타겟없이 무한대로 발사될 경우 부딪히는 효과 처리 사용 여부
		, bUseOscillate(false) // #127 : 흔들림 여부
		, OscillateRange(0.0f) // #127 : 흔들림 크기
	{
	}

	static ET4ActionCommandType StaticActionType() { return ET4ActionCommandType::Launch; }

	FString ToString() const override
	{
		return FString(TEXT("LaunchAction"));
	}
};