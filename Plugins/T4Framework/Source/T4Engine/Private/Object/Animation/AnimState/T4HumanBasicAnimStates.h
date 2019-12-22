// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4HumanAnimStates.h"

/**
  * #48
 */
class FT4HumanBasicCombatStanceAnimState : public FT4HumanAnimState
{
public:
	explicit FT4HumanBasicCombatStanceAnimState(FT4HumanAnimControl* InAnimControl);
	~FT4HumanBasicCombatStanceAnimState() {}

	FName GetName() const override { return T4AnimStateHumanBasicCombatStanceName; }
	ET4AnimStatePriority GetPriority() const override { return ET4AnimStatePriority::AnimPriority_Low; }

protected:
	void Reset() override;

	void Enter() override;
	void Update(const FT4UpdateTime& InUpdateTime) override;
	void Leave() override;

private:
};

class FT4HumanBasicUnarmedStanceAnimState : public FT4HumanAnimState
{
public:
	explicit FT4HumanBasicUnarmedStanceAnimState(FT4HumanAnimControl* InAnimControl);
	~FT4HumanBasicUnarmedStanceAnimState() {}

	FName GetName() const override { return T4AnimStateHumanBasicUnarmedStanceName; }
	ET4AnimStatePriority GetPriority() const override { return ET4AnimStatePriority::AnimPriority_Low; }

protected:
	void Reset() override;

	void Enter() override;
	void Update(const FT4UpdateTime& InUpdateTime) override;
	void Leave() override;

private:
};

class FT4HumanBasicRunningAnimState : public FT4HumanAnimState
{
public:
	explicit FT4HumanBasicRunningAnimState(FT4HumanAnimControl* InAnimControl);
	~FT4HumanBasicRunningAnimState() {}

	FName GetName() const override { return T4AnimStateHumanBasicRunningName; }
	ET4AnimStatePriority GetPriority() const override { return ET4AnimStatePriority::AnimPriority_Low; }

protected:
	void Reset() override;

	void Enter() override;
	void Update(const FT4UpdateTime& InUpdateTime) override;
	void Leave() override;

private:
};
