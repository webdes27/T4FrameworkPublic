// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
  *
 */
class AT4GameObject;
class IT4GameWorld;
class FT4ManualControl;
class UT4CharacterEntityAsset;
class FT4ManualNode
{
public:
	explicit FT4ManualNode(FT4ManualControl* InControl);
	virtual ~FT4ManualNode();

	void OnAdvance(const FT4UpdateTime& InUpdateTime);

	virtual const ET4ActionType GetType() const = 0;

protected:
	virtual void Advance(const FT4UpdateTime& InUpdateTime) {}

	AT4GameObject* GetGameObject() const;
	IT4GameWorld* GetGameWorld() const;

	const UT4CharacterEntityAsset* GetChracterEntityAsset() const;

protected:
	FT4ManualControl* ManualControlRef;
};
