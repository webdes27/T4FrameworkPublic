// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4EngineDefinitions.h"
#include "T4BaseAnimVariables.generated.h"

/**
  * https://docs.unrealengine.com/ko-kr/Engine/Animation
 */
// #107
USTRUCT(BlueprintType, Category = "T4Engine")
struct FT4StateAnimVariables
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = StateVar, Meta = (AllowPrivateAccess = true))
	bool bIsCombat; // #106

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = StateVar, Meta = (AllowPrivateAccess = true))
	bool bIsCrouch; // #109

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = StateVar, Meta = (AllowPrivateAccess = true))
	bool bIsAiming; // #113
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = StateVar, Meta = (AllowPrivateAccess = true))
	bool bIsFalling;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = StateVar, Meta = (AllowPrivateAccess = true))
	bool bIsLockOn;

public:
	FT4StateAnimVariables()
		: bIsCombat(false) // #106
		, bIsCrouch(false) // #109
		, bIsAiming(false) // #113
		, bIsFalling(false)
		, bIsLockOn(false)
	{
	}
};

// #38
USTRUCT(BlueprintType, Category = "T4Engine")
struct FT4MovementAnimVariables
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = MovementVar, Meta = (AllowPrivateAccess = true))
	float YawAngle;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = MovementVar, Meta = (AllowPrivateAccess = true))
	float MoveSpeed; // #38, #106 : 이속으로 변경함 (#38 에서는 4단계 Value 를 사용했었음)
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = MovementVar, Meta = (AllowPrivateAccess = true))
	float AimYawAngle; // #113 : -45 ~ 45

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = MovementVar, Meta = (AllowPrivateAccess = true))
	float AimPitchAngle; // #113 : -45 ~ 45

public: 
	FT4MovementAnimVariables()
		: YawAngle(0.0f)
		, MoveSpeed(0.0f) // #38
		, AimYawAngle(0.0f) // #113 : -45 ~ 45
		, AimPitchAngle(0.0f) // #113 : -45 ~ 45
	{
	}
};

USTRUCT(BlueprintType, Category = "T4Engine")
struct FT4IKAnimVariables
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=IK, Meta=(AllowPrivateAccess = true))
	bool bUsedFootIK;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=IK, Meta=(AllowPrivateAccess = true))
	float LeftFootOffset;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=IK, Meta=(AllowPrivateAccess = true))
	float RightFootOffset;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=IK, Meta=(AllowPrivateAccess = true))
	FVector LeftFootJointTarget;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=IK, Meta=(AllowPrivateAccess = true))
	FVector RightFootJointTarget;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=IK, Meta=(AllowPrivateAccess = true))
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
