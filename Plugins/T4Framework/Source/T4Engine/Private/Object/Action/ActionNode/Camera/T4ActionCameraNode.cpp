// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionCameraNode.h"

#include "Object/Action/T4ActionControl.h"
#include "Object/T4GameObject.h"

#include "Classes/Camera/T4CameraModifier.h" // #100, #101
#include "Classes/Camera/T4PlayerCameraManager.h" // #100, #101
#include "Public/T4Engine.h"
#include "Public/T4EngineUtility.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionCameraNode::FT4ActionCameraNode(FT4ActionControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionNode(InControl, InKey)
{
}

FT4ActionCameraNode::~FT4ActionCameraNode()
{
}

AT4PlayerCameraManager* FT4ActionCameraNode::GetPlayerCameraManager() const // #100
{
	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	IT4ObjectController* GameplayControl = GameWorld->GetPlayerControl();
	if (nullptr == GameplayControl)
	{
		return nullptr;
	}
	return Cast<AT4PlayerCameraManager>(GameplayControl->GetCameraManager());
}

UT4CameraModifier* FT4ActionCameraNode::GetCameraModifier() const // #100
{
	AT4PlayerCameraManager* PlayerCameraManager = GetPlayerCameraManager();
	if (nullptr == PlayerCameraManager || 0 >= PlayerCameraManager->DefaultModifiers.Num())
	{
		return nullptr;
	}
	UCameraModifier* CameraModifier = PlayerCameraManager->FindCameraModifierByClass(
		UT4CameraModifier::StaticClass()
	);
	if (nullptr == CameraModifier)
	{
		return nullptr;
	}
	return Cast<UT4CameraModifier>(CameraModifier);
}

float FT4ActionCameraNode::GetBlendWeight() const // #100, #58, #102 : 0 ~ 1
{
	float EvaluateWeight = EasingCurveBlender.GetBlendWeight(GetPlayingTime(), GetPlayTimeLeft());
	return EvaluateWeight;
}