// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4SkeletalMeshEquipment.h"

#include "Object/Component/T4SkeletalMeshComponent.h"

#include "Object/Animation/AnimInstance/T4BaseAnimVariables.h"
#include "Object/Animation/AnimInstance/T4BaseAnimInstance.h"

#include "Object/T4GameObject.h"

#include "T4Asset/Classes/Entity/T4WeaponEntityAsset.h" // #41
#include "T4Asset/Public/Entity/T4Entity.h"

#include "Animation/AnimMontage.h" // #106

#include "T4EngineInternal.h"

/**
  * #68, #107
 */
FT4SkeletalMeshEquipment::FT4SkeletalMeshEquipment(
	AT4GameObject* InOwnerObject,
	const FT4ActionKey& InActionKey
)
	: FT4BaseEquipment(InOwnerObject, InActionKey)
	, LoadState(ET4EquipLoadState::ELS_Ready)
	, bUseAnimation(false)
{
}

FT4SkeletalMeshEquipment::~FT4SkeletalMeshEquipment()
{
}

void FT4SkeletalMeshEquipment::AttackPrepare(
	const FT4EntityKey& InEntityKey,
	FName InOverrideEquipPoint
)
{
	if (bAttached)
	{
		return;
	}
	WeaponEntityAsset = T4AssetEntityManagerGet()->GetWeaponEntity(
		InEntityKey
	);
	if (nullptr == WeaponEntityAsset)
	{
		T4_LOG(
			Error,
			TEXT("Weapon Entity '%s' not found"),
			*(InEntityKey.ToString())
		);
		return;
	}
	const FT4EntityItemWeaponMeshData& MeshData = WeaponEntityAsset->MeshData;
	check(ET4EntityMeshType::SkeletalMesh == MeshData.MeshType);
	EquipPoint = (InOverrideEquipPoint != NAME_None) ? InOverrideEquipPoint : MeshData.EquipPointName; // #106;
	RelativeRotation = MeshData.RelativeRotation; // #108
	RelativeScale = MeshData.RelativeScale; // #108
	StanceName = MeshData.StanceName; // #111
	bOverlapEvent = MeshData.bOverlapEvent; // #106
	EntityKey = InEntityKey;
	ObjectPath = MeshData.SkeletalMeshAsset.ToSoftObjectPath();
	const FT4EntityItemAnimationData& AnimationData = WeaponEntityAsset->AnimationData;
	bUseAnimation = AnimationData.bUseAnimation;
	if (bUseAnimation)
	{
		if (AnimationData.AnimBlueprintAsset.IsNull() || AnimationData.AnimMontageAsset.IsNull())
		{
			bUseAnimation = false;
		}
	}
	OnStartLoading();
}

void FT4SkeletalMeshEquipment::DetachPrepare()
{
}

void FT4SkeletalMeshEquipment::Reset()
{
	if (bUseAnimation)
	{
		if (!AnimBPClassLoader.CheckReset())
		{
			AnimBPClassLoader.Reset();
		}
		if (!BlendSpaceLoader.CheckReset())
		{
			BlendSpaceLoader.Reset();
		}
		if (!AnimMontageLoader.CheckReset())
		{
			AnimMontageLoader.Reset();
		}
	}
	if (!MeshLoader.CheckReset())
	{
		MeshLoader.Reset();
	}
	ComponentPtr.Reset();
}

void FT4SkeletalMeshEquipment::Advance(const FT4UpdateTime& InUpdateTime)
{
	if (!IsParentLoaded())
	{
		return;
	}
	switch (LoadState)
	{
		case ET4EquipLoadState::ELS_Loading:
			AdvanceLoading(InUpdateTime);
			break;
		
		case ET4EquipLoadState::ELS_Loaded:
			AdvanceLoadComplete(InUpdateTime);
			break;

		case ET4EquipLoadState::ELS_TryAttach:
			AdvanceTryAttach(InUpdateTime);
			break;

		case ET4EquipLoadState::ELS_Completed:
			{
				UT4BaseAnimInstance* AnimInstance = GetAnimInstance(); // #107
				if (nullptr != AnimInstance)
				{
					FT4StateAnimVariables* StateAnimVariables = AnimInstance->GetStateAnimVariables();
					if (nullptr != StateAnimVariables)
					{
						if (OwnerObjectPtr.IsValid())
						{
							const FName SubStanceName = OwnerObjectPtr->GetSubStanceName();
							StateAnimVariables->bIsCombat = (T4Const_CombatSubStanceName == SubStanceName) ? true : false;
						}
					}
					AnimInstance->SetPause(InUpdateTime.bPaused); // #102
					AnimInstance->SetTimeScale(InUpdateTime.TimeScale); // #102
				}
			}
			break;
	};
}

void FT4SkeletalMeshEquipment::AdvanceLoading(const FT4UpdateTime& InUpdateTime)
{
	check(ET4EquipLoadState::ELS_Loading == LoadState);

	if (bUseAnimation) // 애니메이션부터 체크!!
	{
		if (AnimBPClassLoader.IsLoadFailed()) // Failed 먼저 체크!!
		{
			AnimBPClassLoader.Reset();
			bUseAnimation = false;
		}
		else if (!AnimBPClassLoader.IsLoadCompleted())
		{
			return;
		}
		if (BlendSpaceLoader.IsLoadStarted())
		{
			if (BlendSpaceLoader.IsLoadFailed()) // Failed 먼저 체크!!
			{
				BlendSpaceLoader.Reset();
				bUseAnimation = false;
			}
			else if (!BlendSpaceLoader.IsLoadCompleted())
			{
				return;
			}
		}
		if (AnimMontageLoader.IsLoadStarted())
		{
			if (AnimMontageLoader.IsLoadFailed()) // Failed 먼저 체크!!
			{
				AnimMontageLoader.Reset();
				bUseAnimation = false;
			}
			else if (!AnimMontageLoader.IsLoadCompleted())
			{
				return;
			}
		}
	}
	if (MeshLoader.IsLoadFailed()) // Failed 먼저 체크!!
	{
		LoadState = ET4EquipLoadState::ELS_Failed;
	}
	else if (MeshLoader.IsLoadCompleted())
	{
		LoadState = ET4EquipLoadState::ELS_Loaded;
	}
}

void FT4SkeletalMeshEquipment::AdvanceLoadComplete(const FT4UpdateTime& InUpdateTime)
{
	check(ET4EquipLoadState::ELS_Loaded == LoadState);

	if (!IsAttachable())
	{
		return; // #111 : Pending 상태면 Ttach 를 기다린다.
	}

	check(OwnerObjectPtr.IsValid());
	check(!ComponentPtr.IsValid());
	ComponentPtr = NewComponentTemplate<UT4SkeletalMeshComponent>(OwnerObjectPtr.Get(), true);
	ComponentPtr->SetRelativeRotation(RelativeRotation); // #108
	ComponentPtr->SetRelativeScale3D(FVector(RelativeScale)); // #108
	SePrimitiveComponent(Cast<UPrimitiveComponent>(ComponentPtr.Get()));
	SetOverrideMaterials(Cast<UMeshComponent>(ComponentPtr.Get())); // #80
	{
		MeshLoader.Process(ComponentPtr.Get());
	}
	AddDynamicMaterialInstances(ComponentPtr.Get()); // #108
	UpdateMaterialOpacityParameter(); // #108

	LoadState = ET4EquipLoadState::ELS_TryAttach;
}

void FT4SkeletalMeshEquipment::AdvanceTryAttach(const FT4UpdateTime& InUpdateTime)
{
	check(ET4EquipLoadState::ELS_TryAttach == LoadState);

	if (!IsAttachable())
	{
		return; // #111 : Pending 상태면 Ttach 를 기다린다.
	}

	// #108 : EquipPoint 를 찾을때까지 재귀 처리 (Bow => Arrow)
	if (!AttachToParent(FAttachmentTransformRules::KeepRelativeTransform))
	{
		T4_LOG(
			Verbose,
			TEXT("Try Attach Equipment Failed")
		);
		return;
	}

	{
		if (bUseAnimation)
		{
			check(AnimBPClassLoader.IsLoadCompleted());
			if (AnimBPClassLoader.Process(ComponentPtr.Get()))
			{
				UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
				if (nullptr != AnimInstance)
				{
					if (BlendSpaceLoader.IsLoadStarted())
					{
						check(BlendSpaceLoader.IsLoadCompleted());
						UBlendSpaceBase* BlendSpace = BlendSpaceLoader.GetBlendSpace();
						AnimInstance->AddBlendSpace(T4Const_ItemBlendSpaceName, BlendSpace);
					}
					if (AnimMontageLoader.IsLoadStarted())
					{
						check(AnimMontageLoader.IsLoadCompleted());
						UAnimMontage* AnimMotage = AnimMontageLoader.GetAnimMontage();
						AnimInstance->AddAnimMontage(T4Const_DefaultAnimMontageName, AnimMotage);
					}
				}
			}
		}

		ClearOverrideMaterialLoader();
		AnimMontageLoader.SetBinded();
		BlendSpaceLoader.SetBinded();
		AnimBPClassLoader.SetBinded();
		MeshLoader.SetBinded();
	}

	LoadState = ET4EquipLoadState::ELS_Completed;
}

void FT4SkeletalMeshEquipment::StartLoading()
{
	LoadState = ET4EquipLoadState::ELS_Ready;
	if (ObjectPath.IsNull())
	{
		LoadState = ET4EquipLoadState::ELS_NotSet;
		return;
	}
	if (bUseAnimation)
	{
		const FT4EntityItemAnimationData& AnimationData = WeaponEntityAsset->AnimationData;
		if (!AnimationData.AnimBlueprintAsset.IsNull())
		{
			FString AnimBPClassPath = AnimationData.AnimBlueprintAsset.ToString() + TEXT("_C"); // add prerix '_C'
			AnimBPClassLoader.Load(
				AnimBPClassPath,
				false,
				TEXT("Equipment_SkeletalMesh_AnimBlueprint")
			);
			if (!AnimationData.BlendSpaceAsset.IsNull())
			{
				BlendSpaceLoader.Load(
					AnimationData.BlendSpaceAsset.ToSoftObjectPath(),
					false,
					TEXT("Equipment_SkeletalMesh_BlendSpace")
				);
			}
			if (!AnimationData.AnimMontageAsset.IsNull())
			{
				AnimMontageLoader.Load(
					AnimationData.AnimMontageAsset.ToSoftObjectPath(),
					false,
					TEXT("Equipment_SkeletalMesh_AnimMontage")
				);
			}
		}
		else
		{
			bUseAnimation = false;
		}
	}
	MeshLoader.Load(ObjectPath, false, TEXT("Equipment_SkeletalMesh"));
	LoadState = ET4EquipLoadState::ELS_Loading;
}

UT4BaseAnimInstance* FT4SkeletalMeshEquipment::GetAnimInstance() const
{
	if (!ComponentPtr.IsValid())
	{
		return nullptr;
	}
	UT4BaseAnimInstance* AnimInstance = Cast<UT4BaseAnimInstance>(
		ComponentPtr->GetAnimInstance()
	);
	if (nullptr == AnimInstance)
	{
		return nullptr;
	}
	return AnimInstance;
}

bool FT4SkeletalMeshEquipment::PlayAnimation(const FT4AnimParameters& InAnimParameters) // #107
{
	UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
	if (nullptr == AnimInstance)
	{
		return false;
	}
	if (!AnimInstance->HasSection(T4Const_DefaultAnimMontageName, InAnimParameters.SectionName))
	{
		return false;
	}
	FT4AnimParameters ItemAnimParameters = InAnimParameters;
	ItemAnimParameters.AnimMontageName = T4Const_DefaultAnimMontageName;
	ItemAnimParameters.BlendInTimeSec = 0.0f;
	ItemAnimParameters.BlendOutTimeSec = 0.1f;
	FT4AnimInstanceID AnimInstanceID = AnimInstance->PlayAnimation(ItemAnimParameters);
	return (INDEX_NONE != AnimInstanceID) ? true : false;
}