// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ManualNode.h"

#include "Object/Action/T4ManualControl.h"

#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ManualNode::FT4ManualNode(FT4ManualControl* InControl)
	: ManualControlRef(InControl)
{
	check(nullptr != ManualControlRef);
}

FT4ManualNode::~FT4ManualNode()
{
}

void FT4ManualNode::OnAdvance(const FT4UpdateTime& InUpdateTime)
{
	Advance(InUpdateTime);
}

AT4GameObject* FT4ManualNode::GetGameObject() const
{
	check(nullptr != ManualControlRef);
	return ManualControlRef->GetGameObject();
}

IT4GameWorld* FT4ManualNode::GetGameWorld() const
{
	return T4EngineWorldGet(GetGameObject()->GetLayerType());
}

const UT4CharacterEntityAsset* FT4ManualNode::GetChracterEntityAsset() const
{
	AT4GameObject* OwnerObject = GetGameObject();
	if (nullptr == OwnerObject)
	{
		return nullptr;
	}
	const UT4EntityAsset* EntityAsset = OwnerObject->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	if (ET4EntityType::Character != EntityAsset->GetEntityType())
	{
		return nullptr;
	}
	return Cast<const UT4CharacterEntityAsset>(EntityAsset);
}