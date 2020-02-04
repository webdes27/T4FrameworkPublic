// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4GameObject.h"

#include "Animation/T4BaseAnimControl.h"
#include "ActionNode/T4ActionNodeIncludes.h" // #34

#include "Equipment/T4BaseEquipment.h" // #107

#include "Object/Component/T4EnvironmentZoneComponent.h" // #99

#include "Public/T4EngineAnimNotify.h" // #111
#include "Public/T4EngineConstants.h" // #108
#include "Public/T4Engine.h"

#include "Classes/Camera/T4SpringArmComponent.h" // #58

#include "T4Asset/Public/Action/T4ActionContiCommands.h" // #58

#include "Camera/CameraComponent.h" // #100
#include "Components/MeshComponent.h" // #78
#include "Materials/MaterialInstanceDynamic.h" // #78

#include "Kismet/GameplayStatics.h"

#include "T4EngineInternal.h"

/**
  *
 */
AT4GameObject::AT4GameObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, LayerType(ET4LayerType::Max)
	, Name(NAME_None)
	, GameDataIDName(NAME_None)
	, NetworkControlRef(nullptr) // #42, #63 : 참조!! 소멸 책임 없음!
{
	bReplicates = false; // #15
	SetReplicatingMovement(false); // #4.24 : bReplicateMovement = false; // #33
	bNetLoadOnClient = false; // #15
	TearOff(); // #15

	ActionNodeControl.Set(this); // #20 , #76 : 외부 제어!
	ActionTaskControl.Set(this); // #76 : 내부 시스템 제어
}

AT4GameObject::~AT4GameObject()
{
}

void AT4GameObject::TickActor(
	float InDeltaTime,
	enum ELevelTick InTickType,
	FActorTickFunction& InThisTickFunction
)
{
	FT4UpdateTime UpdateTimeInfo; // #102
	{
		UpdateTimeInfo.RealTimeSec = InDeltaTime;
		UpdateTimeInfo.ScaledTimeSec = (ObjectState.bPaused) ? 0.0f : InDeltaTime * ObjectState.TimeScale;
		UpdateTimeInfo.TimeScale = ObjectState.TimeScale;
#if !UE_BUILD_SHIPPING
		if (ObjectState.bDebugPaused) // #63
		{
			UpdateTimeInfo.ScaledTimeSec = 0.0f;
			UpdateTimeInfo.TimeScale = 0.0f;
		}
#endif
		UpdateTimeInfo.bPaused = ObjectState.bPaused;

		ObjectState.LifeTimeSec += UpdateTimeInfo.ScaledTimeSec; // #102
	}

	OnUpdateTickActorBefore(UpdateTimeInfo);

	//Super::TickActor(InDeltaTime, InTickType, InThisTickFunction);
	Super::TickActor(UpdateTimeInfo.ScaledTimeSec, InTickType, InThisTickFunction);

	// WARN : 이동 (MovementComponent) 에 관련된 Action 은 ExecuteDispatchSync 를 통해 실행되도록 처리 필요!
	AdvanceActions(UpdateTimeInfo);
	AdvanceOpacity(UpdateTimeInfo); // #78
	AdvanceRotation(UpdateTimeInfo); // #44
	AdvanceEquipments(UpdateTimeInfo); // #107

	OnUpdateTickActorAfter(UpdateTimeInfo);

	if (ObjectState.bGhost) // #54
	{
		ObjectState.GhostOutTimeLeft -= UpdateTimeInfo.ScaledTimeSec; // InDeltaTime;
	}
}

void AT4GameObject::AdvanceActions(const FT4UpdateTime& InUpdateTime)
{
	ActionTaskControl.Advance(InUpdateTime); // #76
	ActionNodeControl.Advance(InUpdateTime); // #20
}

void AT4GameObject::AdvanceOpacity(const FT4UpdateTime& InUpdateTime) // #78
{
	if (!ObjectState.bTransitionOpacity)
	{
		return;
	}
	ObjectState.TransitionTimeSec += InUpdateTime.ScaledTimeSec;
	if (ObjectState.TransitionTimeSec >= ObjectState.TransitionTimeMaxSec)
	{
		ObjectState.OpacityValue = ObjectState.TargetOpacityValue;
		ObjectState.bTransitionOpacity = false;
	}
	else
	{
		float TimeRatio = FMath::Clamp(ObjectState.TransitionTimeSec / ObjectState.TransitionTimeMaxSec, 0.0f, 1.0f);
		if (ObjectState.OpacityValue < ObjectState.TargetOpacityValue)
		{
			ObjectState.OpacityValue = FMath::Max(
				ObjectState.OpacityValue,
				TimeRatio * ObjectState.TargetOpacityValue
			);
		}
		else
		{
			ObjectState.OpacityValue = FMath::Min(
				ObjectState.OpacityValue,
				(1.0f - ObjectState.TargetOpacityValue) * (1.0f - TimeRatio)
			);
		}
	}
	SetDynamicMaterialInstanceParameter(
		T4EngineConstant::GetMaterialParameterOpacityName(),
		FMath::Clamp(ObjectState.OpacityValue, 0.0f, 1.0f)
	);
#if 0
	T4_LOG(
		Warning,
		TEXT("AdvanceOpacity : %.2f"),
		ObjectState.OpacityValue
	);
#endif
}

void AT4GameObject::AdvanceRotation(const FT4UpdateTime& InUpdateTime)
{
	// #44, #47 : Turn 처리를 Action 에서 Object 단으로 이동. Action 없이도 동작할 수 있도록 지원
	if (!ObjectState.bTurning)
	{
		return;
	}
	FRotator CurrentRotator = GetRotation();
	if (!ObjectState.TurnGoalRotation.Equals(CurrentRotator)) // todo : optimizing
	{
		const float RotationAngle = ObjectState.TurnRotationYawRate * InUpdateTime.ScaledTimeSec;
		FRotator NewRotator = CurrentRotator;
		NewRotator.Yaw = FMath::FixedTurn(CurrentRotator.Yaw, ObjectState.TurnGoalRotation.Yaw, RotationAngle);
		SetActorRotation(NewRotator);
		if (ObjectState.TurnGoalRotation.Equals(NewRotator)) // todo : optimizing
		{
			ObjectState.bTurning = false;
			ObjectState.TurnGoalRotation = FRotator::ZeroRotator;
			ObjectState.TurnRotationYawRate = 0.0f;
		}
	}
}

void AT4GameObject::AdvanceEquipments(const FT4UpdateTime& InUpdateTime) // #107
{
	for (int32 idx = Equipments.Num() - 1; idx >= 0; --idx) // #111
	{
		FT4BaseEquipment* EquipmentBase = Equipments[idx];
		check(nullptr != EquipmentBase);
		EquipmentBase->OnAdvance(InUpdateTime);
		if (EquipmentBase->IsDetachable() && EquipmentBase->IsDestroyable())
		{
			EquipmentBase->OnReset();
			delete EquipmentBase;
			Equipments.RemoveAt(idx);
		}
	}
}

bool AT4GameObject::ShouldTickIfViewportsOnly() const
{
	return T4EngineLayer::IsLevelEditor(LayerType); // #17
}

void AT4GameObject::EndPlay(const EEndPlayReason::Type InEndPlayReason)
{
	NotifyEndPlay(); // #49
	Super::EndPlay(InEndPlayReason);
}

void AT4GameObject::FellOutOfWorld(const class UDamageType& dmgType) // #104
{
	// #104 : World 밖으로 사라져도 강제로 삭제하는 처리는 하지 않는다. 자동 삭제된다!!
	//        Projectile 류가 강제 삭제되는 문제가 있었음!
	if (HasAuthority() || GetLocalRole() == ROLE_None)
	{
		DisableComponentsSimulatePhysics();
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		// Destroy();
	}
}

void AT4GameObject::BeginPlay()
{
	Super::BeginPlay();
	check(ET4LayerType::Max != LayerType);
	ET4LayerType TestLayerType = T4EngineLayer::Get(GetWorld());
	check(TestLayerType == LayerType);
	NotifyBeginPlay(); // #49
}

bool AT4GameObject::HasPlayer() const
{
	if (nullptr == NetworkControlRef)
	{
		return false;
	}
	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	if (!GameWorld->ComparePlayerObject(GetObjectID())) // 서버를 제외한 PlayerController 는 1개 밖에 없다.
	{
		return false;
	}
	return true;
}

APawn* AT4GameObject::GetPawn()
{
	return Cast<APawn>(this);
}

IT4GameWorld* AT4GameObject::GetGameWorld() const
{
	return T4EngineWorldGet(LayerType);
}

IT4ActionControl* AT4GameObject::GetActionControl()
{
	return static_cast<IT4ActionControl*>(&ActionNodeControl); // #20
} 

void AT4GameObject::HandleOnHitOverlapOnlyServer(
	UPrimitiveComponent* InOverlappedComp,
	AActor* InOther,
	UPrimitiveComponent* InOtherComp,
	int32 InOtherBodyIndex,
	bool bInFromSweep,
	const FHitResult& InSweepResult
) // #49
{
#if (WITH_EDITOR || WITH_SERVER_CODE)
	// TODO : 무기 Key 를 넘겨 해당 무기의 활성화 여부를 판단해야 할 것!
	bool HasOverlapEvent = false;
	FName OverlapEventName = NAME_None;
	for (FT4BaseEquipment* EquipmentBase : Equipments)
	{
		check(nullptr != EquipmentBase);
		if (EquipmentBase->IsWeapon())
		{
			if (EquipmentBase->HasOverlapEvent())
			{
				HasOverlapEvent = true;
				OverlapEventName = EquipmentBase->GetOverlapEventName();
			}
		}
	}
	if (!HasOverlapEvent)
	{
		return;
	}
	if (!InSweepResult.Actor.IsValid() || this == InSweepResult.Actor.Get())
	{
		return;
	}
	IT4GameObject* HitGameObject = T4EnginetInternal::TryCastGameObject(InSweepResult.Actor.Get());
	if (nullptr == HitGameObject)
	{
		return;
	}
	GetServerDelegates().OnHitOverlap.Broadcast(OverlapEventName, HitGameObject, InSweepResult);
#endif
}

bool AT4GameObject::HasAction(const FT4ActionKey& InActionKey) const
{
	// #102 : 존재만 해도 true 리턴
	return ActionNodeControl.HasAction(InActionKey);
}

bool AT4GameObject::IsPlayingAction(const FT4ActionKey& InActionKey) const
{
	// #20, #76 : Playing 중인지를 체크. Paused 면 False 가 리턴됨!
	return ActionNodeControl.IsPlaying(InActionKey);
}

const FVector AT4GameObject::GetCOMLocation() const
{
	return GetActorLocation();
}

const FVector AT4GameObject::GetRootLocation() const
{
	FVector RootLocation = GetActorLocation();
	RootLocation.Z -= GetPropertyConst().HalfHeight;
	return RootLocation;
}

const FRotator AT4GameObject::GetRotation() const
{
	return GetActorRotation();
}

const FVector AT4GameObject::GetFrontVector() const // #38
{
	return GetActorRotation().Vector();
}

const FVector AT4GameObject::GetRightVector() const // #38
{
	return GetActorRotation().RotateVector(FVector::RightVector);
}

const FVector AT4GameObject::GetMovementVelocity() const
{
	return GetVelocity();
}

const float AT4GameObject::GetMovementSpeed() const
{
	return GetVelocity().Size();
}

void AT4GameObject::SetLoadComplated()
{
	check(!ObjectState.bLoadComplated);
	ObjectState.bLoadComplated = true;
}

void AT4GameObject::ResetLoadComplated()
{
	ObjectState.bLoadComplated = false;
}

IT4ActionReplayRecorder* AT4GameObject::GetActionReplayRecorder() const // #68
{
	IT4GameWorld* GameWorld = T4EngineWorldGet(LayerType);
	if (nullptr != GameWorld)
	{
		return GameWorld->GetActionReplayRecorder();
	}
	return nullptr;
}

UT4EnvironmentZoneComponent* AT4GameObject::GetEnvironmentComponent() // #99
{
	for (USceneComponent* PrimitiveComponent : AttachedComponents) // TODO : Optimize
	{
		UT4EnvironmentZoneComponent* EnvironmentComponent = Cast<UT4EnvironmentZoneComponent>(PrimitiveComponent);
		if (nullptr != EnvironmentComponent)
		{
			return EnvironmentComponent;
		}
	}
	return nullptr;
}

float AT4GameObject::GetLaningDistance(float InMaxDistance)
{
	// #38
	if (!IsFalling())
	{
		return 0.0f;
	}
	FCollisionQueryParams DistanceTraceParams = FCollisionQueryParams(
		FName(TEXT("LaningDistance")), 
		true, 
		this
	);
	DistanceTraceParams.bTraceComplex = true;
	DistanceTraceParams.bReturnPhysicalMaterial = false;

	const FVector StartLocation = GetRootLocation();
	const FVector EndLocation = StartLocation + FVector(0.0f, 0.0f, -InMaxDistance);

	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);

	IT4WorldCollisionSystem* CollisionSystem = GameWorld->GetCollisionSystem();
	check(nullptr != CollisionSystem);

	FT4HitSingleResult HitResult;
	bool bResult = CollisionSystem->QueryLineTraceSingle(
		ET4CollisionChannel::CollisionVisibility,
		StartLocation,
		EndLocation,
		DistanceTraceParams,
		HitResult
	);
	if (!bResult)
	{
		return 0.0f;
	}
	return StartLocation.Z - HitResult.ResultLocation.Z;
}

void AT4GameObject::StartTurning(
	const FRotator& InRotation,
	float InRotationYawRate
)
{
	// #44 : Turn 에서 호출 (0 == InRotationYawRate) ? immediate
	if (0.0f >= InRotationYawRate)
	{
		SetActorRotation(InRotation);
	}
	else
	{
		ObjectState.bTurning = true;
		ObjectState.TurnGoalRotation = InRotation;
		ObjectState.TurnRotationYawRate = InRotationYawRate;
	}
}

FT4AnimInstanceID AT4GameObject::PlayAnimationAndBroadcast(const FT4AnimParameters& InAnimParameters) // #107
{ 
	// #116 : 무기는 AnimInstanceID 가 필요없다. AT4MovableCharacterObject::PlayAnimationAndBroadcast 에서 호출됨에 유의!
	for (FT4BaseEquipment* EquipmentBase : Equipments)
	{
		check(nullptr != EquipmentBase);
		EquipmentBase->PlayAnimation(InAnimParameters);
	}
	return INDEX_NONE;
} 

void AT4GameObject::InactivePlayTag(
	ET4PlayTagType InPlayTagType, 
	const FT4ActionKey& InActionKey
) // #74, #81
{
	if (ET4PlayTagType::All == InPlayTagType || ET4PlayTagType::Attachment == InPlayTagType)
	{
		// #111 : Weapon 은 Unequip 이 있어 별도 처리
		if (HasEquipment(InActionKey))
		{
			FT4UnequipWeaponAction NewAction;
			NewAction.EquipmentActionKey = InActionKey;
			ExecuteActionTask(&NewAction, nullptr);
		}
		if (ET4PlayTagType::Attachment == InPlayTagType)
		{
			return;
		}
	}
	FT4StopAction NewAction;
	NewAction.ActionKey = InActionKey;
	NewAction.StartTimeSec = 0.0f;
	NewAction.bSameKeyNameRemoveAll = true;
	ExecuteActionTask(&NewAction, nullptr);
}

bool AT4GameObject::PlayMaterialForPlayTag(
	const FT4EntityOverrideMaterialData& InPlayTagData,
	const FT4ActionKey& InActionKey
) // #81
{
	return false;
}

bool AT4GameObject::PlayAttachmentForPlayTag(
	const FT4EntityPlayTagAttachmentData& InPlayTagData,
	const FT4ActionKey& InActionKey
) // #81
{
	if (InPlayTagData.WeaponEntityAsset.IsNull())
	{
		return false;
	}
	FT4EquipWeaponAction NewAction;
	NewAction.EquipmentActionKey = InActionKey;
	NewAction.MainWeaponData.WeaponEntityAsset = InPlayTagData.WeaponEntityAsset;
	NewAction.MainWeaponData.OverrideEquipPoint = InPlayTagData.EquipPoint;
	ExecuteActionTask(&NewAction, nullptr);
	return true;
}

bool AT4GameObject::PlayContiForPlayTag(
	const FT4EntityPlayTagContiData& InPlayTagData,
	const FT4ActionKey& InActionKey
) // #81
{
	if (InPlayTagData.ContiAsset.IsNull())
	{
		return false;
	}
	FT4ContiAction NewAction;
	NewAction.ActionKey = InActionKey;
	NewAction.ContiAsset = InPlayTagData.ContiAsset;
	ExecuteActionTask(&NewAction, nullptr);
	return true;
}

void AT4GameObject::SetCameraTargetBlend(
	float InLocalBlendWeight,
	const FT4CameraWorkSectionKeyData* InSourceSectionKey,
	const FT4CameraWorkSectionKeyData* InTargetSectionKey,
	float InBlendWeight
) // #58 : Only Player obj
{
	UT4SpringArmComponent* SpringArmComponent = FindComponentByClass<UT4SpringArmComponent>();
	if (nullptr == SpringArmComponent)
	{
		return;
	}
	FT4GameObjectProperty& Property = GetProperty();
	FVector SourceAtLocation = SpringArmComponent->BackupSocketOffset;
	if (nullptr != InSourceSectionKey)
	{
		if (InSourceSectionKey->LookAtPoint != NAME_None)
		{
			FTransform Transform;
			if (GetSocketTransform(InSourceSectionKey->LookAtPoint, ERelativeTransformSpace::RTS_Actor, Transform))
			{
				SourceAtLocation = Transform.GetTranslation();
				FTransform LocalTransform(Property.MeshImportRotation);
				SourceAtLocation = LocalTransform.TransformPosition(SourceAtLocation);
			}
			else
			{
				// TODO : 예외처리
			}
		}
		if (InSourceSectionKey->bInverse) // At 위치를 뒤집어 주고, 2배로 거리를 벌려준다. 촛점이 뒤집힘!
		{
			SourceAtLocation += -InSourceSectionKey->ViewDirection * (InSourceSectionKey->Distance * 2.0f);
		}
	}

	FVector TargetAtLocation = SpringArmComponent->BackupSocketOffset;
	if (nullptr != InTargetSectionKey)
	{
		TargetAtLocation = SpringArmComponent->BackupSocketOffset;
		if (InTargetSectionKey->LookAtPoint != NAME_None)
		{
			FTransform Transform;
			if (GetSocketTransform(InTargetSectionKey->LookAtPoint, ERelativeTransformSpace::RTS_Actor, Transform))
			{
				TargetAtLocation = Transform.GetTranslation();
				FTransform LocalTransform(Property.MeshImportRotation);
				TargetAtLocation = LocalTransform.TransformPosition(TargetAtLocation);
			}
			else
			{
				// TODO : 예외처리
			}
		}
		if (InTargetSectionKey->bInverse) // At 위치를 뒤집어 주고, 2배로 거리를 벌려준다. 촛점이 뒤집힘!
		{
			TargetAtLocation += -InTargetSectionKey->ViewDirection * (InTargetSectionKey->Distance * 2.0f);
		}
	}
	FVector FinalAtLocation = FMath::Lerp(SourceAtLocation, TargetAtLocation, InLocalBlendWeight);
	SpringArmComponent->UpdateBlendSocketOffset(FinalAtLocation, InBlendWeight);
}

#if (WITH_EDITOR || WITH_SERVER_CODE)
void AT4GameObject::BeginWeaponHitOverlapEvent(const FName& InHitOverlapEventName) // #49
{
	for (FT4BaseEquipment* EquipmentBase : Equipments)
	{
		check(nullptr != EquipmentBase);
		if (EquipmentBase->IsWeapon())
		{
			EquipmentBase->OnBeginOverlapEvents(InHitOverlapEventName);
		}
	}
}

void AT4GameObject::EndWeaponHitOverlapEvent() // #49
{
	for (FT4BaseEquipment* EquipmentBase : Equipments)
	{
		check(nullptr != EquipmentBase);
		if (EquipmentBase->IsWeapon())
		{
			EquipmentBase->OnEndOverlapEvents();
		}
	}
}
#endif

#if !UE_BUILD_SHIPPING
bool AT4GameObject::SaveActionReplaySnapshot() // #68, #107
{
	check(EntityAssetPath.IsValid());
	IT4ActionReplayRecorder* ActionReplayRecorder = GetActionReplayRecorder();
	check(nullptr != ActionReplayRecorder);
	{
		FT4SpawnObjectAction SpawnObjectAction;
		SpawnObjectAction.ObjectID = GetObjectID();
		SpawnObjectAction.Name = Name;
		SpawnObjectAction.EntityType = ET4EntityType::Character;
		SpawnObjectAction.EntityAssetPath = EntityAssetPath;
		SpawnObjectAction.StanceName = GetStanceName(); // #73
		SpawnObjectAction.SubStanceName = GetSubStanceName(); // #106
		SpawnObjectAction.SpawnLocation = GetNavPoint();
		SpawnObjectAction.SpawnRotation = GetRotation();
		SpawnObjectAction.GameDataIDName = GameDataIDName;
		SpawnObjectAction.bPlayer = HasPlayer();
		ActionReplayRecorder->RecWorldAction(&SpawnObjectAction, nullptr);
	}
	if (0 < Equipments.Num())
	{
		// #68 : 리로드 또는 ActionReplay 을 위해 장착 정보를 보관한다. 단, 현재는 Character 만 사용
		for (FT4BaseEquipment* EquipmentBase : Equipments)
		{
			check(nullptr != EquipmentBase);
			if (EquipmentBase->IsWeapon())
			{
				FT4EquipWeaponAction EquipAction;
				EquipAction.EquipmentActionKey = EquipmentBase->GetActionKey();
				EquipAction.MainWeaponData.WeaponEntityAsset = EquipmentBase->GetObjectPath();
				EquipAction.MainWeaponData.OverrideEquipPoint = EquipmentBase->GetEquipPoint();
				ActionReplayRecorder->RecObjectAction(GetObjectID(), &EquipAction, nullptr);
			}
		}
	}
	{
		// TODO : Exchange
	}
	return true;
}
#endif

#if WITH_EDITOR
void AT4GameObject::EditorSetAimTarget(bool bEnable, const FVector& InAimTarget) // #111
{
	FT4GameObjectState& ObjState = GetState();
	if (!bEnable)
	{
		ObjState.bAiming = false;
		ObjState.AimTargetDirection = FVector::ZeroVector;
	}
	else
	{
		ObjState.bAiming = true;
		ObjState.AimTargetDirection = InAimTarget;
	}
}

bool AT4GameObject::EditorPlayAnimation(
	UAnimSequence* InPlayAnimSequence,
	FName InSectionName,
	float InPlayRate,
	float InBlendInTimeSec,
	float InBlendOutTimeSec
) // #111
{
	IT4AnimControl* AnimControl = GetAnimControl();
	if (nullptr != AnimControl)
	{
		AnimControl->EditorPlayAnimation(InPlayAnimSequence, InPlayRate, InBlendInTimeSec, InBlendOutTimeSec);
	}
	if (InSectionName != NAME_None)
	{
		FT4AnimParameters AnimParameters;
		AnimParameters.AnimMontageName = T4Const_DefaultAnimMontageName;
		AnimParameters.SectionName = InSectionName;
		AnimParameters.PlayRate = InPlayRate;
		AnimParameters.BlendInTimeSec = InBlendInTimeSec;
		AnimParameters.BlendOutTimeSec = InBlendOutTimeSec;
		PlayAnimationAndBroadcast(AnimParameters); // #107
	}
	return true;
}
#endif

bool AT4GameObject::OnCreate(
	ET4LayerType InLayerType,
	const FT4SpawnObjectAction* InAction
)
{
	check(ET4LayerType::Max == LayerType);
	check(ET4LayerType::Max != InLayerType);
	check(InAction->ObjectID.IsValid());
	check(InAction->EntityAssetPath.IsValid());
	LayerType = InLayerType;
	Name = InAction->Name;
	ObjectID = InAction->ObjectID;
	GameDataIDName = InAction->GameDataIDName;
	bool bResult = Create(InAction);
	if (bResult)
	{
		CreateFinished(InAction->SpawnLocation, InAction->SpawnRotation, FVector::OneVector);
	}
	return bResult;
}

bool AT4GameObject::OnCreate(
	ET4LayerType InLayerType,
	ET4ObjectType InWorldObjectType, // #63 : Only World Object
	const FName& InName,
	const FT4ObjectID& InObjectID,
	const FVector& InLocation,
	const FRotator& InRotation,
	const FVector& InScale
)
{
	check(InWorldObjectType == GetObjectType()); // #54 : ClientObject Only
	check(ET4LayerType::Max == LayerType);
	check(ET4LayerType::Max != InLayerType);
	check(InObjectID.IsValid());
	LayerType = InLayerType;
	Name = InName;
	ObjectID = InObjectID;
	CreateFinished(InLocation, InRotation, InScale);
	return true;
}

void AT4GameObject::CreateFinished(
	const FVector& InSpawnLocation,
	const FRotator& InSpawnRotation,
	const FVector& InSpawnScale
)
{
	FVector ApplySpawnLocation = InSpawnLocation;
	ApplySpawnLocation.Z += GetProperty().HalfHeight; // Collision 만큼 다시 올려서 스폰해주어야 한다. 누락하면 바닥에 빠진다.

	const FTransform SpawnTransform(InSpawnRotation, ApplySpawnLocation, InSpawnScale);
	UGameplayStatics::FinishSpawningActor(this, SpawnTransform);
}

void AT4GameObject::OnReset()
{
	ObjectState.bResetting = true; // #111 : Reset 이 호출될 경우 불필요한 설정을 방지하기 위한 Flag

	ActionNodeControl.Reset(); // #20
	ActionTaskControl.Reset(); // #20

	ResetOpacityState(); // #78
	ResetDynamicMaterialInstances(); // #78
	ResetEquipments(); // #106
	Reset();
	ResetLoadComplated();

	NetworkControlRef = nullptr; // #42, #63

	AttachedComponents.Empty(); // #48

	ObjectState.bResetting = false;  // #111
	ObjectState.bTurning = false; // #44
	ObjectState.TurnGoalRotation = FRotator::ZeroRotator; // #44
	ObjectState.TurnRotationYawRate = 0.0f; // #44
	ObjectState.bPaused = false; // #63
	ObjectState.TimeScale = 1.0f;// #102
}

void AT4GameObject::OnWorldEnterStart(float InFadeInTimeSec) // #78
{
	// #78 : AsyncLoading 완료 후 호출된다. 즉, 그릴 준비까지 모두 완료 상태다.
	SetOpacity(1.0f, InFadeInTimeSec); // #78 : Fade Out 처리를 한다.
	WorldEnterStart();
}

void AT4GameObject::OnWorldLeaveStart(float InFadeOutTimeSec)
{
	WorldLeaveStart(); // #36 : Leave 시의 Ghost 처리. Coll 충돌 제외 등...
	ObjectState.bGhost = true; // #54
	if (0.0f >= InFadeOutTimeSec)
	{ 
		ObjectState.GhostOutTimeLeft = 0.0f; // #67
	}
	else
	{
		ObjectState.GhostOutTimeLeft = InFadeOutTimeSec;
		SetOpacity(0.0f, InFadeOutTimeSec); // #78 : Fade Out 처리를 한다.
	}
}

void AT4GameObject::AddDynamicMaterialInstances(UMeshComponent* InMeshComponent) // #78
{
	check(nullptr != InMeshComponent);
	const TArray<UMaterialInterface*>& Materials = InMeshComponent->GetMaterials();
	for (int32 i = 0; i < Materials.Num(); ++i)
	{
		UMaterialInterface* Material = Materials[i];
		check(nullptr != Material);
		UMaterialInstanceDynamic* NewDynamicInstance = InMeshComponent->CreateDynamicMaterialInstance(i, Material);
		check(nullptr != NewDynamicInstance);
		MaterialDynamicInstances.Add(NewDynamicInstance);
	}
#if 0
	const TArray<FName> MaterialSlots = InMeshComponent->GetMaterialSlotNames();
	for (FName SlotName : MaterialSlots)
	{
		int32 ArrayIndex = InMeshComponent->GetMaterialIndex(SlotName);
		check(0 <= ArrayIndex && ArrayIndex < MaterialDynamicInstances.Num());
		MaterialDynamicInstanceMap.Add(SlotName, MaterialDynamicInstances[ArrayIndex]);
	}
#endif
}

void AT4GameObject::ResetDynamicMaterialInstances() // #78
{
	//MaterialDynamicInstanceMap.Empty();
	MaterialDynamicInstances.Empty();
}

void AT4GameObject::SetDynamicMaterialInstanceParameter(FName InParameterName, float InValue) // #78
{
	for (UMaterialInstanceDynamic* DynamicInstance : MaterialDynamicInstances)
	{
		check(nullptr != DynamicInstance);
		DynamicInstance->SetScalarParameterValue(InParameterName, InValue);
	}
}

void AT4GameObject::SetOpacity(
	float InTargetValue,
	float InTransitionMaxTimeSec
) // #78
{
	InTargetValue = FMath::Clamp(InTargetValue, 0.0f, 1.0f);
	if (0.0f == InTransitionMaxTimeSec)
	{
		SetDynamicMaterialInstanceParameter(
			T4EngineConstant::GetMaterialParameterOpacityName(), 
			InTargetValue
		);
		return;
	}
	ObjectState.bTransitionOpacity = true;
	ObjectState.TargetOpacityValue = InTargetValue;
	ObjectState.TransitionTimeSec = 0.0f;
	ObjectState.TransitionTimeMaxSec = InTransitionMaxTimeSec;
}

void AT4GameObject::SetOpacity(
	float InSourceAlpha,
	float InTargetValue,
	float InTransitionMaxTimeSec
) // #78
{
	InSourceAlpha = FMath::Clamp(InSourceAlpha, 0.0f, 1.0f);
	if (InSourceAlpha != ObjectState.OpacityValue)
	{
		SetDynamicMaterialInstanceParameter(
			T4EngineConstant::GetMaterialParameterOpacityName(), 
			InSourceAlpha
		);
		ObjectState.OpacityValue = InSourceAlpha;
	}
	SetOpacity(InTargetValue, InTransitionMaxTimeSec);
}

void AT4GameObject::ResetOpacityState() // #78
{
	ObjectState.bTransitionOpacity = false;
	ObjectState.OpacityValue = 1.0f;
	ObjectState.TargetOpacityValue = 1.0f;
	ObjectState.TransitionTimeMaxSec = 0.0f;
	ObjectState.TransitionTimeSec = 0.0f;
}

#if !UE_BUILD_SHIPPING
void AT4GameObject::SetDebugPause(bool bInPause) // #102
{
	ObjectState.bDebugPaused = bInPause;
	//SetTickableWhenPaused(bPaused);
	//SetActorTickEnabled(!ObjectState.bDebugPaused); // #102 : UpdateTime 으로 제어하기 때문에 ActorTice 은 설정하지 않는다.
	ActionNodeControl.SetDebugPause(ObjectState.bDebugPaused);
}
#endif

void AT4GameObject::SetPause(bool bInPause) // #63
{
	ObjectState.bPaused = bInPause;
}

void AT4GameObject::SetTimeScale(float InTimeScale) // #102
{
	ObjectState.TimeScale = InTimeScale;
}

void AT4GameObject::AddAttachedComponent(
	USceneComponent* InAttachedComponent
) // #48
{
	AttachedComponents.Add(InAttachedComponent);
}

void AT4GameObject::RemoveAttachedComponent(
	USceneComponent* InAttachedComponent
) // #48
{
	AttachedComponents.Remove(InAttachedComponent);
}

// #68, #107 : 리로드 또는 ActionReplay 을 위해 장착 정보를 보관한다. 단, 현재는 Character 만 사용
void AT4GameObject::ResetEquipments()
{
	for (FT4BaseEquipment* EquipmentBase : Equipments) // #111
	{
		EquipmentBase->OnReset();
		delete EquipmentBase;
	}
	Equipments.Empty();
	EquipmentMultiMap.Empty();
}

bool AT4GameObject::HasEquipment(const FT4ActionKey& InActionKey) const // #111
{
	return EquipmentMultiMap.Contains(InActionKey);
}

bool AT4GameObject::AttachEquipment(
	const FT4ActionKey& InActionKey,
	bool bInMainWeapon, // #111
	const FT4EntityKey& InEntityKey,
	const FName InOverrideEquipPoint,
	bool bChangeStance, // #110
	bool bUseAnimNotify // #111 : Stance 변경에 있는 AnimNotify_Equipment Mount 에 따라 Hide => Show 처리가 됨
)
{
	if (!InActionKey.IsValid())
	{
		T4_LOG(Warning, TEXT("Invalid Equipment ActionKey. Check please!!!"));
		return false;
	}
	FT4BaseEquipment* NewEquipment = FT4BaseEquipment::NewInstance(
		this, 
		InActionKey,
		bInMainWeapon,
		InEntityKey, 
		InOverrideEquipPoint,
		bChangeStance,
		bUseAnimNotify
	);
	check(nullptr != NewEquipment);
	EquipmentMultiMap.Add(InActionKey, NewEquipment);
	Equipments.Add(NewEquipment);
	return true;
}

void AT4GameObject::DetachEquipment(
	const FT4ActionKey& InActionKey,
	FName ResotreStanceName,
	bool bUseAnimNotify // #111 : Stance 변경에 있는 AnimNotify_Equipment Unmount 에 따라 Show => Hide 처리가 됨
)
{
	if (!EquipmentMultiMap.Contains(InActionKey))
	{
		T4_LOG(
			Warning,
			TEXT("[SL:%u] Equipment ActionKey '%s' Not found"),
			uint32(LayerType),
			*(InActionKey.ToString())
		);
		return;
	}
	TArray<FT4BaseEquipment*> FoundEquipments;
	EquipmentMultiMap.MultiFind(InActionKey, FoundEquipments);
	for (FT4BaseEquipment* EquipmentBase : FoundEquipments)
	{
		EquipmentBase->OnDetach(ResotreStanceName, bUseAnimNotify);
	}
	EquipmentMultiMap.Remove(InActionKey);
	// Equipments.RemoveAt 은 AdvanceEquipment 에서 처리
}
// ~#68

void AT4GameObject::OnAnimNotifyMessage(const FT4AnimNotifyMessage* InMessage) // #111
{
	check(nullptr != InMessage);
	switch (InMessage->AnimNotifyType)
	{
		case ET4AnimNotifyType::Equipment:
			{
				const FT4AnimNotifyEquipment* AnimNotifyMessage = static_cast<const FT4AnimNotifyEquipment*>(InMessage);
				check(nullptr != AnimNotifyMessage);
				for (FT4BaseEquipment* EquipmentBase : Equipments)
				{
					check(nullptr != EquipmentBase);
					EquipmentBase->OnAnimNotify(*AnimNotifyMessage);
				}
			}
			break;

		case ET4AnimNotifyType::Footstep:
			{
				const FT4AnimNotifyFootstep* AnimNotifyMessage = static_cast<const FT4AnimNotifyFootstep*>(InMessage);
				// TODO
			}
			break;

		default:
			{
				T4_LOG(Error, TEXT("Unknown AnimNotify type '%u'"), uint8(InMessage->AnimNotifyType));
			}
			break;
	};
}

bool AT4GameObject::OnExecuteAction(
	const FT4ActionCommand* InAction,
	const FT4ActionParameters* InActionParam
)
{
	check(nullptr != InAction);
	if (ObjectState.bResetting)
	{
		return true; // #111 : Reset 이 호출될 경우 불필요한 설정을 방지하기 위한 Flag
	}
	bool bResult = ExecuteDispatch(&ActionNodeControl, InAction, InActionParam);
#if !UE_BUILD_SHIPPING
	if (bResult && !InAction->bTransient)
	{
		if (ET4SpawnMode::All == GetObjectID().SpawnMode) // #68
		{
			IT4ActionReplayRecorder* ActionReplayRecorder = GetActionReplayRecorder();
			if (nullptr != ActionReplayRecorder)
			{
				ActionReplayRecorder->RecObjectAction(
					GetObjectID(),
					InAction,
					InActionParam
				);
			}
		}
	}
#endif
	return bResult;
}

bool AT4GameObject::ExecuteDispatch(
	FT4ActionNodeControl* InControl, // #76
	const FT4ActionCommand* InAction,
	const FT4ActionParameters* InActionParam
)
{
	check(nullptr != InAction);
	if (nullptr == InActionParam)
	{
		InActionParam = &FT4ActionParameters::DefaultActionParameter; // #32
	}
	bool bResult = true;
	if (ET4ActionCommandType::Code == InAction->GetActionStructType()) // #100 : Code 외 Conti Action 들은 아래에서 처리하기 위해 체크 추가
	{
		bool bExecuted = ExecuteDispatchImmediate(InControl, InAction, InActionParam, bResult); // #23 : Movement 전 즉시 실행, Input & Packet 류!
		if (bExecuted)
		{
			return bResult;
		}
	}
	// #58 : ActionParameter 는 외부에서 전달해준 데이터를 
	//       RootNode 가(만) SharedPtr 로 만들어 하위 노드들과 공유한다.
	FT4ActionNodeBase* NewActionNode = InControl->CreateRootNode(InAction, InActionParam);
	if (nullptr == NewActionNode)
	{
		T4_LOG(
			Verbose,
			TEXT("[SL:%u] No implementation Action '%s'"),
			uint32(LayerType),
			*(InAction->ToString())
		);
	}
	return (nullptr != NewActionNode) ? true : false;
}

bool AT4GameObject::ExecuteDispatchImmediate(
	FT4ActionNodeControl* InControl, // #76
	const FT4ActionCommand* InAction,
	const FT4ActionParameters* InActionParam,
	bool& bOutResult
)
{
	// #23
	check(nullptr != InControl);
	// #T4_ADD_ACTION_TAG_CODE
	bool bResult = false;
	bool bExecuted = true;
	const ET4ActionType ActionType = InAction->ActionType;
	check(ET4ActionCommandType::Code == InAction->GetActionStructType()); // #63 : Conti 액션들은 여기서 호출되면 안된다!
	switch (ActionType)
	{
		case ET4ActionType::Stop:
			bOutResult = ExecuteStopAction(InControl, *(static_cast<const FT4StopAction*>(InAction))); // #20 : Stop 은 즉시!
			break;

		case ET4ActionType::MoveAsync:
			bOutResult = ExecuteMoveAsyncToAction(*(static_cast<const FT4MoveAsyncAction*>(InAction))); // #40
			break;

		case ET4ActionType::MoveSync:
			bOutResult = ExecuteMoveSyncToAction(*(static_cast<const FT4MoveSyncAction*>(InAction))); // #40
			break;

		case ET4ActionType::Jump:
			bOutResult = ExecuteJumpToAction(*(static_cast<const FT4JumpAction*>(InAction)));
			break;

		case ET4ActionType::Roll: // #46
			bOutResult = ExecuteRollToAction(*(static_cast<const FT4RollAction*>(InAction)));
			break;

		case ET4ActionType::Teleport:
			bOutResult = ExecuteTeleportToAction(*(static_cast<const FT4TeleportAction*>(InAction)));
			break;

		case ET4ActionType::MoveStop: // #52
			bOutResult = ExecuteMoveStopAction(*(static_cast<const FT4MoveStopAction*>(InAction)));
			break;

		case ET4ActionType::MoveSpeedSync: // #52
			bOutResult = ExecuteMoveSpeedSyncAction(*(static_cast<const FT4MoveSpeedSyncAction*>(InAction)));
			break;

		case ET4ActionType::Launch: // #63 : only Projectile
			bOutResult = ExecuteLaunchAction(*(static_cast<const FT4LaunchAction*>(InAction)), InActionParam);
			break;

		case ET4ActionType::Aim: // #113
			bOutResult = ExecuteAimAction(*(static_cast<const FT4AimAction*>(InAction)));
			break;

		case ET4ActionType::LockOn:
			bOutResult = ExecuteLockOnAction(*(static_cast<const FT4LockOnAction*>(InAction)));
			break;

		case ET4ActionType::Stance: // #73
			bOutResult = ExecuteStanceAction(*(static_cast<const FT4StanceAction*>(InAction)));
			break;

		case ET4ActionType::SubStance: // #106
			bOutResult = ExecuteSubStanceAction(*(static_cast<const FT4SubStanceAction*>(InAction)));
			break;

		case ET4ActionType::Costume: // #72
			bOutResult = ExecuteCostumeAction(*(static_cast<const FT4CostumeAction*>(InAction)));
			break;

		case ET4ActionType::Hit: // #76
			bOutResult = ExecuteHitAction(*(static_cast<const FT4HitAction*>(InAction)));
			break;

		case ET4ActionType::Die: // #76
			bOutResult = ExecuteDieAction(*(static_cast<const FT4DieAction*>(InAction)));
			break;

		case ET4ActionType::Resurrect: // #76
			bOutResult = ExecuteResurrectAction(*(static_cast<const FT4ResurrectAction*>(InAction)));
			break;

#if WITH_EDITOR
		case ET4ActionType::Editor: // #37
			bOutResult = ExecuteEditorAction(*(static_cast<const FT4EditorAction*>(InAction)));
			break;
#endif

		default:
			bExecuted = false;
			break;
	};
	if (bExecuted && 
		ET4ActionType::Stop != ActionType && 
		ET4LifecycleType::Looping == InAction->LifecycleType)
	{
		// #23 : ActionNode 를 사용하지 않는 Action의 Key 관리
		InControl->AddSyncActionStatus(
			(ET4ActionCommandType::Code == InAction->GetActionStructType()) ? // #62
				static_cast<const FT4ActionCodeCommand*>(InAction)->ActionKey : FT4ActionKey::EmptyActionKey,
			ActionType
		);
	}
	return bExecuted;
}

bool AT4GameObject::ExecuteStopAction(
	FT4ActionNodeControl* InControl,
	const FT4StopAction& InAction
)
{
	check(ET4ActionType::Stop == InAction.ActionType);
	InControl->DestroyNode(
		InAction.ActionKey, 
		InAction.StartTimeSec,
		InAction.bSameKeyNameRemoveAll
	);
	// #23 : ActionNode 를 사용하지 않는 Action의 Key 관리
	InControl->RemoveSyncActionStatus(InAction.ActionKey);
	return true;
}
