// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4MovablePropParticleObject.h"

#include "Object/Component/T4CapsuleComponent.h"
#include "Object/Component/T4ParticleSystemComponent.h"

#include "Object/Animation/T4PropAnimControl.h"

#include "T4Asset/Classes/Entity/T4PropEntityAsset.h"
#include "T4Asset/Public/Entity/T4Entity.h"

#include "Engine/CollisionProfile.h"

#include "T4EngineInternal.h"

/**
  *
 */
AT4MovablePropParticleObject::AT4MovablePropParticleObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CapsuleComponent = CreateDefaultSubobject<UT4CapsuleComponent>(TEXT("T4CapsuleComponent"));
	CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
	CapsuleComponent->SetCanEverAffectNavigation(false);
	CapsuleComponent->bDynamicObstacle = true;
	RootComponent = CapsuleComponent;

	ParticleSystemComponent = CreateOptionalDefaultSubobject<UT4ParticleSystemComponent>(TEXT("UT4ParticleSystemComponent"));
	if (nullptr != ParticleSystemComponent)
	{
		ParticleSystemComponent->SetupAttachment(CapsuleComponent);
		ParticleSystemComponent->SecondsBeforeInactive = 1;
	}
}

AT4MovablePropParticleObject::~AT4MovablePropParticleObject()
{
	check(nullptr == AnimControl);
	check(!EntityAssetPtr.IsValid());
}

void AT4MovablePropParticleObject::Reset()
{
	// WARN : AsyncLoad 가 걸렸을 수 있음으로 종료 시 명시적으로 Reset 을 호출해야 한다.
	ParticleSystemLoader.Reset();
	if (nullptr != AnimControl)
	{
		delete AnimControl;
		AnimControl = nullptr;
	}
	EntityAssetPtr.Reset();
}

void AT4MovablePropParticleObject::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!IsPendingKill())
	{
		if (nullptr != ParticleSystemComponent)
		{
			// WARN : 애니메이션 틱은 ActorTick(this) 이후 동작하도록 처리한다.
			//        Action 처리 후 애니메이션이 바로 반영되도록 처리하기 위한 것!
			// force animation tick after actor tick updates
			if (ParticleSystemComponent->PrimaryComponentTick.bCanEverTick)
			{
				ParticleSystemComponent->PrimaryComponentTick.AddPrerequisite(
					this,
					PrimaryActorTick
				);
			}
		}
	}

	// Set Notification Delegate
	if (ParticleSystemComponent)
	{
		ParticleSystemComponent->OnSystemFinished.AddUniqueDynamic(
			this,
			&AT4MovablePropParticleObject::OnParticleSystemFinished
		);
	}

	if (ParticleSystemComponent && bPostUpdateTickGroup)
	{
		ParticleSystemComponent->SetTickGroup(TG_PostUpdateWork);
	}
}

void AT4MovablePropParticleObject::UpdateTickActorBefore(const FT4UpdateTime& InUpdateTime)
{
	if (!IsLoaded()) // #8
	{
		bool bLoadComplate = CheckAsyncLoading();
		if (bLoadComplate)
		{
			SetLoadComplated();
		}
		return;
	}
}

void AT4MovablePropParticleObject::UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime)
{
}

void AT4MovablePropParticleObject::PostActorCreated()
{
	Super::PostActorCreated();

	if (ParticleSystemComponent && bPostUpdateTickGroup)
	{
		ParticleSystemComponent->SetTickGroup(TG_PostUpdateWork);
	}
}

void AT4MovablePropParticleObject::SetTemplate(UParticleSystem* NewTemplate)
{
	if (ParticleSystemComponent)
	{
		ParticleSystemComponent->SetTemplate(NewTemplate);
		if (bPostUpdateTickGroup)
		{
			ParticleSystemComponent->SetTickGroup(TG_PostUpdateWork);
		}
	}
}

bool AT4MovablePropParticleObject::Create(
	const FT4SpawnObjectAction* InAction
)
{
	check(!EntityAssetPtr.IsValid());
	EntityAssetPtr = T4AssetEntityManagerGet()->GetPropEntity(InAction->EntityAssetPath);
	if (!EntityAssetPtr.IsValid())
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("AT4MovablePropParticleObject : EntityAsset (%s) Not Found."),
			*(InAction->EntityAssetPath.ToString())
		);
		return false;
	}

	EntityKey.Type = InAction->EntityType; // #35
	EntityKey.Value = EntityAssetPtr->GetEntityKeyPath(); // #35

	ApplyEntityAttributes();

	const FT4EntityPropNormalMeshData& MeshData = EntityAssetPtr->NormalMeshData;
	check(ET4EntityMeshType::ParticleSystem == MeshData.MeshType);

	check(nullptr != ParticleSystemComponent);
	ParticleSystemComponent->bAutoActivate = true;
	ParticleSystemComponent->bAutoDestroy = false;
	ParticleSystemComponent->SetRelativeLocation(FVector::ZeroVector);

	// #8
	const TCHAR* DebugString = *(EntityKey.ToString());
	ParticleSystemLoader.Load(MeshData.ParticleSystemAsset.ToSoftObjectPath(), false, DebugString);
	// ~#8

	return true;
}

void AT4MovablePropParticleObject::CreateFinished(
	const FVector& InSpawnLocation,
	const FRotator& InSpawnRotation,
	const FVector& InSpawnScale
)
{
	check(nullptr != ParticleSystemComponent);
	ParticleSystemComponent->SetWorldLocationAndRotation(InSpawnLocation, InSpawnRotation);
	ParticleSystemComponent->SetWorldScale3D(InSpawnScale);
	Super::CreateFinished(InSpawnLocation, InSpawnRotation, InSpawnScale);
}

bool AT4MovablePropParticleObject::CheckAsyncLoading()
{
	if (!ParticleSystemLoader.IsBinded())
	{
		check(nullptr != ParticleSystemComponent);
		if (!ParticleSystemLoader.Process(ParticleSystemComponent))
		{
			return false;
		}
		ParticleSystemComponent->SetRelativeScale3D(GetProperty().RelativeScale3D);
		ParticleSystemLoader.SetBinded();
	}
	ApplyEntityAttributes();
	return true;
}

void AT4MovablePropParticleObject::ApplyEntityAttributes()
{
	check(EntityAssetPtr.IsValid());
	const FT4EntityPropPhysicalAttribute& PhysicalAttribute = EntityAssetPtr->Physical;
	const FT4EntityPropRenderingAttribute& RenderingAttribute = EntityAssetPtr->Rendering;

	FT4GameObjectProperty& Property = GetProperty();

	Property.HalfHeight = PhysicalAttribute.CapsuleHeight * 0.5f;
	Property.CapsuleRadius = PhysicalAttribute.CapsuleRadius;
	Property.MeshImportRotation = FRotator(0.0f, RenderingAttribute.ImportRotationYaw, 0.0f);
	Property.RelativeScale3D = FVector(RenderingAttribute.Scale);

	{
		check(nullptr != CapsuleComponent);
		CapsuleComponent->SetCapsuleSize(PhysicalAttribute.CapsuleRadius, Property.HalfHeight);
	}
	{
		// 컬리전이 바닥위에 있기 때문에 Mesh 는 HalfHeight 만큼 내려줘서 Root 가 바닥에 있도록 처리한다.
		check(nullptr != ParticleSystemComponent);
		ParticleSystemComponent->SetRelativeLocationAndRotation(
			FVector(0.0f, 0.0f, -Property.HalfHeight),
			Property.MeshImportRotation
		);
	}
}

void AT4MovablePropParticleObject::OnParticleSystemFinished(UParticleSystemComponent* FinishedComponent)
{
	if (bDestroyOnSystemFinish)
	{
		SetLifeSpan(0.0001f);
	}
	//bCurrentlyActive = false;
}

IT4AnimControl* AT4MovablePropParticleObject::GetAnimControl() const
{
	if (nullptr == AnimControl)
	{
		return AT4GameObject::GetAnimControl();
	}
	return static_cast<IT4AnimControl*>(AnimControl); // #14
}