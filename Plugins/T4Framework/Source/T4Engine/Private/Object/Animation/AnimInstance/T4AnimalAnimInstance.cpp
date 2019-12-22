// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AnimalAnimInstance.h"

#include "Public/T4EngineDefinitions.h" // #39
#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h" // #39

#include "Animation/BlendSpace.h"

#include "T4EngineInternal.h"

/**
  * https://docs.unrealengine.com/ko-kr/Engine/Animation
 */
UT4AnimalAnimInstance::UT4AnimalAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UT4AnimalAnimInstance::Reset()
{
}

void UT4AnimalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UT4AnimalAnimInstance::PreUpdateAnimation(float DeltaSeconds)
{
	Super::PreUpdateAnimation(DeltaSeconds);
}
