// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4AnimalBasicAnimInstance.h"

#include "Public/T4EngineDefinitions.h" // #39
#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h" // #39

#include "Animation/BlendSpace.h"

#include "T4EngineInternal.h"

/**
  * https://docs.unrealengine.com/ko-kr/Engine/Animation
 */
UT4AnimalBasicAnimInstance::UT4AnimalBasicAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4AnimalBasicAnimInstance::Reset()
{
}

void UT4AnimalBasicAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UT4AnimalBasicAnimInstance::PreUpdateAnimation(float DeltaSeconds)
{
	Super::PreUpdateAnimation(DeltaSeconds);
}
