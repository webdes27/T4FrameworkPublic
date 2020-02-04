// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ItemBasicAnimInstance.h"

#include "T4EngineInternal.h"

/**
  * https://docs.unrealengine.com/ko-kr/Engine/Animation
 */
UT4ItemBasicAnimInstance::UT4ItemBasicAnimInstance(
	const FObjectInitializer& ObjectInitializer
)	: Super(ObjectInitializer)
{
}

void UT4ItemBasicAnimInstance::Reset()
{
}

void UT4ItemBasicAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UT4ItemBasicAnimInstance::PreUpdateAnimation(float DeltaSeconds)
{
	Super::PreUpdateAnimation(DeltaSeconds);
}
