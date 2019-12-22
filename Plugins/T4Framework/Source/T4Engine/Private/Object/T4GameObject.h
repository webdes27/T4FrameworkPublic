// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Action/T4ActionControl.h" // #20, #76
#include "Action/T4ManualControl.h" // #76
#include "T4Asset/Public/Entity/T4EntityKey.h"
#include "T4Asset/Classes/Entity/T4EntityAsset.h" // #81
#include "Public/Action/T4ActionCodeMinimal.h"
#include "Public/T4Engine.h"
#include "GameFramework/Pawn.h"
#include "T4GameObject.generated.h"

/**
  *
 */
struct FT4ActionStruct;
struct FT4StopAction;
struct FT4SpawnObjectAction;

class USceneComponent;
class UCapsuleComponent;
class UMeshComponent; // #78
class USkeletalMeshComponent;
class UMaterialInstanceDynamic; // #78
class UT4BaseAnimInstance;
class UPrimitiveComponent;
class UT4EnvironmentZoneComponent; // #99

struct FT4CameraWorkSectionKeyData; // #58

class IT4GameWorld;

struct FT4GameObjectState // #76
{
	FT4GameObjectState()
		: bLoadComplated(false)
		, bGhost(false) // #54
		, GhostOutTimeLeft(0.0f) // #54
		, bPaused(false)
		, TimeScale(1.0f) // #102
		, LifeTimeSec(0.0f) // #102
		, bTransitionOpacity(false)
		, OpacityValue(1.0f)
		, TargetOpacityValue(1.0f)
		, TransitionTimeMaxSec(0.0f)
		, TransitionTimeSec(0.0f)
		, bTurning(false) // #44
		, TurnGoalRotation(FRotator::ZeroRotator) // #44
		, TurnRotationYawRate(0.0f) // #44
#if WITH_EDITOR
		, bDebugPaused(false) // #63
#endif
	{
	}

	bool bLoadComplated;

	bool bGhost; // #54
	float GhostOutTimeLeft; // #54

	bool bPaused;
	float TimeScale; // #102
	float LifeTimeSec; // #102

	// #78 : Fade In/Out
	bool bTransitionOpacity;
	float OpacityValue;
	float TargetOpacityValue;
	float TransitionTimeMaxSec;
	float TransitionTimeSec;

	// #44 : Turn 에서 호출 (0 == InRotationYawRate) ? immediate
	bool bTurning;
	FRotator TurnGoalRotation;
	float TurnRotationYawRate;

#if !UE_BUILD_SHIPPING
	bool bDebugPaused; // #63
#endif
};

// #68 : 리로드 또는 playback 을 위해 장착 정보를 보관한다. 단, 현재는 Character 만 사용
struct FT4EquipmentInfo
{
	FName EquipPoint;
	FT4EntityKey EntityKey;
};

UCLASS()
class AT4GameObject : public APawn, public IT4GameObject
{
	GENERATED_UCLASS_BODY()

	// Disable compiler-generated deprecation warnings by implementing our own destructor
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	virtual ~AT4GameObject();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

public:
	virtual void TickActor(
		float InDeltaTime,
		enum ELevelTick InTickType,
		FActorTickFunction& InThisTickFunction
	) override;

	/** If true, actor is ticked even if TickType==LEVELTICK_ViewportsOnly	 */
	bool ShouldTickIfViewportsOnly() const override; // #17

	void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;

	// #104 : World 밖으로 사라져도 강제로 삭제하는 처리는 하지 않는다. 자동 삭제된다!!
	//        Projectile 류가 강제 삭제되는 문제가 있었음!
	void FellOutOfWorld(const class UDamageType& dmgType) override;

protected:
	void BeginPlay() override;

public:
	// IT4GameObject
	ET4LayerType GetLayerType() const override { return LayerType; }
	virtual ET4ObjectType GetObjectType() const override { return ET4ObjectType::None; }

	const FT4ObjectID& GetObjectID() const override { return ObjectID; }

	const FName& GetName() const override { return Name; }

	const FT4EntityKey& GetEntityKey() const override { return EntityKey; } // #35
	virtual const UT4EntityAsset* GetEntityAsset() override { return nullptr; } // #39

	virtual const FName GetStanceName() const override { return NAME_None; } // #73
	const FName& GetGameDataIDName() const override { return GameDataIDName; }

	virtual bool IsLoaded() const override { return ObjectState.bLoadComplated; }
	bool IsPlayer() const override;

	APawn* GetPawn() override;
	IT4GameWorld* GetGameWorld() const;

	bool DoExecuteAction(const FT4ActionStruct* InAction, const FT4ActionParameters* InParam) override; // #76 : ActionControl only

	// #34 : for Server All or Client Only Player
	IT4ObjectController* GetObjectController() override { return GameplayControlRef; } // #34, #42, #63
	void SetObjectController(IT4ObjectController* InControl) override { GameplayControlRef = InControl; } // #34, #42, #63

	virtual IT4AnimControl* GetAnimControl() const override { return nullptr; } // #14
	IT4ActionControl* GetActionControl() override; // #20
	const FT4GameObjectProperty& GetPropertyConst() const override { return ObjectProperty; } // #34

#if (WITH_EDITOR || WITH_SERVER_CODE)
	FT4ServerGameObjectAttribute& GetServerAttribute() override { return ServerObjectAttribute; } // #46
	FT4ServerGameObjectDelegates& GetServerDelegates() override { return ServerObjectDelegates; } // #49

	bool IsWeaponHitOverlapEventEnabled() const { return bHitOverlapEventEnabled; } // #49
	void BeginWeaponHitOverlapEvent(const FName& InHitOverlapEventName) override; // #49
	void EndWeaponHitOverlapEvent() override; // #49
#endif

	float GetLifeTimeSec() const override { return ObjectState.LifeTimeSec; } // #102
	float GetTimeScale() const override { return ObjectState.TimeScale; } // #102

	virtual bool IsLockOn() const { return false; } // #33
	virtual bool IsFalling() const { return false; }
	virtual bool IsFlying() const { return false; }
	virtual bool IsRolling() const { return false; } // #46
	virtual bool IsTurning() const { return ObjectState.bTurning; } // #46

	virtual bool HasPlayingAnimState(const FName& InAnimStateName) const override { return false; } // #47

	virtual bool HasPublicAction(const FT4ActionKey& InActionKey) const override; // #102 : 존재만 해도 true 리턴
	virtual bool IsPlayingPublicAction(const FT4ActionKey& InActionKey) const override; // #20, #76 : Playing 중인지를 체크. Paused 면 False 가 리턴됨!

	virtual const FVector GetCOMLocation() const override; // #18 : WARN : ActorLocation 캐릭터의 경우 Coll Capsule 의 중점이다.
	virtual const FVector GetRootLocation() const override;
	virtual const FVector GetNavPoint() const override { return GetRootLocation(); } // #52

	virtual const FRotator GetRotation() const override;
	virtual const FVector GetFrontVector() const override; // #38
	virtual const FVector GetRightVector() const override; // #38

	virtual const FVector GetMovementVelocity() const override;
	virtual const float GetMovementSpeed() const override;

	virtual const float GetOpacity() const override { return ObjectState.OpacityValue; } // #78

	virtual bool HasReaction(const FName& InReactionName) const override { return false; } // #73
	virtual bool HasLayerTag(const FName& InLayerTagName, ET4LayerTagType InLayerTagType) const override { return false; } // #81
	virtual bool HasActionPoint(const FName& InActionPoint) const override { return false; } // #57 : ActionPoint = Socket or Bone or VirtualBone

	virtual bool GetSocketLocation(const FName& InSocketName, FVector& OutLocation) const override { return false; } // #18
	virtual bool GetSocketRotation(const FName& InSocketName, ERelativeTransformSpace InTransformSpace, FRotator& OutRotation) const override { return false; } // #18
	virtual bool GetSocketScale(const FName& InSocketName, ERelativeTransformSpace InTransformSpace, FVector& OutScale) const override { return false; } // #54
	virtual bool GetSocketTransform(const FName& InSocketName, ERelativeTransformSpace InTransformSpace, FTransform& OutTransform) const override { return false; }  // #58

	virtual void SetHeightOffset(float InOffset) override {} // #18

#if !UE_BUILD_SHIPPING
	FT4GameObjectDebugInfo& GetDebugInfo() override { return DebugInfo; } // #76
#endif
	// ~IT4GameObject

public:
	bool OnCreate(
		ET4LayerType InLayerType,
		const FT4SpawnObjectAction* InAction
	);

	bool OnCreate(
		ET4LayerType InLayerType,
		ET4ObjectType InWorldObjectType, // #63 : Only World Object
		const FName& InName,
		const FT4ObjectID& InObjectID,
		const FVector& InLocation,
		const FRotator& InRotation,
		const FVector& InScale
	); // #54 : ClientObject Only

	void OnReset();

	void OnWorldEnterStart(float InFadeInTimeSec); // #78
	void OnWorldLeaveStart(float InFadeOutTimeSec); // #36 : Leave 시의 Ghost 처리. Coll 충돌 제외 등...

	void OnUpdateTickActorBefore(const FT4UpdateTime& InUpdateTime) { UpdateTickActorBefore(InUpdateTime); }
	void OnUpdateTickActorAfter(const FT4UpdateTime& InUpdateTime) { UpdateTickActorAfter(InUpdateTime); }

	bool IsGhost() const { return ObjectState.bGhost; }
	virtual bool IsDestroyable() { return (ObjectState.bGhost && 0.0f >= ObjectState.GhostOutTimeLeft) ? true : false; } // #54

	float GetLaningDistance(float InMaxDistance); // #38

	virtual void SetAccelerationMoveSpeedScale(float InMoveSpeedScale) {} // #38 : 가감속! MyPC 만...

	virtual void StartTurning(const FRotator& InRotation, float InRotationYawRate); // #44 : TurnNode 에서 호출 (0 == InRotationYawRate) ? immediate

	virtual bool StartPhysics(bool bSimulateBodiesBelow) { return false; } // #76 : ReactionNode 에서 호출
	virtual void StopPhysics() {} // #76 : ReactionNode 에서 호출

	virtual void SetAutoDestroy() {} // #68 : WorldObject 만 받는다.

	virtual void PlayLayerTag(
		const FName InLayerTagName, 
		ET4LayerTagType InLayerTagType,
		const FT4ActionKey& InActionKey
	) 
	{
	} // #74, #81
	virtual void StopLayerTag(const FT4ActionKey& InActionKey); // #74, #81

	virtual bool PlayMaterialByLayerTag(
		const FT4EntityOverrideMaterialData& InLayerTagData,
		const FT4ActionKey& InActionKey
	); // #81
	virtual bool PlayWeaponByLayerTag(
		const FT4EntityLayerTagWeaponData& InLayerTagData, 
		const FT4ActionKey& InActionKey
	); // #81
	virtual bool PlayContiByLayerTag(
		const FT4EntityLayerTagContiData& InLayerTagData, 
		const FT4ActionKey& InActionKey
	); // #81

	virtual void SetCameraTargetBlend(
		float InLocalBlendWeight,
		const FT4CameraWorkSectionKeyData* InSourceSectionKey,
		const FT4CameraWorkSectionKeyData* InTargetSectionKey,
		float InBlendWeight
	); // #58 : Only Player obj

#if !UE_BUILD_SHIPPING
	virtual bool DoActionPlaybackStartRecording() { return false; } // #68

	virtual void SetDebugPause(bool bInPause); // #102
#endif

protected:
	friend class FT4BaseAnimControl;
	friend class FT4HumanAnimControl; // #38
	friend class FT4HumanJumpingAnimState; // #47
	friend class FT4ActionAttachedNode;
	friend class FT4ActionTurnNode; // #44
	friend class FT4ActionTeleportToNode;
	friend class FT4ActionEquipWeaponNode; // #49
	friend class FT4ManualControl; // #76
	friend class FT4ActionReactionNode; // #76
	friend class FT4ActionTimeScaleNode; // #102
	friend class FT4WorldContainer; // #99

	virtual UT4BaseAnimInstance* GetAnimInstance() const { return nullptr; }
	virtual USceneComponent* GetAttachParentComponent() { return nullptr; } // #54
	virtual UCapsuleComponent* GetCapsuleComponent() const { return nullptr; } // #33, #76
	virtual USkeletalMeshComponent* GetSkeletalMeshComponent() const { return nullptr; } // #33, #76
	virtual UT4EnvironmentZoneComponent* GetEnvironmentComponent(); // #99 : 현재는 Attached 에서만 찾는다. ActionNode 를 사용하기 때문

	virtual bool IsSimulatingPhysics() const { return false; } // #76

	virtual bool IsMovementLocked() const { return false; } // #76

	void AddDynamicMaterialInstances(UMeshComponent* InMeshComponent); // #78
	void ResetDynamicMaterialInstances(); // #78

	void SetDynamicMaterialInstanceParameter(FName InParameterName, float InValue); // #78

	void SetOpacity(float InTargetAlpha, float InTransitionMaxTimeSec); // #78
	void SetOpacity(float InSourceAlpha, float InTargetAlpha, float InTransitionMaxTimeSec); // #78
	void ResetOpacityState(); // #78

	void SetPause(bool bInPause); // #63
	void SetTimeScale(float InTimeScale); // #102

	void AddAttachedComponent(USceneComponent* InAttachedComponent); // #48
	void RemoveAttachedComponent(USceneComponent* InAttachedComponent); // #48

	// #68 : 리로드 또는 playback 을 위해 장착 정보를 보관한다. 단, 현재는 Character 만 사용
	virtual void AddEquipmentInfo(const FT4ActionKey& InKey, const FName InEquipPoint, const FT4EntityKey& InEntityKey) {}
	virtual void RemoveEquipmentInfo(const FT4ActionKey& InKey) {}
	// ~#68

	FT4GameObjectState& GetState() { return ObjectState; } // #76
	FT4GameObjectProperty& GetProperty() { return ObjectProperty; } // #34

	FT4ManualControl& GetManualControl() { return ManualControl; } // #76

	void SetLoadComplated();
	void ResetLoadComplated();

	IT4ActionPlaybackRecorder* GetActionPlaybackRecorder() const; // #68 : 녹화를 사용할 경우만 리턴

	UFUNCTION()
	void HandleOnHitOverlapOnlyServer(
		UPrimitiveComponent* InOverlappedComp,
		AActor* InOther,
		UPrimitiveComponent* InOtherComp,
		int32 InOtherBodyIndex,
		bool bInFromSweep,
		const FHitResult& InSweepResult
	); // #49

protected:
	virtual void Reset() {}

	virtual void WorldEnterStart() {} // #78 : AsyncLoading 완료 후 호출된다. 즉, 그릴 준비까지 모두 완료 상태다.
	virtual void WorldLeaveStart() {} // #36 : Leave 시의 Ghost 처리. Coll 충돌 제외 등...

	virtual bool Create(const FT4SpawnObjectAction* InAction)
	{
		check(false); // #54 : 여기 호출되면 안됨!!
		return false;
	}

	virtual void CreateFinished(const FVector& InLocation,  const FRotator& InRotation, const FVector& InScale);

	virtual void NotifyBeginPlay() {} // #49
	virtual void NotifyEndPlay() {} // #49

	virtual void UpdateTickActorBefore(const FT4UpdateTime& InUpdateTime) {} // WARN : ProcessAction 전!
	virtual void UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime) {} // WARN : ProcessAction 후!

	FORCEINLINE bool HasPlayingPrivateAction(const FT4ActionKey& InActionKey) const
	{
		return ManualControl.IsPlaying(InActionKey);
	}
	FORCEINLINE bool ExecutePublicAction(const FT4ActionStruct* InAction, const FT4ActionParameters* InParam) // #76
	{
		return ExecuteDispatch(&ActionControl, InAction, InParam);
	}
	FORCEINLINE bool ExecutePrivateAction(const FT4ActionStruct* InAction, const FT4ActionParameters* InParam) // #76
	{
		return ExecuteDispatch(&ManualControl, InAction, InParam);
	}

	// T4_ADD_ACTION_TAG_CODE
	virtual bool ExecuteStopAction(FT4ActionControl* InControl, const FT4StopAction& InAction); // #20
	virtual bool ExecuteMoveAsyncToAction(const FT4MoveAsyncAction& InAction) { return true; } // #40
	virtual bool ExecuteMoveSyncToAction(const FT4MoveSyncAction& InAction) { return true; } // #40
	virtual bool ExecuteJumpToAction(const FT4JumpAction& InAction) { return true; }
	virtual bool ExecuteRollToAction(const FT4RollAction& InAction) { return true; } // #46
	virtual bool ExecuteTeleportToAction(const FT4TeleportAction& InAction) { return true; }
	virtual bool ExecuteMoveStopAction(const FT4MoveStopAction& InAction) { return true; } // #52
	virtual bool ExecuteMoveSpeedSyncAction(const FT4MoveSpeedSyncAction& InAction) { return true; } // #52
	virtual bool ExecuteLaunchAction(const FT4LaunchAction& InAction, const FT4ActionParameters* InActionParam) { return true; } // #63 : only Projectile
	virtual bool ExecuteLockOnAction(const FT4LockOnAction& InAction) { return true; }
	virtual bool ExecuteChangeStanceAction(const FT4ChangeStanceAction& InAction) { return true; } // #73
	virtual bool ExecuteUnEquipWeaponAction(FT4ActionControl* InControl, const FT4UnEquipWeaponAction& InAction); // #48
	virtual bool ExecuteExchangeCostumeAction(const FT4ExchangeCostumeAction& InAction) { return true; } // #72
	virtual bool ExecuteHitAction(const FT4HitAction& InAction) { return true; } // #76
	virtual bool ExecuteDieAction(const FT4DieAction& InAction) { return true; } // #76
	virtual bool ExecuteResurrectAction(const FT4ResurrectAction& InAction) { return true; } // #76

#if WITH_EDITOR
	virtual bool ExecuteEditorAction(const FT4EditorAction& InAction) { return true; } // #37
#endif

private:
	void AdvanceActions(const FT4UpdateTime& InUpdateTime); // #20
	void AdvanceOpacity(const FT4UpdateTime& InUpdateTime); // #78
	void AdvanceRotation(const FT4UpdateTime& InUpdateTime); // #44

	bool ExecuteDispatch(
		FT4ActionControl* InControl, // #76
		const FT4ActionStruct* InAction,
		const FT4ActionParameters* InParam
	);
	
	bool ExecuteDispatchImmediate(
		FT4ActionControl* InControl, // #76
		const FT4ActionStruct* InAction,
		const FT4ActionParameters* InParam,
		bool& bOutResult
	);

protected:
	UPROPERTY(VisibleAnywhere)
	TArray<UMaterialInstanceDynamic*> MaterialDynamicInstances; // #78

	//UPROPERTY(VisibleAnywhere)
	//TMultiMap<FName, UMaterialInstanceDynamic*> MaterialDynamicInstanceMap; // #78

protected:
	ET4LayerType LayerType;
	FT4ObjectID ObjectID;
	FName Name;
	FT4EntityKey EntityKey; // #35
	FSoftObjectPath EntityAssetPath; // #68
	FName GameDataIDName; // #48

private:
	FT4GameObjectState ObjectState; // #76
	FT4GameObjectProperty ObjectProperty; // #33
	FT4ActionControl ActionControl; // #20 , #76 : 외부 제어!
	FT4ManualControl ManualControl; // #76 : 내부 시스템 제어
	IT4ObjectController* GameplayControlRef; // #42, #63 : 참조!! 소멸 책임 없음!

	TArray<USceneComponent*> AttachedComponents; // #48, #49

#if (WITH_EDITOR || WITH_SERVER_CODE)
	FT4ServerGameObjectAttribute ServerObjectAttribute; // #46 : TODO: Gameplay 테이블에서 읽어서 채울 것!
	FT4ServerGameObjectDelegates ServerObjectDelegates; // #49

	bool bHitOverlapEventEnabled;
	FName HitOverlapEventName; // #49
#endif

#if !UE_BUILD_SHIPPING
	FT4GameObjectDebugInfo DebugInfo; // #76
#endif
};
