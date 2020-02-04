// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4MovablePropStaticObject.h"

#include "Object/Component/T4CapsuleComponent.h"
#include "Object/Component/T4StaticMeshComponent.h"

#include "Object/Animation/T4PropAnimControl.h"

#include "T4Asset/Classes/Entity/T4PropEntityAsset.h"
#include "T4Asset/Public/Entity/T4Entity.h"

#include "Engine/CollisionProfile.h"

#include "T4EngineInternal.h"

/**
  *
 */
AT4MovablePropStaticObject::AT4MovablePropStaticObject(const FObjectInitializer& ObjectInitializer)
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

	StaticMeshComponent = CreateOptionalDefaultSubobject<UT4StaticMeshComponent>(TEXT("UT4StaticMeshComponent"));
	if (nullptr != StaticMeshComponent)
	{
		StaticMeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		StaticMeshComponent->SetupAttachment(CapsuleComponent);
		StaticMeshComponent->Mobility = EComponentMobility::Movable;
		StaticMeshComponent->SetGenerateOverlapEvents(false);
		StaticMeshComponent->bUseDefaultCollision = true;
	}
}

AT4MovablePropStaticObject::~AT4MovablePropStaticObject()
{
	check(nullptr == AnimControl);
	check(!EntityAssetPtr.IsValid());
}

void AT4MovablePropStaticObject::Reset()
{
	// WARN : AsyncLoad 가 걸렸을 수 있음으로 종료 시 명시적으로 Reset 을 호출해야 한다.
	StaticMeshLoader.Reset();
	if (nullptr != AnimControl)
	{
		delete AnimControl;
		AnimControl = nullptr;
	}
	EntityAssetPtr.Reset();
}

void AT4MovablePropStaticObject::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!IsPendingKill())
	{
		if (nullptr != StaticMeshComponent)
		{
			// WARN : 애니메이션 틱은 ActorTick(this) 이후 동작하도록 처리한다.
			//        Action 처리 후 애니메이션이 바로 반영되도록 처리하기 위한 것!
			// force animation tick after actor tick updates
			if (StaticMeshComponent->PrimaryComponentTick.bCanEverTick)
			{
				StaticMeshComponent->PrimaryComponentTick.AddPrerequisite(
					this,
					PrimaryActorTick
				);
			}
		}
	}
}

void AT4MovablePropStaticObject::UpdateTickActorBefore(const FT4UpdateTime& InUpdateTime)
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

void AT4MovablePropStaticObject::UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime)
{
}

bool AT4MovablePropStaticObject::Create(
	const FT4SpawnObjectAction* InAction
)
{
	check(!EntityAssetPtr.IsValid());
	EntityAssetPtr = T4AssetEntityManagerGet()->GetPropEntity(InAction->EntityAssetPath);
	if (!EntityAssetPtr.IsValid())
	{
		T4_LOG(
			Error,
			TEXT("EntityAsset (%s) Not Found"),
			*(InAction->EntityAssetPath.ToString())
		);
		return false;
	}

	EntityKey.Type = InAction->EntityType; // #35
	EntityKey.Value = EntityAssetPtr->GetEntityKeyPath(); // #35

	ApplyEntityAttributes();

	const FT4EntityPropNormalMeshData& MeshData = EntityAssetPtr->NormalMeshData;
	check(ET4EntityMeshType::StaticMesh == MeshData.MeshType);

	// #8
	const FString DebugString = EntityKey.ToString();
	StaticMeshLoader.Load(MeshData.StaticMeshAsset.ToSoftObjectPath(), false, *DebugString);
	// ~#8

	return true;
}

bool AT4MovablePropStaticObject::CheckAsyncLoading()
{
	if (!StaticMeshLoader.IsBinded())
	{
		check(nullptr != StaticMeshComponent);
		if (!StaticMeshLoader.Process(StaticMeshComponent))
		{
			return false;
		}
		StaticMeshComponent->SetRelativeScale3D(GetProperty().RelativeScale3D);
		StaticMeshLoader.SetBinded();
	}
	ApplyEntityAttributes();
	return true;
}

void AT4MovablePropStaticObject::ApplyEntityAttributes()
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
		check(nullptr != StaticMeshComponent);
		StaticMeshComponent->SetRelativeLocationAndRotation(
			FVector(0.0f, 0.0f, -Property.HalfHeight),
			Property.MeshImportRotation
		);
	}
}

IT4AnimControl* AT4MovablePropStaticObject::GetAnimControl() const
{
	if (nullptr == AnimControl)
	{
		return AT4GameObject::GetAnimControl();
	}
	return static_cast<IT4AnimControl*>(AnimControl); // #14
}