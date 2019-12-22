// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4BaseAnimVariables.h"
#include "T4BaseAnimInstance.h"
#include "T4HumanLocomotionAnimInstance.generated.h"

/**
  * https://docs.unrealengine.com/ko-kr/Engine/Animation
 */
class UBlendSpace;
UCLASS()
class UT4HumanLocomotionAnimInstance : public UT4BaseAnimInstance
{
	GENERATED_UCLASS_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	const ET4AnimInstance GetAnimInstanceType() const override { return ET4AnimInstance::Human_Locomotion; }

	FT4MovementAnimVariables* GetMovementAnimVariables() override { return &MovementVariables; }
	FT4IKAnimVariables* GetIKAnimVariables() override { return &IKVariables; }

protected:
	void Reset() override; // #38

	void PreUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = Variable, Meta=(AllowPrivateAccess=true))
	FT4MovementAnimVariables MovementVariables;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = Variable, Meta=(AllowPrivateAccess=true))
	FT4IKAnimVariables IKVariables;
};