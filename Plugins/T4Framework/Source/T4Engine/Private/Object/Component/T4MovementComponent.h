// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Movement/T4MovementStructs.h"
#include "Animation/AnimationAsset.h"
#include "GameFramework/PawnMovementComponent.h"
#include "T4MovementComponent.generated.h"

/**
  * refer GameFramework/CharacterMovementComponent.h
 */
DECLARE_DELEGATE_RetVal_TwoParams(FTransform, FT4OnProcessRootMotion, const FTransform&, UT4MovementComponent*)

#define USES_NO_REPLICATE_MOVEMENT_ISSUE_33 1

class AT4MovableCharacterObject;
UCLASS()
class UT4MovementComponent : public UPawnMovementComponent
{
	GENERATED_UCLASS_BODY()

public:
	//BEGIN UMovementComponent Interface
	float GetMaxSpeed() const override;
	void StopActiveMovement() override;
	bool IsCrouching() const override; // #38 : T4에서는 사용하지 않겠음!!
	bool IsFalling() const override;
	bool IsMovingOnGround() const override;
	float GetGravityZ() const override;
	//END UMovementComponent Interface

	//BEGIN UNavMovementComponent Interface
	// #31
	void RequestDirectMove(const FVector& MoveVelocity, bool bForceMaxSpeed) override;
	void RequestPathMove(const FVector& MoveInput) override;
	bool CanStartPathFollowing() const override;
	bool CanStopPathFollowing() const override;
	float GetPathFollowingBrakingDistance(float MaxSpeed) const override;
	// ~#31
	//END UNaVMovementComponent Interface

	//Begin UPawnMovementComponent Interface
	void NotifyBumpedPawn(APawn* BumpedPawn) override; // #31
	//End UPawnMovementComponent Interface
	
	bool IsWalking() const;
	bool IsLockOn() const; // #38

	bool DoJump(float InJumpZVelocity); // #46
	bool DoRoll(float InRollZVelocity); // #46

	void SetAnalogInputModifier(float InModifierScale) { AnalogInputModifier = InModifierScale; } // #38 : 가감속!!

public:
	void TickComponent(
		float DeltaTime,
		enum ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;
	
	void RegisterComponentTickFunctions(bool bRegister) override;

	void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
	void SetMovementMode(EMovementMode InMovementMode, uint8 NewCustomMode = 0);

public:
	/** Tick function called after physics (sync scene) has finished simulation, before cloth */
	void PostPhysicsTickComponent(
		float DeltaTime, 
		FT4MovementComponentPostPhysicsTickFunction& ThisTickFunction
	);

	void SaveBaseLocation();

protected:
	void BeginPlay() override;

	// UActorComponent interface
	void OnRegister() override;
	// End of UActorComponent interface

protected:
	friend class AT4MovableCharacterObject;

	/** Post-physics tick function for this character */
	UPROPERTY()
	FT4MovementComponentPostPhysicsTickFunction PostPhysicsTickFunction;

private:
	void PerformMovement(float DeltaTime);
	void SimulatedTick(float DeltaSeconds);
	void SimulateMovement(float DeltaTime);

	void StartNewPhysics(float DeltaTime, int32 Iterations);

	void PhysWalking(float DeltaTime, int32 Iterations);
	void PhysNavWalking(float DeltaTime, int32 Iterations);
	void PhysFalling(float DeltaTime, int32 Iterations);
	void PhysicsRotation(float DeltaTime);

	void MaybeUpdateBasedMovement(float DeltaSeconds);
	void MaybeSaveBaseLocation();
	void UpdateBasedMovement(float DeltaSeconds);
	void UpdateBasedRotation(FRotator& FinalRotation, const FRotator& ReducedRotation);

	void CallMovementUpdateDelegate(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity);
	void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity);

	void OnUnableToFollowBaseMove(
		const FVector& DeltaPosition,
		const FVector& OldLocation,
		const FHitResult& MoveOnBaseHit
	);

	void AdjustFloorHeight();

	FVector ProjectLocationFromNavMesh(
		float DeltaSeconds, 
		const FVector& CurrentFeetLocation, 
		const FVector& TargetNavLocation, 
		float UpOffset, 
		float DownOffset
	);
	void FindBestNavMeshLocation(
		const FVector& TraceStart, 
		const FVector& TraceEnd, 
		const FVector& CurrentFeetLocation, 
		const FVector& TargetNavLocation, 
		FHitResult& OutHitResult
	) const;

	void SetNavWalkingPhysics(bool bEnable);

	void StartFalling(
		int32 Iterations, 
		float remainingTime, 
		float timeTick, 
		const FVector& Delta, 
		const FVector& subLoc
	);

	void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations);
	bool IsValidLandingSpot(const FVector& CapsuleLocation, const FHitResult& Hit) const;
	void SetPostLandedPhysics(const FHitResult& Hit);

	void SetDefaultMovementMode();

	void ApplyImpactPhysicsForces(
		const FHitResult& Impact, 
		const FVector& ImpactAcceleration, 
		const FVector& ImpactVelocity
	);

	void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration);

	bool ApplyRequestedMove(
		float DeltaTime, 
		float MaxAccel, 
		float MaxSpeed, 
		float Friction, 
		float BrakingDeceleration, 
		FVector& OutAcceleration,
		float& OutRequestedSpeed
	);

	struct FStepDownResult
	{
		uint32 bComputedFloor : 1;		// True if the floor was computed as a result of the step down.
		FT4FindFloorResult FloorResult;	// The result of the floor test if the floor was updated.

		FStepDownResult()
			: bComputedFloor(false)
		{
		}
	};

	bool StepUp(const FVector& GravDir, const FVector& Delta, const FHitResult &Hit, FStepDownResult* OutStepDownResult = nullptr);

	void MoveAlongFloor(const FVector& InVelocity, float DeltaSeconds, FStepDownResult* OutStepDownResult = nullptr);

	void ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration);

	bool FindNavFloor(const FVector& TestLocation, FNavLocation& NavFloorLocation) const;
	void FindFloor(
		const FVector& CapsuleLocation,
		FT4FindFloorResult& OutFloorResult,
		bool bCanUseCachedLocation,
		const FHitResult* DownwardSweepResult = nullptr
	) const;

	void ComputeFloorDist(
		const FVector& CapsuleLocation,
		float LineDistance,
		float SweepDistance,
		FT4FindFloorResult& OutFloorResult,
		float SweepRadius,
		const FHitResult* DownwardSweepResult = nullptr
	) const;

	bool FloorSweepTest(
		FHitResult& OutHit,
		const FVector& Start,
		const FVector& End,
		ECollisionChannel TraceChannel,
		const struct FCollisionShape& CollisionShape,
		const struct FCollisionQueryParams& Params,
		const struct FCollisionResponseParams& ResponseParam
	) const;

	void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode);

	void TickCharacterPose(float DeltaTime);

	void SetBase(
		UPrimitiveComponent* NewBase,
		const FName BoneName = NAME_None,
		bool bNotifyActor = true
	);
	void SetBaseFromFloor(const FT4FindFloorResult& FloorResult);

	void ApplyAccumulatedForces(float DeltaSeconds);
	void ClearAccumulatedForces();

	bool TryToLeaveNavWalking();

	bool IsWalkable(const FHitResult& Hit) const;
	bool IsWithinEdgeTolerance(
		const FVector& CapsuleLocation,
		const FVector& TestImpactPoint,
		const float CapsuleRadius
	) const;

	void OnCharacterStuckInGeometry(const FHitResult* Hit);

	bool CanStepUp(const FHitResult& Hit) const;

	bool CheckLedgeDirection(const FVector& OldLocation, const FVector& SideStep, const FVector& GravDir) const;
	FVector GetLedgeMove(const FVector& OldLocation, const FVector& Delta, const FVector& GravDir) const;

	bool CheckFall(
		const FT4FindFloorResult& OldFloor,
		const FHitResult& Hit,
		const FVector& Delta,
		const FVector& OldLocation,
		float remainingTime,
		float timeTick,
		int32 Iterations,
		bool bMustJump
	);

	void RevertMove(
		const FVector& OldLocation,
		UPrimitiveComponent* OldBase,
		const FVector& InOldBaseLocation,
		const FT4FindFloorResult& OldFloor,
		bool bFailMove
	);

	ETeleportType GetTeleportType() const;

	bool ShouldCatchAir(const FT4FindFloorResult& OldFloor, const FT4FindFloorResult& NewFloor);

	// Enum used to control GetPawnCapsuleExtent behavior
	enum EShrinkCapsuleExtent
	{
		SHRINK_None,			// Don't change the size of the capsule
		SHRINK_RadiusCustom,	// Change only the radius, based on a supplied param
		SHRINK_HeightCustom,	// Change only the height, based on a supplied param
		SHRINK_AllCustom,		// Change both radius and height, based on a supplied param
	};
	FVector GetPawnCapsuleExtent(const EShrinkCapsuleExtent ShrinkMode, const float CustomShrinkAmount = 0.f) const;
	FCollisionShape GetPawnCapsuleCollisionShape(const EShrinkCapsuleExtent ShrinkMode, const float CustomShrinkAmount = 0.f) const;

	float GetMaxAcceleration() const;
	float GetMaxBrakingDeceleration() const;

	bool ShouldComputePerchResult(const FHitResult& InHit, bool bCheckRadius) const;
	bool ComputePerchResult(
		const float TestRadius,
		const FHitResult& InHit,
		const float InMaxFloorDist,
		FT4FindFloorResult& OutPerchFloorResult
	) const;

	float GetValidPerchRadius() const;
	float GetPerchRadiusThreshold() const;

	void SetWalkableFloorAngle(float InWalkableFloorAngle);
	void SetWalkableFloorZ(float InWalkableFloorZ);

	void MaintainHorizontalGroundVelocity();
	FRotator GetDeltaRotation(float DeltaTime) const;
	FRotator ComputeOrientToMovementRotation(
		const FRotator& CurrentRotation,
		float DeltaTime,
		FRotator& DeltaRotation
	) const;

	bool ShouldRemainVertical() const;

	FVector ComputeGroundMovementDelta(
		const FVector& Delta,
		const FHitResult& RampHit,
		const bool bHitFromLineTrace
	) const;
	FVector ConstrainInputAcceleration(const FVector& InputAcceleration) const;
	FVector ScaleInputAcceleration(const FVector& InputAcceleration) const;

	void Launch(FVector const& LaunchVel);
	bool HandlePendingLaunch();

	FVector GetImpartedMovementBaseVelocity() const;
	FVector GetFallingLateralAcceleration(float DeltaTime);

	FVector GetAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration);
	float BoostAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration);

	FVector NewFallVelocity(const FVector& InitialVelocity, const FVector& Gravity, float DeltaTime) const;

	float GetSimulationTimeStep(float RemainingTime, int32 Iterations) const;

	/** Called when the collision capsule touches another primitive component */
	UFUNCTION()
	void CapsuleTouched(
		UPrimitiveComponent* OverlappedComp,
		AActor* Other,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UPrimitiveComponent* GetMovementBase() const;

	bool HasValidData() const;
	const class INavigationDataInterface* GetNavData() const;

	void PerformAirControlForPathFollowing(FVector Direction, float ZDiff);

protected:
	// General Settings
	float GravityScale;

	uint8 bForceMaxAccel : 1;
	float AnalogInputModifier;

	uint8 bJustTeleported : 1;

	TEnumAsByte<EMovementMode> MovementMode;
	TEnumAsByte<EMovementMode> DefaultLandMovementMode;
	TEnumAsByte<EMovementMode> DefaultWaterMovementMode;
	TEnumAsByte<EMovementMode> GroundMovementMode;
	uint8 CustomMovementMode;

	uint8 bEnableScopedMovementUpdates : 1;
	uint8 bRunPhysicsWithNoController : 1;

	uint8 bRequestedMoveUseAcceleration : 1;


	// Rotation Settings
	FRotator RotationRate;
	uint8 bUseControllerDesiredRotation : 1;
	uint8 bOrientRotationToMovement : 1;


	// Walking Settings
	float MaxMoveSpeed;
	float MaxMoveSpeedLockOn;
	float MaxCustomMovementSpeed;
	float WalkableFloorAngle;
	float WalkableFloorZ;

	FT4FindFloorResult CurrentFloor;

	uint8 bMaintainHorizontalGroundVelocity : 1;
	uint8 bIgnoreBaseRotation : 1;

	uint8 bSweepWhileNavWalking : 1;
	uint8 bCanWalkOffLedges : 1;

	uint8 bAlwaysCheckFloor : 1;
	uint8 bForceNextFloorCheck : 1;
	   
	uint8 bUseFlatBaseForFloorChecks : 1;


	// Jump Settings
	float JumpOffJumpZFactor;
	uint8 bImpartBaseVelocityX : 1;
	uint8 bImpartBaseVelocityY : 1;
	uint8 bImpartBaseVelocityZ : 1;
	uint8 bImpartBaseAngularVelocity : 1;


	// Fly Settings
	float MaxFlySpeed;


	// Swimming Settings
	float MaxSwimSpeed;


	// Physics Interaction
	uint8 bEnablePhysicsInteraction : 1;
	uint8 bTouchForceScaledToMass : 1;
	uint8 bPushForceScaledToMass : 1;
	uint8 bPushForceUsingZOffset : 1;
	uint8 bScalePushForceToVelocity : 1;


	// NavMesh Movement
	uint8 bProjectNavMeshWalking : 1;
	uint8 bProjectNavMeshOnBothWorldChannels : 1;


	// Context
	FVector Acceleration;

	FQuat LastUpdateRotation;
	FVector LastUpdateLocation;
	FVector LastUpdateVelocity;

	uint8 bHasRequestedVelocity : 1;
	FVector RequestedVelocity;

	uint8 bWantsToLeaveNavWalking : 1;
	float NavMeshProjectionTimer;

	FQuat OldBaseQuat;
	FVector OldBaseLocation;

	uint8 bRequestedMoveWithMaxSpeed : 1;

	uint8 bDeferUpdateMoveComponent:1;
	USceneComponent* DeferredUpdatedMoveComponent;

	uint8 bFastAttachedMove : 1;

	FVector PendingImpulseToApply;
	FVector PendingForceToApply;

	uint8 bMovementInProgress : 1;
	uint8 bDeferUpdateBasedMovement : 1;

	FNavLocation CachedNavLocation;
	FHitResult CachedProjectedNavMeshHitResult;

	FVector PendingLaunchVelocity;

private:
	AT4MovableCharacterObject* CharacterOwner;
};

FORCEINLINE_DEBUGGABLE bool UT4MovementComponent::IsWalking() const
{
	return IsMovingOnGround();
}