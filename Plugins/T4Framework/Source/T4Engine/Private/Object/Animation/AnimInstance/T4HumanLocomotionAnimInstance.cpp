// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4HumanLocomotionAnimInstance.h"

#include "T4EngineInternal.h"

/**
  * https://docs.unrealengine.com/ko-kr/Engine/Animation
 */
UT4HumanLocomotionAnimInstance::UT4HumanLocomotionAnimInstance(
	const FObjectInitializer& ObjectInitializer
)	: Super(ObjectInitializer)
{
}

void UT4HumanLocomotionAnimInstance::Reset()
{
}

void UT4HumanLocomotionAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UT4HumanLocomotionAnimInstance::PreUpdateAnimation(float DeltaSeconds)
{
	Super::PreUpdateAnimation(DeltaSeconds);
}
