// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionTaskBase.h"
#include "T4ActionTaskControl.h"

#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionTaskBase::FT4ActionTaskBase(FT4ActionTaskControl* InControl)
	: ManualControlRef(InControl)
{
	check(nullptr != ManualControlRef);
}

FT4ActionTaskBase::~FT4ActionTaskBase()
{
}

void FT4ActionTaskBase::OnAdvance(const FT4UpdateTime& InUpdateTime)
{
	Advance(InUpdateTime);
}

AT4GameObject* FT4ActionTaskBase::GetGameObject() const
{
	check(nullptr != ManualControlRef);
	return ManualControlRef->GetGameObject();
}

IT4GameWorld* FT4ActionTaskBase::GetGameWorld() const
{
	return T4EngineWorldGet(GetGameObject()->GetLayerType());
}

const UT4CharacterEntityAsset* FT4ActionTaskBase::GetChracterEntityAsset() const
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