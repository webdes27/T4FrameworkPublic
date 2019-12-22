// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4HumanBasicAnimInstance.h"

#include "T4EngineInternal.h"

/**
  * https://docs.unrealengine.com/ko-kr/Engine/Animation
 */
UT4HumanBasicAnimInstance::UT4HumanBasicAnimInstance(
	const FObjectInitializer& ObjectInitializer
)	: Super(ObjectInitializer)
{
}

void UT4HumanBasicAnimInstance::Reset()
{
}

void UT4HumanBasicAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UT4HumanBasicAnimInstance::PreUpdateAnimation(float DeltaSeconds)
{
	Super::PreUpdateAnimation(DeltaSeconds);
}
