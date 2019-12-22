// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4HumanAnimControl.h"

#include "Object/Animation/AnimInstance/T4BaseAnimInstance.h"
#include "Object/Animation/AnimInstance/T4BaseAnimVariables.h"
#include "Object/T4GameObject.h"

#include "AnimState/T4HumanBasicAnimStates.h" // #48
#include "AnimState/T4HumanLocomotionAnimStates.h" // #47

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

static const float MoveSpeedLevelStandValueStart = 0.0f;
static const float MoveSpeedLevelWalkValueStart = 1.0f;
static const float MoveSpeedLevelRunValueStart = 2.0f;
static const float MoveSpeedLevelFastRunValueStart = 3.0f;

/**
  *
 */
FT4HumanAnimControl::FT4HumanAnimControl(AT4GameObject* InGameObject)
	: FT4BaseAnimControl(InGameObject)
	, DefaultUnarmedStanceStateName(NAME_None) // #48
	, DefaultCombatStanceStateName(NAME_None) // #48
	, DefaultRunningStateName(NAME_None) // #48
	, LastFootstepType(ET4FootstepType::Footstep_Left)
	, CurrentVelocityNormal(FVector::ZeroVector)
	, PrevVelocityNormal(FVector::ZeroVector) // #38
	, MinAccelerationScale(0.1f) // #38
	, MaxAcceleratedMoveTimeSec(0.25f) // #38
	, MaxFootStanceIdlePlayCount(2) // #38
	, JumpVelocity(FVector::ZeroVector) // #46
	, RollVelocity(FVector::ZeroVector) // #46
	, TurnGoalRotation(FRotator::ZeroRotator) // #47
{
	// #38
	MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_Stand] = 0.0f;
	MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_Walk] = 200.0f;
	MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_Run] = 400.0f;
	MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_FastRun] = 600.0f;
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
	GameObject->GetSocketRotation(LeftFootSocketName, RTS_Component, LeftFootSocketRotation);
	FVector LeftFootUpVector = LeftFootSocketRotation.RotateVector(FVector::ForwardVector);
	if (-0.5f > FVector::DotProduct(LeftFootUpVector, FVector::UpVector))
	{
		FootIKInfo.bInverseLeftFootOffset = true;
	}

	FRotator RightFootSocketRotation = FRotator::ZeroRotator;
	GameObject->GetSocketRotation(RightFootSocketName, RTS_Component, RightFootSocketRotation);
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
				RegisterAnimState(T4AnimStateHumanJumpingName, new FT4HumanJumpingAnimState(this));
				RegisterAnimState(T4AnimStateHumanRollingName, new FT4HumanRollingAnimState(this)); // #46

				RegisterAnimState(T4AnimStateHumanVoidName, new FT4HumanVoidAnimState(this)); // #76

				RegisterAnimState(T4AnimStateHumanBasicCombatStanceName, new FT4HumanBasicCombatStanceAnimState(this)); // #48
				RegisterAnimState(T4AnimStateHumanBasicUnarmedStanceName, new FT4HumanBasicUnarmedStanceAnimState(this)); // #48
				RegisterAnimState(T4AnimStateHumanBasicRunningName, new FT4HumanBasicRunningAnimState(this)); // #48

				DefaultUnarmedStanceStateName = T4AnimStateHumanBasicUnarmedStanceName; // #48
				DefaultCombatStanceStateName = T4AnimStateHumanBasicCombatStanceName; // #48
				DefaultRunningStateName = T4AnimStateHumanBasicRunningName; // #48

				ActiveAnimStateName = DefaultUnarmedStanceStateName;
			}
			break;

		case ET4AnimInstance::Human_Locomotion: // #38
			{
				RegisterAnimState(T4AnimStateHumanJumpingName, new FT4HumanJumpingAnimState(this));
				RegisterAnimState(T4AnimStateHumanRollingName, new FT4HumanRollingAnimState(this)); // #46
			
				RegisterAnimState(T4AnimStateHumanVoidName, new FT4HumanVoidAnimState(this)); // #76

				RegisterAnimState(T4AnimStateHumanLocomotionCombatLFStanceName, new FT4HumanLocomotionCombatLeftStanceAnimState(this));
				RegisterAnimState(T4AnimStateHumanLocomotionCombatRFStanceName, new FT4HumanLocomotionCombatRightStanceAnimState(this));
				RegisterAnimState(T4AnimStateHumanLocomotionUnarmedStanceName, new FT4HumanLocomotionUnarmedStanceAnimState(this));

				RegisterAnimState(T4AnimStateHumanLocomotionTurningName, new FT4HumanLocomotionTurningAnimState(this)); // #44

				RegisterAnimState(T4AnimStateHumanLocomotionRunStartName, new FT4HumanLocomotionRunStartAnimState(this));
				RegisterAnimState(T4AnimStateHumanLocomotionRunStopName, new FT4HumanLocomotionRunStopAnimState(this));
				RegisterAnimState(T4AnimStateHumanLocomotionRunningName, new FT4HumanLocomotionRunningAnimState(this));

				DefaultUnarmedStanceStateName = T4AnimStateHumanLocomotionUnarmedStanceName; // #48
				DefaultCombatStanceStateName = T4AnimStateHumanLocomotionCombatLFStanceName; // #48
				DefaultRunningStateName = T4AnimStateHumanLocomotionRunningName; // #48

				ActiveAnimStateName = DefaultUnarmedStanceStateName;
			}
			break;

		default:
			{
				UE_LOG(
					LogT4Engine,
					Error,
					TEXT("AutoRegisterAnimStates : Unknown anim instance type '%u'"),
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
	if (T4AnimSetAnimMontageSkillName == InAnimParameters.AnimMontageName)
	{
		const FName ActiveAnimState = GetActiveAnimStateName();
		if (T4AnimStateHumanLocomotionCombatRFStanceName == ActiveAnimState ||
			T4AnimStateHumanLocomotionUnarmedStanceName == ActiveAnimState)
		{
			// Idle 일 경우에만 Skill 애니류 사용시 Combat Idle 로 자동 변경...
			TryChangeAnimState(T4AnimStateHumanLocomotionCombatLFStanceName, false, true);
		}
	}
}

void FT4HumanAnimControl::SetAnimSetAsset(UT4AnimSetAsset* InAnimSetAsset)
{
	check(nullptr != InAnimSetAsset);

	FT4AnimSetDefaultAttribute& LocomotionAttribute = InAnimSetAsset->DefaultAttributes; // #38
	MinAccelerationScale = LocomotionAttribute.MinAccelerationScale;
	MaxAcceleratedMoveTimeSec = LocomotionAttribute.MaxAcceleratedMoveTimeSec;
	MaxFootStanceIdlePlayCount = LocomotionAttribute.MaxFootStanceIdlePlayCount;

	FT4AnimSetBlendSpaceAttribute& BlendSpaceAttribute = InAnimSetAsset->BlendSpaceAttributes; // #38
	MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_Stand] = 0.0f;
	MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_Walk] = BlendSpaceAttribute.StartWalkSpeedLevel;
	MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_Run] = BlendSpaceAttribute.StartRunSpeedLevel;
	MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_FastRun] = BlendSpaceAttribute.StartFastRunSpeedLevel;

	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	check(nullptr != AnimInstance);
	AnimInstance->SetAnimSetAsset(InAnimSetAsset);
}

bool FT4HumanAnimControl::DoJump(const FVector& InVelocity) // #46
{
	JumpVelocity = InVelocity;
	bool bResult = TryChangeAnimState(T4AnimStateHumanJumpingName, false, false); // #47
	return bResult;
}

bool FT4HumanAnimControl::DoRoll(const FVector& InVelocity) // #46
{
	RollVelocity = InVelocity;
	bool bResult = TryChangeAnimState(T4AnimStateHumanRollingName, false, false); // #47
	return bResult;
}

bool FT4HumanAnimControl::DoTurn(const FRotator& InRotation) // #47
{
	TurnGoalRotation = InRotation;
	if (!HasAnimState(T4AnimStateHumanLocomotionTurningName))
	{
		return true; // ET4AnimInstance::Human_Basic 일 경우는 없을 수 있음으로 예외 처리
	}
	bool bResult = TryChangeAnimState(T4AnimStateHumanLocomotionTurningName, false, true); // #47
	return bResult;
}

bool FT4HumanAnimControl::DoVoid() // #76
{
	bool bResult = TryChangeAnimState(T4AnimStateHumanVoidName, false, true);
	return bResult;
}

void FT4HumanAnimControl::Reset()
{
	// #38
	ResetFootIK();

	DefaultUnarmedStanceStateName = NAME_None; // #48
	DefaultCombatStanceStateName = NAME_None; // #48
	DefaultRunningStateName = NAME_None; // #48

	LastFootstepType = ET4FootstepType::Footstep_Left;
	CurrentVelocityNormal = FVector::ZeroVector;
	PrevVelocityNormal = FVector::ZeroVector; // #38

	// #46
	JumpVelocity = FVector::ZeroVector;
	RollVelocity = FVector::ZeroVector;
	// #46
	TurnGoalRotation = FRotator::ZeroRotator; // #47
}

FT4MovementAnimVariables* FT4HumanAnimControl::GetMovementAnimVariables()
{
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	if (nullptr == AnimInstance)
	{
		return nullptr;
	}
	return AnimInstance->GetMovementAnimVariables();
}

void FT4HumanAnimControl::Advance(const FT4UpdateTime& InUpdateTime)
{
	check(bBegunPlay);
	check(GameObject.IsValid());

	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	check(nullptr != AnimInstance);
	{
		AnimInstance->SetPause(InUpdateTime.bPaused); // #102
		AnimInstance->SetTimeScale(InUpdateTime.TimeScale); // #102
	}

	FT4MovementAnimVariables* MovementAnimVariables = GetMovementAnimVariables();
	check(nullptr != MovementAnimVariables);
	{
		MovementAnimVariables->bIsFalling = GameObject->IsFalling();
		MovementAnimVariables->bIsLockOn = GameObject->IsLockOn();
	}
	{
		PrevVelocityNormal = CurrentVelocityNormal;
	}

	float CurrentMoveSpeed = GameObject->GetMovementSpeed();
	if (0.0f < CurrentMoveSpeed || 0.0f < MovementAnimVariables->SpeedLevel)
	{
		MovementAnimVariables->SpeedLevel = UpdateSpeedLevel(CurrentMoveSpeed);
	}
	   
	if (MovementAnimVariables->bIsLockOn)
	{
		// LockOn 이동은 Fast Run 모션에 제한을 둔다. 물론, 별도의 애니를 추가했다면 풀어줘도 무방.
		// Mannequin 을 기준으로 옆뛰기등은 모션 하나로 속도만 빨리해 처리한다.
		MovementAnimVariables->SpeedLevel = FMath::Min(
			MovementAnimVariables->SpeedLevel, 
			MoveSpeedLevelRunValueStart
		);
		FRotator ActorRotation = GameObject->GetRotation();
		float BlendSpaceAngle = AnimInstance->CalculateDirection(CurrentVelocityNormal, ActorRotation);
		MovementAnimVariables->DirectionDegree = BlendSpaceAngle;
	}
	else
	{
		MovementAnimVariables->DirectionDegree = 0.0f; // WARN : 정면으로 다시 맞춰주어야 한다!
	}

	CurrentVelocityNormal = GameObject->GetMovementVelocity(); // #38
	CurrentVelocityNormal.Normalize();

	AdvanceFootIK(InUpdateTime);
}

void FT4HumanAnimControl::AdvanceFootIK(const FT4UpdateTime& InUpdateTime)
{
	check(GameObject.IsValid());
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
	GameObject->SetHeightOffset(FootIKInfo.CurrentCOMHeightOffset);

	IKAnimVariables->bUsedFootIK = true;

#if !UE_BUILD_SHIPPING
	DebugDrawFootIKInfo(LeftFootHitLocation, RightFootHitLocation);
#endif
}

void FT4HumanAnimControl::ResetFootIK()
{
	check(GameObject.IsValid());

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
	GameObject->SetHeightOffset(0.0f);
}

bool FT4HumanAnimControl::CalaculateFootIKOffset(
	const FName& InSocketName,
	float InAddTraceOffset,
	float& OutOffset,
	FVector* OutHitLocation
)
{
	check(GameObject.IsValid());
	FVector FootLocation = FVector::ZeroVector;
	if (!GameObject->GetSocketLocation(InSocketName, FootLocation))
	{
		return false;
	}

	FCollisionQueryParams IK_TraceParams = FCollisionQueryParams(
		FName(TEXT("IK_Foot_Trace")), 
		true, 
		GameObject.Get()
	);
	IK_TraceParams.MobilityType = EQueryMobilityType::Static; // #49 : IK 는 Static 류에만 반응하자!
	IK_TraceParams.bTraceComplex = true;
	IK_TraceParams.bReturnPhysicalMaterial = false;

	const FVector ActorLocation = GameObject->GetCOMLocation(); // Capsule 중점!
	const float HalfHeight = GameObject->GetPropertyConst().HalfHeight;

	FVector StartLocation = FootLocation;
	StartLocation.Z = ActorLocation.Z;
	FVector EndLocation = StartLocation;
	EndLocation.Z -= HalfHeight + InAddTraceOffset;

	FVector FootHitLocation = FVector::ZeroVector;
	bool bResult = QueryIKLineTrace(StartLocation, EndLocation, IK_TraceParams, FootHitLocation);
	if (!bResult)
	{
		UE_LOG(
			LogT4Engine,
			Verbose,
			TEXT("AdvanceFootIK : line trace failed. '%s' foot!"),
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

float FT4HumanAnimControl::UpdateSpeedLevel(float InCurrentMoveSpeed)
{
	// #38
	if (0.0f >= InCurrentMoveSpeed)
	{
		return MoveSpeedLevelStandValueStart;
	}
	// TODO : optimize
	if (InCurrentMoveSpeed >= MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_FastRun])
	{
		return MoveSpeedLevelFastRunValueStart;
	}
	else if (InCurrentMoveSpeed < MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_Walk])
	{
		return FMath::Min( 
			MoveSpeedLevelWalkValueStart,
			InCurrentMoveSpeed * (1.0f / MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_Walk])
		);
	}
	else if (InCurrentMoveSpeed < MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_Run])
	{
		return FMath::Min(
			MoveSpeedLevelRunValueStart, 
			MoveSpeedLevelWalkValueStart + (
				(InCurrentMoveSpeed - MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_Walk]) *
				(1.0f / (MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_Run] - MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_Walk]))
			)
		);
	}
	else if (InCurrentMoveSpeed < MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_FastRun])
	{
		return FMath::Min(
			MoveSpeedLevelFastRunValueStart, 
			MoveSpeedLevelRunValueStart + (
				(InCurrentMoveSpeed - MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_Run]) *
				(1.0f / (MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_FastRun] - MoveSpeedLevelLimit[ET4MoveSpeed::MoveSpeed_Run]))
			)
		);
	}
	return MoveSpeedLevelStandValueStart;
}

void FT4HumanAnimControl::HandleOnAnimNotify(FName InNotifyName)
{
	if (T4AnimSetAnimNotifyLeftFootstepName == InNotifyName)
	{
		LastFootstepType = ET4FootstepType::Footstep_Left;
	}
	else if (T4AnimSetAnimNotifyRightFootstepName == InNotifyName)
	{
		LastFootstepType = ET4FootstepType::Footstep_Right;
	}
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

	const float HalfHeight = GameObject->GetPropertyConst().HalfHeight;
	FRotator ActorRotation = GameObject->GetRotation(); // Capsule 중점!
	ActorRotation.Yaw -= 90.0f; // Local

	{
		// pelvis
		FVector PelvisLocation = FVector::ZeroVector;
		if (!GameObject->GetSocketLocation(TEXT("Pelvis"), PelvisLocation))
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
		if (!GameObject->GetSocketLocation(TEXT("Thigh_l"), ThighLocation))
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
		if (!GameObject->GetSocketLocation(TEXT("Thigh_r"), ThighLocation))
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
