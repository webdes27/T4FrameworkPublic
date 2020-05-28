// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4EngineTypes.h"
#include "Public/T4EngineDefinitions.h"

#include "T4Asset/Public/ActionSet/T4ActionSetTypes.h"
#include "T4Asset/Public/Entity/T4EntityTypes.h" // #126
#include "T4Asset/Public/AnimSet/T4AnimSetTypes.h" // #129

/**
  *
 */
struct FT4UpdateTime // #102
{
	float RealTimeSec; // 실제 시간
	float ScaledTimeSec; // 시간 스케일이 적용된 시간
	float TimeScale;
	bool bPaused;

	static FT4UpdateTime EmptyUpdateTime;
};

struct FT4WorldActorProperty // #34
{
	FT4WorldActorProperty()
	{
		Reset();
	}

	void Reset()
	{
		bIsLockOn = false;

		// #33
		MoveSpeed[(uint8)ET4MoveMode::Sync] = 0.0f;
		MoveSpeed[(uint8)ET4MoveMode::Async] = 0.0f;
		MoveAccelerationScale = 1.0f; // #38, #52 (0.1 ~ 1)
		
		RotationYawRate = 520.0f; // #44, #46

		BoundType = ET4EntityBoundType::None; // #126
		BoundRadius = 50.0f; // #126
		BoundHeight = 100.0f; // #126
		BoundHalfHeight = 50.0f; // #18, #126

		RelativeLocation = FVector::ZeroVector; // #126
		RelativeRotation = FRotator::ZeroRotator;
		RelativeScale3D = FVector::OneVector; // #37

#if WITH_EDITOR // #140
		TestDefaultMoveSpeed = 0.0f;
		TestCombatMoveSpeed = 0.0f;
		TestSprintMoveSpeed = 0.0f;
		TestJumpMaxHeight = 0.0f; // #140
		TestJumpMaxSpeedXY = 0.0f; // #140
#endif
	}

	const float GetMoveSpeed(const ET4MoveMode InMoveType) const
	{
		return MoveSpeed[(uint8)InMoveType]; // #33
	}

	void SetMoveSpeed(ET4MoveMode InMoveType, float InMoveSpeed)
	{
		MoveSpeed[(uint8)InMoveType] = InMoveSpeed;
	}

	bool bIsLockOn;

	float MoveSpeed[(uint8)ET4MoveMode::Count]; // #33
	float MoveAccelerationScale; // #38, #52 (0.1 ~ 1)
	
	float RotationYawRate; // #46

	ET4EntityBoundType BoundType; // #126
	float BoundRadius; // #126
	float BoundHeight; // #126
	float BoundHalfHeight; // #18, #126

	FVector RelativeLocation; // #126
	FRotator RelativeRotation; // #30, #126
	FVector RelativeScale3D; // #37

#if WITH_EDITOR // #140
	float TestDefaultMoveSpeed;
	float TestCombatMoveSpeed;
	float TestSprintMoveSpeed;
	float TestJumpMaxHeight;
	float TestJumpMaxSpeedXY;
#endif // ~#140
};

#if !UE_BUILD_SHIPPING
struct FT4WorldActorDebugInfo
{
	FT4WorldActorDebugInfo()
	{
		Reset();
	}

	void Reset()
	{
		DebugBitFlags = 0;
	}

	uint32 DebugBitFlags; // #76 : ET4EngineDebugFlag
};
#endif

#if (WITH_EDITOR || WITH_SERVER_CODE)
struct FT4ServerWorldSystemActorDelegates // #49
{
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FT4OnHitOverlap, const FName&, class IT4WorldActor*, const FHitResult&);
	FT4OnHitOverlap OnHitOverlap;
};
#endif

struct FT4AnimParam // #38
{
	FT4AnimParam()
		: AnimationLayer(AnimLayer_Skill)
		, SectionName(NAME_None)
		, PlayCount(ET4PlayCount::OneShot) // #134
		, PlayRate(1.0f)
		, OffsetTimeSec(0.0f)
		, BlendInTimeSec(T4Const_DefaultAnimBlendTimeSec)
		, BlendOutTimeSec(T4Const_DefaultAnimBlendTimeSec)
		, PlayCutOffTimeSec(0.0f) // #134 : 플레이중인 애니를 짜르고 들어간다. Sequential 에서만 사용됨
		, MaxLoopPlayTimeSec(0.0f) // #134 : Looping 시간을 지정할 경우, ActionSet Lifecycle = Duration 외에는 사용하지 말것!!
	{
	}

	ET4AnimationLayer AnimationLayer;
	FName SectionName;
	ET4PlayCount PlayCount; // #134
	float PlayRate;
	float OffsetTimeSec;
	float BlendInTimeSec;
	float BlendOutTimeSec;
	float PlayCutOffTimeSec; // #134 : 플레이중인 애니를 짜르고 들어간다. Sequential 에서만 사용됨
	float MaxLoopPlayTimeSec; // #134 : Looping 시간을 지정할 경우, ActionSet Lifecycle = Duration 외에는 사용하지 말것!!
};

class IT4WorldActor;
struct FT4HitSingleResult
{
	FT4HitSingleResult()
		: QueryCollisionChannel(ECollisionChannel::ECC_MAX)
		, ResultObject(nullptr)
		, ResultLocation(FVector::ZeroVector)
		, ResultImpactNormal(FVector::ForwardVector)
		, ResultHitBone(NAME_None)
	{
	}

	ECollisionChannel QueryCollisionChannel;

	IT4WorldActor* ResultObject;
	FVector ResultLocation;
	FVector ResultImpactNormal; // #112
	FName ResultHitBone; // #112 : Only Skeletal Mesh
};

class UT4EnvironmentAsset;
struct FT4EnvironmentZoneInfo // #94
{
	FName ZoneName;
	ET4ZoneType ZoneType;
	float LayerBlendWeight;
	float BlendPriority;
	UT4EnvironmentAsset* EnvironmentAsset;
};

struct FWorldContext;
struct FT4WorldConstructionValues // #87
{
	FT4WorldConstructionValues()
		: WorldSource(ET4WorldSource::None)
		, WorldContextGameOrEditorOnly(nullptr)
#if WITH_EDITOR
		, bPreviewThumbnailMode(false)
#endif
	{
	}

	ET4WorldSource WorldSource;
	FSoftObjectPath MapEntityOrLevelObjectPath; // MapEntity or LevelAsset ObjectPath <LongPackageName.AssetName>
	FWorldContext* WorldContextGameOrEditorOnly; // Game (PIE)만 설정

#if WITH_EDITOR
	bool bPreviewThumbnailMode;
#endif
};
