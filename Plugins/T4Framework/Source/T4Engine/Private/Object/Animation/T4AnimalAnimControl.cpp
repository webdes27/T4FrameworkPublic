// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AnimalAnimControl.h"

#include "Object/Animation/AnimInstance/T4BaseAnimInstance.h"
#include "Object/Animation/AnimInstance/T4BaseAnimVariables.h"
#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif

#include "T4EngineInternal.h"

/**
  *
 */
FT4AnimalAnimControl::FT4AnimalAnimControl(AT4GameObject* InGameObject)
	: FT4BaseAnimControl(InGameObject)
{
}

FT4AnimalAnimControl::~FT4AnimalAnimControl()
{
}

void FT4AnimalAnimControl::BeginPlay()
{
	check(!bBegunPlay);
	bBegunPlay = true;
}

void FT4AnimalAnimControl::Reset()
{
	// #38
}

void FT4AnimalAnimControl::Advance(const FT4UpdateTime& InUpdateTime)
{
	check(bBegunPlay);
	check(GameObject.IsValid());

	AdvanceLockOn(InUpdateTime);
}

void FT4AnimalAnimControl::AdvanceLockOn(const FT4UpdateTime& InUpdateTime)
{
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	check(nullptr != AnimInstance);
	FT4MovementAnimVariables* MovementAnimVariables = AnimInstance->GetMovementAnimVariables();
	check(nullptr != MovementAnimVariables);
	if (!MovementAnimVariables->bIsLockOn)
	{
		return;
	}
	FVector NormalVelocity = GameObject->GetMovementVelocity();
	NormalVelocity.Normalize();
	FRotator ActorRotation = GameObject->GetRotation();
	float BlendSpaceAngle = AnimInstance->CalculateDirection(NormalVelocity, ActorRotation);
	MovementAnimVariables->DirectionDegree = BlendSpaceAngle;

	/*
			AController* Controller = InGameObject->GetController();
			FRotator ControlRotation = Controller->GetRotation();
			UE_LOG(
				LogT4Engine,
				Verbose,
				TEXT("FT4HumanAnimControl : BS Angle %.f (%.f), Vel (%.f, %.f, %.f), ARot (%.f, %.f, %.f), CRot (%.f, %.f, %.f)"),
				BlendSpaceAngle, CurrSpeed,
				NormalizeVelocity.X, NormalizeVelocity.Y, NormalizeVelocity.Z,
				ActorRotation.Pitch, ActorRotation.Yaw, ActorRotation.Roll,
				ControlRotation.Pitch, ControlRotation.Yaw, ControlRotation.Roll
			);
	*/
}
