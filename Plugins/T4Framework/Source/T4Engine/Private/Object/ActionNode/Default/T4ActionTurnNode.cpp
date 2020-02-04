// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionTurnNode.h"

#include "Object/ActionNode/T4ActionNodeControl.h"
#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionTurnNode::FT4ActionTurnNode(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionNodeBase(InControl, InKey)
	, TargetType(ET4TargetType::Default)
	, TargetYawAngle(TNumericLimits<float>::Max())
	, RotationYawRate(0.0f)
{
}

FT4ActionTurnNode::~FT4ActionTurnNode()
{
}

FT4ActionTurnNode* FT4ActionTurnNode::CreateNode(
	FT4ActionNodeControl* InControl,
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

bool FT4ActionTurnNode::Create(const FT4ActionCommand* InAction)
{
	check(ET4ActionType::Turn == InAction->ActionType);
	const FT4TurnAction& ConvAction = *(static_cast<const FT4TurnAction*>(InAction));
	TargetType = ConvAction.TargetType;
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
	switch (TargetType)
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

		default:
			{
				T4_LOG(
					Error,
					TEXT("Unknown TargetType '%u'"),
					uint8(TargetType)
				);
			}
			break;
	}
	if (!bResult)
	{
		if (ET4TargetType::Default != TargetType)
		{
			T4_LOG(
				Warning,
				TEXT("Apply current rotation."),
				uint8(TargetType)
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
	check(ET4TargetType::TargetObject == TargetType);
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
	check(ET4TargetType::TargetLocation == TargetType);
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
	check(ET4TargetType::TargetDirection == TargetType);
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
	check(ET4TargetType::TargetCustom == TargetType);
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
