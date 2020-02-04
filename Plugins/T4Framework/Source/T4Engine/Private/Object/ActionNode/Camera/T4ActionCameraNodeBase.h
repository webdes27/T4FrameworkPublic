// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/ActionNode/T4ActionNodeUtility.h"
#include "Object/ActionNode/T4ActionNodeBase.h"
#include "Public/T4Engine.h"

/**
  * #101
 */
class UT4CameraModifier;
class UCameraModifier_CameraShake; // #100
class AT4PlayerCameraManager;
class FT4ActionNodeControl;
class FT4ActionCameraNodeBase : public FT4ActionNodeBase
{
public:
	explicit FT4ActionCameraNodeBase(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionCameraNodeBase();

protected:
	AT4PlayerCameraManager* GetPlayerCameraManager() const; // #100
	UT4CameraModifier* GetCameraModifier() const; // #100, #101

	float GetBlendWeight() const; // #100, #58, #102 : 0 ~ 1

protected:
	FT4ActionEasingCurveBlender EasingCurveBlender;
};
