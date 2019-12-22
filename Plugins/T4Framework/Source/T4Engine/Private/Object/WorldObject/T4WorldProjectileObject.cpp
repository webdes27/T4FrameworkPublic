// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WorldProjectileObject.h"

#include "Object/Component/T4SphereComponent.h"
#include "Object/Component/T4ProjectileMovementComponent.h"
#include "Object/Component/T4SceneComponent.h"

#include "Object/Action/T4ActionControl.h" // #20

#include "Public/Action/T4ActionParameters.h"

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
	, bArrived(false)
	, PlayingTimeSec(0.0f)
	, MaxPlayTimeSec(0.0f)
	, GoalLocation(FVector::ZeroVector)
{
	CollisionComponent = CreateDefaultSubobject<UT4SphereComponent>(TEXT("T4SphereComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
	CollisionComponent->OnComponentHit.AddDynamic(this, &AT4WorldProjectileObject::HandleOnHit);
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
	PlayingTimeSec += InUpdateTime.ScaledTimeSec;

	if (bArrived)
	{
		return;
	}
	float CurrentLifeTimeSec = MaxPlayTimeSec - PlayingTimeSec;
	if (0.0f >= CurrentLifeTimeSec)
	{
		return; // 삭제 예정!
	}
	if (nullptr == ProjectileComponent)
	{
		return;
	}
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
	IT4GameObject* TargetObject = GameWorld->GetContainer()->FindGameObject(TargetObjectID);
	if (nullptr == TargetObject)
	{
		return; // TODO : 타겟이 사라졌을 경우에 대한 처리!
	}
	FVector TargetVector = TargetObject->GetCOMLocation() - GetRootLocation();
	float TargetDistance = TargetVector.Size(); // #63

	// #63 : 나중에는 Attacker 의 발사지점, Target 의 피격 지점으로 거리에 따른 속도를 계산해야 함.
	//       현재는 미관상, 양쪽의 Radius 를 반영해 ProjSpeed 를 계산하도록 처리
	TargetDistance -= TargetObject->GetPropertyConst().CapsuleRadius;
	if (0.0f >= TargetDistance)
	{
		// TODO : 도착??
		return;
	}
	FVector ShootDirection = TargetVector / TargetDistance;
	float CurrentSpeed = TargetDistance / CurrentLifeTimeSec;
	ProjectileComponent->Velocity = ShootDirection * CurrentSpeed;
}

void AT4WorldProjectileObject::UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime)
{
	if (nullptr != ProjectileComponent)
	{
		if (0.0f < MaxPlayTimeSec && MaxPlayTimeSec <= PlayingTimeSec )
		{
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

void AT4WorldProjectileObject::SetArrived(bool bPlayEndConti)
{
	if (bArrived)
	{
		return;
	}
	// #63 : Head 는 끄고, Projectile 도 멈춘다.
	FT4StopAction NewAction;
	NewAction.ActionKey = ProjectileHeadActionKey;
	NewAction.StartTimeSec = 0.0f;
	NewAction.bSameKeyNameRemoveAll = true;
	ExecutePublicAction(&NewAction, nullptr);
	ProjectileComponent->Velocity = FVector::ZeroVector;
	if (bPlayEndConti)
	{
		if (EndContiAssetPath.IsValid())
		{
			FT4ContiAction NewContiAction;
			NewContiAction.ActionKey = ProjectileEndActionKey;
			NewContiAction.ActionKey.bOverrideExisting = true; // #49 : 동기화가 중요하니 무조건 플레이를 보장한다.
			NewContiAction.ContiAsset = EndContiAssetPath;
			ExecutePublicAction(&NewContiAction, nullptr);
		}
	}
	// #63 : Conti 의 MaxPlaytTimeSec 또는 서버에서 계산된 Hit 시간까지의 ProjectileDurationSec
	GetGameWorld()->GetContainer()->DestroyClientObject(GetObjectID());
	bArrived = true;
}

void AT4WorldProjectileObject::HandleOnHit(
	UPrimitiveComponent* InHitComponent,
	AActor* InOtherActor,
	UPrimitiveComponent* InOtherComponent,
	FVector InNormalImpulse,
	const FHitResult& InHit
)
{
	if (InOtherActor != this && InOtherComponent->IsSimulatingPhysics())
	{
		InOtherComponent->AddImpulseAtLocation(ProjectileComponent->Velocity * 100.0f, InHit.ImpactPoint);
	}
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
	TargetObjectID = InAction.TargetObjectID; // #63 : Target 이 있을 경우만 설정됨!
	MaxPlayTimeSec = InAction.MaxPlayTimeSec; // #63 : Conti 의 MaxPlaytTimeSec 또는 서버에서 계산된 Hit 시간까지의 ProjectileDurationSec
	EndContiAssetPath = InAction.EndContiAsset.ToSoftObjectPath();
	FVector ShootDirection = GetFrontVector();
	if (InActionParam->CheckBits(ET4TargetParamBits::ObjectIDBit))
	{
		IT4GameObject* TargetObject = nullptr;
		bool bResult = InActionParam->GetTargetObject(
			GetLayerType(),
			&TargetObject
		);
		if (bResult)
		{
			ShootDirection = FVector(
				TargetObject->GetRootLocation() - GetRootLocation()
			);
			ShootDirection.Z = 0.0f;
			ShootDirection.Normalize();
		}
	}
	else if (InActionParam->CheckBits(ET4TargetParamBits::LocationBit))
	{
		FVector TargetLocation = FVector::ZeroVector;
		bool bResult = InActionParam->GetTargetLocation(TargetLocation);
		if (bResult)
		{
			ShootDirection = FVector(
				TargetLocation - GetRootLocation()
			);
			ShootDirection.Z = 0.0f;
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
			ShootDirection.Z = 0.0f;
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
		DoExecuteAction(&NewContiAction, InActionParam);
	}
	return true;
}