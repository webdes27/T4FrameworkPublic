// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/Action/ActionNode/T4ActionNodeUtility.h"
#include "Object/Action/ActionNode/T4ActionNode.h"
#include "Public/T4Engine.h"

/**
  * #101
 */
class UT4CameraModifier;
class UCameraModifier_CameraShake; // #100
class AT4PlayerCameraManager;
class FT4ActionControl;
class FT4ActionCameraNode : public FT4ActionNode
{
public:
	explicit FT4ActionCameraNode(FT4ActionControl* InControl, const FT4ActionKey& InKey);
	virtual ~FT4ActionCameraNode();

protected:
	AT4PlayerCameraManager* GetPlayerCameraManager() const; // #100
	UT4CameraModifier* GetCameraModifier() const; // #100, #101

	float GetBlendWeight() const; // #100, #58, #102 : 0 ~ 1

protected:
	FT4ActionEasingCurveBlender EasingCurveBlender;
};
