// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionCameraNodeBase.h"

#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Object/T4GameObject.h"

#include "Classes/Camera/T4CameraModifier.h" // #100, #101
#include "Classes/Camera/T4PlayerCameraManager.h" // #100, #101
#include "Public/T4Engine.h"
#include "Public/T4EngineEnvironment.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionCameraNodeBase::FT4ActionCameraNodeBase(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionNodeBase(InControl, InKey)
{
}

FT4ActionCameraNodeBase::~FT4ActionCameraNodeBase()
{
}

AT4PlayerCameraManager* FT4ActionCameraNodeBase::GetPlayerCameraManager() const // #100
{
	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	return Cast<AT4PlayerCameraManager>(GameWorld->GetPlayerCameraManager());
}

UT4CameraModifier* FT4ActionCameraNodeBase::GetCameraModifier() const // #100
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

float FT4ActionCameraNodeBase::GetBlendWeight() const // #100, #58, #102 : 0 ~ 1
{
	float EvaluateWeight = EasingCurveBlender.GetBlendWeight(GetPlayingTime(), GetPlayTimeLeft());
	return EvaluateWeight;
}