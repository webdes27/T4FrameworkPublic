// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4EngineDefinitions.h"
#include "T4BaseAnimVariables.generated.h"

/**
  * https://docs.unrealengine.com/ko-kr/Engine/Animation
 */
 // #38
USTRUCT(BlueprintType, Category = "T4Engine")
struct FT4MovementAnimVariables
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=Pawn, Meta=(AllowPrivateAccess=true))
	bool bIsFalling;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=Pawn, Meta=(AllowPrivateAccess=true))
	bool bIsLockOn;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=Pawn, Meta=(AllowPrivateAccess=true))
	float FootStance; // #38 : 1.0f (Left), -1.0f (Right)

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=Pawn, Meta=(AllowPrivateAccess=true))
	float SpeedLevel; // #38 : 0 (Idle) => 1 (Walk) => 2 (Run) => 3 (Fast Run)

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=Pawn, Meta=(AllowPrivateAccess=true))
	float DirectionDegree;

public: 
	FT4MovementAnimVariables()
		: bIsFalling(false)
		, bIsLockOn(false)
		, FootStance(T4AnimSetLeftStanceValue) // #38
		, SpeedLevel(0.0f) // #38
		, DirectionDegree(0.0f)
	{
	}
};

USTRUCT(BlueprintType, Category = "T4Engine")
struct FT4IKAnimVariables
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=IK, Meta=(AllowPrivateAccess=true))
	bool bUsedFootIK;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=IK, Meta=(AllowPrivateAccess=true))
	float LeftFootOffset;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=IK, Meta=(AllowPrivateAccess=true))
	float RightFootOffset;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=IK, Meta=(AllowPrivateAccess=true))
	FVector LeftFootJointTarget;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=IK, Meta=(AllowPrivateAccess=true))
	FVector RightFootJointTarget;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=IK, Meta=(AllowPrivateAccess=true))
	FVector COMOffset;

public:
	FT4IKAnimVariables()
		: bUsedFootIK(false)
		, LeftFootOffset(0.0f)
		, RightFootOffset(0.0f)
		, LeftFootJointTarget(FVector::ZeroVector)
		, RightFootJointTarget(FVector::ZeroVector)
		, COMOffset(FVector::ZeroVector)
	{
	}
};
