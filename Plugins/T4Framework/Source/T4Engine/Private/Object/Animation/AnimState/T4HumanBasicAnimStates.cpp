// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4HumanBasicAnimStates.h"

#include "Object/Animation/AnimInstance/T4BaseAnimInstance.h"
#include "Object/Animation/AnimInstance/T4BaseAnimVariables.h"
#include "Object/T4GameObject.h"

#include "Object/Animation/T4HumanAnimControl.h"

#include "Public/T4Engine.h"

#include "T4EngineInternal.h"

/**
  * #48
 */
FT4HumanBasicCombatStanceAnimState::FT4HumanBasicCombatStanceAnimState(FT4HumanAnimControl* InAnimControl)
	: FT4HumanAnimState(InAnimControl)
{
	Reset();
}

void FT4HumanBasicCombatStanceAnimState::Reset()
{
}

void FT4HumanBasicCombatStanceAnimState::Enter()
{
	check(nullptr != AnimControl);
}

void FT4HumanBasicCombatStanceAnimState::Update(const FT4UpdateTime& InUpdateTime)
{
	check(nullptr != AnimControl);

	if (AnimControl->IsMoving())
	{
		ChangeNextState(AnimControl, T4AnimStateHumanBasicRunningName);
		return;
	}
}

void FT4HumanBasicCombatStanceAnimState::Leave()
{
	check(nullptr != AnimControl);
}

FT4HumanBasicUnarmedStanceAnimState::FT4HumanBasicUnarmedStanceAnimState(FT4HumanAnimControl* InAnimControl)
	: FT4HumanAnimState(InAnimControl)
{
	Reset();
}

void FT4HumanBasicUnarmedStanceAnimState::Reset()
{
}

void FT4HumanBasicUnarmedStanceAnimState::Enter()
{
	check(nullptr != AnimControl);
}

void FT4HumanBasicUnarmedStanceAnimState::Update(const FT4UpdateTime& InUpdateTime)
{
	check(nullptr != AnimControl);

	if (AnimControl->IsMoving())
	{
		ChangeNextState(AnimControl, T4AnimStateHumanBasicRunningName);
		return;
	}
}

void FT4HumanBasicUnarmedStanceAnimState::Leave()
{
	check(nullptr != AnimControl);
}

FT4HumanBasicRunningAnimState::FT4HumanBasicRunningAnimState(FT4HumanAnimControl* InAnimControl)
	: FT4HumanAnimState(InAnimControl)
{
	Reset();
}

void FT4HumanBasicRunningAnimState::Reset()
{
}

void FT4HumanBasicRunningAnimState::Enter()
{
	check(nullptr != AnimControl);
}

void FT4HumanBasicRunningAnimState::Update(const FT4UpdateTime& InUpdateTime)
{
	check(nullptr != AnimControl);
	if (AnimControl->IsStand())
	{
		ChangeNextState(AnimControl, AnimControl->GetDefaultCombatStanceStateName());
	}
}

void FT4HumanBasicRunningAnimState::Leave()
{
	check(nullptr != AnimControl);
}