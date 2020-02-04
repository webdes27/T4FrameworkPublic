// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4HumanAnimControl.h"

#include "Object/Animation/AnimInstance/T4BaseAnimInstance.h"
#include "Object/Animation/AnimInstance/T4BaseAnimVariables.h"
#include "Object/T4GameObject.h"

#include "AnimState/T4HumanBasicAnimStates.h" // #48

#include "Public/T4Engine.h"

#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h" // #38

#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif

#include "T4EngineInternal.h"

// CVars
namespace T4HumanAnimControlCVars
{
#if !UE_BUILD_SHIPPING

	static float IKFootLeftOffset = 0.0f;
	static FVector IKFootLeftJointOffset = FVector::ZeroVector;

	static float IKFootRightOffset = 0.0f;
	static FVector IKFootRightJointOffset = FVector::ZeroVector;

	static float IKFootCOMOffset = 0.0f;
	static int32 IKFootReset = 0;
	static int32 IKFootDebug = 0;
	static int32 IKFootDisable = 0;

	FAutoConsoleVariableRef CVarAnimLeftFootIKOffset(
		TEXT("t4.Debug.Ik.LFootOffset"),
		IKFootLeftOffset,
		TEXT("Float Offset"),
		ECVF_Default
	);
	FAutoConsoleVariableRef CVarAnimLeftFootIKJointOffsetX(
		TEXT("t4.Debug.Ik.LFootJointOffset.X"),
		IKFootLeftJointOffset.X,
		TEXT("Float Offset"),
		ECVF_Default
	);

	FAutoConsoleVariableRef CVarAnimLeftFootIKJointOffsetY(
		TEXT("t4.Debug.Ik.LFootJointOffset.Y"),
		IKFootLeftJointOffset.Y,
		TEXT("Float Offset"),
		ECVF_Default
	);
	FAutoConsoleVariableRef CVarAnimLeftFootIKJointOffsetZ(
		TEXT("t4.Debug.Ik.LFootJointOffset.Z"),
		IKFootLeftJointOffset.Z,
		TEXT("Float Offset"),
		ECVF_Default
	);

	FAutoConsoleVariableRef CVarAnimRightFootIKOffset(
		TEXT("t4.Debug.Ik.RFootOffset"),
		IKFootRightOffset,
		TEXT("Float Offset"),
		ECVF_Default
	);
	FAutoConsoleVariableRef CVarAnimRightFootIKJointOffsetX(
		TEXT("t4.Debug.Ik.RFootJointOffset.X"),
		IKFootRightJointOffset.X,
		TEXT("Float Offset"),
		ECVF_Default
	);
	FAutoConsoleVariableRef CVarAnimRightFootIKJointOffsetY(
		TEXT("t4.Debug.Ik.RFootJointOffset.Y"),
		IKFootRightJointOffset.Y,
		TEXT("Float Offset"),
		ECVF_Default
	);
	FAutoConsoleVariableRef CVarAnimRightFootIKJointOffsetZ(
		TEXT("t4.Debug.Ik.RFootJointOffset.Z"),
		IKFootRightJointOffset.Z,
		TEXT("Float Offset"),
		ECVF_Default
	);

	FAutoConsoleVariableRef CVarAnimCOMOffset(
		TEXT("t4.Debug.Ik.COMOffset"),
		IKFootCOMOffset,
		TEXT("Float Offset"),
		ECVF_Default
	);
	FAutoConsoleVariableRef CVarAnimIKDebugReset(
		TEXT("t4.Debug.Ik.Reset"),
		IKFootReset,
		TEXT("true or false"),
		ECVF_Default
	);
	FAutoConsoleVariableRef CVarAnimIKDebug(
		TEXT("t4.Debug.Ik.Debug"),
		IKFootDebug,
		TEXT("true or False"),
		ECVF_Default
	);
	FAutoConsoleVariableRef CVarAnimIKDisable(
		TEXT("t4.Debug.Ik.Disable"),
		IKFootDisable,
		TEXT("true or False"),
		ECVF_Default
	);

#endif // !UE_BUILD_SHIPPING
}

static const FVector DefaultLeftFootJointTarget = FVector(0.0f, 100.0f, -25.0f);
static const FVector DefaultRightFootJointTarget = FVector(0.0f, 100.0f, 25.0f);

static const FName LeftFootSocketName = TEXT("foot_l");
static const FName RightFootSocketName = TEXT("foot_r");

/**
  *
 */
FT4HumanAnimControl::FT4HumanAnimControl(AT4GameObject* InGameObject)
	: FT4BaseAnimControl(InGameObject)
	, DefaultUnarmedStanceStateName(NAME_None) // #48
	, DefaultCombatStanceStateName(NAME_None) // #48
	, DefaultRunningStateName(NAME_None) // #48
	, CurrentVelocityNormal(FVector::ZeroVector)
	, PrevVelocityNormal(FVector::ZeroVector) // #38
	, JumpVelocity(FVector::ZeroVector) // #46
	, RollVelocity(FVector::ZeroVector) // #46
	, TurnGoalRotation(FRotator::ZeroRotator) // #47
{
	YawAngleLerp.Initialize(2000.0f, 0.0f); // #113
	MoveSpeedLerp.Initialize(0.25f, 0.0f); // #113
	AimYawAngleLerp.Initialize(0.2f, 0.0f); // #113
	AimPitchAngleLerp.Initialize(0.2f, 0.0f); // #113
}

FT4HumanAnimControl::~FT4HumanAnimControl()
{
}

void FT4HumanAnimControl::BeginPlay()
{
	check(!bBegunPlay);

	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	check(nullptr != AnimInstance);

	AnimInstance->GetOnAnimNotify().BindRaw(
		this,
		&FT4HumanAnimControl::HandleOnAnimNotify
	); // #38

	// #18 : Foot Bone 의 Axis 에 따라 Offset 을 달리 적용해주어야 함
	//       즉, 모델에 따라 L/R Foot Bone 의 ZAxis 가 틀려 Offset 계산도 이를 감안해 처리할 필요가 있음
	//       Anim BP 에서 처리하기 보다는 1회 계산 후 기준을 잡는 것으로 처리하겠음
	//       단, Bone 생성 룰을 맞추면 Assert 조건으로 처리해도 무방하나, T4 는 범용(?)을 추구함으로
	//       이 처리를 유지하는 것으로 하겠음. (단, Z말고 다른 축이라면???)

	FRotator LeftFootSocketRotation = FRotator::ZeroRotator;
	OwnerObjectPtr->GetSocketRotation(LeftFootSocketName, RTS_Component, LeftFootSocketRotation);
	FVector LeftFootUpVector = LeftFootSocketRotation.RotateVector(FVector::ForwardVector);
	if (-0.5f > FVector::DotProduct(LeftFootUpVector, FVector::UpVector))
	{
		FootIKInfo.bInverseLeftFootOffset = true;
	}

	FRotator RightFootSocketRotation = FRotator::ZeroRotator;
	OwnerObjectPtr->GetSocketRotation(RightFootSocketName, RTS_Component, RightFootSocketRotation);
	FVector RightFootUpVector = RightFootSocketRotation.RotateVector(FVector::ForwardVector);
	if (-0.5f > FVector::DotProduct(RightFootUpVector, FVector::UpVector))
	{
		FootIKInfo.bInverseRightFootOffset = true;
	}

	bBegunPlay = true;

	OnAutoRegisterAnimStates(); // #47
}

void FT4HumanAnimControl::AutoRegisterAnimStates() // #47
{
	FT4BaseAnimControl::AutoRegisterAnimStates();

	// #47
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	check(nullptr != AnimInstance);
	const ET4AnimInstance AnimInstanceType = AnimInstance->GetAnimInstanceType();
	switch (AnimInstanceType)
	{
		case ET4AnimInstance::Human_Basic: // #38
			{
				RegisterAnimState(T4Const_JumpAnimStateName, new FT4HumanJumpingAnimState(this));
				RegisterAnimState(T4Const_RollAnimStateName, new FT4HumanRollingAnimState(this)); // #46

				RegisterAnimState(T4Const_VoidAnimStateName, new FT4HumanVoidAnimState(this)); // #76

				RegisterAnimState(T4Const_CombatStanceAnimStateName, new FT4HumanBasicCombatStanceAnimState(this)); // #48
				RegisterAnimState(T4Const_UnarmedStanceAnimStateName, new FT4HumanBasicUnarmedStanceAnimState(this)); // #48
				RegisterAnimState(T4Const_RunAnimStateName, new FT4HumanBasicRunningAnimState(this)); // #48

				DefaultUnarmedStanceStateName = T4Const_UnarmedStanceAnimStateName; // #48
				DefaultCombatStanceStateName = T4Const_CombatStanceAnimStateName; // #48
				DefaultRunningStateName = T4Const_RunAnimStateName; // #48

				ActiveAnimStateName = DefaultUnarmedStanceStateName;
			}
			break;

		default:
			{
				T4_LOG(
					Error,
					TEXT("Unknown anim instance type '%u'"),
					uint32(AnimInstanceType)
				);
			}
			break;
	}
}

void FT4HumanAnimControl::NotifyPlayAnimation(
	const FT4AnimParameters& InAnimParameters
) // #47
{
	if (T4Const_SkillAnimMontageName == InAnimParameters.AnimMontageName)
	{
#if 0 // #106 : Locomotion 삭제 (#38)
		const FName ActiveAnimState = GetActiveAnimStateName();
		if (T4AnimStateHumanLocomotionCombatRFStanceName == ActiveAnimState ||
			T4AnimStateHumanLocomotionUnarmedStanceName == ActiveAnimState)
		{
			// Idle 일 경우에만 Skill 애니류 사용시 Combat Idle 로 자동 변경...
			TryChangeAnimState(T4AnimStateHumanLocomotionCombatLFStanceName, false, true);
		}
#endif
	}
}

void FT4HumanAnimControl::SetAnimSetAsset(UT4AnimSetAsset* InAnimSetAsset)
{
	check(nullptr != InAnimSetAsset);

	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	check(nullptr != AnimInstance);
	AnimInstance->SetAnimSetAsset(InAnimSetAsset);
}

bool FT4HumanAnimControl::DoJump(const FVector& InVelocity) // #46
{
	JumpVelocity = InVelocity;
	bool bResult = TryChangeAnimState(T4Const_JumpAnimStateName, false, false); // #47
	return bResult;
}

bool FT4HumanAnimControl::DoRoll(const FVector& InVelocity) // #46
{
	RollVelocity = InVelocity;
	bool bResult = TryChangeAnimState(T4Const_RollAnimStateName, false, false); // #47
	return bResult;
}

bool FT4HumanAnimControl::DoTurn(const FRotator& InRotation) // #47
{
	TurnGoalRotation = InRotation;
#if 0 // #106 : Locomotion 삭제 (#38)
	if (!HasAnimState(T4AnimStateHumanLocomotionTurningName))
	{
		return true; // ET4AnimInstance::Human_Basic 일 경우는 없을 수 있음으로 예외 처리
	}
	bool bResult = TryChangeAnimState(T4AnimStateHumanLocomotionTurningName, false, true); // #47
	return bResult;
#else
	return true;
#endif
}

bool FT4HumanAnimControl::DoVoid() // #76
{
	bool bResult = TryChangeAnimState(T4Const_VoidAnimStateName, false, true);
	return bResult;
}

void FT4HumanAnimControl::Reset()
{
	// #38
	ResetFootIK();

	DefaultUnarmedStanceStateName = NAME_None; // #48
	DefaultCombatStanceStateName = NAME_None; // #48
	DefaultRunningStateName = NAME_None; // #48

	CurrentVelocityNormal = FVector::ZeroVector;
	PrevVelocityNormal = FVector::ZeroVector; // #38

	// #46
	JumpVelocity = FVector::ZeroVector;
	RollVelocity = FVector::ZeroVector;
	// #46
	TurnGoalRotation = FRotator::ZeroRotator; // #47
}

void FT4HumanAnimControl::Advance(const FT4UpdateTime& InUpdateTime)
{
	check(bBegunPlay);
	check(OwnerObjectPtr.IsValid());

	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	check(nullptr != AnimInstance);
	{
		AnimInstance->SetPause(InUpdateTime.bPaused); // #102
		AnimInstance->SetTimeScale(InUpdateTime.TimeScale); // #102
	}

	FT4StateAnimVariables* StateAnimVariables = GetStateAnimVariables();
	check(nullptr != StateAnimVariables);

	StateAnimVariables->bIsCombat = OwnerObjectPtr->IsCombat(); // #106 : Combat 만 우선 임시처리
	StateAnimVariables->bIsFalling = OwnerObjectPtr->IsFalling();
	StateAnimVariables->bIsLockOn = OwnerObjectPtr->IsLockOn();
	StateAnimVariables->bIsAiming = OwnerObjectPtr->IsAiming(); // #113

	FT4MovementAnimVariables* MovementAnimVariables = GetMovementAnimVariables();
	check(nullptr != MovementAnimVariables);

	PrevVelocityNormal = CurrentVelocityNormal;
	CurrentVelocityNormal = OwnerObjectPtr->GetMovementVelocity(); // #38
	CurrentVelocityNormal.Normalize();

	bool bMoveStart = false; // #113

	// Movement BS
	{
		float OldMoveSpeed = MoveSpeedLerp.GetCurrentValue();
		const float CurrentMoveSpeed = OwnerObjectPtr->GetMovementSpeed();
		MoveSpeedLerp.TrySetGoalValue(CurrentMoveSpeed); // #106 : 이속으로 변경함 (#38 에서는 4단계 Value 를 사용했었음)
		MovementAnimVariables->MoveSpeed = MoveSpeedLerp.UpdateValue(InUpdateTime.ScaledTimeSec);
		if (0.0f >= OldMoveSpeed && 0.0f < MovementAnimVariables->MoveSpeed)
		{
			bMoveStart = true;
		}
	}

	// Movement BS
	{
		float UpdateGoalYawAngle = 0.0f;
		if (StateAnimVariables->bIsLockOn)
		{
			const float CurrentValue = YawAngleLerp.GetCurrentAngle();
			FRotator ActorRotation = OwnerObjectPtr->GetRotation();
			UpdateGoalYawAngle = AnimInstance->CalculateDirection(CurrentVelocityNormal, ActorRotation);
			if (0.0f > CurrentValue && 179.0f <= UpdateGoalYawAngle ||
				0.0f < CurrentValue && -179.0f >= UpdateGoalYawAngle)
			{
				// #113 : -180, 180 으로 맞춰주기 때문에 보간을 해버리면 Angle 이 튐으로 보정처리 해준다.
				UpdateGoalYawAngle = -UpdateGoalYawAngle;
			}
			if (bMoveStart)
			{
				YawAngleLerp.SetGoalAndCurrentAngle(UpdateGoalYawAngle);
			}
			else
			{
				if (46.0f < FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentValue, UpdateGoalYawAngle)))
				{
					// #113 : 이동중, Current & Goal 간 차이가 45 도 이상이면 즉시 회전하도록 처리한다.
					//        좌 <=> 우, 전 <=> 후 이동 같은 경우 중간 모션이 보이면 어색하다.
					YawAngleLerp.SetGoalAndCurrentAngle(UpdateGoalYawAngle);
				}
				else
				{
					YawAngleLerp.TrySetGoalAngle(UpdateGoalYawAngle);
				}
			}
		}
		else
		{
			if (46.0f < FMath::Abs(YawAngleLerp.GetCurrentAngle())) // Lock On 이 끝나 정면(Angle=0)을 보기 때문에 Current 만 확인하면 된다.
			{
				YawAngleLerp.SetGoalAndCurrentAngle(0.0f); // 45도 이상 복귀각이 크면 즉시 적용하도록
			}
			else
			{
				YawAngleLerp.TrySetGoalAngle(0.0f); // 45도 이하면 보간하여 정면으로
			}
		}
		MovementAnimVariables->YawAngle = YawAngleLerp.UpdateAngle(InUpdateTime.ScaledTimeSec);
#if 0
		if (OwnerObjectPtr->HasPlayer() && 0.0f != MovementAnimVariables->YawAngle)
		{
			T4_LOG(
				Warning, 
				TEXT("[%s] MoveSpeed = %.2f, YawAngle = %.2f (UpdateGoalYawAngle = %.2f, Vel = %.2f, %.2f, ActorYaw = %.2f"), 
				(StateAnimVariables->bIsLockOn) ? TEXT("LockOn") : TEXT("LockOff"),
				MovementAnimVariables->MoveSpeed,
				MovementAnimVariables->YawAngle, 
				UpdateGoalYawAngle,
				CurrentVelocityNormal.X, CurrentVelocityNormal.Y,
				OwnerObjectPtr->GetRotation().Yaw
			);
		}
#endif
	}

	// #113 : Aim BS
	{
		if (!StateAnimVariables->bIsAiming)
		{
			AimYawAngleLerp.TrySetGoalValue(0.0f);
			AimPitchAngleLerp.TrySetGoalValue(0.0f);
		}
		else
		{
			const FT4GameObjectState& ObjectState = OwnerObjectPtr->GetState();
			FRotator ToLocalRotation = OwnerObjectPtr->GetRotation().GetInverse().RotateVector(ObjectState.AimTargetDirection).Rotation();
			AimYawAngleLerp.TrySetGoalValue(ToLocalRotation.Yaw);
			AimPitchAngleLerp.TrySetGoalValue(ToLocalRotation.Pitch);
		}
		MovementAnimVariables->AimYawAngle = AimYawAngleLerp.UpdateValue(InUpdateTime.ScaledTimeSec); // #113
		MovementAnimVariables->AimPitchAngle = AimPitchAngleLerp.UpdateValue(InUpdateTime.ScaledTimeSec); // #113
		if (0.0f == MovementAnimVariables->AimYawAngle && 0.0f == MovementAnimVariables->AimPitchAngle)
		{
			StateAnimVariables->bIsAiming = false;
		}
		else
		{
			StateAnimVariables->bIsAiming = true;
		}
#if 0
		if (OwnerObjectPtr->IsPlayer() && 0.0f != MovementAnimVariables->YawAngle)
		{
			T4_LOG(
				Warning,
				TEXT("Aim Horizontal = %.2f, Vertical = %.2f"),
				MovementAnimVariables->AimYawAngle,
				MovementAnimVariables->AimPitchAngle
			);
		}
#endif
	}

	AdvanceFootIK(InUpdateTime);
}

void FT4HumanAnimControl::AdvanceFootIK(const FT4UpdateTime& InUpdateTime)
{
	check(OwnerObjectPtr.IsValid());
#if !UE_BUILD_SHIPPING
	if (0 != T4HumanAnimControlCVars::IKFootDisable)
	{
		ResetFootIK();
		return;
	}
#endif
	if (!FootIKInfo.bEnableFootIK)
	{
		return;
	}
	if (!CurrentVelocityNormal.IsNearlyZero())
	{
		// IK 는 제자리에서만 사용한다.
		ResetFootIK();
		return;
	}

	// http://api.unrealengine.com/KOR/Engine/Animation/IKSetups/

	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	check(nullptr != AnimInstance);

	FT4IKAnimVariables* IKAnimVariables = AnimInstance->GetIKAnimVariables();
	if (nullptr == IKAnimVariables)
	{
		FootIKInfo.bEnableFootIK = false; // Animal
		return;
	}

	const float AddTraceOffset = 50.0f;

	FVector LeftFootHitLocation = FVector::ZeroVector;
	if (!CalaculateFootIKOffset(
		LeftFootSocketName,
		AddTraceOffset,
		IKAnimVariables->LeftFootOffset,
		&LeftFootHitLocation
	))
	{
		ResetFootIK();
		return;
	}

	FVector RightFootHitLocation = FVector::ZeroVector;
	if (!CalaculateFootIKOffset(
		RightFootSocketName,
		AddTraceOffset,
		IKAnimVariables->RightFootOffset,
		&RightFootHitLocation
	))
	{
		ResetFootIK();
		return;
	}

	// TODO : 두 발의 위치가 일정 이상 차이가 날 경우 무시하도록 처리 필요

	float COMHeightOffset = FMath::Abs(IKAnimVariables->LeftFootOffset - IKAnimVariables->RightFootOffset);

	IKAnimVariables->LeftFootOffset += COMHeightOffset;
	IKAnimVariables->RightFootOffset += COMHeightOffset;

	IKAnimVariables->LeftFootJointTarget = DefaultLeftFootJointTarget;
	IKAnimVariables->RightFootJointTarget = DefaultRightFootJointTarget;

	IKAnimVariables->COMOffset = FVector(-COMHeightOffset, 0.0f, 0.0f);

#if !UE_BUILD_SHIPPING
	ApplyFootIKConsobleVar();
#endif

	// #18 : Foot Bone 의 Axis 에 따라 Offset 을 달리 적용해주어야 함
	if (FootIKInfo.bInverseLeftFootOffset)
	{
		IKAnimVariables->LeftFootOffset = -IKAnimVariables->LeftFootOffset;
	}
	if (FootIKInfo.bInverseRightFootOffset)
	{
		IKAnimVariables->RightFootOffset = -IKAnimVariables->RightFootOffset;
	}

	FootIKInfo.CurrentCOMHeightOffset = IKAnimVariables->COMOffset.X;
	OwnerObjectPtr->SetHeightOffset(FootIKInfo.CurrentCOMHeightOffset);

	IKAnimVariables->bUsedFootIK = true;

#if !UE_BUILD_SHIPPING
	DebugDrawFootIKInfo(LeftFootHitLocation, RightFootHitLocation);
#endif
}

void FT4HumanAnimControl::ResetFootIK()
{
	check(OwnerObjectPtr.IsValid());

	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	check(nullptr != AnimInstance);

	FT4IKAnimVariables* IKAnimVariables = AnimInstance->GetIKAnimVariables();
	check(nullptr != IKAnimVariables);

	// run Ik disable
	IKAnimVariables->bUsedFootIK = false;
	IKAnimVariables->LeftFootOffset = 0.0f;
	IKAnimVariables->RightFootOffset = 0.0f;
	IKAnimVariables->LeftFootJointTarget = DefaultLeftFootJointTarget;
	IKAnimVariables->RightFootJointTarget = DefaultRightFootJointTarget;
	OwnerObjectPtr->SetHeightOffset(0.0f);
}

bool FT4HumanAnimControl::CalaculateFootIKOffset(
	const FName& InSocketName,
	float InAddTraceOffset,
	float& OutOffset,
	FVector* OutHitLocation
)
{
	check(OwnerObjectPtr.IsValid());
	FVector FootLocation = FVector::ZeroVector;
	if (!OwnerObjectPtr->GetSocketLocation(InSocketName, FootLocation))
	{
		return false;
	}

	FCollisionQueryParams IK_TraceParams = FCollisionQueryParams(
		FName(TEXT("IK_Foot_Trace")), 
		true, 
		OwnerObjectPtr.Get()
	);
	IK_TraceParams.MobilityType = EQueryMobilityType::Static; // #49 : IK 는 Static 류에만 반응하자!
	IK_TraceParams.bTraceComplex = true;
	IK_TraceParams.bReturnPhysicalMaterial = false;

	const FVector ActorLocation = OwnerObjectPtr->GetCOMLocation(); // Capsule 중점!
	const float HalfHeight = OwnerObjectPtr->GetPropertyConst().HalfHeight;

	FVector StartLocation = FootLocation;
	StartLocation.Z = ActorLocation.Z;
	FVector EndLocation = StartLocation;
	EndLocation.Z -= HalfHeight + InAddTraceOffset;

	FVector FootHitLocation = FVector::ZeroVector;
	bool bResult = QueryIKLineTrace(StartLocation, EndLocation, IK_TraceParams, FootHitLocation);
	if (!bResult)
	{
		T4_LOG(
			Verbose,
			TEXT("line trace failed. '%s' foot!"),
			*(InSocketName.ToString())
		);
		OutOffset = 0.0f;
		if (nullptr != OutHitLocation)
		{
			*OutHitLocation = FVector::ZeroVector;
		}
	}
	else
	{
		OutOffset = (FootHitLocation.Z - EndLocation.Z) - InAddTraceOffset;
		if (nullptr != OutHitLocation)
		{
			*OutHitLocation = FootHitLocation;
		}
	}
	return true;
}

void FT4HumanAnimControl::HandleOnAnimNotify(FName InNotifyName)
{
#if 0
	if (T4Const_AnimNotifyFootStepLeftName == InNotifyName)
	{
		LastFootstepType = ET4FootstepType::Footstep_Left;
	}
	else if (T4Const_AnimNotifyFootStepRightName == InNotifyName)
	{
		LastFootstepType = ET4FootstepType::Footstep_Right;
	}
#endif
}

void FT4HumanAnimControl::ApplyFootIKConsobleVar()
{
#if !UE_BUILD_SHIPPING
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	check(nullptr != AnimInstance);

	FT4IKAnimVariables* IKAnimVariables = AnimInstance->GetIKAnimVariables();
	check(nullptr != IKAnimVariables);

	if (0 != T4HumanAnimControlCVars::IKFootReset)
	{
		T4HumanAnimControlCVars::IKFootLeftOffset = 0.0f;
		T4HumanAnimControlCVars::IKFootLeftJointOffset = FVector::ZeroVector;

		T4HumanAnimControlCVars::IKFootRightOffset = 0.0f;
		T4HumanAnimControlCVars::IKFootRightJointOffset = FVector::ZeroVector;

		T4HumanAnimControlCVars::IKFootCOMOffset = 0.0f;
		T4HumanAnimControlCVars::IKFootReset = 0;
	}
	if (0.0f != T4HumanAnimControlCVars::IKFootLeftOffset)
	{
		IKAnimVariables->LeftFootOffset += T4HumanAnimControlCVars::IKFootLeftOffset;
	}
	if (0.0f != T4HumanAnimControlCVars::IKFootLeftJointOffset.X)
	{
		IKAnimVariables->LeftFootJointTarget.X += T4HumanAnimControlCVars::IKFootLeftJointOffset.X;
	}
	if (0.0f != T4HumanAnimControlCVars::IKFootLeftJointOffset.Y)
	{
		IKAnimVariables->LeftFootJointTarget.Y += T4HumanAnimControlCVars::IKFootLeftJointOffset.Y;
	}
	if (0.0f != T4HumanAnimControlCVars::IKFootLeftJointOffset.Z)
	{
		IKAnimVariables->LeftFootJointTarget.Z += T4HumanAnimControlCVars::IKFootLeftJointOffset.Z;
	}
	if (0.0f != T4HumanAnimControlCVars::IKFootRightOffset)
	{
		IKAnimVariables->RightFootOffset += T4HumanAnimControlCVars::IKFootRightOffset;
	}
	if (0.0f != T4HumanAnimControlCVars::IKFootRightJointOffset.X)
	{
		IKAnimVariables->RightFootJointTarget.X += T4HumanAnimControlCVars::IKFootRightJointOffset.X;
	}
	if (0.0f != T4HumanAnimControlCVars::IKFootRightJointOffset.Y)
	{
		IKAnimVariables->RightFootJointTarget.Y += T4HumanAnimControlCVars::IKFootRightJointOffset.Y;
	}
	if (0.0f != T4HumanAnimControlCVars::IKFootRightJointOffset.Z)
	{
		IKAnimVariables->RightFootJointTarget.Z += T4HumanAnimControlCVars::IKFootRightJointOffset.Z;
	}
	if (0.0f != T4HumanAnimControlCVars::IKFootCOMOffset)
	{
		IKAnimVariables->COMOffset.X += T4HumanAnimControlCVars::IKFootCOMOffset;
	}
#endif
}

void FT4HumanAnimControl::DebugDrawFootIKInfo(
	const FVector& InLeftFootHitLocation,
	const FVector& InRightFootHitLocation
)
{
#if !UE_BUILD_SHIPPING
	if (0 == T4HumanAnimControlCVars::IKFootDebug)
	{
		return;
	}

	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	check(nullptr != AnimInstance);

	FT4IKAnimVariables* IKAnimVariables = AnimInstance->GetIKAnimVariables();
	check(nullptr != IKAnimVariables);

	UWorld* World = GetGameWorld()->GetWorld();
	FString WriteString;

	const float HalfHeight = OwnerObjectPtr->GetPropertyConst().HalfHeight;
	FRotator ActorRotation = OwnerObjectPtr->GetRotation(); // Capsule 중점!
	ActorRotation.Yaw -= 90.0f; // Local

	{
		// pelvis
		FVector PelvisLocation = FVector::ZeroVector;
		if (!OwnerObjectPtr->GetSocketLocation(TEXT("Pelvis"), PelvisLocation))
		{
			return;
		}
		DrawDebugSphere(
			World,
			PelvisLocation,
			7.0f,
			10,
			FColor::Green,
			false,
			0.01f,
			255
		);
		FVector COMOffsetLocation = PelvisLocation + (
			-FVector::UpVector * (HalfHeight - FootIKInfo.CurrentCOMHeightOffset)
		);
		DrawDebugLine(
			World,
			PelvisLocation,
			COMOffsetLocation,
			FColor::Green
		);
		DrawDebugSphere(
			World,
			COMOffsetLocation,
			7.0f,
			10,
			FColor::Green,
			false,
			0.01f,
			255
		);
		WriteString = FString::Printf(TEXT("COM Offset = %.2f"), -FootIKInfo.CurrentCOMHeightOffset);
		DrawDebugString(World, COMOffsetLocation, WriteString, nullptr, FColor::White, 0.01f, true);

		// left
		FVector ThighLocation = FVector::ZeroVector;
		if (!OwnerObjectPtr->GetSocketLocation(TEXT("Thigh_l"), ThighLocation))
		{
			return;
		}

		FVector ComponentToWorld;
		ComponentToWorld.X = -IKAnimVariables->LeftFootJointTarget.Z;
		ComponentToWorld.Y = IKAnimVariables->LeftFootJointTarget.Y;
		ComponentToWorld.Z = IKAnimVariables->LeftFootJointTarget.X;
		ComponentToWorld = ActorRotation.RotateVector(ComponentToWorld);
		FVector JointTargetLocation = ThighLocation + ComponentToWorld;

		DrawDebugSphere(
			World,
			InLeftFootHitLocation,
			7.0f,
			10,
			FColor::Blue,
			false,
			0.01f,
			255
		);
		DrawDebugSphere(
			World,
			JointTargetLocation,
			5.0f,
			10,
			FColor::Blue,
			false,
			0.01f,
			255
		);
		WriteString = FString::Printf(TEXT("Left Foot Offset = %.2f"), IKAnimVariables->LeftFootOffset);
		DrawDebugString(World, InLeftFootHitLocation, WriteString, nullptr, FColor::White, 0.01f, true);

		WriteString = FString::Printf(
			TEXT("Left Foot JointTarget = %.2f, %.2f, %.2f"),
			IKAnimVariables->LeftFootJointTarget.X,
			IKAnimVariables->LeftFootJointTarget.Y,
			IKAnimVariables->LeftFootJointTarget.Z
		);
		DrawDebugString(World, JointTargetLocation, WriteString, nullptr, FColor::White, 0.01f, true);
	}

	{
		// right
		FVector ThighLocation = FVector::ZeroVector;
		if (!OwnerObjectPtr->GetSocketLocation(TEXT("Thigh_r"), ThighLocation))
		{
			return;
		}

		FVector ComponentToWorld;
		ComponentToWorld.X = -IKAnimVariables->RightFootJointTarget.Z;
		ComponentToWorld.Y = IKAnimVariables->RightFootJointTarget.Y;
		ComponentToWorld.Z = IKAnimVariables->RightFootJointTarget.X;
		ComponentToWorld = ActorRotation.RotateVector(ComponentToWorld);
		FVector JointTargetLocation = ThighLocation + ComponentToWorld;

		DrawDebugSphere(
			World,
			InRightFootHitLocation,
			7.0f,
			10,
			FColor::Red,
			false,
			0.01f,
			255
		);
		DrawDebugSphere(
			World,
			JointTargetLocation,
			5.0f,
			10,
			FColor::Red,
			false,
			0.01f,
			255
		);
		WriteString = FString::Printf(
			TEXT("Right Foot Offset = %.2f"),
			IKAnimVariables->RightFootOffset
		);
		DrawDebugString(World, InRightFootHitLocation, WriteString, nullptr, FColor::White, 0.01f, true);

		WriteString = FString::Printf(
			TEXT("Right Foot JointTarget = %.2f, %.2f, %.2f"),
			IKAnimVariables->RightFootJointTarget.X,
			IKAnimVariables->RightFootJointTarget.Y,
			IKAnimVariables->RightFootJointTarget.Z
		);
		DrawDebugString(World, JointTargetLocation, WriteString, nullptr, FColor::White, 0.01f, true);
	}
#endif
}
