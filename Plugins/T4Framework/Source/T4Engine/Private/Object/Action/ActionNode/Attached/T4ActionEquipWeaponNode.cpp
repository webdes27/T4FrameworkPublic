// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionEquipWeaponNode.h"

#include "Object/Action/T4ActionControl.h"
#include "Object/T4GameObject.h"

#include "T4Asset/Classes/Entity/T4WeaponEntityAsset.h"
#include "T4Asset/Public/Entity/T4Entity.h"

#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"

#include "Engine/CollisionProfile.h" // #49

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionEquipWeaponNode::FT4ActionEquipWeaponNode(FT4ActionControl* InControl, const FT4ActionKey& InKey)
	: FT4ActionAttachedNode(InControl, InKey)
{
}

FT4ActionEquipWeaponNode::~FT4ActionEquipWeaponNode()
{
	check(0 == OverrideMaterialLoaders.Num());
	check(MeshLoader.CheckReset());
}

FT4ActionEquipWeaponNode* FT4ActionEquipWeaponNode::CreateNode(
	FT4ActionControl* InControl,
	const FT4EquipWeaponAction& InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	check(ET4ActionType::EquipWeapon == InAction.ActionType);
	FT4ActionEquipWeaponNode* NewNode = new FT4ActionEquipWeaponNode(InControl, InAction.ActionKey);
	check(nullptr != NewNode);
	check(NewNode->GetType() == InAction.ActionType);
	// switch
	return NewNode;
}

bool FT4ActionEquipWeaponNode::Create(const FT4ActionStruct* InAction)
{
	check(ET4ActionType::EquipWeapon == InAction->ActionType);
	const FT4EquipWeaponAction& ConvAction = *(static_cast<const FT4EquipWeaponAction*>(InAction));
	const UT4WeaponEntityAsset* EntityAsset = T4AssetEntityManagerGet()->GetWeaponEntity(
		ConvAction.WeaponEntityAsset.ToSoftObjectPath()
	);

	if (nullptr == EntityAsset)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4ActionEquipWeaponNode : Item Entity (%s) Not Found."),
			*(ConvAction.WeaponEntityAsset.ToString())
		);
		return false;
	}

	// #68 : 리로드 또는 playback 을 위해 장착 정보를 보관한다. 단, 현재는 Character 만 사용
	AT4GameObject* ParentGameObject = GetGameObject();
	check(nullptr != ParentGameObject);
	ParentGameObject->AddEquipmentInfo(
		ConvAction.ActionKey,
		ConvAction.EquipPoint,
		FT4EntityKey(EntityAsset->GetEntityType(), EntityAsset->GetEntityKeyPath())
	);

	const FT4EntityItemWeaponMeshData& ItemMesh = EntityAsset->MeshData;
	check(ET4EntityMeshType::StaticMesh == ItemMesh.MeshType); // TODO : SkeletalMesh

	SetAttachInfo(
		ET4AttachParent::Object, // #54 : 무기는 자신에게만...
		false, // #76 : Parent ActionPoint 가 없다면 본래 세팅을 따르도록...
		ConvAction.EquipPoint,  // #57 : BoneOrSocketName
		ItemMesh.StaticMeshAsset.ToSoftObjectPath(),
		ConvAction.LoadingPolicy
	);

	// #80
	const FT4EntityOverrideMaterialData& OverrideMaterialData = ItemMesh.StaticMeshOverrideMaterialData;
	for (FName SlotName : OverrideMaterialData.MaterialSortedSlotNames)
	{
		FSoftObjectPath MaterialAssetPath;
		if (OverrideMaterialData.MaterialMap.Contains(SlotName))
		{
			MaterialAssetPath = OverrideMaterialData.MaterialMap[SlotName].ToSoftObjectPath();
		}
		OverrideMaterialPaths.Add(MaterialAssetPath);
	}

	if (ET4LoadingPolicy::Async == LoadingPolicy)
	{
		OnStartLoading();
	}
#if WITH_EDITOR
	ActionCached = ConvAction;
#endif
	return true;
}

void FT4ActionEquipWeaponNode::ClearAsyncLoader() // #80
{
	for (FT4MaterialLoader& MaterialLoader : OverrideMaterialLoaders) // #80
	{
		MaterialLoader.Reset();
	}
	OverrideMaterialLoaders.Empty();
	OverrideMaterialPaths.Empty();
	MeshLoader.Reset();
}

void FT4ActionEquipWeaponNode::Destroy()
{
	ensure(!StaticMeshComponent.IsValid());
	ClearAsyncLoader(); // #80

	// #68 : 리로드 또는 playback 을 위해 장착 정보를 보관한다. 단, 현재는 Character 만 사용
	AT4GameObject* ParentGameObject = GetGameObject();
	check(nullptr != ParentGameObject);
	ParentGameObject->RemoveEquipmentInfo(ActionKey);
}

void FT4ActionEquipWeaponNode::AdvanceLoading(const FT4UpdateTime& InUpdateTime)
{
	for (FT4MaterialLoader& MaterialLoader : OverrideMaterialLoaders) // #80
	{
		if (MaterialLoader.IsLoadStarted() && !MaterialLoader.IsLoadCompleted())
		{
			return;
		}
	}
	if (MeshLoader.IsLoadFailed()) // Failed 먼저 체크!!
	{
		SetLoadState(ALS_Failed);
	}
	else if (MeshLoader.IsLoadCompleted())
	{
		SetLoadState(ALS_Completed);
	}
}

void FT4ActionEquipWeaponNode::Advance(const FT4UpdateTime& InUpdateTime)
{
	if (CheckPlayState(APS_Ready))
	{
		// #56
		check(ET4LoadingPolicy::Async == LoadingPolicy);
		AddOffsetTimeSec(InUpdateTime.ScaledTimeSec); // #54 : Case-2
		if (CheckLoadState(ALS_Failed))
		{
			SetPlayState(APS_Stopped);
		}
		else if (CheckLoadState(ALS_Completed))
		{
			PlayInternal(GetOffsetTimeSec());
		}
	}
}

bool FT4ActionEquipWeaponNode::Play()
{
	check(!MeshLoader.IsBinded());
	CreateComponent();
	OnAttachToParent(Cast<USceneComponent>(StaticMeshComponent.Get()), true);
	if (ET4LoadingPolicy::Sync == LoadingPolicy)
	{
		// #8, #56 : 사용 제한 필요!!! 만약을 대비해 준비는 해둔 것!
		UStaticMesh* StaticMesh = Cast<UStaticMesh>(AssetPath.TryLoad());
		if (nullptr != StaticMesh)
		{
			StaticMeshComponent->SetStaticMesh(StaticMesh);
		}
	}
	else if (ET4LoadingPolicy::Async == LoadingPolicy)
	{
		if (CheckLoadState(ALS_Completed))
		{
			PlayInternal(GetOffsetTimeSec());
		}
		else if (CheckLoadState(ALS_Failed))
		{
			UE_LOG(
				LogT4Engine,
				Error,
				TEXT("FT4ActionEquipWeaponNode::Play Failed to load StaticMesh '%s'."),
				*(AssetPath.ToString())
			);
			OnStop();
			return false;
		}
		else
		{
			// #54 : 리소스가 로드가 안되어 Offset Time 적용이 필요할 경우,
			//       PlayState 를 APS_Ready 로 바꾼 후 로딩 완료 후 OffsetTimeSec 로 플레이 되도록 처리
			SetPlayState(APS_Ready);
			return false;
		}
	}
	return true;
}

void FT4ActionEquipWeaponNode::Stop()
{
	ClearAsyncLoader(); // #80
	if (StaticMeshComponent.IsValid()) // #49
	{
#if (WITH_EDITOR || WITH_SERVER_CODE)
		AT4GameObject* OwnerGameObject = GetGameObject();
		check(nullptr != OwnerGameObject);
		StaticMeshComponent->OnComponentBeginOverlap.RemoveDynamic(
			OwnerGameObject,
			&AT4GameObject::HandleOnHitOverlapOnlyServer
		);
#endif
		OnDetachFromParent(Cast<USceneComponent>(StaticMeshComponent.Get()), true);
		StaticMeshComponent->DestroyComponent();
		StaticMeshComponent.Reset();
	}
}

void FT4ActionEquipWeaponNode::StartLoading()
{
	for (const FSoftObjectPath& ObjectPath : OverrideMaterialPaths) // #80
	{
		FT4MaterialLoader& MaterialLoader = OverrideMaterialLoaders.AddDefaulted_GetRef();
		if (!ObjectPath.IsNull())
		{
			MaterialLoader.Load(ObjectPath, false, TEXT("EquipWeaponItem_OverrideMaterial"));
		}
	}
	MeshLoader.Load(AssetPath, false, TEXT("EquipWeaponItem"));
}

bool FT4ActionEquipWeaponNode::IsAutoFinished() const
{
	check(IsPlayed());
	check(ET4LifecycleType::Auto == LifecycleType);
	// 외부 Stop 이 아니면 종료되지 않도록 처리
	return false;
}

void FT4ActionEquipWeaponNode::CreateComponent()
{
	check(!StaticMeshComponent.IsValid());
	StaticMeshComponent = NewComponentTemplate<UT4StaticMeshComponent>(true);
	{
		// #49 : 테스트로 서버의 Hit 판정을 Overlap 을 통해 처리해본다.
#if (WITH_EDITOR || WITH_SERVER_CODE)
		StaticMeshComponent->SetGenerateOverlapEvents(false); // #49 : 옵션으로 켠다. Begin/End WeaponHitOverlapEvent
		StaticMeshComponent->SetCollisionObjectType(T4COLLISION_WEAPON);
		StaticMeshComponent->SetCollisionProfileName(TEXT("T4HitOverlapOnlyWeapon"));
		StaticMeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		StaticMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

		AT4GameObject* OwnerGameObject = GetGameObject();
		check(nullptr != OwnerGameObject);
		StaticMeshComponent->OnComponentBeginOverlap.AddUniqueDynamic(
			OwnerGameObject,
			&AT4GameObject::HandleOnHitOverlapOnlyServer
		);
#else
		StaticMeshComponent->SetGenerateOverlapEvents(false);
		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		StaticMeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		StaticMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		StaticMeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		StaticMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
#endif
	}
}

bool FT4ActionEquipWeaponNode::PlayInternal(float InOffsetTimeSec) // #56
{
	check(ET4LoadingPolicy::Async == LoadingPolicy);
	check(!MeshLoader.IsBinded());
	check(MeshLoader.IsLoadCompleted());
	check(StaticMeshComponent.IsValid());
	check(nullptr != StaticMeshComponent);
	for (FT4MaterialLoader& MaterialLoader : OverrideMaterialLoaders) // #80
	{
		UMaterialInterface* MaterialInterface = nullptr;
		if (MaterialLoader.IsLoadStarted())
		{
			check(MaterialLoader.IsLoadCompleted());
			MaterialInterface = MaterialLoader.GetMaterialInterface();
			check(nullptr != MaterialInterface);
		}
		StaticMeshComponent->OverrideMaterials.Add(MaterialInterface);
		MaterialLoader.SetBinded();
	}
	MeshLoader.Process(StaticMeshComponent.Get());
	MeshLoader.SetBinded();
	ClearAsyncLoader(); // #80
	SetPlayState(APS_Playing);
	return true;
}
