// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameObject.h"

#include "Animation/T4BaseAnimControl.h"
#include "Action/ActionNode/T4ActionNodeIncludes.h" // #34

#include "Object/Component/T4EnvironmentZoneComponent.h" // #99

#include "Public/T4Engine.h"
#include "Classes/Camera/T4SpringArmComponent.h" // #58

#include "T4Asset/Public/Action/T4ActionContiStructs.h" // #58

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
	, GameplayControlRef(nullptr) // #42, #63 : 참조!! 소멸 책임 없음!
#if (WITH_EDITOR || WITH_SERVER_CODE)
	, bHitOverlapEventEnabled(false) // #49
	, HitOverlapEventName(NAME_None) // #49
#endif
{
	bReplicates = false; // #15
	SetReplicatingMovement(false); // #4.24 : bReplicateMovement = false; // #33
	bNetLoadOnClient = false; // #15
	TearOff(); // #15

	ActionControl.Set(this); // #20 , #76 : 외부 제어!
	ManualControl.Set(this); // #76 : 내부 시스템 제어
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

	OnUpdateTickActorAfter(UpdateTimeInfo);

	if (ObjectState.bGhost) // #54
	{
		ObjectState.GhostOutTimeLeft -= UpdateTimeInfo.ScaledTimeSec; // InDeltaTime;
	}
}

void AT4GameObject::AdvanceActions(const FT4UpdateTime& InUpdateTime)
{
	ManualControl.Advance(InUpdateTime); // #76
	ActionControl.Advance(InUpdateTime); // #20
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
		T4MaterialParameterOpacityParamName,
		FMath::Clamp(ObjectState.OpacityValue, 0.0f, 1.0f)
	);
#if 0
	UE_LOG(
		LogT4Engine,
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

bool AT4GameObject::IsPlayer() const
{
	if (nullptr == GameplayControlRef)
	{
		return false;
	}
	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	// 서버를 제외한 PlayerController 는 1개 밖에 없다.
	if (GameplayControlRef != GameWorld->GetPlayerControl())
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
	return static_cast<IT4ActionControl*>(&ActionControl); // #20
} 

#if (WITH_EDITOR || WITH_SERVER_CODE)
void AT4GameObject::BeginWeaponHitOverlapEvent(const FName& InHitOverlapEventName) // #49
{
	// TODO : Key
	bHitOverlapEventEnabled = true;
	HitOverlapEventName = InHitOverlapEventName;
	for (USceneComponent* AttachedComponent : AttachedComponents) // TODO : Optimize
	{
		UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(AttachedComponent); // #99
		if (nullptr != PrimitiveComponent)
		{
			PrimitiveComponent->SetGenerateOverlapEvents(true);
		}
	}
}

void AT4GameObject::EndWeaponHitOverlapEvent() // #49
{
	if (!bHitOverlapEventEnabled)
	{
		return;
	}
	// TODO : Key
	for (USceneComponent* AttachedComponent : AttachedComponents) // TODO : Optimize
	{
		UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(AttachedComponent); // #99
		if (nullptr != PrimitiveComponent)
		{
			PrimitiveComponent->SetGenerateOverlapEvents(false);
		}
	}
	bHitOverlapEventEnabled = false;
	HitOverlapEventName = NAME_None;
}
#endif

bool AT4GameObject::HasPublicAction(const FT4ActionKey& InActionKey) const
{
	// #102 : 존재만 해도 true 리턴
	return ActionControl.HasAction(InActionKey);
}

bool AT4GameObject::IsPlayingPublicAction(const FT4ActionKey& InActionKey) const
{
	// #20, #76 : Playing 중인지를 체크. Paused 면 False 가 리턴됨!
	return ActionControl.IsPlaying(InActionKey);
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

IT4ActionPlaybackRecorder* AT4GameObject::GetActionPlaybackRecorder() const // #68
{
	IT4GameWorld* GameWorld = T4EngineWorldGet(LayerType);
	if (nullptr != GameWorld)
	{
		return GameWorld->GetActionPlaybackRecorder();
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

	FT4HitSingleResult HitResult;
	bool bResult = GameWorld->GetCollisionSystem()->QueryLineTraceSingle(
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

void AT4GameObject::StopLayerTag(const FT4ActionKey& InActionKey) // #74, #81
{
	FT4StopAction NewAction;
	NewAction.ActionKey = InActionKey;
	NewAction.StartTimeSec = 0.0f;
	NewAction.bSameKeyNameRemoveAll = true;
	ExecutePrivateAction(&NewAction, nullptr);
}

bool AT4GameObject::PlayMaterialByLayerTag(
	const FT4EntityOverrideMaterialData& InLayerTagData,
	const FT4ActionKey& InActionKey
) // #81
{
	return false;
}

bool AT4GameObject::PlayWeaponByLayerTag(
	const FT4EntityLayerTagWeaponData& InLayerTagData,
	const FT4ActionKey& InActionKey
) // #81
{
	if (InLayerTagData.WeaponEntityAsset.IsNull())
	{
		return false;
	}
	FT4EquipWeaponAction NewAction;
	NewAction.ActionKey = InActionKey;
	NewAction.WeaponEntityAsset = InLayerTagData.WeaponEntityAsset;
	NewAction.EquipPoint = InLayerTagData.EquipPoint;
	NewAction.LifecycleType = ET4LifecycleType::Looping;
	ExecutePrivateAction(&NewAction, nullptr);
	return true;
}

bool AT4GameObject::PlayContiByLayerTag(
	const FT4EntityLayerTagContiData& InLayerTagData,
	const FT4ActionKey& InActionKey
) // #81
{
	if (InLayerTagData.ContiAsset.IsNull())
	{
		return false;
	}
	FT4ContiAction NewAction;
	NewAction.ActionKey = InActionKey;
	NewAction.ContiAsset = InLayerTagData.ContiAsset;
	ExecutePrivateAction(&NewAction, nullptr);
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
	ActionControl.Reset(); // #20
	ManualControl.Reset(); // #20

	ResetOpacityState(); // #78
	ResetDynamicMaterialInstances(); // #78
	Reset();
	ResetLoadComplated();

	GameplayControlRef = nullptr; // #42, #63

	AttachedComponents.Empty(); // #48

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
		SetDynamicMaterialInstanceParameter(T4MaterialParameterOpacityParamName, InTargetValue);
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
		SetDynamicMaterialInstanceParameter(T4MaterialParameterOpacityParamName, InSourceAlpha);
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
	ActionControl.SetDebugPause(ObjectState.bDebugPaused);
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
	if (!InSweepResult.Actor.IsValid() || this == InSweepResult.Actor.Get())
	{
		return;
	}
	if (!bHitOverlapEventEnabled || HitOverlapEventName == NAME_None)
	{
		ensure(false); // ??
		return;
	}
	IT4GameObject* HitGameObject = T4EnginetUtil::TryCastGameObject(InSweepResult.Actor.Get());
	if (nullptr == HitGameObject)
	{
		return;
	}
	GetServerDelegates().OnHitOverlap.Broadcast(HitOverlapEventName, HitGameObject, InSweepResult);
#endif
}

bool AT4GameObject::DoExecuteAction(
	const FT4ActionStruct* InAction,
	const FT4ActionParameters* InActionParam
)
{
	check(nullptr != InAction);
	bool bResult = ExecuteDispatch(&ActionControl, InAction, InActionParam);
#if !UE_BUILD_SHIPPING
	if (bResult)
	{
		if (ET4SpawnMode::All == GetObjectID().SpawnMode) // #68
		{
			IT4ActionPlaybackRecorder* ActionPlaybackRecorder = GetActionPlaybackRecorder();
			if (nullptr != ActionPlaybackRecorder)
			{
				ActionPlaybackRecorder->RecObjectAction(
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
	FT4ActionControl* InControl, // #76
	const FT4ActionStruct* InAction,
	const FT4ActionParameters* InActionParam
)
{
	check(nullptr != InAction);
	if (nullptr == InActionParam)
	{
		InActionParam = &FT4ActionParameters::DefaultActionParameter; // #32
	}
	bool bResult = true;
	if (ET4ActionStructType::Code == InAction->GetActionStructType()) // #100 : Code 외 Conti Action 들은 아래에서 처리하기 위해 체크 추가
	{
		bool bExecuted = ExecuteDispatchImmediate(InControl, InAction, InActionParam, bResult); // #23 : Movement 전 즉시 실행, Input & Packet 류!
		if (bExecuted)
		{
			return bResult;
		}
	}
	// #58 : ActionParameter 는 외부에서 전달해준 데이터를 
	//       RootNode 가(만) SharedPtr 로 만들어 하위 노드들과 공유한다.
	FT4ActionNode* NewActionNode = InControl->CreateRootNode(InAction, InActionParam);
	if (nullptr == NewActionNode)
	{
		UE_LOG(
			LogT4Engine,
			Verbose,
			TEXT("[SL:%u] ExecuteDispatch '%s' failed. no implementation."),
			uint32(LayerType),
			*(InAction->ToString())
		);
	}
	return (nullptr != NewActionNode) ? true : false;
}

bool AT4GameObject::ExecuteDispatchImmediate(
	FT4ActionControl* InControl, // #76
	const FT4ActionStruct* InAction,
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
	check(ET4ActionStructType::Code == InAction->GetActionStructType()); // #63 : Conti 액션들은 여기서 호출되면 안된다!
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

		case ET4ActionType::LockOn:
			bOutResult = ExecuteLockOnAction(*(static_cast<const FT4LockOnAction*>(InAction)));
			break;

		case ET4ActionType::ChangeStance: // #73
			bOutResult = ExecuteChangeStanceAction(*(static_cast<const FT4ChangeStanceAction*>(InAction)));
			break;

		case ET4ActionType::UnEquipWeapon: // #48
			bOutResult = ExecuteUnEquipWeaponAction(InControl, *(static_cast<const FT4UnEquipWeaponAction*>(InAction)));
			break;

		case ET4ActionType::ExchangeCostume: // #72
			bOutResult = ExecuteExchangeCostumeAction(*(static_cast<const FT4ExchangeCostumeAction*>(InAction)));
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
			(ET4ActionStructType::Code == InAction->GetActionStructType()) ? // #62
				static_cast<const FT4CodeActionStruct*>(InAction)->ActionKey : FT4ActionKey::EmptyActionKey,
			ActionType
		);
	}
	return bExecuted;
}

bool AT4GameObject::ExecuteStopAction(
	FT4ActionControl* InControl,
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

bool AT4GameObject::ExecuteUnEquipWeaponAction(
	FT4ActionControl* InControl,
	const FT4UnEquipWeaponAction& InAction
) // #48
{
	check(ET4ActionType::UnEquipWeapon == InAction.ActionType);
	InControl->DestroyNode(
		InAction.ActionKey,
		InAction.StartTimeSec,
		true
	);
	return true;
}
