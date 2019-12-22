// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4MovableItemObject.h"

#include "Object/Component/T4CapsuleComponent.h"
#include "Object/Component/T4StaticMeshComponent.h"
#include "Object/Component/T4SkeletalMeshComponent.h"

#include "Object/Animation/T4ItemAnimControl.h"

#include "T4Asset/Classes/Entity/T4WeaponEntityAsset.h"
#include "T4Asset/Classes/Entity/T4CostumeEntityAsset.h"
#include "T4Asset/Public/Entity/T4Entity.h"

#include "Engine/CollisionProfile.h"

#include "T4EngineInternal.h"

/**
  *
 */
AT4MovableItemObject::AT4MovableItemObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, CapsuleComponent(nullptr)
	, StaticMeshComponent(nullptr)
	, SkeletalMeshComponent(nullptr)
	, MeshType(ET4EntityMeshType::None)
	, ActiveMeshComponent(nullptr)
	, bOverrideMaterialLoading(false) // #80
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
}

AT4MovableItemObject::~AT4MovableItemObject()
{
	check(nullptr == AnimControl);
	check(!EntityAssetPtr.IsValid());
}

void AT4MovableItemObject::Reset()
{
	// WARN : AsyncLoad 가 걸렸을 수 있음으로 종료 시 명시적으로 Reset 을 호출해야 한다.
	StaticMeshLoader.Reset();
	SkeletalMeshLoader.Reset();
	ClearOverrideMaterialLoader(); // #80
	if (nullptr != StaticMeshComponent)
	{
		StaticMeshComponent->UnregisterComponent();
		StaticMeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		StaticMeshComponent = nullptr;
	}
	if (nullptr != SkeletalMeshComponent)
	{
		SkeletalMeshComponent->UnregisterComponent();
		SkeletalMeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		SkeletalMeshComponent = nullptr;
	}
	ActiveMeshComponent = nullptr;
	if (nullptr != AnimControl)
	{
		delete AnimControl;
		AnimControl = nullptr;
	}
	MeshType = ET4EntityMeshType::None;
	EntityAssetPtr.Reset();
}

void AT4MovableItemObject::ClearOverrideMaterialLoader() // #80
{
	for (FT4MaterialLoader& MaterialLoader : OverrideMaterialLoaders)
	{
		MaterialLoader.Reset();
	}
	OverrideMaterialLoaders.Empty();
}

void AT4MovableItemObject::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!IsPendingKill())
	{
		if (nullptr != ActiveMeshComponent)
		{
			// WARN : 애니메이션 틱은 ActorTick(this) 이후 동작하도록 처리한다.
			//        Action 처리 후 애니메이션이 바로 반영되도록 처리하기 위한 것!
			// force animation tick after actor tick updates
			if (ActiveMeshComponent->PrimaryComponentTick.bCanEverTick)
			{
				ActiveMeshComponent->PrimaryComponentTick.AddPrerequisite(
					this,
					PrimaryActorTick
				);
			}
		}
	}
}

void AT4MovableItemObject::UpdateTickActorBefore(const FT4UpdateTime& InUpdateTime)
{
	bool bCurrentLoadComplated = IsLoaded();
	if (!bCurrentLoadComplated) // #8
	{
		bool bLoadComplate = CheckAsyncLoading();
		if (bLoadComplate)
		{
			SetLoadComplated();
		}
	}
	if (!bCurrentLoadComplated)
	{
		return;
	}

	if (bOverrideMaterialLoading) // #80
	{
		bool bOverrideMaterialLoadComplated = CheckOverrideMaterialAsyncLoading(false);
		if (bOverrideMaterialLoadComplated)
		{

		}
	}
}

void AT4MovableItemObject::UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime)
{
}

bool AT4MovableItemObject::Create(
	const FT4SpawnObjectAction* InAction
)
{
	check(!EntityAssetPtr.IsValid());
	EntityAssetPtr = T4AssetEntityManagerGet()->GetItemEntity(InAction->EntityAssetPath);
	if (!EntityAssetPtr.IsValid())
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("AT4MovableItemObject : EntityAsset (%s) Not Found."),
			*(InAction->EntityAssetPath.ToString())
		);
		return false;
	}

	EntityKey.Type = InAction->EntityType; // #35
	EntityKey.Value = EntityAssetPtr->GetEntityKeyPath(); // #35

	const FT4EntityItemDropMeshData& MeshData = EntityAssetPtr->DropMeshData;

	bool bResult = true;
	MeshType = MeshData.MeshType;
	switch (MeshData.MeshType)
	{
		case ET4EntityMeshType::StaticMesh:
			bResult = CreateStaticMesh();
			break;

		case ET4EntityMeshType::SkeletalMesh:
			bResult = CreateSkeletalMesh();
			break;

#if 0
		case ET4EntityMeshType::ParticleSystem:
			bResult = CreateSkeletalMesh(); // todo
			break;
#endif

		default:
			{
				UE_LOG(
					LogT4Engine,
					Error,
					TEXT("AT4MovableItemObject : Unknown Item Mesh type '%u'"),
					uint8(MeshData.MeshType)
				);
			}
			break;
	}

	if (!bResult)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("AT4MovableItemObject : Create failed")
		);
		return false;
	}

	ApplyEntityAttributes();
	return bResult;
}

bool AT4MovableItemObject::CreateStaticMesh() // #80
{
	check(EntityAssetPtr.IsValid());
	check(nullptr == StaticMeshComponent);
	const FT4EntityItemDropMeshData& MeshData = EntityAssetPtr->DropMeshData;

	StaticMeshComponent = NewObject<UT4StaticMeshComponent>(this);
	check(nullptr != StaticMeshComponent);
	{
		StaticMeshComponent->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
		StaticMeshComponent->SetupAttachment(CapsuleComponent);
		StaticMeshComponent->Mobility = EComponentMobility::Movable;
		StaticMeshComponent->SetGenerateOverlapEvents(false);
		StaticMeshComponent->bUseDefaultCollision = true;
		StaticMeshComponent->AttachToComponent(
			CapsuleComponent,
			FAttachmentTransformRules::SnapToTargetIncludingScale
		);
		StaticMeshComponent->RegisterComponent();
		ActiveMeshComponent = Cast<UMeshComponent>(StaticMeshComponent);
	}

	SetOverrideMaterialAsyncLoading(); // #80

	// #8
	const TCHAR* DebugString = *(EntityKey.ToString());
	StaticMeshLoader.Load(MeshData.StaticMeshAsset.ToSoftObjectPath(), false, DebugString);
	// ~#8

	return true;
}

bool AT4MovableItemObject::CreateSkeletalMesh() // #80
{
	check(EntityAssetPtr.IsValid());
	check(nullptr == SkeletalMeshComponent);
	const FT4EntityItemDropMeshData& MeshData = EntityAssetPtr->DropMeshData;

	SkeletalMeshComponent = NewObject<UT4SkeletalMeshComponent>(this);
	check(nullptr != SkeletalMeshComponent);
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
		SkeletalMeshComponent->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
		SkeletalMeshComponent->SetGenerateOverlapEvents(false);
		SkeletalMeshComponent->SetCanEverAffectNavigation(false);
		SkeletalMeshComponent->AttachToComponent(
			CapsuleComponent,
			FAttachmentTransformRules::SnapToTargetIncludingScale
		);
		SkeletalMeshComponent->RegisterComponent();
		ActiveMeshComponent = Cast<UMeshComponent>(SkeletalMeshComponent);
	}

	SetOverrideMaterialAsyncLoading(); // #80

	// #8
	const TCHAR* DebugString = *(EntityKey.ToString());
	SkeletalMeshLoader.Load(MeshData.SkeletalMeshAsset.ToSoftObjectPath(), false, DebugString);
	// ~#8

	return true;
}

bool AT4MovableItemObject::CheckAsyncLoading()
{
	if (bOverrideMaterialLoading) // #80
	{
		bool bOverrideMaterialLoadComplated = CheckOverrideMaterialAsyncLoading(true);
		if (!bOverrideMaterialLoadComplated)
		{
			return false;
		}
	}

	if (ET4EntityMeshType::StaticMesh == MeshType)
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
	}
	else if (ET4EntityMeshType::SkeletalMesh == MeshType)
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
	}

	ApplyEntityAttributes();
	return true;
}

bool AT4MovableItemObject::CheckOverrideMaterialAsyncLoading(bool bInitialize) // #80
{
	if (!bOverrideMaterialLoading)
	{
		return true;
	}
	for (FT4MaterialLoader& MaterialLoader : OverrideMaterialLoaders)
	{
		if (MaterialLoader.IsLoadStarted() && !MaterialLoader.IsLoadCompleted())
		{
			return false;
		}
	}

	check(nullptr != ActiveMeshComponent);
	for (int32 i = 0; i < OverrideMaterialLoaders.Num(); ++i)
	{
		FT4MaterialLoader& MaterialLoader = OverrideMaterialLoaders[i];
		UMaterialInterface* MaterialInterface = nullptr;
		if (MaterialLoader.IsLoadStarted())
		{
			check(MaterialLoader.IsLoadCompleted());
			MaterialInterface = MaterialLoader.GetMaterialInterface();
			check(nullptr != MaterialInterface);
		}
		if (bInitialize)
		{
			ActiveMeshComponent->OverrideMaterials.Add(MaterialInterface);
		}
		else
		{
			ActiveMeshComponent->SetMaterial(i, MaterialInterface);
		}
	}

	ClearOverrideMaterialLoader();
	bOverrideMaterialLoading = false;
	return true;
}

void AT4MovableItemObject::SetOverrideMaterialAsyncLoading() // #80
{
	if (bOverrideMaterialLoading)
	{
		check(false); // TODO : 로딩중 처리!
		return;
	}
	check(nullptr != ActiveMeshComponent);
	check(0 == OverrideMaterialLoaders.Num());
	const FT4EntityItemDropMeshData& MeshData = EntityAssetPtr->DropMeshData;
	const FT4EntityOverrideMaterialData* OverrideMaterialData = nullptr;
	if (ET4EntityMeshType::StaticMesh == MeshType)
	{
		if (MeshData.StaticMeshAsset.IsNull())
		{
			return;
		}
		OverrideMaterialData = &MeshData.StaticMeshOverrideMaterialData;
	}
	else if (ET4EntityMeshType::SkeletalMesh == MeshType)
	{
		if (MeshData.SkeletalMeshAsset.IsNull())
		{
			return;
		}
		OverrideMaterialData = &MeshData.SkeletalMeshOverrideMaterialData;
	}
	else
	{
		check(false);
	}

	check(nullptr != OverrideMaterialData);
	if (0 >= OverrideMaterialData->MaterialSortedSlotNames.Num()) // #80
	{
		ActiveMeshComponent->EmptyOverrideMaterials();
		return;
	}
	const TCHAR* DebugString = *(EntityKey.ToString());
	for (FName SlotName : OverrideMaterialData->MaterialSortedSlotNames)
	{
		check(OverrideMaterialData->MaterialMap.Contains(SlotName));
		TSoftObjectPtr<UMaterialInterface> MaterialInstance = OverrideMaterialData->MaterialMap[SlotName];
		FT4MaterialLoader& NewMateralLoader = OverrideMaterialLoaders.AddDefaulted_GetRef();
		if (!MaterialInstance.IsNull()) // Override Material 이기 때문이 null 이면 원본을 사용하도록 비워둔다.
		{
			NewMateralLoader.Load(MaterialInstance.ToSoftObjectPath(), false, DebugString);
		}
	}
	bOverrideMaterialLoading = true;
}

void AT4MovableItemObject::ApplyEntityAttributes()
{
	check(EntityAssetPtr.IsValid());
	const FT4EntityItemPhysicalAttribute& PhysicalAttribute = EntityAssetPtr->DropMeshPhysical;
	const FT4EntityItemRenderingAttribute& RenderingAttribute = EntityAssetPtr->DropMeshRendering;

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
		check(nullptr != ActiveMeshComponent);
		ActiveMeshComponent->SetRelativeLocationAndRotation(
			FVector(0.0f, 0.0f, -Property.HalfHeight),
			Property.MeshImportRotation
		);
		if (T4EngineLayer::IsLevelEditor(LayerType) && nullptr != SkeletalMeshComponent)
		{
			SkeletalMeshComponent->SetUpdateAnimationInEditor(true); // #17
		}
	}
}

IT4AnimControl* AT4MovableItemObject::GetAnimControl() const
{
	if (nullptr == AnimControl)
	{
		return AT4GameObject::GetAnimControl();
	}
	return static_cast<IT4AnimControl*>(AnimControl); // #14
}

#if WITH_EDITOR
void AT4MovableItemObject::RecreateAll() // #80
{
	FT4SpawnObjectAction ReCreateAction;
	ReCreateAction.EntityType = EntityKey.Type;
	ReCreateAction.EntityAssetPath = TSoftObjectPtr<UT4CharacterEntityAsset>(
		EntityAssetPtr.Get()
	).ToSoftObjectPath();

	ResetLoadComplated();
	Reset();

	bool bResult = Create(&ReCreateAction);
	if (bResult)
	{

	}
}

bool AT4MovableItemObject::ExecuteEditorAction(const FT4EditorAction& InAction)
{
	// #37
	check(ET4ActionType::Editor == InAction.ActionType);

	switch (InAction.EditorActionType)
	{
		case ET4EditorAction::ReloadObject: // #37
			RecreateAll();
			break;

		case ET4EditorAction::RestoreReaction: // #76
			GetManualControl().EditorRestoreReaction();
			break;

		case ET4EditorAction::UpdateOverrideMaterials: // #80
			SetOverrideMaterialAsyncLoading();
			break;

		default:
		{
			UE_LOG(
				LogT4Engine,
				Error,
				TEXT("AT4MovableItemObject : Unknown Editor Action type '%u'"),
				uint8(InAction.EditorActionType)
			);
		}
		break;
	}

	return true;
}
#endif