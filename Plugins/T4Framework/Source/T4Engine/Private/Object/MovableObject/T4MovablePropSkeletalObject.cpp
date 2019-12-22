// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4MovablePropSkeletalObject.h"

#include "Object/Component/T4CapsuleComponent.h"
#include "Object/Component/T4SkeletalMeshComponent.h"

#include "T4Asset/Classes/Entity/T4PropEntityAsset.h"
#include "T4Asset/Public/Entity/T4Entity.h"

#include "Engine/CollisionProfile.h"

#include "T4EngineInternal.h"

/**
  *
 */
AT4MovablePropSkeletalObject::AT4MovablePropSkeletalObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, AnimControl(nullptr)
{
	CapsuleComponent = CreateDefaultSubobject<UT4CapsuleComponent>(TEXT("T4CapsuleComponent"));
	CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
	CapsuleComponent->SetCanEverAffectNavigation(false);
	CapsuleComponent->bDynamicObstacle = true;
	RootComponent = CapsuleComponent;

	SkeletalMeshComponent = CreateOptionalDefaultSubobject<UT4SkeletalMeshComponent>(TEXT("T4SkeletalMeshComponent"));
	if (nullptr != SkeletalMeshComponent)
	{
		SkeletalMeshComponent->AlwaysLoadOnClient = true;
		SkeletalMeshComponent->AlwaysLoadOnServer = true;
		SkeletalMeshComponent->bOwnerNoSee = false;
#if (WITH_EDITOR || WITH_SERVER_CODE)
		// #49 : SkeletalMesh 에 대해 서버애서 OverlapEvent 가 동작하기 위해서 아래의 옵션 적용. 렌더링과 관계없이 업데이트한다.
		SkeletalMeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
#else
		SkeletalMeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
#endif
		SkeletalMeshComponent->bCastDynamicShadow = true;
		SkeletalMeshComponent->bAffectDynamicIndirectLighting = true;
		SkeletalMeshComponent->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		SkeletalMeshComponent->SetupAttachment(CapsuleComponent);
		SkeletalMeshComponent->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
		SkeletalMeshComponent->SetGenerateOverlapEvents(false);
		SkeletalMeshComponent->SetCanEverAffectNavigation(false);
	}
}

AT4MovablePropSkeletalObject::~AT4MovablePropSkeletalObject()
{
	check(nullptr == AnimControl);
	check(!EntityAssetPtr.IsValid());
}

void AT4MovablePropSkeletalObject::Reset()
{
	// WARN : AsyncLoad 가 걸렸을 수 있음으로 종료 시 명시적으로 Reset 을 호출해야 한다.
	SkeletalMeshLoader.Reset();
	if (nullptr != AnimControl)
	{
		delete AnimControl;
		AnimControl = nullptr;
	}
	EntityAssetPtr.Reset();
}

void AT4MovablePropSkeletalObject::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!IsPendingKill())
	{
		if (nullptr != SkeletalMeshComponent)
		{
			// WARN : 애니메이션 틱은 ActorTick(this) 이후 동작하도록 처리한다.
			//        Action 처리 후 애니메이션이 바로 반영되도록 처리하기 위한 것!
			// force animation tick after actor tick updates
			if (SkeletalMeshComponent->PrimaryComponentTick.bCanEverTick)
			{
				SkeletalMeshComponent->PrimaryComponentTick.AddPrerequisite(
					this,
					PrimaryActorTick
				);
			}
		}
	}
}

void AT4MovablePropSkeletalObject::UpdateTickActorBefore(const FT4UpdateTime& InUpdateTime)
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

void AT4MovablePropSkeletalObject::UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime)
{
}

bool AT4MovablePropSkeletalObject::Create(
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
			TEXT("AT4MovablePropSkeletalObject : EntityAsset (%s) Not Found."),
			*(InAction->EntityAssetPath.ToString())
		);
		return false;
	}

	EntityKey.Type = InAction->EntityType; // #35
	EntityKey.Value = EntityAssetPtr->GetEntityKeyPath(); // #35

	const FT4EntityPropNormalMeshData& MeshData = EntityAssetPtr->NormalMeshData;
	check(ET4EntityMeshType::SkeletalMesh == MeshData.MeshType);

	// #8
	const TCHAR* DebugString = *(EntityKey.ToString());
	SkeletalMeshLoader.Load(MeshData.SkeletalMeshAsset.ToSoftObjectPath(), false, DebugString);
	// ~#8

	return true;
}

bool AT4MovablePropSkeletalObject::CheckAsyncLoading()
{
	if (!SkeletalMeshLoader.IsBinded())
	{
		check(nullptr != SkeletalMeshComponent);
		if (!SkeletalMeshLoader.Process(SkeletalMeshComponent))
		{
			return false;
		}
		SkeletalMeshComponent->SetRelativeScale3D(GetProperty().RelativeScale3D);
		SkeletalMeshLoader.SetBinded();
	}
	return true;
}

void AT4MovablePropSkeletalObject::ApplyEntityAttributes()
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
		check(nullptr != SkeletalMeshComponent);
		SkeletalMeshComponent->SetRelativeLocationAndRotation(
			FVector(0.0f, 0.0f, -Property.HalfHeight),
			Property.MeshImportRotation
		);
		if (T4EngineLayer::IsLevelEditor(LayerType))
		{
			SkeletalMeshComponent->SetUpdateAnimationInEditor(true); // #17
		}
	}
}

IT4AnimControl* AT4MovablePropSkeletalObject::GetAnimControl() const
{
	if (nullptr == AnimControl)
	{
		return AT4GameObject::GetAnimControl();
	}
	return static_cast<IT4AnimControl*>(AnimControl); // #14
}