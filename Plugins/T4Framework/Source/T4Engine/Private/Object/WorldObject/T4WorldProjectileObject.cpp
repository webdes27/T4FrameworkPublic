// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4WorldProjectileObject.h"

#include "Object/Component/T4SphereComponent.h"
#include "Object/Component/T4ProjectileMovementComponent.h"
#include "Object/Component/T4SceneComponent.h"

#include "Object/ActionNode/T4ActionNodeControl.h" // #20

#include "Public/Action/T4ActionParameters.h"

#include "T4Asset/Classes/Conti/T4ContiAsset.h"

#include "T4EngineInternal.h"

static const FT4ActionKey ProjectileHeadActionKey(TEXT("ProjectileHead"), false); // #63
static const FT4ActionKey ProjectileEndActionKey(TEXT("ProjectileEnd"), false); // #63

/**
  *
 */
AT4WorldProjectileObject::AT4WorldProjectileObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CollisionComponent(nullptr)
	, ProjectileComponent(nullptr)
	, AttachRootComponent(nullptr)
	, bDestroying(false) // #112
	, bArrived(false)
	, bHitAttached(false) // #112
	, PlayingTimeSec(0.0f)
	, MaxPlayTimeSec(0.0f)
	, GoalLocation(FVector::ZeroVector)
	, bEnableHitAttached(false) // #112
	, HitAttachedTimeSec(0.0f) // #112
	, ProjectileLength(80.0f) // #112 : Projectile 의 길이, 충돌 계산에서 Offset 으로 사용. (원점 에서의 길이)
	, ProjectileLengthEnter(ProjectileLength * 0.8f) // #112
	, ThrowTargetPoint(NAME_None) // #106
	, FlyingHitCount(0)
{
	CollisionComponent = CreateDefaultSubobject<UT4SphereComponent>(TEXT("T4SphereComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
	RootComponent = CollisionComponent;

	// Use this component to drive this projectile's movement.
	ProjectileComponent = CreateDefaultSubobject<UT4ProjectileMovementComponent>(TEXT("T4ProjectileMovementComponent"));
	ProjectileComponent->SetUpdatedComponent(CollisionComponent);
	ProjectileComponent->InitialSpeed = 3000.0f;
	ProjectileComponent->MaxSpeed = 3000.0f;
	ProjectileComponent->bRotationFollowsVelocity = true;
	ProjectileComponent->bShouldBounce = false;
	ProjectileComponent->bIsSliding = false;
	ProjectileComponent->ProjectileGravityScale = 0.0f; // 중력을 사용하지 않음!
	ProjectileComponent->Bounciness = 0.3f;

	AttachRootComponent = CreateDefaultSubobject<UT4SceneComponent>(TEXT("T4SceneComponent"));
	AttachRootComponent->SetupAttachment(CollisionComponent);
}

AT4WorldProjectileObject::~AT4WorldProjectileObject()
{
}

void AT4WorldProjectileObject::Reset()
{
}

void AT4WorldProjectileObject::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!IsPendingKill())
	{
		if (nullptr != AttachRootComponent)
		{
			// WARN : AttachRootComponent 는 ActorTick(this) 이후 동작하도록 처리한다.
			//        Action 처리 후 동작되도록 처리하기 위한 것!
			if (AttachRootComponent->PrimaryComponentTick.bCanEverTick)
			{
				AttachRootComponent->PrimaryComponentTick.AddPrerequisite(
					this,
					PrimaryActorTick
				);
			}
		}
	}
}

void AT4WorldProjectileObject::UpdateTickActorBefore(const FT4UpdateTime& InUpdateTime)
{
	if (bDestroying) // #112
	{
		return;
	}
	if (bArrived)
	{
		if (bHitAttached)
		{
			HitAttachedTimeSec -= InUpdateTime.ScaledTimeSec;
			if (0.0f >= HitAttachedTimeSec)
			{
				SetThisDestroy(); // #112 : 지정 시간 후 삭제!
			}
		}
		return;
	}
	PlayingTimeSec += InUpdateTime.ScaledTimeSec;
	float FlyingLifeTimeSec = MaxPlayTimeSec - PlayingTimeSec;
	if (0.0f >= FlyingLifeTimeSec)
	{
		return; // 삭제 예정!
	}
	if (nullptr == ProjectileComponent)
	{
		return;
	}
	if (TargetObjectID.IsValid())
	{
		AdvanceToTarget(InUpdateTime, FlyingLifeTimeSec); // #112
	}
	else
	{
		AdvanceToInfinity(InUpdateTime, FlyingLifeTimeSec); // #112
	}
}

void AT4WorldProjectileObject::AdvanceToTarget(
	const FT4UpdateTime& InUpdateTime, 
	float InFlyingLifeTimeSec
) // #112
{
	if (!TargetObjectID.IsValid())
	{
		return;
	}
	// #63 : 타겟이 있을 경우 타겟을 무조건 맞춰야 한다.
	IT4GameWorld* GameWorld = GetGameWorld();
	if (nullptr == GameWorld)
	{
		return;
	}
	IT4WorldContainer* WorldContainer = GameWorld->GetContainer();
	check(nullptr != WorldContainer);
	IT4GameObject* TargetObject = WorldContainer->FindGameObject(TargetObjectID);
	if (nullptr == TargetObject)
	{
		TargetObjectID.Empty();
		AdvanceToInfinity(InUpdateTime, InFlyingLifeTimeSec); // #112 : 타겟이 사라지면 Infinity 처리로 변경
		return;
	}
	FVector TargetLocation = TargetObject->GetCOMLocation();
	if (ThrowTargetPoint != NAME_None)
	{
		TargetObject->GetSocketLocation(ThrowTargetPoint, TargetLocation); // #106 : 화살의 Goal 설정
	}
	const FVector ProjectileLocation = GetActorLocation();
	FVector TargetVector = TargetLocation - ProjectileLocation;
	float TargetDistance = TargetVector.Size(); // #63

	// #63 : 나중에는 Attacker 의 발사지점, Target 의 피격 지점으로 거리에 따른 속도를 계산해야 함.
	//       현재는 미관상, 양쪽의 Radius 를 반영해 ProjSpeed 를 계산하도록 처리
	TargetDistance -= TargetObject->GetPropertyConst().CapsuleRadius;
	if (ProjectileLength >= TargetDistance)
	{
		FVector HitTestShootDirection = TargetVector;
		HitTestShootDirection.Normalize();
		FT4HitSingleResult HitResult;
		if (CheckHit(ET4CollisionChannel::CollisionVisibility, HitTestShootDirection, ProjectileLength * 2.0f, HitResult))
		{
			SetArrivedForTarget(
				TargetObject, 
				HitResult.ResultLocation, 
				HitTestShootDirection,
				ThrowTargetPoint, // HitResult.ResultHitBone, // #116 : 방향이 일치하는데, HitBone 이 잘못잡히는 문제가 있어 ThrowTargetPoint 로 설정
				true
			);
		}
		else
		{
			bEnableHitAttached = false; // Hit 가 실패해서 즉시 사라지도록 처리한다.
			SetArrived(true);
		}
		return;
	}
	const FVector ShootDirection = TargetVector / TargetDistance;
	float CurrentSpeed = TargetDistance / InFlyingLifeTimeSec;
	ProjectileComponent->Velocity = ShootDirection * CurrentSpeed;
}

void AT4WorldProjectileObject::AdvanceToInfinity(
	const FT4UpdateTime& InUpdateTime,
	float InFlyingLifeTimeSec
) // #112
{
	check(nullptr != ProjectileComponent);
	FVector ShootDirection = ProjectileComponent->Velocity;
	ShootDirection.Normalize();
	FT4HitSingleResult HitResult;
	if (!CheckHit(ET4CollisionChannel::WorldStatic, ShootDirection, ProjectileLengthEnter, HitResult))
	{
		return;
	}
	if (nullptr != HitResult.ResultObject)
	{
		// 캐릭터에 맞았다. 튕겨져 나간다!
		ShootDirection.X *= FMath::RandRange(1.0f, 3.0f);
		ShootDirection.Y *= FMath::RandRange(1.0f, 3.0f);
		ShootDirection.Z *= FMath::RandRange(1.0f, 3.0f);
		ShootDirection.Normalize();
		ProjectileComponent->Velocity = ShootDirection * ProjectileComponent->Velocity.Size();
		if (0 == FlyingHitCount) // 첫번째 한번만...
		{
			PlayEndConti(true); // #112 : 비켜 맞아도 End Conti 가 있다면 출력해준다. 재질별 처리를 해주면 될 듯...
		}
		FlyingHitCount++;
		return;
	}
	SetArrived(true);
}

bool AT4WorldProjectileObject::CheckHit(
	ET4CollisionChannel InCollisionChnnel,
	const FVector& InShootDirection,
	float InMaxDistance,
	FT4HitSingleResult& OutHitResult
)
{
	IT4GameWorld* GameWorld = GetGameWorld();
	if (nullptr == GameWorld)
	{
		return false;
	}
	const AActor* IgnoreActor = nullptr;
	IT4WorldContainer* WorldContainer = GameWorld->GetContainer();
	check(nullptr != WorldContainer);
	IT4GameObject* OwnerObject = WorldContainer->FindGameObject(OwnerObjectID);
	if (nullptr != OwnerObject)
	{
		IgnoreActor = OwnerObject->GetPawn();
	}
	IT4WorldCollisionSystem* CollisionSystem = GameWorld->GetCollisionSystem();
	check(nullptr != CollisionSystem);
	FCollisionQueryParams TraceProjectileParams = FCollisionQueryParams(
		FName(TEXT("Projectile")),
		true,
		IgnoreActor
	);
	//TraceProjectileParams.MobilityType = EQueryMobilityType::Any;
	//TraceProjectileParams.bTraceComplex = true;
	//TraceProjectileParams.bReturnPhysicalMaterial = false;
	const FVector StartLocation = GetActorLocation();
	bool bResult = CollisionSystem->QueryLineTraceSingle(
		InCollisionChnnel,
		StartLocation,
		InShootDirection,
		InMaxDistance,
		TraceProjectileParams,
		OutHitResult
	);
	if (!bResult)
	{
		return false;
	}
	return true;
}

void AT4WorldProjectileObject::UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime)
{
	if (nullptr != ProjectileComponent)
	{
		if (0.0f < MaxPlayTimeSec && MaxPlayTimeSec <= PlayingTimeSec )
		{
			bEnableHitAttached = false; // 시간 종료이기 때문에 HitAttached 를 사용하지 않는다.
			SetArrived(true);
		}
	}
}

bool AT4WorldProjectileObject::IsDestroyable() 
{
	if (!Super::IsDestroyable())
	{
		return false;
	}
	// #63 : End Conti 가 모두 끝나면 종료되도록 처리!
	if (0 >= GetActionControl()->NumChildActions(ProjectileEndActionKey))
	{
		return true;
	}
	return false;
}

#if WITH_EDITOR
void AT4WorldProjectileObject::SetDebugPause(bool bInPause) // #68
{
	Super::SetDebugPause(bInPause);
	if (nullptr != ProjectileComponent)
	{
		ProjectileComponent->SetComponentTickEnabled(!bInPause);
	}
}
#endif

USceneComponent* AT4WorldProjectileObject::GetAttachParentComponent() // #54
{
	return Cast<USceneComponent>(AttachRootComponent);
}

void AT4WorldProjectileObject::StopHeadConti() // #112
{
	FT4StopAction NewAction;
	NewAction.bTransient = true; // Replay 녹화에서 제외. 상위 Action 이 녹화되어서 저장할 필요가 없다.
	NewAction.ActionKey = ProjectileHeadActionKey;
	NewAction.StartTimeSec = 0.0f;
	NewAction.bSameKeyNameRemoveAll = true;
	ExecuteActionNode(&NewAction, nullptr);
}

void AT4WorldProjectileObject::ProjectileMoveStop() // #112
{
	check(nullptr != ProjectileComponent);
	ProjectileComponent->Velocity = FVector::ZeroVector;
}

void AT4WorldProjectileObject::PlayEndConti(bool bInPlayEndConti) // #112
{
	if (!bInPlayEndConti)
	{
		return;
	}
	if (!EndContiAssetPath.IsNull())
	{
		FT4ContiAction NewContiAction;
		NewContiAction.bTransient = true; // Replay 녹화에서 제외. 상위 Action 이 녹화되어서 저장할 필요가 없다.
		NewContiAction.ActionKey = ProjectileEndActionKey;
		NewContiAction.ActionKey.bOverrideExisting = true; // #49 : 동기화가 중요하니 무조건 플레이를 보장한다.
		NewContiAction.ContiAsset = EndContiAssetPath;
		ExecuteActionNode(&NewContiAction, nullptr);
	}
}

void AT4WorldProjectileObject::SetArrived(bool bPlayEndConti)
{
	if (bArrived)
	{
		return;
	}
	// #63 : Head 는 끄고, Projectile 도 멈춘다.
	if (!bEnableHitAttached) // #112
	{
		StopHeadConti();
	}
	ProjectileMoveStop();
	PlayEndConti(bPlayEndConti);
	if (!bEnableHitAttached) // #112
	{
		SetThisDestroy(); // #63 : Conti 의 MaxPlaytTimeSec 또는 서버에서 계산된 Hit 시간까지의 ProjectileDurationSec
	}
	bHitAttached = bEnableHitAttached;
	bArrived = true;
}

void AT4WorldProjectileObject::SetArrivedForTarget(
	IT4GameObject* InTargetObject,
	const FVector& InHitLocation,
	const FVector& InShootDirection,
	FName InHitBone, 
	bool bPlayEndConti
) // #112
{
	if (bArrived)
	{
		return;
	}
	// #112 : Head 는 삭제하고, TargetObject 의 HitBone 에 새로 Conti 를 플레이한다.
	StopHeadConti(); 
	ProjectileMoveStop();
	PlayEndConti(bPlayEndConti);
	if (bEnableHitAttached && !HeadContiAssetPath.IsNull())
	{
		check(nullptr != InTargetObject);

		bool bVaild = false;
		FVector HitAddLocation = FVector::ZeroVector;
		FRotator HitAddRotation = InShootDirection.Rotation();
		if (InHitBone != NAME_None)
		{
			// #112 : Keep arrows stick into characters
			FTransform SocketTransform;
			if (InTargetObject->GetSocketTransform(InHitBone, ERelativeTransformSpace::RTS_World, SocketTransform))
			{
				// #112 : HitBone 을 기준으로 Offset 과 Rotation 을 구해준다. (SetOverrideProjectileAttachTransformRule 참조!)
				//        즉, World 상에서 Hit 위치를 Attach 에서도 유지하기 위한 처리.
				const float HitProjectileLength = ProjectileLengthEnter; // 화살의 꽂히는 정도를 약간의 랜덤성을 준다.
				const FVector HitWorldOffset = InHitLocation - SocketTransform.GetLocation();
				const FQuat ToLocalQuat = SocketTransform.Rotator().GetInverse().Quaternion();
				if (FMath::Square(ProjectileLength * 0.25f) >= HitWorldOffset.SizeSquared()) // 오차를 감안해 Hit 지점이 Bone 과 지나치게 멀 경우는 Offset 제외
				{
					HitAddLocation = ToLocalQuat * HitWorldOffset;
				}
				FVector LocalShootDirection = ToLocalQuat * InShootDirection; // Local Shoot Direction 을 Projectile Length 를 감안해 뒤로 밀어준다. (비쥬얼적인 Hit 지점 이동)
				HitAddLocation += -LocalShootDirection * HitProjectileLength;
				bVaild = true;
			}
		}
		if (bVaild)
		{
			FT4ActionParameters NewActionParameters;
			NewActionParameters.SetOverrideDurationSec(HitAttachedTimeSec);
			NewActionParameters.SetOverrideActionPoint(InHitBone);
			NewActionParameters.SetOverrideProjectileAttachTransformRule();
			NewActionParameters.SetOverrideLocalOrWorldLocation(HitAddLocation);
			NewActionParameters.SetOverrideLocalOrWorldRotation(HitAddRotation);

			FT4ContiAction NewContiAction;
			NewContiAction.ActionKey = ProjectileHeadActionKey;
			NewContiAction.ActionKey.bOverrideExisting = true; // #49 : 동기화가 중요하니 무조건 플레이를 보장한다.
			NewContiAction.ContiAsset = HeadContiAssetPath;
			InTargetObject->OnExecuteAction(&NewContiAction, &NewActionParameters);
		}
	}
	SetThisDestroy();
	bArrived = true;
	bHitAttached = false;
}

void AT4WorldProjectileObject::SetThisDestroy()
{
	if (bDestroying)
	{
		return;
	}
	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	GameWorld->GetContainer()->DestroyClientObject(GetObjectID());
	bDestroying = true; // TODO : FadeOut
}

bool AT4WorldProjectileObject::ExecuteLaunchAction(
	const FT4LaunchAction& InAction,
	const FT4ActionParameters* InActionParam
) // #63 : only Projectile
{ 
	check(nullptr != ProjectileComponent);
	if (nullptr == InActionParam)
	{
		return false;
	}
	OwnerObjectID = InAction.OwnerObjectID; // #112
	bEnableHitAttached = InAction.bEnableHitAttached; // #112
	HitAttachedTimeSec = InAction.HitAttachedTimeSec; // #112
	ProjectileLength = InAction.ProjectileLength; // #112 : Projectile 의 길이, 충돌 계산에서 Offset 으로 사용. (원점 에서의 길이)
	ProjectileLengthEnter = ProjectileLength * FMath::RandRange(0.65f, 0.9f);
	ThrowTargetPoint = InAction.ThrowTargetPoint; // #106 : Target 이 있을 경우만 사용됨
	MaxPlayTimeSec = InAction.MaxPlayTimeSec; // #63 : Conti 의 MaxPlaytTimeSec 또는 서버에서 계산된 Hit 시간까지의 ProjectileDurationSec
	EndContiAssetPath = InAction.EndContiAsset.ToSoftObjectPath();
	FVector ShootDirection = GetFrontVector();
	if (InActionParam->CheckBits(ET4TargetParamBits::ObjectIDBit))
	{
		IT4GameObject* TargetObject = nullptr;
		bool bResult = InActionParam->GetTargetObject(GetLayerType(), &TargetObject);
		if (bResult)
		{
			FVector TargetLocation = TargetObject->GetCOMLocation();
			if (ThrowTargetPoint != NAME_None)
			{
				TargetObject->GetSocketLocation(ThrowTargetPoint, TargetLocation); // #106 : 화살의 Goal 설정
			}
			ShootDirection = FVector(TargetLocation - GetRootLocation());
			ShootDirection.Z = 0.0f;
			ShootDirection.Normalize();

			TargetObjectID = TargetObject->GetObjectID(); // #112
		}
		else
		{
			// #112 : Target 이 사라졌을 경우를 대비해 Direction 이 서버에서부터 넘어오게 되어 있다.
			FVector TargetDirection = FVector::ZeroVector;
			bResult = InActionParam->GetTargetDirection(TargetDirection);
			if (bResult)
			{
				ShootDirection = TargetDirection;
				ShootDirection.Z = 0.0f;
				ShootDirection.Normalize();
			}
		}
	}
	else if (InActionParam->CheckBits(ET4TargetParamBits::LocationBit))
	{
		FVector TargetLocation = FVector::ZeroVector;
		bool bResult = InActionParam->GetTargetLocation(TargetLocation);
		if (bResult)
		{
			ShootDirection = FVector(TargetLocation - GetRootLocation());
			ShootDirection.Normalize();
		}
	}
	else if (InActionParam->CheckBits(ET4TargetParamBits::DirectionBit))
	{
		FVector TargetDirection = FVector::ZeroVector;
		bool bResult = InActionParam->GetTargetDirection(TargetDirection);
		if (bResult)
		{
			ShootDirection = TargetDirection;
			ShootDirection.Normalize();
		}
	}
	ProjectileComponent->InitialSpeed = InAction.MoveSpeed;
	ProjectileComponent->MaxSpeed = InAction.MoveSpeed * 1.5f; // #63 : 유도체의 경우 속도가 가변적일 수 있다.
	ProjectileComponent->Velocity = ShootDirection * ProjectileComponent->InitialSpeed;
	if (InAction.HeadContiAsset.IsValid())
	{
		FT4ContiAction NewContiAction;
		NewContiAction.ActionKey = ProjectileHeadActionKey;
		NewContiAction.ActionKey.bOverrideExisting = true; // #49 : 동기화가 중요하니 무조건 플레이를 보장한다.
		NewContiAction.ContiAsset = InAction.HeadContiAsset;
		OnExecuteAction(&NewContiAction, InActionParam);
		HeadContiAssetPath = InAction.HeadContiAsset.ToSoftObjectPath(); // #112
	}
	return true;
}