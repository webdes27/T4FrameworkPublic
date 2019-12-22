// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4HumanAnimStates.h"

#include "Object/Animation/AnimInstance/T4BaseAnimInstance.h"
#include "Object/Animation/AnimInstance/T4BaseAnimVariables.h"
#include "Object/T4GameObject.h"

#include "Object/Animation/T4HumanAnimControl.h"

#include "Public/T4Engine.h"

#include "T4EngineInternal.h"

/**
  * #38, #47
 */
FT4HumanAnimState::FT4HumanAnimState(FT4HumanAnimControl* InAnimControl)
	: AnimControl(InAnimControl)
{
}

AT4GameObject* FT4HumanAnimState::GetGameObject()
{
	check(nullptr != AnimControl);
	AT4GameObject* GameObject = AnimControl->GetGameObject();
	check(nullptr != GameObject);
	return GameObject;
}

// #38 : 점프
FT4HumanJumpingAnimState::FT4HumanJumpingAnimState(FT4HumanAnimControl* InAnimControl)
	: FT4HumanAnimState(InAnimControl)
{
	Reset();
}

void FT4HumanJumpingAnimState::Reset()
{
	JumpEndAnimSequenceDurationSec = 0.0f;
	JumpEndAnimInstanceID = INDEX_NONE;
}

void FT4HumanJumpingAnimState::Enter()
{
	check(nullptr != AnimControl);

	FT4AnimParameters PlayAnimParameters;
	PlayAnimParameters.AnimMontageName = T4AnimSetAnimMontageDefaultName;
	PlayAnimParameters.SectionName = T4AnimSetDefaultSectionJumpStartName;
	PlayAnimParameters.BlendInTimeSec = LocomotionBlendInTimeSec;
	FT4AnimInstanceID JumpStartAnimInstanceID = AnimControl->PlayAnimation(PlayAnimParameters);

	JumpEndAnimSequenceDurationSec = AnimControl->GetDurationSec(
		T4AnimSetAnimMontageDefaultName,
		T4AnimSetDefaultSectionJumpEndName
	);
}

void FT4HumanJumpingAnimState::Update(const FT4UpdateTime& InUpdateTime)
{
	check(nullptr != AnimControl);
	AT4GameObject* GameObject = GetGameObject();
	check(nullptr != GameObject);
	if (!GameObject->IsFalling())
	{
		ChangeNextState(AnimControl, AnimControl->GetDefaultUnarmedStanceStateName());
		return;
	}
	if (INDEX_NONE != JumpEndAnimInstanceID)
	{
		return; // 도착 애니가 플레이되었음으로 더이상 진행할 필요가 없다.
	}
	FVector CurrentVelocity = GameObject->GetMovementVelocity();
	if (0.0f > CurrentVelocity.Z)
	{
		if (INDEX_NONE == JumpEndAnimInstanceID)
		{
			if (0.0f < JumpEndAnimSequenceDurationSec)
			{
				const float MaxZDistance = JumpEndAnimSequenceDurationSec * FMath::Abs(CurrentVelocity.Z);
				float LandingDistance = GameObject->GetLaningDistance(MaxZDistance);
				if (0.0f >= LandingDistance)
				{
					return;
				}
				FT4AnimParameters PlayAnimParameters;
				PlayAnimParameters.AnimMontageName = T4AnimSetAnimMontageDefaultName;
				PlayAnimParameters.SectionName = T4AnimSetDefaultSectionJumpEndName;
				JumpEndAnimInstanceID = AnimControl->PlayAnimation(PlayAnimParameters);
			}
		}
	}
}

void FT4HumanJumpingAnimState::Leave()
{
	check(nullptr != AnimControl);
	if (INDEX_NONE != JumpEndAnimInstanceID)
	{
		AnimControl->StopAnimation(JumpEndAnimInstanceID, T4AnimSetBlendTimeSec);
		JumpEndAnimInstanceID = INDEX_NONE;
	}
}

// #46 : 구르기
FT4HumanRollingAnimState::FT4HumanRollingAnimState(FT4HumanAnimControl* InAnimControl)
	: FT4HumanAnimState(InAnimControl)
{
	Reset();
}

void FT4HumanRollingAnimState::Reset()
{
	RollStartAnimInstanceID = INDEX_NONE;
}

void FT4HumanRollingAnimState::Enter()
{
	check(nullptr != AnimControl);
	AT4GameObject* GameObject = GetGameObject();
	check(nullptr != GameObject);

	const FVector PrevVelocityNormal = AnimControl->GetPrevVelocityNormal();
	FVector RollVelocityNormal = AnimControl->GetRollVelocity();
	RollVelocityNormal.Z = 0.0f;
	RollVelocityNormal.Normalize();

	FName RollSectionSelected = NAME_None;

	const FVector FrontNormal = GameObject->GetFrontVector();
	const float FrontDot = FVector::DotProduct(RollVelocityNormal, FrontNormal);
	if (0.85f <= FrontDot)
	{
		RollSectionSelected = T4AnimSetDefaultSectionRollFrontName;
	}
	else if (-0.85f >= FrontDot)
	{
		RollSectionSelected = T4AnimSetDefaultSectionRollBackName;
	}
	else
	{
		const FVector RightNormal = GameObject->GetRightVector();
		float RightDot = FVector::DotProduct(PrevVelocityNormal, RightNormal);
		if (0.85f <= RightDot)
		{
			RollSectionSelected = T4AnimSetDefaultSectionRollRightName;
		}
		else if (-0.85f >= RightDot)
		{
			RollSectionSelected = T4AnimSetDefaultSectionRollLeftName;
		}
		else if (0.0f <= FrontDot)
		{
			if (0.0f <= RightDot)
			{
				RollSectionSelected = T4AnimSetDefaultSectionRollFrontRightName;
			}
			else
			{
				RollSectionSelected = T4AnimSetDefaultSectionRollFrontLeftName;
			}
		}
		else // if (0.0f > FrontDot)
		{
			if (0.0f <= RightDot)
			{
				RollSectionSelected = T4AnimSetDefaultSectionRollBackRightName;
			}
			else
			{
				RollSectionSelected = T4AnimSetDefaultSectionRollBackLeftName;
			}
		}
	}

	if (RollSectionSelected == NAME_None)
	{
		UE_LOG(
			LogT4Engine,
			Verbose,
			TEXT("FT4HumanRollingAnimState : Failed to Enter. Invalid RollSectionName.")
		);
		return;
	}

	FT4AnimParameters PlayAnimParameters;
	PlayAnimParameters.AnimMontageName = T4AnimSetAnimMontageDefaultName;
	PlayAnimParameters.SectionName = RollSectionSelected;
	PlayAnimParameters.BlendInTimeSec = LocomotionBlendInTimeSec;
	RollStartAnimInstanceID = AnimControl->PlayAnimation(PlayAnimParameters);
}

void FT4HumanRollingAnimState::Update(const FT4UpdateTime& InUpdateTime)
{
	check(nullptr != AnimControl);
	if (INDEX_NONE == RollStartAnimInstanceID ||
		AnimControl->IsPlayingAndBlendOutStarted(RollStartAnimInstanceID))
	{
		if (AnimControl->IsMoving())
		{
			// #46 : 구르기 회피기이기 때문에 이동중이라면 바로 Running State 로 전이시켜준다.
			ChangeNextState(AnimControl, AnimControl->GetDefaultRunningStateName());
		}
		else
		{
			// #46 : 기본 스탠스인 왼발 공대로...
			ChangeNextState(AnimControl, AnimControl->GetDefaultCombatStanceStateName());
		}
	}
}

void FT4HumanRollingAnimState::Leave()
{
	check(nullptr != AnimControl);
	if (INDEX_NONE != RollStartAnimInstanceID)
	{
		AnimControl->StopAnimation(RollStartAnimInstanceID, T4AnimSetBlendTimeSec);
		RollStartAnimInstanceID = INDEX_NONE;
	}
}

// #76 : 사망
FT4HumanVoidAnimState::FT4HumanVoidAnimState(FT4HumanAnimControl* InAnimControl)
	: FT4HumanAnimState(InAnimControl)
{
	Reset();
}

void FT4HumanVoidAnimState::Reset()
{
}

void FT4HumanVoidAnimState::Enter()
{
	check(nullptr != AnimControl);
	AT4GameObject* GameObject = GetGameObject();
	check(nullptr != GameObject);
}

void FT4HumanVoidAnimState::Update(const FT4UpdateTime& InUpdateTime)
{
	check(nullptr != AnimControl);
}

void FT4HumanVoidAnimState::Leave()
{
	check(nullptr != AnimControl);
}