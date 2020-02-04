// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4BaseAnimControl.h"

/**
  *
 */
class AT4GameObject;
class UT4BaseAnimInstance;
class FT4PropAnimControl : public FT4BaseAnimControl
{
public:
	explicit FT4PropAnimControl(AT4GameObject* InGameObject);
	virtual ~FT4PropAnimControl();

protected:
	void Reset() override; // #38
};
