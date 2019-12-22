// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4BaseAnimControl.h"

/**
  *
 */
class FT4AnimalAnimControl : public FT4BaseAnimControl
{
public:
	explicit FT4AnimalAnimControl(AT4GameObject* InGameObject);
	virtual ~FT4AnimalAnimControl();

public:
	void BeginPlay() override; // #18 : 모델 로딩이 완료된 시점에 1회 불림

protected:
	void Reset() override; // #38
	void Advance(const FT4UpdateTime& InUpdateTime) override;

	void AdvanceLockOn(const FT4UpdateTime& InUpdateTime);
};
