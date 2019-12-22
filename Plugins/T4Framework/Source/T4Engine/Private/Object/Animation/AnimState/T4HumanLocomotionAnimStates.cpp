// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4HumanLocomotionAnimStates.h"

#include "Object/Animation/AnimInstance/T4BaseAnimInstance.h"
#include "Object/Animation/AnimInstance/T4BaseAnimVariables.h"
#include "Object/T4GameObject.h"

#include "Object/Animation/T4HumanAnimControl.h"

#include "Public/T4Engine.h"

#include "T4EngineInternal.h"

/**
  * #38, #47
 */
// #38 : 오른손 잡이 기준 전투 스탠스. 왼발 스탠스에 Addtive Idle 출력 후 일정 시간 뒤에 Unarmed Stance 로 전환
FT4HumanLocomotionCombatLeftStanceAnimState::FT4HumanLocomotionCombatLeftStanceAnimState(FT4HumanAnimControl* InAnimControl)
	: FT4HumanAnimState(InAnimControl)
{
	Reset();
}

void FT4HumanLocomotionCombatLeftStanceAnimState::Reset()
{
	FootStanceValue = 0.0f;
	NumPlayAnimationCount = 0;
	UnarmedStanceAnimInstanceID = INDEX_NONE;
	IdleAddtiveAnimInstanceID = INDEX_NONE;
}

void FT4HumanLocomotionCombatLeftStanceAnimState::Enter()
{
	check(nullptr != AnimControl);
	if (0 >= NumPlayAnimationCount)
	{
		// #47 : Left/Right Combat Stance 에서 FootStance Value 를 보간하여 이후 애니메이션과 자연스러운 블랜딩이 되도록 수정
		FT4MovementAnimVariables* MovementAnimVariables = AnimControl->GetMovementAnimVariables();
		check(nullptr != MovementAnimVariables);
		FootStanceValue = MovementAnimVariables->FootStance;
	}
	FT4AnimParameters IdleAnimParameters;
	IdleAnimParameters.AnimMontageName = T4AnimSetAnimMontageAdditiveName;
	IdleAnimParameters.SectionName = T4AnimSetAdditiveSectionIdleName;
	IdleAddtiveAnimInstanceID = AnimControl->PlayAnimation(IdleAnimParameters);
	NumPlayAnimationCount++;
}

void FT4HumanLocomotionCombatLeftStanceAnimState::Update(const FT4UpdateTime& InUpdateTime)
{
	check(nullptr != AnimControl);

	FootStanceValue += (InUpdateTime.ScaledTimeSec * 4.0f); // 0.25f

	// #47 : Left/Right Combat Stance 에서 FootStance Value 를 보간하여 이후 애니메이션과 자연스러운 블랜딩이 되도록 수정
	FT4MovementAnimVariables* MovementAnimVariables = AnimControl->GetMovementAnimVariables();
	check(nullptr != MovementAnimVariables);
	MovementAnimVariables->FootStance = FMath::Min(T4AnimSetLeftStanceValue, FootStanceValue);

	if (AnimControl->IsMoving())
	{
		ChangeNextState(AnimControl, T4AnimStateHumanLocomotionRunStartName);
		return;
	}
	if (INDEX_NONE != UnarmedStanceAnimInstanceID)
	{
		if (AnimControl->IsPlayingAndBlendOutStarted(UnarmedStanceAnimInstanceID))
		{
			ChangeNextState(AnimControl, AnimControl->GetDefaultUnarmedStanceStateName());
		}
		return;
	}
	if (INDEX_NONE != IdleAddtiveAnimInstanceID)
	{
		if (AnimControl->IsPlayingAndBlendOutStarted(IdleAddtiveAnimInstanceID))
		{
			if (AnimControl->GetMaxFootStanceIdlePlayCount() < NumPlayAnimationCount)
			{
				FT4AnimParameters IdleTransitionAnimParameters;
				IdleTransitionAnimParameters.AnimMontageName = T4AnimSetAnimMontageAdditiveName;
				IdleTransitionAnimParameters.SectionName = T4AnimSetDefaultSectionStanceLFToUnaremdName;
				UnarmedStanceAnimInstanceID = AnimControl->PlayAnimation(IdleTransitionAnimParameters);
			}
			else
			{
				Enter();
			}
		}
	}
}

void FT4HumanLocomotionCombatLeftStanceAnimState::Leave()
{
	check(nullptr != AnimControl);
	FT4MovementAnimVariables* MovementAnimVariables = AnimControl->GetMovementAnimVariables();
	check(nullptr != MovementAnimVariables);
	MovementAnimVariables->FootStance = T4AnimSetLeftStanceValue;
	if (INDEX_NONE != IdleAddtiveAnimInstanceID)
	{
		AnimControl->StopAnimation(IdleAddtiveAnimInstanceID, T4AnimSetBlendTimeSec); // #38
		IdleAddtiveAnimInstanceID = INDEX_NONE;
	}
	if (INDEX_NONE != UnarmedStanceAnimInstanceID)
	{
		AnimControl->StopAnimation(UnarmedStanceAnimInstanceID, T4AnimSetBlendTimeSec); // #38
		UnarmedStanceAnimInstanceID = INDEX_NONE;
	}
}

// #38 : 오른발 스탠스 상태로 Addtive Idle 출력 후 일정 시간 뒤에 Unarmed Stance 로 전환
//       일반적인 애니메이션 시작 포즈가 왼발 스탠스 이기 때문에 전이가 Skill 시전등에서 왼발 스텐스로 바꿔주는 처리가 필요하다.
FT4HumanLocomotionCombatRightStanceAnimState::FT4HumanLocomotionCombatRightStanceAnimState(FT4HumanAnimControl* InAnimControl)
	: FT4HumanAnimState(InAnimControl)
{
	Reset();
}

void FT4HumanLocomotionCombatRightStanceAnimState::Reset()
{
	FootStanceValue = 0.0f;
	NumPlayAnimationCount = 0;
	IdleAddtiveAnimInstanceID = INDEX_NONE;
	UnarmedStanceAnimInstanceID = INDEX_NONE;
}

void FT4HumanLocomotionCombatRightStanceAnimState::Enter()
{
	check(nullptr != AnimControl);
	if (0 >= NumPlayAnimationCount)
	{
		// #47 : Left/Right Combat Stance 에서 FootStance Value 를 보간하여 이후 애니메이션과 자연스러운 블랜딩이 되도록 수정
		FT4MovementAnimVariables* MovementAnimVariables = AnimControl->GetMovementAnimVariables();
		check(nullptr != MovementAnimVariables);
		FootStanceValue = MovementAnimVariables->FootStance;
	}
	FT4AnimParameters IdleAnimParameters;
	IdleAnimParameters.AnimMontageName = T4AnimSetAnimMontageAdditiveName;
	IdleAnimParameters.SectionName = T4AnimSetAdditiveSectionIdleName;
	IdleAddtiveAnimInstanceID = AnimControl->PlayAnimation(IdleAnimParameters);
	NumPlayAnimationCount++;
}

void FT4HumanLocomotionCombatRightStanceAnimState::Update(const FT4UpdateTime& InUpdateTime)
{
	check(nullptr != AnimControl);

	FootStanceValue -= (InUpdateTime.ScaledTimeSec * 4.0f); // 0.25f

	// #47 : Left/Right Combat Stance 에서 FootStance Value 를 보간하여 이후 애니메이션과 자연스러운 블랜딩이 되도록 수정
	FT4MovementAnimVariables* MovementAnimVariables = AnimControl->GetMovementAnimVariables();
	check(nullptr != MovementAnimVariables);
	MovementAnimVariables->FootStance = FMath::Max(T4AnimSetRightStanceValue, FootStanceValue);

	if (AnimControl->IsMoving())
	{
		ChangeNextState(AnimControl, T4AnimStateHumanLocomotionRunStartName);
		return;
	}
	if (INDEX_NONE != UnarmedStanceAnimInstanceID)
	{
		if (AnimControl->IsPlayingAndBlendOutStarted(UnarmedStanceAnimInstanceID))
		{
			ChangeNextState(AnimControl, AnimControl->GetDefaultUnarmedStanceStateName());
		}
		return;
	}
	if (INDEX_NONE != IdleAddtiveAnimInstanceID)
	{
		if (AnimControl->IsPlayingAndBlendOutStarted(IdleAddtiveAnimInstanceID))
		{
			if (AnimControl->GetMaxFootStanceIdlePlayCount() < NumPlayAnimationCount)
			{
				FT4AnimParameters IdleTransitionAnimParameters;
				IdleTransitionAnimParameters.AnimMontageName = T4AnimSetAnimMontageAdditiveName;
				IdleTransitionAnimParameters.SectionName = T4AnimSetDefaultSectionStanceRFToUnaremdlName;
				UnarmedStanceAnimInstanceID = AnimControl->PlayAnimation(IdleTransitionAnimParameters);
			}
			else
			{
				Enter();
			}
		}
	}
}

void FT4HumanLocomotionCombatRightStanceAnimState::Leave()
{
	check(nullptr != AnimControl);
	FT4MovementAnimVariables* MovementAnimVariables = AnimControl->GetMovementAnimVariables();
	check(nullptr != MovementAnimVariables);
	MovementAnimVariables->FootStance = T4AnimSetRightStanceValue;
	if (INDEX_NONE != IdleAddtiveAnimInstanceID)
	{
		AnimControl->StopAnimation(IdleAddtiveAnimInstanceID, T4AnimSetBlendTimeSec); // #38
		IdleAddtiveAnimInstanceID = INDEX_NONE;
	}
	if (INDEX_NONE != UnarmedStanceAnimInstanceID)
	{
		AnimControl->StopAnimation(UnarmedStanceAnimInstanceID, T4AnimSetBlendTimeSec); // #38
		UnarmedStanceAnimInstanceID = INDEX_NONE;
	}
}

// #38 : 비전투 스탠스, Addtive Idle 이 무한 루핑이며, 이동이 있을 경우 RunStart State 로 전이한다.
FT4HumanLocomotionUnarmedStanceAnimState::FT4HumanLocomotionUnarmedStanceAnimState(FT4HumanAnimControl* InAnimControl)
	: FT4HumanAnimState(InAnimControl)
{
	Reset();
}

void FT4HumanLocomotionUnarmedStanceAnimState::Reset()
{
	IdleAddtiveAnimInstanceID = INDEX_NONE;
}

void FT4HumanLocomotionUnarmedStanceAnimState::Enter()
{
	check(nullptr != AnimControl);
	FT4MovementAnimVariables* MovementAnimVariables = AnimControl->GetMovementAnimVariables();
	check(nullptr != MovementAnimVariables);
	MovementAnimVariables->FootStance = T4AnimSetUnarmedStanceValue; // UnarmedStance 로 전환!

	FT4AnimParameters IdleAnimParameters;
	IdleAnimParameters.AnimMontageName = T4AnimSetAnimMontageAdditiveName;
	IdleAnimParameters.SectionName = T4AnimSetAdditiveSectionIdleName;
	IdleAddtiveAnimInstanceID = AnimControl->PlayAnimation(IdleAnimParameters);
}

void FT4HumanLocomotionUnarmedStanceAnimState::Update(const FT4UpdateTime& InUpdateTime)
{
	check(nullptr != AnimControl);
	if (AnimControl->IsMoving())
	{
		ChangeNextState(AnimControl, T4AnimStateHumanLocomotionRunStartName);
		return;
	}
	if (INDEX_NONE != IdleAddtiveAnimInstanceID)
	{
		if (AnimControl->IsPlayingAndBlendOutStarted(IdleAddtiveAnimInstanceID))
		{
			Enter();
		}
	}
}

void FT4HumanLocomotionUnarmedStanceAnimState::Leave()
{
	check(nullptr != AnimControl);
	if (INDEX_NONE != IdleAddtiveAnimInstanceID)
	{
		AnimControl->StopAnimation(IdleAddtiveAnimInstanceID, T4AnimSetBlendTimeSec); // #38
		IdleAddtiveAnimInstanceID = INDEX_NONE;
	}
}

// #44 : 제자리 회전에서 사용되는 State, 현재 발의 위치에 따라 Turn 애니와 Stance State 전환을 처리한다.
//       리소스가 없어 테스트는 못해본 상황 (2019.06.18)
FT4HumanLocomotionTurningAnimState::FT4HumanLocomotionTurningAnimState(FT4HumanAnimControl* InAnimControl)
	: FT4HumanAnimState(InAnimControl)
{
	Reset();
}

void FT4HumanLocomotionTurningAnimState::Reset()
{
	TurningAnimInstanceID = INDEX_NONE;
}

void FT4HumanLocomotionTurningAnimState::Enter()
{
	check(nullptr != AnimControl);
	const float GoalRotationYaw = AnimControl->GetTurnGoalRotation().Yaw;
	const FRotator CurrentRotator = GetGameObject()->GetRotation();
	ET4TurnAngle CurrentTurnAngleType = GetTurnAngleType(CurrentRotator.Yaw, GoalRotationYaw);

	const uint32 NumTurnAngles = ET4TurnAngle::TurnAngle_Back + 1;
	static const FName TurnSectionNames[ET4FootStance::FootStance_Nums][NumTurnAngles] =
	{
		{ T4AnimSetDefaultSectionTurnLFToLeft90Name, T4AnimSetDefaultSectionTurnLFToRight90Name, T4AnimSetDefaultSectionTurnLFToRight180Name },
		{ T4AnimSetDefaultSectionTurnRFToLeft90Name, T4AnimSetDefaultSectionTurnRFToRight90Name, T4AnimSetDefaultSectionTurnRFToLeft180Name },
		{ NAME_None, NAME_None, NAME_None },
	};

	FT4MovementAnimVariables* MovementAnimVariables = AnimControl->GetMovementAnimVariables();
	check(nullptr != MovementAnimVariables);
	ET4FootStance FootStanceType = ET4FootStance::FootStance_Unarmed;
	if (T4AnimSetLeftStanceValue == MovementAnimVariables->FootStance)
	{
		FootStanceType = ET4FootStance::FootStance_Left;
	}
	else if (T4AnimSetRightStanceValue == MovementAnimVariables->FootStance)
	{
		FootStanceType = ET4FootStance::FootStance_Right;
	}

	FT4AnimParameters PlayAnimParameters;
	PlayAnimParameters.AnimMontageName = T4AnimSetAnimMontageAdditiveName;
	PlayAnimParameters.SectionName = TurnSectionNames[FootStanceType][CurrentTurnAngleType];
	PlayAnimParameters.BlendInTimeSec = LocomotionBlendInTimeSec;
	TurningAnimInstanceID = AnimControl->PlayAnimation(PlayAnimParameters);
}

void FT4HumanLocomotionTurningAnimState::Update(const FT4UpdateTime& InUpdateTime)
{
	check(nullptr != AnimControl);
	if (AnimControl->IsMoveStop())
	{
		ChangeNextState(AnimControl, T4AnimStateHumanLocomotionRunStopName);
		return;
	}
	else if (AnimControl->IsMoveStart())
	{
		ChangeNextState(AnimControl, T4AnimStateHumanLocomotionRunStartName);
		return;
	}
	else if (AnimControl->IsMoving())
	{
		ChangeNextState(AnimControl, AnimControl->GetDefaultRunningStateName());
		return;
	}
	if (INDEX_NONE == TurningAnimInstanceID)
	{
		ChangeNextState(AnimControl, AnimControl->GetDefaultCombatStanceStateName());
		return;
	}
	if (INDEX_NONE != TurningAnimInstanceID)
	{
		if (AnimControl->IsPlayingAndBlendOutStarted(TurningAnimInstanceID))
		{
			FT4MovementAnimVariables* MovementAnimVariables = AnimControl->GetMovementAnimVariables();
			check(nullptr != MovementAnimVariables);
			ET4FootStance FootStanceType = ET4FootStance::FootStance_Unarmed;
			if (0.0f < MovementAnimVariables->FootStance)
			{
				ChangeNextState(AnimControl, AnimControl->GetDefaultCombatStanceStateName());
			}
			else if (0.0f > MovementAnimVariables->FootStance)
			{
				ChangeNextState(AnimControl, T4AnimStateHumanLocomotionCombatRFStanceName);
			}
			else
			{
				ChangeNextState(AnimControl, AnimControl->GetDefaultUnarmedStanceStateName());
			}
		}
	}
}

void FT4HumanLocomotionTurningAnimState::Leave()
{
	check(nullptr != AnimControl);
	if (INDEX_NONE != TurningAnimInstanceID)
	{
		AnimControl->StopAnimation(TurningAnimInstanceID, T4AnimSetBlendTimeSec);
		TurningAnimInstanceID = INDEX_NONE;
	}
}

ET4TurnAngle FT4HumanLocomotionTurningAnimState::GetTurnAngleType(
	float InCurrentYaw,
	float InDesiredYaw
)
{
	// todo : optimizing
	const FVector CurrentVector = FRotator(0.0f, InCurrentYaw, 0.0f).Vector();
	const FVector OldVector = FRotator(0.0f, InDesiredYaw, 0.0f).Vector();
	const float FrontDot = FVector::DotProduct(CurrentVector, OldVector);
	if (FrontDot > 0.99f)
	{
		return ET4TurnAngle::TurnAngle_Same;
	}
	else if (FrontDot < -0.75f)
	{
		return ET4TurnAngle::TurnAngle_Back;
	}
	InCurrentYaw = FRotator::ClampAxis(InCurrentYaw);
	InDesiredYaw = FRotator::ClampAxis(InDesiredYaw);
	if (InCurrentYaw > InDesiredYaw)
	{
		return ET4TurnAngle::TurnAngle_Right;
	}
	return ET4TurnAngle::TurnAngle_Left;
}

// #38 : 정지에서 이동이 시작될 경우의 State. 이속 보정을 통해 가속 처리를 하고 있으며
//       Run 속도가 되면 Running State 로 전이한다.
FT4HumanLocomotionRunStartAnimState::FT4HumanLocomotionRunStartAnimState(FT4HumanAnimControl* InAnimControl)
	: FT4HumanAnimState(InAnimControl)
{
	Reset();
}

void FT4HumanLocomotionRunStartAnimState::Reset()
{
	AccelerationTransitionTimeSec = 0.0f;
}

void FT4HumanLocomotionRunStartAnimState::Enter()
{
	AT4GameObject* GameObject = GetGameObject();
	check(nullptr != GameObject);
	const float MinAccelerationScale = AnimControl->GetMinAccelerationScale();
	GameObject->SetAccelerationMoveSpeedScale(MinAccelerationScale);
}

void FT4HumanLocomotionRunStartAnimState::Update(const FT4UpdateTime& InUpdateTime)
{
	check(nullptr != AnimControl);
	AccelerationTransitionTimeSec += InUpdateTime.ScaledTimeSec;
	AT4GameObject* GameObject = GetGameObject();
	check(nullptr != GameObject);
	const float MaxAcceleratedMoveTimeSec = AnimControl->GetMaxAcceleratedMoveTimeSec();
	if (MaxAcceleratedMoveTimeSec <= AccelerationTransitionTimeSec)
	{
		ChangeNextState(AnimControl, AnimControl->GetDefaultRunningStateName());
	}
	else
	{
		const float MinAccelerationScale = AnimControl->GetMinAccelerationScale();
		const float TransitionAccelRatio = AccelerationTransitionTimeSec / MaxAcceleratedMoveTimeSec;
		GameObject->SetAccelerationMoveSpeedScale(FMath::Max(MinAccelerationScale, TransitionAccelRatio));
	}
}

void FT4HumanLocomotionRunStartAnimState::Leave()
{
	AT4GameObject* GameObject = GetGameObject();
	check(nullptr != GameObject);
	GameObject->SetAccelerationMoveSpeedScale(1.0f);
}

// #38 : 이동중 정지시의 State, Footstep 정보로 이동중 정지 애니를 출력하고, 
//       애니가 끝나면 FootStance Type 에 따라 Combat or Unset State 로 전이한다.
FT4HumanLocomotionRunStopAnimState::FT4HumanLocomotionRunStopAnimState(FT4HumanAnimControl* InAnimControl)
	: FT4HumanAnimState(InAnimControl)
{
	Reset();
}

void FT4HumanLocomotionRunStopAnimState::Reset()
{
	FootStanceSelected = ET4FootStance::FootStance_Unarmed;
	RunStopAnimInstanceID = INDEX_NONE;
}

void FT4HumanLocomotionRunStopAnimState::Enter()
{
	check(nullptr != AnimControl);

	AT4GameObject* GameObject = GetGameObject();
	check(nullptr != GameObject);

	FT4MovementAnimVariables* MovementAnimVariables = AnimControl->GetMovementAnimVariables();
	check(nullptr != MovementAnimVariables);

	const FVector PrevVelocityNormal = AnimControl->GetPrevVelocityNormal();

	FName RunStopSectionNameSelected = NAME_None;

	{
		// #44 : 일반 이동은 좌/우/기본 스탠스를 사용한다.
		FootStanceSelected = ET4FootStance::FootStance_Left;
		if (ET4FootstepType::Footstep_Right == AnimControl->GetLastFootstepType())
		{
			FootStanceSelected = ET4FootStance::FootStance_Right;
		}

		static const FName FootStanceSectionNames[ET4MoveAngle::MoveAngle_Nums][ET4FootStance::FootStance_Nums] =
		{
			{ T4AnimSetDefaultSectionStanceLFFromFrontName, T4AnimSetDefaultSectionStanceRFFromFrontName, NAME_None },
			{ T4AnimSetDefaultSectionStanceLFFromBackName, T4AnimSetDefaultSectionStanceRFFromBackName, NAME_None },
			{ T4AnimSetDefaultSectionStanceLFFromLeftName, T4AnimSetDefaultSectionStanceRFFromLeftName, NAME_None },
			{ T4AnimSetDefaultSectionStanceLFFromRightName, T4AnimSetDefaultSectionStanceRFFromRightName, NAME_None },
		};

		// NOTE : 전진/후진 정지는 Footstep 값에 따라 기준발
		//        전진 대각선 이동시 정지는 이동 방향 기준발
		//        수평 및 후진 대각선 이동시 정지는 이동 방향 반대발 기준

		float FootStanceRightDot = 0.0f;
		const FVector FrontNormal = GameObject->GetFrontVector();
		const float FootStanceFrontDot = FVector::DotProduct(PrevVelocityNormal, FrontNormal);
		if (0.85f <= FootStanceFrontDot)
		{
			RunStopSectionNameSelected = FootStanceSectionNames[MoveAngle_Front][FootStanceSelected];
		}
		else if (-0.85f >= FootStanceFrontDot)
		{
			RunStopSectionNameSelected = FootStanceSectionNames[MoveAngle_Back][FootStanceSelected];
		}
		else
		{
			const FVector RightNormal = GameObject->GetRightVector();
			FootStanceRightDot = FVector::DotProduct(PrevVelocityNormal, RightNormal);
			if (0.25f < FootStanceFrontDot)
			{
				// 전진 대각선으로 뛸 경우 이동 방향의 발이 기준발이 되도록 처리 
				if (0.0f <= FootStanceRightDot)
				{
					FootStanceSelected = ET4FootStance::FootStance_Right;
				}
				else
				{
					FootStanceSelected = ET4FootStance::FootStance_Left;
				}
			}
			else
			{
				// 수평 및 후진 대각선으로 뛸 경우 이동 방향의 반대 발이 기준발이 되도록 처리
				if (0.0f <= FootStanceRightDot)
				{
					FootStanceSelected = ET4FootStance::FootStance_Left;
				}
				else
				{
					FootStanceSelected = ET4FootStance::FootStance_Right;
				}
			}
			if (0.0f <= FootStanceRightDot)
			{
				RunStopSectionNameSelected = FootStanceSectionNames[MoveAngle_Right][FootStanceSelected];
			}
			else
			{
				RunStopSectionNameSelected = FootStanceSectionNames[MoveAngle_Left][FootStanceSelected];
			}
		}
	}

	if (RunStopSectionNameSelected == NAME_None)
	{
		return;
	}

	FT4AnimParameters PlayAnimParameters;
	PlayAnimParameters.AnimMontageName = T4AnimSetAnimMontageAdditiveName;
	PlayAnimParameters.SectionName = RunStopSectionNameSelected;
	PlayAnimParameters.BlendInTimeSec = LocomotionBlendInTimeSec;
	RunStopAnimInstanceID = AnimControl->PlayAnimation(PlayAnimParameters);
}

void FT4HumanLocomotionRunStopAnimState::Update(const FT4UpdateTime& InUpdateTime)
{
	check(nullptr != AnimControl);
	if (AnimControl->IsMoving())
	{
		ChangeNextState(AnimControl, T4AnimStateHumanLocomotionRunStartName);
		return;
	}
	if (INDEX_NONE == RunStopAnimInstanceID)
	{
		ChangeNextState(AnimControl, AnimControl->GetDefaultCombatStanceStateName());
		return;
	}
	if (INDEX_NONE != RunStopAnimInstanceID)
	{
		if (AnimControl->IsPlayingAndBlendOutStarted(RunStopAnimInstanceID))
		{
			FT4MovementAnimVariables* MovementAnimVariables = AnimControl->GetMovementAnimVariables();
			check(nullptr != MovementAnimVariables);
			if (ET4FootStance::FootStance_Right == FootStanceSelected)
			{
				MovementAnimVariables->FootStance = T4AnimSetRightStanceValue;
				ChangeNextState(AnimControl, T4AnimStateHumanLocomotionCombatRFStanceName);
			}
			else
			{
				MovementAnimVariables->FootStance = T4AnimSetLeftStanceValue;
				ChangeNextState(AnimControl, AnimControl->GetDefaultCombatStanceStateName());
			}
		}
	}
}

void FT4HumanLocomotionRunStopAnimState::Leave()
{
	check(nullptr != AnimControl);
	if (INDEX_NONE != RunStopAnimInstanceID)
	{
		AnimControl->StopAnimation(RunStopAnimInstanceID, T4AnimSetBlendTimeSec); // #38
		RunStopAnimInstanceID = INDEX_NONE;
	}
}

// #38 : 뛰기 State, 정지가 되면 RunStop 으로 전이한다.
FT4HumanLocomotionRunningAnimState::FT4HumanLocomotionRunningAnimState(FT4HumanAnimControl* InAnimControl)
	: FT4HumanAnimState(InAnimControl)
{
	Reset();
}

void FT4HumanLocomotionRunningAnimState::Reset()
{

}

void FT4HumanLocomotionRunningAnimState::Enter()
{

}

void FT4HumanLocomotionRunningAnimState::Update(const FT4UpdateTime& InUpdateTime)
{
	check(nullptr != AnimControl);
	if (AnimControl->IsMoveStop())
	{
		ChangeNextState(AnimControl, T4AnimStateHumanLocomotionRunStopName);
	}
	else if (AnimControl->IsStand())
	{
		ChangeNextState(AnimControl, AnimControl->GetDefaultCombatStanceStateName());
	}
}

void FT4HumanLocomotionRunningAnimState::Leave()
{

}
