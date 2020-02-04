// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4BaseAnimVariables.h"
#include "T4BaseAnimInstance.h"
#include "T4AnimalBasicAnimInstance.generated.h"

/**
  * https://docs.unrealengine.com/ko-kr/Engine/Animation
 */
class UBlendSpace;
UCLASS()
class UT4AnimalBasicAnimInstance : public UT4BaseAnimInstance
{
	GENERATED_UCLASS_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	const ET4AnimInstance GetAnimInstanceType() const override { return ET4AnimInstance::Animal_Basic; }

	FT4MovementAnimVariables* GetMovementAnimVariables() override { return &MovementVariables; }

protected:
	void Reset() override; // #38

	void PreUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category=Status, Meta=(AllowPrivateAccess=true))
	FT4MovementAnimVariables MovementVariables;
};