// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4BaseAnimVariables.h"
#include "T4BaseAnimInstance.h"
#include "T4ItemBasicAnimInstance.generated.h"

/**
  * https://docs.unrealengine.com/ko-kr/Engine/Animation
 */
UCLASS()
class UT4ItemBasicAnimInstance : public UT4BaseAnimInstance
{
	GENERATED_UCLASS_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	const ET4AnimInstance GetAnimInstanceType() const override { return ET4AnimInstance::Item_Basic; }

	FT4StateAnimVariables* GetStateAnimVariables() override { return &StateVariables; }

protected:
	void Reset() override; // #38

	void PreUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = Variable, Meta = (AllowPrivateAccess = true))
	FT4StateAnimVariables StateVariables;
};