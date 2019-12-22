// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4HumanAnimStates.h"

/**
  * #38, #47
 */
// #38 : 오른손 잡이 기준 전투 스탠스. 왼발 스탠스에 Addtive Idle 출력 후 일정 시간 뒤에 Unarmed Stance 로 전환
class FT4HumanLocomotionCombatLeftStanceAnimState : public FT4HumanAnimState
{
public:
	explicit FT4HumanLocomotionCombatLeftStanceAnimState(FT4HumanAnimControl* InAnimControl);
	~FT4HumanLocomotionCombatLeftStanceAnimState() {}

	FName GetName() const override { return T4AnimStateHumanLocomotionCombatLFStanceName; }
	ET4AnimStatePriority GetPriority() const override { return ET4AnimStatePriority::AnimPriority_Low; }

protected:
	void Reset() override;

	void Enter() override;
	void Update(const FT4UpdateTime& InUpdateTime) override;
	void Leave() override;

private:
	float FootStanceValue;
	uint32 NumPlayAnimationCount;
	FT4AnimInstanceID IdleAddtiveAnimInstanceID; // #38
	FT4AnimInstanceID UnarmedStanceAnimInstanceID; // #38
};

// #38 : 오른발 스탠스 상태로 Addtive Idle 출력 후 일정 시간 뒤에 Unarmed Stance 로 전환
//       일반적인 애니메이션 시작 포즈가 왼발 스탠스 이기 때문에 전이가 Skill 시전등에서 왼발 스텐스로 바꿔주는 처리가 필요하다.
class FT4HumanLocomotionCombatRightStanceAnimState : public FT4HumanAnimState
{
public:
	explicit FT4HumanLocomotionCombatRightStanceAnimState(FT4HumanAnimControl* InAnimControl);
	~FT4HumanLocomotionCombatRightStanceAnimState() {}

	FName GetName() const override { return T4AnimStateHumanLocomotionCombatRFStanceName; }
	ET4AnimStatePriority GetPriority() const override { return ET4AnimStatePriority::AnimPriority_Low; }

protected:
	void Reset() override;

	void Enter() override;
	void Update(const FT4UpdateTime& InUpdateTime) override;
	void Leave() override;

private:
	float FootStanceValue;
	uint32 NumPlayAnimationCount;
	FT4AnimInstanceID IdleAddtiveAnimInstanceID; // #38
	FT4AnimInstanceID UnarmedStanceAnimInstanceID; // #38
};

// #38 : 비전투 스탠스, Addtive Idle 이 무한 루핑이며, 이동이 있을 경우 RunStart State 로 전이한다.
class FT4HumanLocomotionUnarmedStanceAnimState : public FT4HumanAnimState
{
public:
	explicit FT4HumanLocomotionUnarmedStanceAnimState(FT4HumanAnimControl* InAnimControl);
	~FT4HumanLocomotionUnarmedStanceAnimState() {}

	FName GetName() const override { return T4AnimStateHumanLocomotionUnarmedStanceName; }
	ET4AnimStatePriority GetPriority() const override { return ET4AnimStatePriority::AnimPriority_Low; }

protected:
	void Reset() override;

	void Enter() override;
	void Update(const FT4UpdateTime& InUpdateTime) override;
	void Leave() override;

private:
	FT4AnimInstanceID IdleAddtiveAnimInstanceID; // #38
};

// #44 : 제자리 회전에서 사용되는 State, 현재 발의 위치에 따라 Turn 애니와 Stance State 전환을 처리한다.
//       리소스가 없어 테스트는 못해본 상황 (2019.06.18)
class FT4HumanLocomotionTurningAnimState : public FT4HumanAnimState
{
public:
	explicit FT4HumanLocomotionTurningAnimState(FT4HumanAnimControl* InAnimControl);
	~FT4HumanLocomotionTurningAnimState() {}

	FName GetName() const override { return T4AnimStateHumanLocomotionTurningName; }
	ET4AnimStatePriority GetPriority() const override { return ET4AnimStatePriority::AnimPriority_Low; }

protected:
	void Reset() override;

	void Enter() override;
	void Update(const FT4UpdateTime& InUpdateTime) override;
	void Leave() override;

private:
	ET4TurnAngle GetTurnAngleType(float InCurrentYaw, float InDesiredYaw);

private:
	FT4AnimInstanceID TurningAnimInstanceID;
};

// #38 : 정지에서 이동이 시작될 경우의 State. 이속 보정을 통해 가속 처리를 하고 있으며
//       Run 속도가 되면 Running State 로 전이한다.
class FT4HumanLocomotionRunStartAnimState : public FT4HumanAnimState
{
public:
	explicit FT4HumanLocomotionRunStartAnimState(FT4HumanAnimControl* InAnimControl);
	~FT4HumanLocomotionRunStartAnimState() {}

	FName GetName() const override { return T4AnimStateHumanLocomotionRunStartName; }
	ET4AnimStatePriority GetPriority() const override { return ET4AnimStatePriority::AnimPriority_Low; }

protected:
	void Reset() override;

	void Enter() override;
	void Update(const FT4UpdateTime& InUpdateTime) override;
	void Leave() override;

private:
	float AccelerationTransitionTimeSec; // #38
};

// #38 : 이동중 정지시의 State, Footstep 정보로 이동중 정지 애니를 출력하고, 
//       애니가 끝나면 FootStance Type 에 따라 Combat or Unset State 로 전이한다.
class FT4HumanLocomotionRunStopAnimState : public FT4HumanAnimState
{
public:
	explicit FT4HumanLocomotionRunStopAnimState(FT4HumanAnimControl* InAnimControl);
	~FT4HumanLocomotionRunStopAnimState() {}

	FName GetName() const override { return T4AnimStateHumanLocomotionRunStopName; }
	ET4AnimStatePriority GetPriority() const override { return ET4AnimStatePriority::AnimPriority_Low; }

protected:
	void Reset() override;

	void Enter() override;
	void Update(const FT4UpdateTime& InUpdateTime) override;
	void Leave() override;

private:
	ET4FootStance FootStanceSelected;
	FT4AnimInstanceID RunStopAnimInstanceID; // #38
};

// #38 : 뛰기 State, 정지가 되면 RunStop 으로 전이한다.
class FT4HumanLocomotionRunningAnimState : public FT4HumanAnimState
{
public:
	explicit FT4HumanLocomotionRunningAnimState(FT4HumanAnimControl* InAnimControl);
	~FT4HumanLocomotionRunningAnimState() {}

	FName GetName() const override { return T4AnimStateHumanLocomotionRunningName; }
	ET4AnimStatePriority GetPriority() const override { return ET4AnimStatePriority::AnimPriority_Low; }

protected:
	void Reset() override;

	void Enter() override;
	void Update(const FT4UpdateTime& InUpdateTime) override;
	void Leave() override;
};
