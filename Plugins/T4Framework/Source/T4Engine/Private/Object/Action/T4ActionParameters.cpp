// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Public/Action/T4ActionParameters.h"
#include "Public/Action/T4ActionKey.h"

#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "T4EngineInternal.h"

T4ENGINE_API const FT4ActionKey FT4ActionKey::EmptyActionKey; // #32
T4ENGINE_API const FT4ActionParameters FT4ActionParameters::DefaultActionParameter; // #32

/**
  *
 */
bool FT4ActionParameters::GetTargetObject(
	ET4LayerType InLayerType,
	IT4GameObject** OutTargetObject, 
	const TCHAR* InDebugString
) const // #28
{ 
	if (!CheckBits(ET4TargetParamBits::ObjectIDBit))
	{
		UE_LOG(
			LogT4Engine,
			Warning,
			TEXT("%s : must be set ActionParameters::SetGameObjectID."),
			(nullptr != InDebugString) ? InDebugString : TEXT("GetTargetObject")
		);
		return false;
	}
	IT4GameWorld* GameWorld = T4EngineWorldGet(InLayerType);
	check(nullptr != GameWorld);
	const FT4ActionTargetParameters& TargetParameters = GetTargetParams();
	IT4GameObject* TargetObject = GameWorld->GetContainer()->FindGameObject(TargetParameters.TargetObjectID);
	if (nullptr == TargetObject)
	{
		UE_LOG(
			LogT4Engine,
			Warning,
			TEXT("%s : TargetObject '%s' Not Found."),
			(nullptr != InDebugString) ? InDebugString : TEXT("GetTargetObject"),
			*(TargetParameters.TargetObjectID.ToString())
		);
		return false;
	}
	*OutTargetObject = TargetObject;
	return true;
}

bool FT4ActionParameters::GetTargetLocation(
	FVector& OutTargetLocation,
	const TCHAR* InDebugString
) const // #28
{
	if (!CheckBits(ET4TargetParamBits::LocationBit))
	{
		UE_LOG(
			LogT4Engine,
			Warning,
			TEXT("%s : must be set ActionParameters::SetTargetLocation."),
			(nullptr != InDebugString) ? InDebugString : TEXT("GetTargetLocation")
		);
		return false;
	}
	const FT4ActionTargetParameters& TargetParameters = GetTargetParams();
	OutTargetLocation = TargetParameters.TargetLocation;
	return true;
}

bool FT4ActionParameters::GetTargetDirection(
	FVector& OutTargetDirection,
	const TCHAR* InDebugString
) const // #28
{
	if (!CheckBits(ET4TargetParamBits::DirectionBit))
	{
		UE_LOG(
			LogT4Engine,
			Warning,
			TEXT("%s : must be set ActionParameters::SetTargetDirection."),
			(nullptr != InDebugString) ? InDebugString : TEXT("GetTargetDirection")
		);
		return false;
	}
	const FT4ActionTargetParameters& TargetParameters = GetTargetParams();
	OutTargetDirection = TargetParameters.TargetDirection;
	return true;
}