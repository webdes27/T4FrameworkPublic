// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4BaseAnimControl.h"

/**
  *
 */
class AT4GameObject;
class UT4BaseAnimInstance;
class FT4ItemAnimControl : public FT4BaseAnimControl
{
public:
	explicit FT4ItemAnimControl(AT4GameObject* InGameObject);
	virtual ~FT4ItemAnimControl();

protected:
	void Reset() override; // #38
};
