// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4BaseAnimStates.h"

// #47 : 일부 스탠스의 경우 조작감 향상을 위해 BlendIn 시간을 좀 더 빠르게 처리한다.
static const float LocomotionBlendInTimeSec = 0.15f;

/**
  * #38, #47
 */
class AT4GameObject;
class FT4HumanAnimControl;
class FT4HumanAnimState : public FT4BaseAnimState
{
public:
	explicit FT4HumanAnimState(FT4HumanAnimControl* InAnimControl);
	virtual ~FT4HumanAnimState() {}

	virtual FName GetName() const override { return NAME_None; }
	virtual ET4AnimStatePriority GetPriority() const override { return ET4AnimStatePriority::AnimPriority_Low; }

protected:
	void Reset() override {};

	void Enter() override {};
	void Update(const FT4UpdateTime& InUpdateTime) override {};
	void Leave() override {};

	AT4GameObject* GetOwnerObject();

protected:
	FT4HumanAnimControl* AnimControl;
};

// #38 : 점프
class FT4HumanJumpingAnimState : public FT4HumanAnimState
{
public:
	explicit FT4HumanJumpingAnimState(FT4HumanAnimControl* InAnimControl);
	~FT4HumanJumpingAnimState() {}

	FName GetName() const override { return T4Const_JumpAnimStateName; }
	ET4AnimStatePriority GetPriority() const override { return ET4AnimStatePriority::AnimPriority_High; }

protected:
	void Reset() override;

	void Enter() override;
	void Update(const FT4UpdateTime& InUpdateTime) override;
	void Leave() override;

private:
	float JumpEndAnimSequenceDurationSec;
	FT4AnimInstanceID JumpEndAnimInstanceID;
};

// #46 : 구르기
class FT4HumanRollingAnimState : public FT4HumanAnimState
{
public:
	explicit FT4HumanRollingAnimState(FT4HumanAnimControl* InAnimControl);
	~FT4HumanRollingAnimState() {}

	FName GetName() const override { return T4Const_RollAnimStateName; }
	ET4AnimStatePriority GetPriority() const override { return ET4AnimStatePriority::AnimPriority_High; }

protected:
	void Reset() override;

	void Enter() override;
	void Update(const FT4UpdateTime& InUpdateTime) override;
	void Leave() override;

private:
	FT4AnimInstanceID RollStartAnimInstanceID;
};

// #76 : 사망
class FT4HumanVoidAnimState : public FT4HumanAnimState
{
public:
	explicit FT4HumanVoidAnimState(FT4HumanAnimControl* InAnimControl);
	~FT4HumanVoidAnimState() {}

	FName GetName() const override { return T4Const_VoidAnimStateName; }
	ET4AnimStatePriority GetPriority() const override { return ET4AnimStatePriority::AnimPriority_High; }

protected:
	void Reset() override;

	void Enter() override;
	void Update(const FT4UpdateTime& InUpdateTime) override;
	void Leave() override;
};
