// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionTurnNode.h"

#include "Object/Action/T4ActionControl.h"
#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionTurnNode::FT4ActionTurnNode(FT4ActionControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionNode(InControl, InKey)
	, TurnType(ET4TargetType::Default)
	, TargetYawAngle(TNumericLimits<float>::Max())
	, RotationYawRate(0.0f)
{
}

FT4ActionTurnNode::~FT4ActionTurnNode()
{
}

FT4ActionTurnNode* FT4ActionTurnNode::CreateNode(
	FT4ActionControl* InControl,
	const FT4TurnAction& InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	check(ET4ActionType::Turn == InAction.ActionType);
	FT4ActionTurnNode* NewNode = new FT4ActionTurnNode(InControl, InAction.ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionTurnNode::Create(const FT4ActionStruct* InAction)
{
	check(ET4ActionType::Turn == InAction->ActionType);
	const FT4TurnAction& ConvAction = *(static_cast<const FT4TurnAction*>(InAction));
	TurnType = ConvAction.TurnType;
	RotationYawRate = ConvAction.RotationYawRate; // #44
	TargetYawAngle = ConvAction.TargetYawAngle; // #40
#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionTurnNode::Destroy()
{
}

void FT4ActionTurnNode::Advance(const FT4UpdateTime& InUpdateTime)
{
}

bool FT4ActionTurnNode::Play()
{
	bool bResult = PlayInternal(GetOffsetTimeSec()); // #56
	return bResult;
}

void FT4ActionTurnNode::Stop()
{
}

bool FT4ActionTurnNode::IsAutoFinished() const
{
	check(IsPlayed());
	check(ET4LifecycleType::Auto == LifecycleType);
	AT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	return !OwnerGameObject->IsTurning(); // #44
}

bool FT4ActionTurnNode::PlayInternal(float InOffsetTimeSec)
{
	// #56
	bool bResult = false;
	switch (TurnType)
	{
		case ET4TargetType::TargetObject:
			bResult = SetRotationTargetObject();
			break;

		case ET4TargetType::TargetLocation:
			bResult = SetRotationTargetLocation();
			break;

		case ET4TargetType::TargetDirection:
			bResult = SetRotationTargetDirection();
			break;

		case ET4TargetType::TargetCustom:
			bResult = SetRotationTargetYawAngle(); // #40
			break;
	}
	if (!bResult)
	{
		if (ET4TargetType::Default != TurnType)
		{
			UE_LOG(
				LogT4Engine,
				Warning,
				TEXT("FT4ActionTurnNode::PlayInternal : '%u' fallback. apply current rotation."),
				uint8(TurnType)
			);
		}
		AT4GameObject* OwnerGameObject = GetGameObject();
		check(nullptr != OwnerGameObject);
		SetLookAt(OwnerGameObject->GetRotation());
	}
	return true;
}

bool FT4ActionTurnNode::SetRotationTargetObject()
{
	check(ET4TargetType::TargetObject == TurnType);
	IT4GameObject* TargetObject = nullptr;
	bool bResult = FindTargetObjectInParameter(&TargetObject, TEXT("SetRotationTargetObject"));
	if (!bResult)
	{
		return false;
	}
	AT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	FRotator LookAtTarget = OwnerGameObject->GetRotation();
	LookAtTarget.Yaw = FVector(
		TargetObject->GetRootLocation() - OwnerGameObject->GetRootLocation()
	).ToOrientationRotator().Yaw;
	SetLookAt(LookAtTarget);
	return true;
}

bool FT4ActionTurnNode::SetRotationTargetLocation()
{
	check(ET4TargetType::TargetLocation == TurnType);
	FVector TargetLocation;
	bool bResult = FindTargetLocationInParameter(TargetLocation, TEXT("SetRotationTargetLocation"));
	if (!bResult)
	{
		return false;
	}
	AT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	FRotator LookAtTarget = OwnerGameObject->GetRotation();
	LookAtTarget.Yaw = FVector(TargetLocation - OwnerGameObject->GetRootLocation()).ToOrientationRotator().Yaw;
	SetLookAt(LookAtTarget);
	return true;
}

bool FT4ActionTurnNode::SetRotationTargetDirection()
{
	check(ET4TargetType::TargetDirection == TurnType);
	FVector TargetDirection;
	bool bResult = FindTargetDirectionInParameter(TargetDirection, TEXT("SetRotationTargetDirection"));
	if (!bResult)
	{
		return false;
	}
	AT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	FRotator LookAtTarget = OwnerGameObject->GetRotation();
	LookAtTarget.Yaw = TargetDirection.Rotation().Yaw;
	SetLookAt(LookAtTarget);
	return true;
}

bool FT4ActionTurnNode::SetRotationTargetYawAngle()
{
	check(ET4TargetType::TargetCustom == TurnType);
	AT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	FRotator LookAtTarget = OwnerGameObject->GetRotation();
	LookAtTarget.Yaw = TargetYawAngle;
	SetLookAt(LookAtTarget);
	return true;
}

void FT4ActionTurnNode::SetLookAt(const FRotator& InRotation)
{
	AT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	const FRotator CurrentRotation = OwnerGameObject->GetRotation();
	if (CurrentRotation.Equals(InRotation))
	{
		return;
	}
	OwnerGameObject->StartTurning(InRotation, RotationYawRate);
}
