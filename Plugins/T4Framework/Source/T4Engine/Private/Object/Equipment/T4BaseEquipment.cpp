// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4BaseEquipment.h"
#include "T4StaticMeshEquipment.h"
#include "T4SkeletalMeshEquipment.h"
#include "T4ParticleSystemEquipment.h" // #110

#include "Object/T4GameObject.h"

#include "Public/T4EngineConstants.h" // #108

#include "T4Asset/Classes/Entity/T4WeaponEntityAsset.h" // #41
#include "T4Asset/Public/Entity/T4Entity.h"

#include "Components/MeshComponent.h" // #108
#include "Materials/MaterialInstanceDynamic.h" // #108
#include "Engine/CollisionProfile.h" // #49

#include "T4EngineInternal.h"

/**
  * #68, #107
 */
FT4BaseEquipment::FT4BaseEquipment(AT4GameObject* InOwnerObject, const FT4ActionKey& InActionKey)
	: bAttached(false)
	, bDetachable(false) // #111
	, bPendingAttach(false) // #111
	, bPendingDetach(false) // #111
	, bMainWeapon(false) // #111
	, ActionKey(InActionKey) // #111
	, EquipPoint(NAME_None)
	, RelativeRotation(FRotator::ZeroRotator) // #108
	, RelativeScale(1.0f) // #108
	, StanceName(NAME_None) // #111
	, bOverlapEvent(false) // #106
	, OwnerObjectPtr(InOwnerObject)
	, OverrideMaterialLoadState(ET4EquipLoadState::ELS_Ready)
	, CurrentOpacityValue(-1.0f)
#if (WITH_EDITOR || WITH_SERVER_CODE)
	, bOverlapEventEnabled(false)
	, OverlapEventName(NAME_None) // #49
#endif
{
}

FT4BaseEquipment::~FT4BaseEquipment()
{
	check(0 == OverrideMaterialLoaders.Num());
	check(!bAttached);
}

FT4BaseEquipment* FT4BaseEquipment::NewInstance(
	AT4GameObject* InOwnerObject,
	const FT4ActionKey& InActionKey,
	bool bInMainWeapon, // #111
	const FT4EntityKey& InEntityKey,
	const FName InOverrideEquipPoint,
	bool bInChangeStance, // #110
	bool bInUseAnimNotify // #111 : Stance 변경에 있는 AnimNotify_Equipment Mount 에 따라 Hide => Show 처리가 됨
)
{
	check(nullptr != InOwnerObject);
	switch (InEntityKey.Type)
	{
		case ET4EntityType::Weapon:
			{
				const UT4WeaponEntityAsset* WeaponEntityAsset = T4AssetEntityManagerGet()->GetWeaponEntity(
					InEntityKey
				);
				if (nullptr == WeaponEntityAsset)
				{
					T4_LOG(
						Error,
						TEXT("Weapon Entity '%s' not found"),
						*(InEntityKey.ToString())
					);
					return nullptr;
				}
				FT4BaseEquipment* EquipmentBase = nullptr;
				const FT4EntityItemWeaponMeshData& MeshData = WeaponEntityAsset->MeshData;
				switch (MeshData.MeshType)
				{
					case ET4EntityMeshType::StaticMesh:
						{
							FT4StaticMeshEquipment* NewEquipment = new FT4StaticMeshEquipment(InOwnerObject, InActionKey);
							check(nullptr != NewEquipment);
							EquipmentBase = static_cast<FT4BaseEquipment*>(NewEquipment);
						}
						break;

					case ET4EntityMeshType::SkeletalMesh:
						{
							FT4SkeletalMeshEquipment* NewEquipment = new FT4SkeletalMeshEquipment(InOwnerObject, InActionKey);
							check(nullptr != NewEquipment);
							EquipmentBase = static_cast<FT4BaseEquipment*>(NewEquipment);
						}
						break;

					case ET4EntityMeshType::ParticleSystem: // #110
						{
							FT4ParticleSystemEquipment* NewEquipment = new FT4ParticleSystemEquipment(InOwnerObject, InActionKey);
							check(nullptr != NewEquipment);
							EquipmentBase = static_cast<FT4BaseEquipment*>(NewEquipment);
						}
						break;

					default:
						{
							T4_LOG(
								Error,
								TEXT("Unknown Mesh type '%u'"),
								uint8(MeshData.MeshType)
							);
						}
						break;
				};
				if (nullptr != EquipmentBase)
				{
					EquipmentBase->OnAttach(
						bInMainWeapon,
						InEntityKey,
						InOverrideEquipPoint,
						bInChangeStance,
						bInUseAnimNotify
					);
					return EquipmentBase;
				}
			}
			break;

		default:
			{
				T4_LOG(
					Error,
					TEXT("Unknown Entity type '%u'"),
					uint8(InEntityKey.Type)
				);
			}
			break;
	};
	return nullptr;
}

void FT4BaseEquipment::OnReset()
{
	if (bAttached)
	{
		OnDetach(NAME_None, false);
	}
	Reset();
	ResetDynamicMaterialInstances(); // #78, #108
}

void FT4BaseEquipment::OnAttach(
	bool bInMainWeapon, // #111
	const FT4EntityKey& InEntityKey,
	FName InOverrideEquipPoint,
	bool bInChangeStance,
	bool bInUseAnimNotify // #111 : Stance 변경에 있는 AnimNotify_Equipment Mount 에 따라 Hide => Show 처리가 됨
)
{
	bMainWeapon = bInMainWeapon; // #111 : MainWeapon 만 Stance 를 변경하도록 처리
	AttackPrepare(InEntityKey, InOverrideEquipPoint);
	// Detach 와 같이 !bUseAnimNotify 일때 Attach 를 호출하지 않는 것인 비동기 로딩에서 IsAttachable 로 제어하기 때문
	if (bInUseAnimNotify)
	{
		bPendingAttach = true;
	}
	if (bInMainWeapon && bInChangeStance)
	{
		if (StanceName != NAME_None) // #110 : Weapon 의 Stance 설정을 사용할 경우...
		{
			ChangeStanceByWeapon(StanceName);
		}
	}
}

void FT4BaseEquipment::OnDetach(
	FName InRestoreStanceName, // #110
	bool bUseAnimNotify // #111 : Stance 변경에 있는 AnimNotify_Equipment Unmount 에 따라 Show => Hide 처리가 됨
) 
{
	if (bMainWeapon) // #111 : MainWeapon 만 Stance 를 변경하도록 처리
	{
		if (InRestoreStanceName != NAME_None) // #110: InRestoreStanceName 로 Stance 설정을 사용할 경우...
		{
			ChangeStanceByWeapon(InRestoreStanceName);
		}
	}
	if (bUseAnimNotify)
	{
		bPendingDetach = true;
	}
	else
	{
		Detach();
	}
	bDetachable = true; // 삭제 가능!
}

void FT4BaseEquipment::Attach() // #111
{
}

void FT4BaseEquipment::Detach()
{
	if (!bAttached)
	{
		return;
	}
	DetachPrepare();
	DetachFromParent(FDetachmentTransformRules::KeepRelativeTransform);
	ResePrimitiveComponent();
}

void FT4BaseEquipment::OnAdvance(const FT4UpdateTime& InUpdateTime)
{
	if (ET4EquipLoadState::ELS_Loading == OverrideMaterialLoadState)
	{
		for (FT4MaterialLoader& MaterialLoader : OverrideMaterialLoaders) // #80
		{
			if (MaterialLoader.IsLoadStarted() && !MaterialLoader.IsLoadCompleted())
			{
				return;
			}
		}
		OverrideMaterialLoadState = ET4EquipLoadState::ELS_Loaded;
	}
	Advance(InUpdateTime);
	AdvanceOpacity(InUpdateTime); // #108
}

void FT4BaseEquipment::AdvanceOpacity(const FT4UpdateTime& InUpdateTime) // #78, #108
{
	if (!OwnerObjectPtr.IsValid() || !OwnerObjectPtr->IsLoaded())
	{
		return;
	}
	float ParentOpaicty = OwnerObjectPtr->GetOpacity();
	if (ParentOpaicty == CurrentOpacityValue)
	{
		return;
	}
	CurrentOpacityValue = ParentOpaicty;
	UpdateMaterialOpacityParameter();
}

void FT4BaseEquipment::OnAnimNotify(const FT4AnimNotifyEquipment& InAnimNotify) // #111
{
	if (InAnimNotify.SameStanceName != NAME_None)
	{
		// #111 : 같은 Stance 를 사용할 경우만 처리...다른 무기에 대한 제어 방지
		if (StanceName != InAnimNotify.SameStanceName)
		{
#if 1
			T4_LOG(
				Verbose,
				TEXT("[%s] Skip Miss Matching (this: %s = AnimNotify: %s)"),
				* (ObjectPath.GetAssetName()),
				*(StanceName.ToString()),
				*(InAnimNotify.SameStanceName.ToString())
			);
#endif
			return;
		}
	}
#if WITH_EDITOR
#if 1
	T4_LOG(
		Verbose,
		TEXT("[%s] PendingAttch (%i), PendingDetach (%i), AnimNotify (%s), DebugStr (%s)"),
		*(ObjectPath.GetAssetName()),
		bPendingAttach,
		bPendingDetach,
		(ET4EquipmentType::Mount == InAnimNotify.EquipmentType) ? TEXT("Mount") : TEXT("Unmount"),
		*InAnimNotify.DebugSting
	);
#endif
#endif
	if (bPendingAttach)
	{
		if (ET4EquipmentType::Unmount == InAnimNotify.EquipmentType)
		{
			bPendingAttach = false; // 비동기 로딩을 완료하고 Attach 해준다!
		}
	}
	else if (bPendingDetach)
	{
		if (ET4EquipmentType::Mount == InAnimNotify.EquipmentType)
		{
			Detach(); // 마운트 타이밍에 Detach 해준다.
			bPendingDetach = false; // 이제 삭제될 준비가 되었다.
		}
	}
	AnimNotify(InAnimNotify);
}

void FT4BaseEquipment::UpdateMaterialOpacityParameter() // #108
{
	SetDynamicMaterialInstanceParameter(
		T4EngineConstant::GetMaterialParameterOpacityName(),
		(-1.0f == CurrentOpacityValue) ? 0.0f : CurrentOpacityValue
	);
}

void FT4BaseEquipment::OnStartLoading()
{
	OverrideMaterialLoadState = ET4EquipLoadState::ELS_Ready;
	for (const FSoftObjectPath& OverrideMaterialPath : OverrideMaterialPaths) // #80
	{
		if (!OverrideMaterialPath.IsNull())
		{
			FT4MaterialLoader& MaterialLoader = OverrideMaterialLoaders.AddDefaulted_GetRef();
			MaterialLoader.Load(OverrideMaterialPath, false, TEXT("Equipment_OverrideMaterial"));
			OverrideMaterialLoadState = ET4EquipLoadState::ELS_Loading;
		}
	}
	if (ET4EquipLoadState::ELS_Ready == OverrideMaterialLoadState)
	{
		OverrideMaterialLoadState = ET4EquipLoadState::ELS_NotSet;
	}
	StartLoading();
}

#if (WITH_EDITOR || WITH_SERVER_CODE)
void FT4BaseEquipment::OnBeginOverlapEvents(const FName& InOverlapEventName) // #49
{
	if (PrimitiveComponentPtr.IsValid())
	{
		PrimitiveComponentPtr->SetGenerateOverlapEvents(true);
	}
	bOverlapEventEnabled = true;
	OverlapEventName = InOverlapEventName;
}

void FT4BaseEquipment::OnEndOverlapEvents() // #49
{
	if (PrimitiveComponentPtr.IsValid())
	{
		PrimitiveComponentPtr->SetGenerateOverlapEvents(false);
	}
	bOverlapEventEnabled = false;
	OverlapEventName = NAME_None;
}
#endif

const FSoftObjectPath FT4BaseEquipment::GetObjectPath() const
{
	if (!EntityKey.IsValid())
	{
		return nullptr;
	}
	return FSoftObjectPath(EntityKey.Value.ToString());
}

bool FT4BaseEquipment::IsParentLoaded() const
{
	const bool bParentLoaded = (OwnerObjectPtr.IsValid() && OwnerObjectPtr->IsLoaded()) ? true : false; // #108
	return bParentLoaded;
}

void FT4BaseEquipment::SePrimitiveComponent(UPrimitiveComponent* InPrimitiveComponent)
{
	check(nullptr != InPrimitiveComponent);
	PrimitiveComponentPtr = InPrimitiveComponent;
	if (bOverlapEvent) // #106
	{
		// #49 : 테스트로 서버의 Hit 판정을 Overlap 을 통해 처리해본다.
#if (WITH_EDITOR || WITH_SERVER_CODE)
		PrimitiveComponentPtr->SetGenerateOverlapEvents(false); // #49 : 옵션으로 켠다. Begin/End WeaponHitOverlapEvent
		PrimitiveComponentPtr->SetCollisionObjectType(T4COLLISION_WEAPON);
		PrimitiveComponentPtr->SetCollisionProfileName(TEXT("T4HitOverlapOnlyWeapon"));
		PrimitiveComponentPtr->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		PrimitiveComponentPtr->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		PrimitiveComponentPtr->OnComponentBeginOverlap.AddUniqueDynamic(
			OwnerObjectPtr.Get(),
			&AT4GameObject::HandleOnHitOverlapOnlyServer
		);
#else
		PrimitiveComponentPtr->SetGenerateOverlapEvents(false);
		PrimitiveComponentPtr->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		PrimitiveComponentPtr->SetCollisionObjectType(ECC_WorldDynamic);
		PrimitiveComponentPtr->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PrimitiveComponentPtr->SetCollisionResponseToAllChannels(ECR_Ignore);
		PrimitiveComponentPtr->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		PrimitiveComponentPtr->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
#endif
	}
	else
	{
		PrimitiveComponentPtr->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void FT4BaseEquipment::ResePrimitiveComponent()
{
	if (PrimitiveComponentPtr.IsValid())
	{
#if (WITH_EDITOR || WITH_SERVER_CODE)
		PrimitiveComponentPtr->OnComponentBeginOverlap.RemoveDynamic(
			OwnerObjectPtr.Get(),
			&AT4GameObject::HandleOnHitOverlapOnlyServer
		);
#endif
		PrimitiveComponentPtr.Reset();
	}
}

USceneComponent* FT4BaseEquipment::TryGetAttachComponent(
	USceneComponent* InParentComponent
) // #108
{
	if (nullptr == InParentComponent)
	{
		return nullptr;
	}
	if (InParentComponent->DoesSocketExist(EquipPoint))
	{
		return InParentComponent;
	}
	const TArray<USceneComponent*>& AttachChildren = InParentComponent->GetAttachChildren();
	for (USceneComponent* ChildComponent : AttachChildren)
	{
		USceneComponent* AttachChildComponent = TryGetAttachComponent(ChildComponent);
		if (nullptr != AttachChildComponent)
		{
			return AttachChildComponent;
		}
	}
	return nullptr;
}

bool FT4BaseEquipment::AttachToParent(FAttachmentTransformRules InAttachTransformRule)
{
	check(!bAttached);
	check(PrimitiveComponentPtr.IsValid());
	USceneComponent* AttachComponent = TryGetAttachComponent(OwnerObjectPtr->GetAttachParentComponent());
	if (nullptr == AttachComponent)
	{
		return false;
	}
	PrimitiveComponentPtr->AttachToComponent(AttachComponent, InAttachTransformRule, EquipPoint);
	PrimitiveComponentPtr->RegisterComponent();
	Attach(); // #111
	bAttached = true;
	return true;
}

void FT4BaseEquipment::DetachFromParent(FDetachmentTransformRules InDetachTransformRule)
{
	check(bAttached);
	check(PrimitiveComponentPtr.IsValid());
	PrimitiveComponentPtr->UnregisterComponent();
	PrimitiveComponentPtr->DetachFromComponent(InDetachTransformRule);
	bAttached = false;
}

void FT4BaseEquipment::ClearOverrideMaterialLoader() // #80
{
	if (ET4EquipLoadState::ELS_Loaded != OverrideMaterialLoadState)
	{
		return;
	}
	for (FT4MaterialLoader& MaterialLoader : OverrideMaterialLoaders) // #80
	{
		MaterialLoader.Reset();
	}
	OverrideMaterialLoaders.Empty();
	OverrideMaterialPaths.Empty();
	OverrideMaterialLoadState = ET4EquipLoadState::ELS_Completed;
}

void FT4BaseEquipment::SetOverrideMaterials(UMeshComponent* InMeshComponent) // #80
{
	check(nullptr != InMeshComponent);
	for (FT4MaterialLoader& MaterialLoader : OverrideMaterialLoaders) // #80
	{
		UMaterialInterface* MaterialInterface = nullptr;
		if (MaterialLoader.IsLoadStarted())
		{
			check(MaterialLoader.IsLoadCompleted());
			MaterialInterface = MaterialLoader.GetMaterialInterface();
			check(nullptr != MaterialInterface);
		}
		InMeshComponent->OverrideMaterials.Add(MaterialInterface);
		MaterialLoader.SetBinded();
	}
}

void FT4BaseEquipment::AddDynamicMaterialInstances(UMeshComponent* InMeshComponent) // #78
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

void FT4BaseEquipment::ResetDynamicMaterialInstances() // #78
{
	//MaterialDynamicInstanceMap.Empty();
	MaterialDynamicInstances.Empty();
}

void FT4BaseEquipment::SetDynamicMaterialInstanceParameter(FName InParameterName, float InValue) // #78
{
	for (UMaterialInstanceDynamic* DynamicInstance : MaterialDynamicInstances)
	{
		check(nullptr != DynamicInstance);
		DynamicInstance->SetScalarParameterValue(InParameterName, InValue);
	}
}

void FT4BaseEquipment::ChangeStanceByWeapon(FName InStanceName) // #110
{
	if (!OwnerObjectPtr.IsValid())
	{
		return;
	}
	FT4StanceAction NewSubAction;
	NewSubAction.StanceName = InStanceName;
	NewSubAction.bTransient = true;
	OwnerObjectPtr->OnExecuteAction(&NewSubAction, nullptr);
}