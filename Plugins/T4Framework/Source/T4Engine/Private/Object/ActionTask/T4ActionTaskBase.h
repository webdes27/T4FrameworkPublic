// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4EngineStructs.h"

/**
  *
 */
class AT4GameObject;
class IT4GameWorld;
class FT4ActionTaskControl;
class UT4CharacterEntityAsset;
class FT4ActionTaskBase
{
public:
	explicit FT4ActionTaskBase(FT4ActionTaskControl* InControl);
	virtual ~FT4ActionTaskBase();

	void OnAdvance(const FT4UpdateTime& InUpdateTime);

	virtual const ET4ActionType GetType() const = 0;

	virtual bool IsPending() const { return false; } // #116
	virtual void Flush() {} // #111 : 외부에서 강제로 즉시 적용할 경우 호출됨

protected:
	virtual void Advance(const FT4UpdateTime& InUpdateTime) {}

	AT4GameObject* GetGameObject() const;
	IT4GameWorld* GetGameWorld() const;

	const UT4CharacterEntityAsset* GetChracterEntityAsset() const;

protected:
	FT4ActionTaskControl* ManualControlRef;
};
