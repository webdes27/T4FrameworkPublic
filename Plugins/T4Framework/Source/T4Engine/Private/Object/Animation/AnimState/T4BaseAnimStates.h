// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4EngineDefinitions.h"
#include "Public/T4Engine.h"

/**
  * #47
 */
class FT4BaseAnimState : public IT4AnimState
{
public:
	explicit FT4BaseAnimState()
		: AdvanceTime(0.0f)
	{
	}
	virtual ~FT4BaseAnimState() {}

	virtual FName GetName() const override { return NAME_None; }
	virtual ET4AnimStatePriority GetPriority() const override { return ET4AnimStatePriority::AnimPriority_Low; }

	void OnReset()
	{
		AdvanceTime = 0.0f;
		Reset();
	}

	void OnEnter() override
	{
		OnReset();
		Enter();
	}

	void OnUpdate(const FT4UpdateTime& InUpdateTime) override
	{
		AdvanceTime += InUpdateTime.ScaledTimeSec;
		Update(InUpdateTime);
	}

	void OnLeave() override
	{
		Leave();
	}

protected:
	virtual void Reset() {}

	virtual void Enter() {}
	virtual void Update(const FT4UpdateTime& InUpdateTime) {}
	virtual void Leave() {}

	bool ChangeNextState(FT4BaseAnimControl* InAnimControl, const FName& InNextStateName);

protected:
	float AdvanceTime;
};

// #47
class FT4EmptyAnimState : public FT4BaseAnimState
{
public:
	FT4EmptyAnimState()
	{
	}

	FName GetName() const override { return T4AnimStateEmptyName; }
	ET4AnimStatePriority GetPriority() const override { return ET4AnimStatePriority::AnimPriority_Low; }

protected:
	void Enter() override
	{
	}

	void Update(const FT4UpdateTime& InUpdateTime) override
	{
	}

	void Leave() override
	{
	}
};

class FT4ErrorAnimState : public FT4BaseAnimState
{
public:
	FT4ErrorAnimState()
	{
	}

	FName GetName() const override { return T4AnimStateErrorName; }
	ET4AnimStatePriority GetPriority() const override { return ET4AnimStatePriority::AnimPriority_Low; }

protected:
	void Enter() override
	{
	}

	void Update(const FT4UpdateTime& InUpdateTime) override
	{
	}

	void Leave() override
	{
	}
};