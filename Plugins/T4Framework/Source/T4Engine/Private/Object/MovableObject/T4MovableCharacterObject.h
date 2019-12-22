// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Object/T4GameObject.h"
#include "Object/Component/Movement/T4MovementStructs.h"

#include "Public/Asset/T4AssetLoader.h"

#include "T4MovableCharacterObject.generated.h"

/**
  *
 */
class UT4CharacterEntityAsset;
class FT4BaseAnimControl;
class FT4CharacterDataLoader;
class FT4CharacterAnimationDataLoader;

class UT4CapsuleComponent;
class UT4MovementComponent;
class UT4SkinnedMeshComponent;
class UT4SkeletalMeshComponent;

UCLASS()
class AT4MovableCharacterObject : public AT4GameObject
{
	GENERATED_UCLASS_BODY()

	// Disable compiler-generated deprecation warnings by implementing our own destructor
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	virtual ~AT4MovableCharacterObject();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

public:
	// APawn
	void PostInitializeComponents() override;
	UPawnMovementComponent* GetMovementComponent() const override;
	UPrimitiveComponent* GetMovementBase() const override final { return BasedMovement.MovementBase;}

	void PossessedBy(AController* NewController) override;
	void UnPossessed() override;

public:
	// IT4GameObject
	ET4ObjectType GetObjectType() const override { return ET4ObjectType::Entity_Character; }

	const UT4EntityAsset* GetEntityAsset() override; // #39

	const FName GetStanceName() const override { return StanceNameSelected; } // #73

	IT4AnimControl* GetAnimControl() const override; // #14

	bool IsLockOn() const override; // #33
	bool IsFalling() const override;
	bool IsFlying() const override;

	bool HasPlayingAnimState(const FName& InAnimStateName) const override; // #47

	const FVector GetNavPoint() const override; // #52

	bool HasReaction(const FName& InReactionName) const override; // #73
	bool HasLayerTag(const FName& InLayerTagName, ET4LayerTagType InLayerTagType) const override; // #81
	bool HasActionPoint(const FName& InActionPoint) const override; // #57 : ActionPoint = Socket or Bone or VirtualBone

	bool GetSocketLocation(const FName& InSocketName, FVector& OutLocation) const override; // #18
	bool GetSocketRotation(
		const FName& InSocketName, 
		ERelativeTransformSpace InTransformSpace, 
		FRotator& OutRotation
	) const override; // #18
	bool GetSocketScale(
		const FName& InSocketName,
		ERelativeTransformSpace InTransformSpace,
		FVector& OutScale
	) const override; // #54
	bool GetSocketTransform(
		const FName& InSocketName,
		ERelativeTransformSpace InTransformSpace,
		FTransform& OutTransform
	) const override; // #58

	void SetHeightOffset(float InOffset) override; // #18

#if WITH_EDITOR
	void SetDebugPause(bool bInPause) override; // #102
#endif

public:
	// AT4GameObject
	void SetAccelerationMoveSpeedScale(float InMoveSpeedScale) override; // #38 : 가감속! MyPC 만...
	
	void StartTurning(const FRotator& InRotation, float InRotationYawRate) override; // #44 : Turn 에서 호출 (0 == InRotationYawRate) ? immediate
	
	bool StartPhysics(bool bSimulateBodiesBelow) override; // #76 : ReactionNode 에서 호출
	void StopPhysics() override;  // #76 : ReactionNode 에서 호출

	void PlayLayerTag(
		const FName InLayerTagName, 
		ET4LayerTagType InLayerTagType, 
		const FT4ActionKey& InActionKey
	) override; // #74, #81
	void StopLayerTag(const FT4ActionKey& InActionKey) override; // #74, #81

	bool PlayMaterialByLayerTag(
		const FT4EntityOverrideMaterialData& InLayerTagData,
		const FT4ActionKey& InActionKey
	) override; // #81

#if !UE_BUILD_SHIPPING
	bool DoActionPlaybackStartRecording() override; // #68
#endif

protected:
	friend class UT4MovementComponent; // #33
	friend class FT4ManualReactionNode; // #76

	// AT4GameObject
	UT4BaseAnimInstance* GetAnimInstance() const override;
	USceneComponent* GetAttachParentComponent() override; // #54
	UCapsuleComponent* GetCapsuleComponent() const override; // #33
	USkeletalMeshComponent* GetSkeletalMeshComponent() const override; // #33, #76

	bool IsSimulatingPhysics() const override { return bPhysicsEnabled; } // #76

	bool IsMovementLocked() const override { return bPhysicsEnabled && !bPhysicsSimulateBodiesBelow; } // #76

	// #68
	void AddEquipmentInfo(
		const FT4ActionKey& InKey,
		const FName InActionPoint,
		const FT4EntityKey& InEntityKey
	) override;
	void RemoveEquipmentInfo(const FT4ActionKey& InKey) override;
	// ~#68

	// ~AT4GameObject

	// #33
	void SetBase(UPrimitiveComponent* NewBase, const FName BoneName = NAME_None, bool bNotifyActor = true);

	void SaveRelativeBasedMovement(
		const FVector& NewRelativeLocation,
		const FRotator& NewRotation,
		bool bRelativeRotation
	);

	FORCEINLINE const FGroundMovementInfo& GetBasedMovement() const { return BasedMovement; }
	// ~#33

protected:
	void Reset() override;

	void WorldEnterStart() override; // #78
	void WorldLeaveStart() override; // #36 : Leave 시의 Ghost 처리. Coll 충돌 제외 등...

	bool Create(const FT4SpawnObjectAction* InAction) override;

	void UpdateTickActorBefore(const FT4UpdateTime& InUpdateTime) override;
	void UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime) override;

	// T4_ADD_ACTION_TAG_CODE
	bool ExecuteMoveAsyncToAction(const FT4MoveAsyncAction& InAction) override; // #40
	bool ExecuteMoveSyncToAction(const FT4MoveSyncAction& InAction) override; // #40
	bool ExecuteJumpToAction(const FT4JumpAction& InAction) override; // #46
	bool ExecuteRollToAction(const FT4RollAction& InAction) override; // #46
	bool ExecuteTeleportToAction(const FT4TeleportAction& InAction) override;
	bool ExecuteMoveStopAction(const FT4MoveStopAction& InAction) override; // #52
	bool ExecuteMoveSpeedSyncAction(const FT4MoveSpeedSyncAction& InAction) override; // #52
	bool ExecuteLockOnAction(const FT4LockOnAction& InAction) override;
	bool ExecuteChangeStanceAction(const FT4ChangeStanceAction& InAction) override; // #73
	bool ExecuteExchangeCostumeAction(const FT4ExchangeCostumeAction& InAction) override; // #72
	bool ExecuteHitAction(const FT4HitAction& InAction) override; // #76
	bool ExecuteDieAction(const FT4DieAction& InAction) override; // #76
	bool ExecuteResurrectAction(const FT4ResurrectAction& InAction) override; // #76

#if WITH_EDITOR
	bool ExecuteEditorAction(const FT4EditorAction& InAction) override; // #37
#endif

private:
	void ResetMeshes();  // #37
	void ResetSimulatePhysics(); // #76

	void ClearAsyncLoaders(); // #72
	void ClearOverrideMaterialLoader();  // #80
	void ClearMeshDataLoader(); // #72
	void ClearPhysicsAssetDataLoader(); // #76
	void ClearAnimBlueprintClassLoader();
	void ClearAsyncAnimationDataLoader(); // #73

	bool CheckAsyncLoading();
	bool CheckOverrideMaterialAsyncLoading(bool bInitialize); // #80
	bool CheckMeshAsyncLoading(bool bInitialize); // #72
	bool CheckPhysicsAssetAsyncLoading(); // #76
	bool CheckAnimationAsyncLoading(); // #73

	void SetEntityAttributes();
	bool SetStance(const FName InStanceName); // #73
	void SetPhysicsComponentSettings(bool bRagdoll); // #76

	void SetFullbodyOverrideMaterialAsyncLoading(); // #80
	void AddCompositeOverrideMaterialAsyncLoading(
		const FName InPartName,
		const FT4EntityOverrideMaterialData& InOverrideMaterialData,
		const TCHAR* InDebugString
	); // #80

#if WITH_EDITOR
	void RecreateAll(); // #76
	void ReloadOverrideMaterialAsyncLoading(); // #80
#endif

#if !UE_BUILD_SHIPPING
	void DebugDrawObjectInfo();
	void DebugDrawServerLocation(); // #52
#endif

public:
	UPROPERTY(Category=Character, VisibleAnywhere)
	UT4CapsuleComponent* CapsuleComponent;

	UPROPERTY(Category=Character, VisibleAnywhere)
	UT4MovementComponent* MovementComponent;

	UPROPERTY(Category=Character, VisibleAnywhere)
	UT4SkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY()
	FGroundMovementInfo BasedMovement;

	UPROPERTY()
	TMap<FName, UT4SkinnedMeshComponent*> MasterPoseModularMeshes; // #37, #72

private:
	TWeakObjectPtr<const UT4CharacterEntityAsset> CharacterEntityPtr;

	FT4CharacterAnimationDataLoader* AnimationDataLoader; // #39, #69
	FT4AnimBlueprintClassLoader AnimBPClassLoader;

	bool bOverrideMaterialLoading; // #80
	struct FT4OverrideMaterialLoadInfo
	{
		FName PartName; // NAME_None = Master
		TArray<FT4MaterialLoader> MaterialLoaders;
		bool bClear; // #78 : 적용된 Mesh 에서 Override Material 만 삭제할 경우 사용
	};
	TArray<FT4OverrideMaterialLoadInfo> OverrideMaterialLoadInfos; // #80
	FT4ActionKey LayerSetMaterialActionKey; // #81

	// #37, #72
	struct FT4SkeletalMeshLoadInfo
	{
		FName PartName;
		FT4SkeletalMeshLoader SkeletalMeshLoader;
	};
	bool bMeshAsyncLoading;
	TArray<FT4SkeletalMeshLoadInfo> PendingMeshLoadInfos;
	ET4EntityCharacterMeshType MeshType;
	ET4EntityCharacterModularType ModularType; // #72
	FName MasterPosePartName;
	// ~#72

	bool bPhysicsAssetAsyncLoading; // #76
	FT4PhysicsAssetLoader PhysicsAssetLoader; // #76

	bool bStanceAsyncLoading; // #73
	FName StanceNameSelected; // #73

	FT4BaseAnimControl* AnimControl;

	TMap<FT4ActionKey, FT4EquipmentInfo> EquipmentInfos; // #68 : 리로드 또는 playback 을 위해 장착 정보를 보관한다. 단, 현재는 Character 만 사용

	// #76
	bool bPhysicsEnabled;
	bool bPhysicsSimulateBodiesBelow;
	FTransform PhysicsRestoreTM;
	// ~#76

#if WITH_EDITOR
	// #52
	FVector SyncLocationForServer;
	FVector SyncDirectionForServer;
#endif
};
