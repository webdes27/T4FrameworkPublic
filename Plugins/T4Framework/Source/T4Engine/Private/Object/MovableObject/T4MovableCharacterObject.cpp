// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4MovableCharacterObject.h"

#include "Object/Animation/T4BaseAnimControl.h" // #14

#include "Object/Component/T4CapsuleComponent.h"
#include "Object/Component/T4SkinnedMeshComponent.h" // #37
#include "Object/Component/T4SkeletalMeshComponent.h"
#include "Object/Component/T4MovementComponent.h"
#include "Object/Component/Movement/T4MovementUtils.h"

#include "Object/Animation/AnimInstance/T4BaseAnimInstance.h"

#include "Asset/Loader/T4AnimationDataLoader.h"

#include "Public/T4EngineAnimNotify.h" // #111
#include "Public/Action/T4ActionParameters.h" // #113
#include "Public/Action/T4ActionWorldCommands.h"

#include "T4Asset/Classes/Entity/T4WeaponEntityAsset.h" // #37
#include "T4Asset/Classes/Entity/T4CostumeEntityAsset.h" // #37
#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h"
#include "T4Asset/Public/Entity/T4Entity.h"

#include "Engine/CollisionProfile.h" // #49

#if !UE_BUILD_SHIPPING
#include "DrawDebugHelpers.h"
#endif

#include "T4EngineInternal.h"

#define USES_ACCELERATION_MOVE_ISSUE_52 1 // #52 : Player 위치 동기화 문제 체크!

static const FT4ActionKey T4CharacterStancePlayTagKey(TEXT("T4CharacterStancePlayTagKey")); // #74
static const FT4ActionKey T4ActionOverrideRotationPrimaryKey(TEXT("T4ActionOverrideRotationPrimaryKey"), true, true);

// CVars
namespace T4MovableCharacterObjectCVars
{
#if !UE_BUILD_SHIPPING

	static int32 DebugCharCapsule = 0;
	FAutoConsoleVariableRef CVarGameObjectCapsuleVisible(
		TEXT("t4.Debug.Obj.Capsule"),
		DebugCharCapsule,
		TEXT("0: Hide, 1: Show"),
		ECVF_Default
	);

	// #52
	static int32 DebugServerLocation = 0;
	FAutoConsoleVariableRef CVarGameObjectServerLocationVisible(
		TEXT("t4.Debug.Obj.SrvLoc"),
		DebugServerLocation,
		TEXT("0: Hide, 1: Show"),
		ECVF_Default
	);

#endif // !UE_BUILD_SHIPPING
}

/**
  *
 */
AT4MovableCharacterObject::AT4MovableCharacterObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, AnimationDataLoader(nullptr) // #39
	, bOverrideMaterialLoading(false) // #80
	, bMeshAsyncLoading(false) // #72
	, MeshType(ET4EntityCharacterMeshType::None) // #72
	, ModularType(ET4EntityCharacterModularType::None) // #72
	, MasterPosePartName(NAME_None) // #72
	, bPhysicsAssetAsyncLoading(false) // #76
	, bStancePreloading(false) // #111
	, StanceNamePreload(NAME_None) // #111
	, RealStanceNamePreload(NAME_None) // #111
	, ActivePlayTagByStance(NAME_None) // #111
	, bStanceAsyncLoading(false) // #73
	, StanceNameSelected(NAME_None) // #73
	, SubStanceNameSelected(NAME_None) // #106
	, AnimControl(nullptr)
	, bPhysicsEnabled(false) // #76
	, bPhysicsSimulateBodiesBelow(false) // #76
	, PhysicsRestoreTM(FTransform::Identity) // #76
#if WITH_EDITOR
	, SyncLocationForServer(FVector::ZeroVector) // #52
	, SyncDirectionForServer(FVector::ZeroVector) // #52
#endif
{
	CapsuleComponent = CreateDefaultSubobject<UT4CapsuleComponent>(TEXT("T4CapsuleComponent"));
	CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	CapsuleComponent->SetShouldUpdatePhysicsVolume(true);
	CapsuleComponent->SetCanEverAffectNavigation(false);
	//CapsuleComponent->SetCollisionObjectType(ECollisionChannel::ECC_Pawn); // #112
	//CapsuleComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore); // #112
	CapsuleComponent->bDynamicObstacle = true;

	RootComponent = CapsuleComponent;

	MovementComponent = CreateDefaultSubobject<UT4MovementComponent>(TEXT("T4MovementComponent"));
	if (nullptr != MovementComponent)
	{
		MovementComponent->UpdatedComponent = CapsuleComponent;
	}

	SkeletalMeshComponent = CreateOptionalDefaultSubobject<UT4SkeletalMeshComponent>(TEXT("T4SkeletalMeshComponent"));
	check(nullptr != SkeletalMeshComponent);
	{
		SkeletalMeshComponent->AlwaysLoadOnClient = true;
		SkeletalMeshComponent->AlwaysLoadOnServer = true;
		SkeletalMeshComponent->bOwnerNoSee = false;
		SkeletalMeshComponent->bCastDynamicShadow = true;
		SkeletalMeshComponent->bAffectDynamicIndirectLighting = true;
		SkeletalMeshComponent->PrimaryComponentTick.TickGroup = TG_PrePhysics;
		SkeletalMeshComponent->SetupAttachment(CapsuleComponent);
		SkeletalMeshComponent->SetCanEverAffectNavigation(false);
		
		SkeletalMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic); // #112
		SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		// #49 : 테스트로 서버의 Hit 판정을 Overlap 을 통해 처리해본다.
#if (WITH_EDITOR || WITH_SERVER_CODE)
		SkeletalMeshComponent->SetGenerateOverlapEvents(true);
		
		// #49 : SkeletalMesh 에 대해 서버애서 OverlapEvent 가 동작하기 위해서 아래의 옵션 적용. 렌더링과 관계없이 업데이트한다.
		SkeletalMeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones; 
#else
		SkeletalMeshComponent->SetGenerateOverlapEvents(false);
		SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		SkeletalMeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
#endif
	}
	SetPhysicsComponentSettings(false);
}

AT4MovableCharacterObject::~AT4MovableCharacterObject()
{
	check(nullptr == AnimationDataLoader);
	check(nullptr == AnimControl);
	check(!CharacterEntityPtr.IsValid());
}

void AT4MovableCharacterObject::Reset()
{
	ClearAsyncLoaders(); // WARN : AsyncLoad 가 걸렸을 수 있음으로 종료시 명시적으로 Reset 을 호출해야 한다.
	StanceNameSelected = T4Const_DefaultStanceName; // #73
	SubStanceNameSelected = T4Const_DefaultSubStanceName; // #106
	InactivePlayTag(ET4PlayTagType::All, T4CharacterStancePlayTagKey); // #74
	ResetMeshes(); // #37, #72
	if (nullptr != AnimControl)
	{
		delete AnimControl;
		AnimControl = nullptr;
	}
	CharacterEntityPtr.Reset();
}

void AT4MovableCharacterObject::ResetMeshes()  // #37
{
	for (TMap<FName, UT4SkinnedMeshComponent*>::TConstIterator It(MasterPoseModularMeshes); It; ++It) // #37
	{
		UT4SkinnedMeshComponent* SkinnedMeshComponent = It->Value;
		if (nullptr != SkinnedMeshComponent)
		{
			SkinnedMeshComponent->UnregisterComponent();
			SkinnedMeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		}
	}
	MasterPoseModularMeshes.Empty();
	MeshType = ET4EntityCharacterMeshType::None;
	ModularType = ET4EntityCharacterModularType::None;
	MasterPosePartName = NAME_None;
}

void AT4MovableCharacterObject::ResetSimulatePhysics() // #76
{
	if (!bPhysicsEnabled)
	{
		return;
	}
	// FPhysicsAssetEditorSharedData::EnableSimulation(bool bEnableSimulation)
	//SkeletalMeshComponent->SetPhysicsBlendWeight(0.0f);
	SkeletalMeshComponent->ResetAllBodiesSimulatePhysics();
#if 1
	SkeletalMeshComponent->SetSimulatePhysics(false);
#else
	//SkeletalMeshComponent->SetAllBodiesBelowPhysicsBlendWeight(TEXT("pelvis"), 0.0f);
	//SkeletalMeshComponent->SetAllBodiesBelowSimulatePhysics(TEXT("pelvis"), false);
#endif
	if (!bPhysicsSimulateBodiesBelow)
	{
		SetPhysicsComponentSettings(false);
	}
	if (nullptr == SkeletalMeshComponent->GetAttachParent())
	{
		// #76 : SetSimulatePhysics 이 호출되면 DetachFromParent 가 불리며 링크가 끊어진다.
		//       복구시에는 다시 Attach 를 해주어야 정상적으로 컨트롤이 된다!!
		SkeletalMeshComponent->AttachToComponent(CapsuleComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}
	if (!bPhysicsSimulateBodiesBelow)
	{
		SkeletalMeshComponent->SetWorldTransform(PhysicsRestoreTM); // Since simulation, actor location changes. Reset to identity 
	}
	SkeletalMeshComponent->RefreshBoneTransforms(); // Force an update of the skeletal mesh to get it back to ref pose
	
	PhysicsRestoreTM = FTransform::Identity;
	bPhysicsSimulateBodiesBelow = false;
	bPhysicsEnabled = false;
}

void AT4MovableCharacterObject::ClearAsyncLoaders() // #72
{
	ClearOverrideMaterialLoader(); // #80
	ClearMeshDataLoader(); // #72
	ClearPhysicsAssetDataLoader(); // #76
	ClearAsyncAnimationDataLoader(); // #73
	ClearAnimBlueprintClassLoader();
}

void AT4MovableCharacterObject::ClearOverrideMaterialLoader() // #80
{
	for (FT4OverrideMaterialLoadInfo& MaterialLoadInfo : OverrideMaterialLoadInfos)
	{
		for (FT4MaterialLoader& MaterialLoader : MaterialLoadInfo.MaterialLoaders)
		{
			MaterialLoader.Reset();
		}
		MaterialLoadInfo.MaterialLoaders.Empty();
	}
	OverrideMaterialLoadInfos.Empty();
}

void AT4MovableCharacterObject::ClearMeshDataLoader() // #72
{
	for (FT4SkeletalMeshLoadInfo& LoadInfo : PendingMeshLoadInfos) // #72
	{
		LoadInfo.SkeletalMeshLoader.Reset();
	}
	PendingMeshLoadInfos.Empty();
}

void AT4MovableCharacterObject::ClearPhysicsAssetDataLoader() // #76
{
	PhysicsAssetLoader.Reset();
	bPhysicsAssetAsyncLoading = false;
}

void AT4MovableCharacterObject::ClearAnimBlueprintClassLoader()
{
	AnimBPClassLoader.Reset();
}

void AT4MovableCharacterObject::ClearAsyncAnimationDataLoader() // #73
{
	if (!bStanceAsyncLoading && !bStancePreloading)
	{
		check(nullptr == AnimationDataLoader);
		return;
	}
	if (nullptr != AnimationDataLoader)
	{
		delete AnimationDataLoader; // #39
		AnimationDataLoader = nullptr;
	}
	bStanceAsyncLoading = false;
	bStancePreloading = false;
	StanceNamePreload = NAME_None;
}

void AT4MovableCharacterObject::WorldEnterStart() // #78
{
	// #78 : AsyncLoading 완료 후 호출된다. 즉, 그릴 준비까지 모두 완료 상태다.
}

void AT4MovableCharacterObject::WorldLeaveStart()
{
	// #36 : Leave 시의 Ghost 처리. Coll 충돌 제외 등...
}

void AT4MovableCharacterObject::PostInitializeComponents()
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

		if (nullptr != MovementComponent && nullptr != CapsuleComponent)
		{
			MovementComponent->UpdateNavAgent(*CapsuleComponent);
			MovementComponent->SetMovementMode(MOVE_Falling);
		}
	}
}

UPawnMovementComponent* AT4MovableCharacterObject::GetMovementComponent() const
{
	return MovementComponent;
}

void AT4MovableCharacterObject::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AT4MovableCharacterObject::UnPossessed()
{
	Super::UnPossessed();
}

void AT4MovableCharacterObject::UpdateTickActorBefore(const FT4UpdateTime& InUpdateTime)
{
	bool bCurrentLoadComplated = IsLoaded();
	if (!bCurrentLoadComplated) // #8
	{
		bCurrentLoadComplated = CheckAsyncLoading();
		if (bCurrentLoadComplated)
		{
			SetLoadComplated();
		}
	}
	if (!bCurrentLoadComplated)
	{
		return;
	}

	bool bUpdateRenderState = false;

	if (bOverrideMaterialLoading) // #80
	{
		bool bOverrideMaterialLoadComplated = CheckOverrideMaterialAsyncLoading(false);
		if (bOverrideMaterialLoadComplated)
		{
			bUpdateRenderState = true;
		}
	}

	if (bMeshAsyncLoading) // #72
	{
		bool bMeshLoadComplated = CheckMeshAsyncLoading(false);
		if (bMeshLoadComplated)
		{
			switch (MeshType)
			{
				case ET4EntityCharacterMeshType::FullBody:
					{
						ClearMeshDataLoader();
					}
					break;

				case ET4EntityCharacterMeshType::Composite:
					{
						switch (ModularType)
						{
							case ET4EntityCharacterModularType::MasterPose:
								{
									for (TMap<FName, UT4SkinnedMeshComponent*>::TConstIterator It(MasterPoseModularMeshes); It; ++It) // #37
									{
										UT4SkinnedMeshComponent* SkinnedMeshComponent = It->Value;
										check(nullptr != SkinnedMeshComponent);
										if (nullptr == SkinnedMeshComponent->GetAttachParent()) // 새로 추가된 Component 만 Attach
										{
											SkinnedMeshComponent->AttachToComponent(
												SkeletalMeshComponent,
												FAttachmentTransformRules::SnapToTargetIncludingScale
											);
											SkinnedMeshComponent->RegisterComponent();
											SkinnedMeshComponent->SetMasterPoseComponent(SkeletalMeshComponent);
										}
									}
									ClearMeshDataLoader();
								}
								break;

							case ET4EntityCharacterModularType::MeshMerge:
								{
									// TODO:
								}
								break;

							default:
								{
									T4_LOG(
										Error,
										TEXT("Unknown Entity Modular Type '%u'"),
										uint8(ModularType)
									);
								}
								break;
						}
					}
					break;

				default:
					{
						T4_LOG(
							Error,
							TEXT("Unknown Entity Type '%u'"),
							uint8(ModularType)
						);
					}
					break;
			}

			bUpdateRenderState = true;
		}
	}

	if (bPhysicsAssetAsyncLoading) // #76
	{
		bool bPhysicsAssetLoadComplated = CheckPhysicsAssetAsyncLoading();
		if (bPhysicsAssetLoadComplated)
		{
			// nothing
		}
	}

	if (bStanceAsyncLoading) // #73
	{
		bool bAnimationLoadComplated = CheckAnimationAsyncLoading();
		if (bAnimationLoadComplated)
		{
			// nothing
		}
	}
	else if (bStancePreloading) // #111
	{
		if (nullptr != AnimationDataLoader)
		{
			AnimationDataLoader->ProcessPre();
		}
	}

	if (bUpdateRenderState)
	{
		if (nullptr != SkeletalMeshComponent)
		{
			SkeletalMeshComponent->MarkRenderStateDirty();
		}
	}
}

void AT4MovableCharacterObject::UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime)
{
	if (nullptr != AnimControl)
	{
		AnimControl->OnAdvance(InUpdateTime);
	}

#if !UE_BUILD_SHIPPING
	DebugDrawObjectInfo();
	DebugDrawServerLocation(); // #52
#endif
}

bool AT4MovableCharacterObject::Create(const FT4SpawnObjectAction* InAction)
{
	check(nullptr != SkeletalMeshComponent);
	{
		// #111 : AimNotify supported
		SkeletalMeshComponent->SetLayerType(GetLayerType());
		SkeletalMeshComponent->SetOwnerObjectID(GetObjectID());
	}
	check(!CharacterEntityPtr.IsValid());
	CharacterEntityPtr = T4AssetEntityManagerGet()->GetActorEntity(InAction->EntityAssetPath);
	if (!CharacterEntityPtr.IsValid())
	{
		T4_LOG(
			Error,
			TEXT("EntityAsset (%s) not found"),
			*(InAction->EntityAssetPath.ToString())
		);
		return false;
	}

	EntityKey.Type = InAction->EntityType; // #35
	EntityKey.Value = CharacterEntityPtr->GetEntityKeyPath(); // #35
	EntityAssetPath = InAction->EntityAssetPath; // #68

	SetEntityAttributes(); // #74

	const FString DebugString = EntityKey.ToString();

	// #37, #72
	MeshType = CharacterEntityPtr->MeshType;
	switch (MeshType)
	{
		case ET4EntityCharacterMeshType::FullBody:
			{
				const FT4EntityCharacterFullBodyMeshData& MeshData = CharacterEntityPtr->FullBodyMeshData;
				if (!MeshData.SkeletalMeshAsset.IsNull())
				{
					SetFullbodyOverrideMaterialAsyncLoading(); // #80

					FT4SkeletalMeshLoadInfo& NewMeshLoadInfo = PendingMeshLoadInfos.AddDefaulted_GetRef();
					NewMeshLoadInfo.PartName = NAME_None;
					NewMeshLoadInfo.SkeletalMeshLoader.Load(
						MeshData.SkeletalMeshAsset.ToSoftObjectPath(),
						false,
						*DebugString
					);
					bMeshAsyncLoading = true; // #72
				}
			}
			break;

		case ET4EntityCharacterMeshType::Composite:
			{
				check(0 == OverrideMaterialLoadInfos.Num()); // #80
				check(0 == MasterPoseModularMeshes.Num());
				const FT4EntityCharacterCompositeMeshData& MeshData = CharacterEntityPtr->CopmpositeMeshData;
				if (0 < MeshData.DefaultPartsData.Num())
				{
					ModularType = MeshData.ModularType;
					MasterPosePartName = NAME_None;
					for (TMap<FName, FT4EntityCharacterCompositePartMeshData>::TConstIterator It(MeshData.DefaultPartsData); It; ++It)
					{
						const FName PartName = It->Key;
						const FT4EntityCharacterCompositePartMeshData& CompositePartMeshData = It->Value;
						if (CompositePartMeshData.CostumeEntityAsset.IsNull())
						{
							continue;
						}

						if (MasterPosePartName == NAME_None)
						{
							MasterPosePartName = PartName; // WARN : bMaster 체크가 없다면 첫번째를 사용한다!
						}
						else
						{
							UT4SkinnedMeshComponent* NewSkinnedMeshComponent = NewObject<UT4SkinnedMeshComponent>(SkeletalMeshComponent->GetOuter());
							check(nullptr != NewSkinnedMeshComponent);
							MasterPoseModularMeshes.Add(PartName, NewSkinnedMeshComponent);
						}

						// #35 에서 EntityAsset 은 초기에 모두 로딩시켜 두었기 때문에 믿고 LoadSynchronous 로 얻는다.
						UT4CostumeEntityAsset* CostumeEntityAsset = CompositePartMeshData.CostumeEntityAsset.LoadSynchronous();
						check(nullptr != CostumeEntityAsset);

						AddCompositeOverrideMaterialAsyncLoading(PartName, CostumeEntityAsset->MeshData.OverrideMaterialData, *DebugString); // #80

						{
							const FSoftObjectPath PartAssetPath = CostumeEntityAsset->MeshData.SkeletalMeshAsset.ToSoftObjectPath();
							FT4SkeletalMeshLoadInfo& NewMeshLoadInfo = PendingMeshLoadInfos.AddDefaulted_GetRef();
							NewMeshLoadInfo.PartName = PartName;
							NewMeshLoadInfo.SkeletalMeshLoader.Load(PartAssetPath, false, *DebugString);
							bMeshAsyncLoading = true; // #72
						}
					}
				}
			}
			break;

		default:
			{
				T4_LOG(
					Error,
					TEXT("Unknown Actor Mesh type '%u'"),
					uint8(MeshType)
				);
				return false;
			}
	}

	if (!bMeshAsyncLoading) // #72
	{
		T4_LOG(
			Warning,
			TEXT("No Set Mesh Settings!")
		);
		return true;
	}

	if (ET4EntityCharacterMeshType::FullBody == CharacterEntityPtr->MeshType) // #76
	{
		// #76 : Fullbody SK 라면 기본 세팅된 PhsycisAsset 을 그대로 사용하고, Override 할 경우만 재설정한다.
		const FT4EntityCharacterFullBodyMeshData& MeshData = CharacterEntityPtr->FullBodyMeshData;
		if (!MeshData.OverridePhysicsAsset.IsNull())
		{
			PhysicsAssetLoader.Load(
				MeshData.OverridePhysicsAsset.ToSoftObjectPath(),
				false,
				*DebugString
			);
			bPhysicsAssetAsyncLoading = true;
		}
	}

	// Validation
	if (CharacterEntityPtr->AnimBlueprintAsset.IsNull())
	{
		T4_LOG(
			Warning,
			TEXT(" No Set AnimBlueprintAsset!")
		);
	}
	else
	{
		FString AnimBPClassPath = CharacterEntityPtr->AnimBlueprintAsset.ToString() + TEXT("_C"); // add prerix '_C'
		AnimBPClassLoader.Load(AnimBPClassPath, false, *DebugString);
	}

	if (InAction->StanceName == NAME_None) // #73
	{
		ChangeStance(T4Const_DefaultStanceName, true);
	}
	else
	{
		ChangeStance(InAction->StanceName, true);
	}

	if (InAction->SubStanceName == NAME_None) // #109, #111
	{
		ChangeSubStance(T4Const_DefaultSubStanceName, true);
	}
	else
	{
		ChangeSubStance(InAction->SubStanceName, true);
	}

#if WITH_EDITOR
	{
		// #52
		SyncLocationForServer = InAction->SpawnLocation;
		SyncDirectionForServer = InAction->SpawnRotation.Vector();
	}
#endif

	return true;
}

bool AT4MovableCharacterObject::CheckAsyncLoading()
{
	if (bOverrideMaterialLoading) // #80
	{
		bool bOverrideMaterialLoadComplated = CheckOverrideMaterialAsyncLoading(true);
		if (!bOverrideMaterialLoadComplated)
		{
			return false;
		}
	}

	if (bMeshAsyncLoading) // #72
	{
		bool bMeshLoadComplated = CheckMeshAsyncLoading(true);
		if (!bMeshLoadComplated)
		{
			return false; 
		}
		if (ET4EntityCharacterMeshType::Composite == MeshType)
		{
			switch (ModularType)
			{
				case ET4EntityCharacterModularType::MasterPose:
					{
						for (TMap<FName, UT4SkinnedMeshComponent*>::TConstIterator It(MasterPoseModularMeshes); It; ++It) // #37
						{
							UT4SkinnedMeshComponent* SkinnedMeshComponent = It->Value;
							check(nullptr != SkinnedMeshComponent);
							check(nullptr == SkinnedMeshComponent->GetAttachParent());
							SkinnedMeshComponent->AttachToComponent(
								SkeletalMeshComponent,
								FAttachmentTransformRules::SnapToTargetIncludingScale
							);
							SkinnedMeshComponent->RegisterComponent();
							SkinnedMeshComponent->SetMasterPoseComponent(SkeletalMeshComponent);
						}
					}
					break;

				case ET4EntityCharacterModularType::MeshMerge:
					{
						// TODO:
#if 0
						USkeletalMesh* CompositeMesh = NewObject<USkeletalMesh>(GetTransientPackage(), NAME_None, RF_Transient);
						check(nullptr != CompositeMesh);

						TArray<FSkelMeshMergeSectionMapping> InForceSectionMapping;
						// create an instance of the FSkeletalMeshMerge utility
						FSkeletalMeshMerge MeshMergeUtil(CompositeMesh, SourceMeshList, InForceSectionMapping, 0);

						// merge the source meshes into the composite mesh
						if (!MeshMergeUtil.DoMerge())
						{
							// handle errors
							// ...
							T4_LOG(Log, TEXT("DoMerge Error: Merge Mesh Test Failed"));
							return true;
#endif
					}
					break;

				default:
					{
						T4_LOG(
							Error,
							TEXT("Unknown Entity Modular Type '%u'"),
							uint8(ModularType)
						);
					}
					break;
			}
		}
	}

	if (bPhysicsAssetAsyncLoading) // #76
	{
		bool bPhysicsAssetLoadComplated = CheckPhysicsAssetAsyncLoading();
		if (!bPhysicsAssetLoadComplated)
		{
			return false;
		}
	}

	if (AnimBPClassLoader.IsLoadStarted() && !AnimBPClassLoader.IsBinded())
	{
		if (!AnimBPClassLoader.Process(SkeletalMeshComponent))
		{
			return false;
		}

		{
			// #38
			UT4BaseAnimInstance* AnimInstance = GetAnimInstance();
			check(nullptr != AnimInstance);

			// #5
			check(nullptr == AnimControl);
			AnimControl = FT4BaseAnimControl::CreateNewControl(this, AnimInstance->GetAnimInstanceType());
			if (nullptr == AnimControl)
			{
				T4_LOG(
					Error,
					TEXT("No Set AnimInstanceType!")
				);
				return false;
			}
		}

		// #14
		AnimBPClassLoader.SetBinded();
	}

	// #73
	bool bAnimationLoadComplated = CheckAnimationAsyncLoading();
	if (!bAnimationLoadComplated)
	{
		return false;
	}

	if (nullptr != AnimControl)
	{
		AnimControl->BeginPlay(); // #18
	}

	ClearAsyncLoaders(); // #72

	// #78 : 메시 로딩이 완료되면 Materal 의 Opacity 를 0 으로 만들어두고,
	//       캐릭터 비동기 로딩이 완료되는 시점에 Fade In 이 동작하도록 처리한다.
	OnWorldEnterStart(T4Const_ObjectWorldEnterTimeSec); // #78

	// #111 : 로딩전에 Pending 된 무기가 있다면 화면에 보이도록 Flush 해준다.
	{
		FT4AnimNotifyEquipment NewMessage;
		NewMessage.EquipmentType = ET4EquipmentType::Unmount;
		NewMessage.SameStanceName = GetStanceName();
#if WITH_EDITOR
		NewMessage.DebugSting = TEXT("CheckAsyncLoading");
#endif
		OnAnimNotifyMessage(&NewMessage);
	}
	return true;
}

bool AT4MovableCharacterObject::CheckOverrideMaterialAsyncLoading(bool bInitialize) // #80
{
	if (!bOverrideMaterialLoading)
	{
		return true;
	}
	for (FT4OverrideMaterialLoadInfo& MaterialLoadInfo : OverrideMaterialLoadInfos)
	{
		for (FT4MaterialLoader& MaterialLoader : MaterialLoadInfo.MaterialLoaders)
		{
			if (MaterialLoader.IsLoadStarted() && !MaterialLoader.IsLoadCompleted())
			{
				return false;
			}
		}
	}
	for (FT4OverrideMaterialLoadInfo& MaterialLoadInfo : OverrideMaterialLoadInfos)
	{
		UMeshComponent* MeshComponentSelected = nullptr;
		if (MaterialLoadInfo.PartName == NAME_None)
		{
			MeshComponentSelected = Cast<UMeshComponent>(SkeletalMeshComponent);
		}
		else
		{
			if (!MasterPoseModularMeshes.Contains(MaterialLoadInfo.PartName))
			{
				return false;
			}
			MeshComponentSelected = Cast<UMeshComponent>(MasterPoseModularMeshes[MaterialLoadInfo.PartName]);
		}
		check(nullptr != MeshComponentSelected);
		if (MaterialLoadInfo.bClear)
		{
			if (!bInitialize)
			{
				MeshComponentSelected->EmptyOverrideMaterials();
			}
		}
		else
		{
			for (int32 i = 0; i < MaterialLoadInfo.MaterialLoaders.Num(); ++i)
			{
				FT4MaterialLoader& MaterialLoader = MaterialLoadInfo.MaterialLoaders[i];
				UMaterialInterface* MaterialInterface = nullptr;
				if (MaterialLoader.IsLoadStarted())
				{
					check(MaterialLoader.IsLoadCompleted());
					MaterialInterface = MaterialLoader.GetMaterialInterface();
					check(nullptr != MaterialInterface);
				}
				if (bInitialize)
				{
					MeshComponentSelected->OverrideMaterials.Add(MaterialInterface);
				}
				else
				{
					MeshComponentSelected->SetMaterial(i, MaterialInterface);
				}
			}
		}
	}
	ClearOverrideMaterialLoader();
	bOverrideMaterialLoading = false;
	return true;
}

bool AT4MovableCharacterObject::CheckMeshAsyncLoading(bool bInitialize) // #72
{
	if (!bMeshAsyncLoading)
	{
		return true;
	}
	for (TArray<FT4SkeletalMeshLoadInfo>::TIterator It(PendingMeshLoadInfos); It; ++It)
	{
		FT4SkeletalMeshLoadInfo& LoadInfo = *It;
		if (!LoadInfo.SkeletalMeshLoader.IsBinded())
		{
			if (!LoadInfo.SkeletalMeshLoader.IsLoadCompleted())
			{
				return false;
			}
		}
	}
	for (TArray<FT4SkeletalMeshLoadInfo>::TIterator It(PendingMeshLoadInfos); It; ++It)
	{
		FT4SkeletalMeshLoadInfo& LoadInfo = *It;
		check(!LoadInfo.SkeletalMeshLoader.IsBinded());
		check(LoadInfo.SkeletalMeshLoader.IsLoadCompleted());
		USkinnedMeshComponent* UpdateMeshComponent = Cast<USkinnedMeshComponent>(SkeletalMeshComponent);
		check(nullptr != UpdateMeshComponent);
		switch (MeshType)
		{
			case ET4EntityCharacterMeshType::FullBody:
				{
					LoadInfo.SkeletalMeshLoader.Process(UpdateMeshComponent);
					LoadInfo.SkeletalMeshLoader.SetBinded();
				}
				break;

			case ET4EntityCharacterMeshType::Composite:
				{
					if (ET4EntityCharacterModularType::MasterPose == ModularType)
					{
						UT4SkinnedMeshComponent* NewSkinnedMeshComponent = nullptr;
						if (MasterPosePartName != LoadInfo.PartName)
						{
							if (!MasterPoseModularMeshes.Contains(LoadInfo.PartName))
							{
								NewSkinnedMeshComponent = NewObject<UT4SkinnedMeshComponent>(SkeletalMeshComponent->GetOuter());
								check(nullptr != NewSkinnedMeshComponent);
								MasterPoseModularMeshes.Add(LoadInfo.PartName, NewSkinnedMeshComponent);
								UpdateMeshComponent = Cast<USkinnedMeshComponent>(NewSkinnedMeshComponent);
							}
							else
							{
								UpdateMeshComponent = MasterPoseModularMeshes[LoadInfo.PartName]; // Exchange
							}
						}
						LoadInfo.SkeletalMeshLoader.Process(UpdateMeshComponent);
						LoadInfo.SkeletalMeshLoader.SetBinded();
					}
					else
					{
						check(ET4EntityCharacterModularType::MeshMerge == ModularType);
						// TODO:
					}
				}
				break;

			default:
				{
					T4_LOG(
						Error,
						TEXT("Unknown Entity Mesh type '%u'"),
						uint8(MeshType)
					);
				}
				break;
		}
	}

	if (!bInitialize)
	{
		ResetDynamicMaterialInstances();
	}

	// #78 : 메시 로딩이 완료되면 Materal 의 Opacity 를 0 으로 만들어두고,
	//       캐릭터 비동기 로딩이 완료되는 시점에 Fade In 이 동작하도록 처리한다.
	AddDynamicMaterialInstances(Cast<UMeshComponent>(SkeletalMeshComponent)); // #78

	if (ET4EntityCharacterMeshType::Composite == MeshType)
	{
		for (TMap<FName, UT4SkinnedMeshComponent*>::TConstIterator It(MasterPoseModularMeshes); It; ++It) // #37
		{
			UT4SkinnedMeshComponent* SkinnedMeshComponent = It->Value;
			check(nullptr != SkinnedMeshComponent);
			AddDynamicMaterialInstances(Cast<UMeshComponent>(SkinnedMeshComponent));
		}
	}

	if (bInitialize)
	{
		SetOpacity(0.0f, 0.0f); // #78
	}

	bMeshAsyncLoading = false;
	return true;
}

bool AT4MovableCharacterObject::CheckPhysicsAssetAsyncLoading() // #76
{
	if (!bPhysicsAssetAsyncLoading)
	{
		return true;
	}
	// TODO : Composite Parts
	if (!PhysicsAssetLoader.IsBinded())
	{
		if (PhysicsAssetLoader.IsLoadCompleted())
		{
			check(nullptr != SkeletalMeshComponent);
			PhysicsAssetLoader.Process(SkeletalMeshComponent);
			PhysicsAssetLoader.SetBinded();
			bPhysicsAssetAsyncLoading = false;
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool AT4MovableCharacterObject::CheckAnimationAsyncLoading() // #73
{
	if (!bStanceAsyncLoading || nullptr == AnimControl)
	{
		return true;
	}
	if (nullptr != AnimationDataLoader && !AnimationDataLoader->IsBinded())
	{
		// #69 : TODO : 단계별 로딩을 통해 AnimMontage 로딩이 없을 경우 AnimSet 에 기록된 AnimSequence Duration
		//       으로 애니메이션 State 가 동작할 수 있도록 처리해주어야 한다. 타이밍 문제 해소 필요!
		if (!AnimationDataLoader->Process(AnimControl))
		{
			return false;
		}
		AnimationDataLoader->SetBinded();
	}
	ClearAsyncAnimationDataLoader(); // #73
	return true;
}

void AT4MovableCharacterObject::SetEntityAttributes()
{
	check(CharacterEntityPtr.IsValid());
	const FT4EntityCharacterPhysicalAttribute& PhysicalAttribute = CharacterEntityPtr->Physical;
	const FT4EntityCharacterRenderingAttribute& RenderingAttribute = CharacterEntityPtr->Rendering;

	FT4GameObjectProperty& Property = GetProperty();

	// #46 : TODO: Gameplay 테이블에서 읽어서 채울 것!
	const float MoveSpeedBySubStance = GetMoveSpeedBySubStance();
	{
		Property.SetMoveSpeed(ET4MoveMode::Async, MoveSpeedBySubStance); // #33, #109
		Property.JumpZVelocity = PhysicalAttribute.RotationYawRate; // #109
		Property.RollZVelocity = PhysicalAttribute.RotationYawRate; // #109
		Property.RotationYawRate = PhysicalAttribute.RotationYawRate; // #44, #46
	}

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
	{
		SkeletalMeshComponent->SetReceivesDecals(RenderingAttribute.bReceivesDecals); // #54
		SkeletalMeshComponent->SetRelativeScale3D(Property.RelativeScale3D);
	}
	{
		check(nullptr != MovementComponent);
		// #44 : MovementComponent 에서의 캐릭터 방향전환 보간을 끈다. RotationAction 으로 별도로 처리
		//MovementComponent->bOrientRotationToMovement = true;
		MovementComponent->RotationRate = FRotator(0.0f, Property.RotationYawRate, 0.0f);
		MovementComponent->MaxMoveSpeed = MoveSpeedBySubStance;
	}
}

void AT4MovableCharacterObject::SetPhysicsComponentSettings(bool bRagdoll) // #76
{
	if (nullptr == SkeletalMeshComponent)
	{
		return;
	}
	SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
	if (bRagdoll)
	{
		if (nullptr != CapsuleComponent)
		{
			CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		}
		SkeletalMeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
		SkeletalMeshComponent->SetCollisionProfileName(TEXT("Ragdoll"));
		return;
	}
	if (nullptr != CapsuleComponent)
	{
		// #49
		CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
		CapsuleComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}
#if (WITH_EDITOR || WITH_SERVER_CODE)
	SkeletalMeshComponent->SetCollisionObjectType(T4COLLISION_GAMEOBJECT);
	SkeletalMeshComponent->SetCollisionProfileName(TEXT("T4HitOverlapOnlyGameObject"));
#else
	SkeletalMeshComponent->SetCollisionObjectType(ECC_PhysicsBody);
	SkeletalMeshComponent->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
#endif
}

void AT4MovableCharacterObject::SetFullbodyOverrideMaterialAsyncLoading() // #80
{
	if (bOverrideMaterialLoading)
	{
		check(false); // TODO : 로딩중 처리!
		return;
	}
	check(0 == OverrideMaterialLoadInfos.Num());
	const FT4EntityCharacterFullBodyMeshData& FullbodyMeshData = CharacterEntityPtr->FullBodyMeshData;
	if (FullbodyMeshData.SkeletalMeshAsset.IsNull())
	{
		return;
	}
	check(nullptr != SkeletalMeshComponent);
	const FT4EntityOverrideMaterialData& OverrideMaterialData = FullbodyMeshData.OverrideMaterialData; // #80
	if (0 >= OverrideMaterialData.MaterialSortedSlotNames.Num()) // #80
	{
		SkeletalMeshComponent->EmptyOverrideMaterials();
		return;
	}
	PlayMaterialForPlayTag(OverrideMaterialData, FT4ActionKey::EmptyActionKey);
}

void AT4MovableCharacterObject::AddCompositeOverrideMaterialAsyncLoading(
	const FName InPartName,
	const FT4EntityOverrideMaterialData& InOverrideMaterialData,
	const TCHAR* InDebugString
) // #80
{
	FT4OverrideMaterialLoadInfo& NewMateralLoadInfo = OverrideMaterialLoadInfos.AddDefaulted_GetRef();
	NewMateralLoadInfo.PartName = (MasterPosePartName == InPartName) ? NAME_None : InPartName;
	NewMateralLoadInfo.bClear = (0 >= InOverrideMaterialData.MaterialSortedSlotNames.Num()) ? true : false; // #78 : 적용된 Mesh 에서 Override Material 만 삭제할 경우 사용
	if (!NewMateralLoadInfo.bClear)
	{
		for (FName SlotName : InOverrideMaterialData.MaterialSortedSlotNames)
		{
			check(InOverrideMaterialData.MaterialMap.Contains(SlotName));
			TSoftObjectPtr<UMaterialInterface> MaterialInstance = InOverrideMaterialData.MaterialMap[SlotName];
			FT4MaterialLoader& NewMateralLoader = NewMateralLoadInfo.MaterialLoaders.AddDefaulted_GetRef();
			if (!MaterialInstance.IsNull()) // Override Material 이기 때문이 null 이면 원본을 사용하도록 비워둔다.
			{
				NewMateralLoader.Load(MaterialInstance.ToSoftObjectPath(), false, InDebugString);
			}
		}
	}
	bOverrideMaterialLoading = true;
}

const UT4EntityAsset* AT4MovableCharacterObject::GetEntityAsset()
{
	// #39
	if (!CharacterEntityPtr.IsValid())
	{
		return nullptr;
	}
	return Cast<UT4EntityAsset>(CharacterEntityPtr.Get());
}

IT4AnimControl* AT4MovableCharacterObject::GetAnimControl() const
{
	return static_cast<IT4AnimControl*>(AnimControl); // #14
}

bool AT4MovableCharacterObject::HasPlayingAnimState(const FName& InAnimStateName) const // #47
{
	if (nullptr == AnimControl)
	{
		return false;
	}
	return (AnimControl->GetActiveAnimStateName() == InAnimStateName) ? true : false;
}

const FVector AT4MovableCharacterObject::GetNavPoint() const
{
	if (nullptr == MovementComponent)
	{
		return FVector::ZeroVector;
	}
	return MovementComponent->GetActorFeetLocation();
}

bool AT4MovableCharacterObject::HasReaction(const FName& InReactionName) const // #73
{
	check(CharacterEntityPtr.IsValid());
	return CharacterEntityPtr->ReactionSetData.ReactionMap.Contains(InReactionName);
}

bool AT4MovableCharacterObject::HasPlayTag(
	const FName& InPlayTagName, 
	ET4PlayTagType InPlayTagType
) const // #81
{
	check(CharacterEntityPtr.IsValid());
	const FT4EntityPlayTagData& PlayTagData = CharacterEntityPtr->PlayTagData;
	if (ET4PlayTagType::All == InPlayTagType || ET4PlayTagType::Material == InPlayTagType)
	{
		for (const FT4EntityPlayTagMaterialData& PlayTagMaterialData : PlayTagData.MaterialTags) // #80
		{
			if (PlayTagMaterialData.PlayTag == InPlayTagName)
			{
				return true;
			}
		}
	}
	if (ET4PlayTagType::All == InPlayTagType || ET4PlayTagType::Attachment == InPlayTagType)
	{
		for (const FT4EntityPlayTagAttachmentData& PlayTagAttachmentData : PlayTagData.AttachmentTags)
		{
			if (PlayTagAttachmentData.PlayTag == InPlayTagName)
			{
				return true;
			}
		}
	}
	if (ET4PlayTagType::All == InPlayTagType || ET4PlayTagType::Conti == InPlayTagType)
	{
		for (const FT4EntityPlayTagContiData& PlayTagContiData : PlayTagData.ContiTags)
		{
			if (PlayTagContiData.PlayTag == InPlayTagName)
			{
				return true;
			}
		}
	}
	return false;
}

bool AT4MovableCharacterObject::HasActionPoint(const FName& InActionPoint) const
{
	// TODO : #57 : ActionPoint = Socket or Bone or VirtualBone
	check(nullptr != SkeletalMeshComponent);
	return SkeletalMeshComponent->DoesSocketExist(InActionPoint);
}

bool AT4MovableCharacterObject::GetSocketLocation(
	const FName& InSocketName,
	FVector& OutLocation
) const // #18
{
	check(nullptr != SkeletalMeshComponent);
	if (!SkeletalMeshComponent->DoesSocketExist(InSocketName))
	{
		return false;
	}
	OutLocation = SkeletalMeshComponent->GetSocketLocation(InSocketName);
	return true;
}

bool AT4MovableCharacterObject::GetSocketRotation(
	const FName& InSocketName,
	ERelativeTransformSpace InTransformSpace,
	FRotator& OutRotation
) const // #18
{
	check(nullptr != SkeletalMeshComponent);
	if (!SkeletalMeshComponent->DoesSocketExist(InSocketName))
	{
		return false;
	}
	OutRotation = SkeletalMeshComponent->GetSocketTransform(
		InSocketName, 
		InTransformSpace
	).GetRotation().Rotator();
	return true;
}

bool AT4MovableCharacterObject::GetSocketScale(
	const FName& InSocketName,
	ERelativeTransformSpace InTransformSpace,
	FVector& OutScale
) const // #54
{
	check(nullptr != SkeletalMeshComponent);
	if (!SkeletalMeshComponent->DoesSocketExist(InSocketName))
	{
		return false;
	}
	OutScale = SkeletalMeshComponent->GetSocketTransform(
		InSocketName,
		InTransformSpace
	).GetScale3D();
	return true;
}

bool AT4MovableCharacterObject::GetSocketTransform(
	const FName& InSocketName,
	ERelativeTransformSpace InTransformSpace,
	FTransform& OutTransform
) const // #58
{
	check(nullptr != SkeletalMeshComponent);
	if (!SkeletalMeshComponent->DoesSocketExist(InSocketName))
	{
		return false;
	}
	OutTransform = SkeletalMeshComponent->GetSocketTransform(
		InSocketName,
		InTransformSpace
	);
	return true;
}

void AT4MovableCharacterObject::SetHeightOffset(float InOffset)
{
	check(nullptr != CapsuleComponent); // #18
	check(nullptr != SkeletalMeshComponent);

	float ApplyHalfHeight = GetPropertyConst().HalfHeight + (InOffset * 0.5f);
	CapsuleComponent->SetCapsuleHalfHeight(ApplyHalfHeight);
}

void AT4MovableCharacterObject::SetOutline(bool bInUse) // #115
{
	check(nullptr != SkeletalMeshComponent);
	SkeletalMeshComponent->SetRenderCustomDepth(bInUse);
}

#if WITH_EDITOR
void AT4MovableCharacterObject::SetDebugPause(bool bInPause) // #68
{
	Super::SetDebugPause(bInPause);
	if (nullptr != MovementComponent)
	{
		//CapsuleComponent->SetTickableWhenPaused(bInPause);
		CapsuleComponent->SetComponentTickEnabled(!bInPause);
	}
	if (nullptr != MovementComponent)
	{
		//MovementComponent->SetTickableWhenPaused(bInPause);
		MovementComponent->SetComponentTickEnabled(!bInPause);
	}
	if (nullptr != SkeletalMeshComponent)
	{
		//SkeletalMeshComponent->SetTickableWhenPaused(bInPause);
		SkeletalMeshComponent->SetComponentTickEnabled(!bInPause);
	}
}
#endif

void AT4MovableCharacterObject::SetAccelerationMoveSpeedScale(float InMoveSpeedScale)
{
#if USES_ACCELERATION_MOVE_ISSUE_52
	// #38 : 가감속! Clinet 사이드 MyPC 만...
	if (!HasPlayer())
	{
		return;
	}
	check(!T4EngineLayer::IsServer(LayerType));
	check(nullptr != MovementComponent);
	MovementComponent->SetAnalogInputModifier(InMoveSpeedScale);
	GetProperty().MoveAccelerationScale = InMoveSpeedScale; // #52
#endif
}

void AT4MovableCharacterObject::StartTurning(
	const FRotator& InRotation,
	float InRotationYawRate
)
{
	// #44, #47 : Turn 처리를 Action 에서 Object 단으로 이동. Action 없이도 동작할 수 있도록 지원
	AT4GameObject::StartTurning(InRotation, InRotationYawRate);
	if (0.0f < InRotationYawRate)
	{
		if (nullptr != AnimControl)
		{
			AnimControl->DoTurn(InRotation); // 턴 애니 처리
		}
	}
}

bool AT4MovableCharacterObject::StartPhysics(bool bSimulateBodiesBelow) // #76 : ReactionNode 에서 호출
{
	check(nullptr != SkeletalMeshComponent);
	if (!bPhysicsEnabled)
	{
		PhysicsRestoreTM = SkeletalMeshComponent->GetComponentToWorld(); // 첫번째만 보관하면 된다.
		if (!bSimulateBodiesBelow)
		{
			SetPhysicsComponentSettings(true);
		}
		bPhysicsEnabled = true;
	}
	else if (bSimulateBodiesBelow)
	{
		return false; // #76 : fullbody physics 시뮬레이션 중일 경우는 Die 류임으로 분할 처리는 하지 않는다.
	}
	bPhysicsSimulateBodiesBelow = bSimulateBodiesBelow;
	return true;
}

void AT4MovableCharacterObject::StopPhysics() // #76 : ReactionNode 에서 호출
{
	ResetSimulatePhysics();
}

FT4AnimInstanceID AT4MovableCharacterObject::PlayAnimationAndBroadcast(
	const FT4AnimParameters& InAnimParameters
) // #107
{
	FT4AnimInstanceID AnimInstanceID = INDEX_NONE;
	if (nullptr != AnimControl)
	{
		AnimInstanceID = AnimControl->PlayAnimation(InAnimParameters);
	}
	{
		AT4GameObject::PlayAnimationAndBroadcast(InAnimParameters); // Only Equipment
	}
	return AnimInstanceID;
}

void AT4MovableCharacterObject::ActivePlayTag(
	const FName InPlayTagName,
	ET4PlayTagType InPlayTagType,
	const FT4ActionKey& InActionKey
) // #74, #81
{
	check(CharacterEntityPtr.IsValid());
	const FT4EntityPlayTagData& PlayTagData = CharacterEntityPtr->PlayTagData;
	if (ET4PlayTagType::All == InPlayTagType || ET4PlayTagType::Material == InPlayTagType)
	{
		for (const FT4EntityPlayTagMaterialData& PlayTagMaterialData : PlayTagData.MaterialTags) // #80
		{
			if (PlayTagMaterialData.PlayTag != InPlayTagName)
			{
				continue;
			}
			const FT4EntityOverrideMaterialData& OverrideMaterialData = PlayTagMaterialData.OverrideMaterialData;
			if (0 >= OverrideMaterialData.MaterialSortedSlotNames.Num())
			{
				continue;
			}
			bool bResult = PlayMaterialForPlayTag(OverrideMaterialData, InActionKey); // #81
			if (bResult)
			{
				break; // #81 : 머트리얼은 첫 한개만 로드
			}
		}
	}
	if (ET4PlayTagType::All == InPlayTagType || ET4PlayTagType::Attachment == InPlayTagType)
	{
		for (const FT4EntityPlayTagAttachmentData& PlayTagAttachmentData : PlayTagData.AttachmentTags)
		{
			if (PlayTagAttachmentData.PlayTag != InPlayTagName)
			{
				continue;
			}
			PlayAttachmentForPlayTag(PlayTagAttachmentData, InActionKey);
		}
	}
	if (ET4PlayTagType::All == InPlayTagType || ET4PlayTagType::Conti == InPlayTagType)
	{
		for (const FT4EntityPlayTagContiData& PlayTagContiData : PlayTagData.ContiTags)
		{
			if (PlayTagContiData.PlayTag != InPlayTagName)
			{
				continue;
			}
			PlayContiForPlayTag(PlayTagContiData, InActionKey);
		}
	}
}

void AT4MovableCharacterObject::InactivePlayTag(
	ET4PlayTagType InPlayTagType,
	const FT4ActionKey& InActionKey
) // #74, #81
{
	AT4GameObject::InactivePlayTag(InPlayTagType, InActionKey);
	if (PlayTagMaterialActionKey.IsValid())
	{
		SetFullbodyOverrideMaterialAsyncLoading();
		PlayTagMaterialActionKey.Reset();
	}
}

bool AT4MovableCharacterObject::PlayMaterialForPlayTag(
	const FT4EntityOverrideMaterialData& InPlayTagData,
	const FT4ActionKey& InActionKey
) // #81
{
	if (bOverrideMaterialLoading)
	{
		//check(false); // TODO : 로딩중 처리!
		return false;
	}
	if (ET4EntityCharacterMeshType::FullBody != MeshType)
	{
		return false;
	}
	if (0 >= InPlayTagData.MaterialSortedSlotNames.Num()) // #80
	{
		return false;
	}
	bool bLoadStart = false;
	const FString DebugString = EntityKey.ToString();
	FT4OverrideMaterialLoadInfo& NewMateralLoadInfo = OverrideMaterialLoadInfos.AddDefaulted_GetRef();
	NewMateralLoadInfo.PartName = NAME_None;
	for (FName SlotName : InPlayTagData.MaterialSortedSlotNames)
	{
		check(InPlayTagData.MaterialMap.Contains(SlotName));
		TSoftObjectPtr<UMaterialInterface> MaterialInstance = InPlayTagData.MaterialMap[SlotName];
		FT4MaterialLoader& NewMateralLoader = NewMateralLoadInfo.MaterialLoaders.AddDefaulted_GetRef();
		if (!MaterialInstance.IsNull()) // Override Material 이기 때문이 null 이면 원본을 사용하도록 비워둔다.
		{
			NewMateralLoader.Load(MaterialInstance.ToSoftObjectPath(), false, *DebugString);
			bLoadStart = true;
		}
	}
	if (!bLoadStart)
	{
		OverrideMaterialLoadInfos.Empty();
		return false;
	}
	PlayTagMaterialActionKey = InActionKey;
	bOverrideMaterialLoading = true;
	return true;
}

bool AT4MovableCharacterObject::StartStanceLoading(
	FName InStanceName, 
	FName& OutStanceNameSelected,
	FName& OutActivePlayTag
) // #111
{
	if (!CharacterEntityPtr.IsValid())
	{
		return false;
	}
	if (nullptr != AnimationDataLoader)
	{
		ClearAsyncAnimationDataLoader();
	}
	check(nullptr == AnimationDataLoader);
	const FT4EntityCharacterStanceSetData& StanceSetData = CharacterEntityPtr->StanceSetData;
	if (0 >= StanceSetData.StanceMap.Num())
	{
		T4_LOG(
			Warning,
			TEXT("No Set DefaultAnimSet!")
		);
		return false;
	}
	else if (!StanceSetData.StanceMap.Contains(InStanceName))
	{
		InStanceName = T4Const_DefaultStanceName; // Stance 가 없다면 Default 로 Fallback 처리한다.
		if (!StanceSetData.StanceMap.Contains(InStanceName))
		{
			T4_LOG(
				Warning,
				TEXT("Stance '%s' Not found!"),
				*(InStanceName.ToString())
			);
			return false;
		}
		else
		{
			T4_LOG(
				Verbose,
				TEXT("Stance '%s' Not found! Set 'Default' Stance"),
				*(InStanceName.ToString())
			);
		}
	}
	const FT4EntityCharacterStanceData& StanceDataSelected = StanceSetData.StanceMap[InStanceName];
	if (StanceDataSelected.AnimSetAsset.IsNull())
	{
		T4_LOG(
			Warning,
			TEXT("No Set '%s' Stance AnimSetAsset!"),
			*(InStanceName.ToString())
		);
		return false;
	}
	const FString DebugString = EntityKey.ToString();
	AnimationDataLoader = new FT4CharacterAnimationDataLoader; // #39
	AnimationDataLoader->Load(&StanceDataSelected, false, *DebugString);
	OutStanceNameSelected = InStanceName;
	OutActivePlayTag = StanceDataSelected.ActivePlayTag;
	return true;
}

bool AT4MovableCharacterObject::PreloadStance(FName InStanceName) // #111
{
	bool bResultLoading = StartStanceLoading(InStanceName, RealStanceNamePreload, ActivePlayTagByStance);
	if (!bResultLoading)
	{
		return false;
	}
	check(nullptr != AnimationDataLoader);
	bStancePreloading = true;
	StanceNamePreload = InStanceName;
	return true;
}

bool AT4MovableCharacterObject::ChangeStance(
	FName InStanceName, 
	bool bInInitialize
) // #73
{
	bool bResultPreload = false;
	FName ActivePlayTagSelected = NAME_None;
	if (bStancePreloading && StanceNamePreload == InStanceName)
	{
		// #111 : 미리 로딩을 걸어두었다면...
		InStanceName = RealStanceNamePreload; // #111 : fallback 대비!
		ActivePlayTagSelected = ActivePlayTagByStance;
		bStancePreloading = false;
		StanceNamePreload = NAME_None;
		ActivePlayTagByStance = NAME_None;
		bResultPreload = true;
	}

	if (!bResultPreload)
	{
		bool bResultLoading = StartStanceLoading(InStanceName, InStanceName, ActivePlayTagSelected);
		if (!bResultLoading)
		{
			return false;
		}
	}

	StanceNameSelected = InStanceName;
	bStanceAsyncLoading = true;

	check(nullptr != AnimationDataLoader);
	bool bAnimationLoadComplated = false;
	if (!bInInitialize)
	{
		bAnimationLoadComplated = CheckAnimationAsyncLoading();
#if 0
		// #111 : Stance 전환은 AnimSet 전환이 있어서 변경 타임에 반드시 업데이트 되도록 처리해준다.
		//        랙은 전환 모션을 넣어서 Preload 를 활용할 것!
		while(!bAnimationLoadComplated)
		{
			bAnimationLoadComplated = CheckAnimationAsyncLoading();
			FPlatformProcess::Sleep(0.1f);
		}
#endif
		if (!bAnimationLoadComplated)
		{
			T4_LOG(Verbose, TEXT("Stance '%s' PreLoading Miss!!"), *(InStanceName.ToString()));
		}
	}

	// #73, #74
	InactivePlayTag(ET4PlayTagType::All, T4CharacterStancePlayTagKey);
	if (ActivePlayTagSelected != NAME_None)
	{
		// #74 : 아래 기능은 캐릭터의 생성/소멸 주기와 함께한다.
		ActivePlayTag(ActivePlayTagSelected, ET4PlayTagType::All, T4CharacterStancePlayTagKey);
	}
	// ~#73, #74
	return true;
}

bool AT4MovableCharacterObject::ChangeSubStance(
	FName InSubStanceName, 
	bool bInInitialize
) // #109, #111
{
	FName OldSubStanceName = SubStanceNameSelected; // #111
	SubStanceNameSelected = InSubStanceName;
	if (bInInitialize || HasPlayer())
	{
		// #109 : SubStance 별로 속도를 제어한다.
		const float MoveSpeedBySubStance = GetMoveSpeedBySubStance();
		GetProperty().SetMoveSpeed(ET4MoveMode::Async, MoveSpeedBySubStance); // #33
		check(nullptr != MovementComponent);
		MovementComponent->MaxMoveSpeed = MoveSpeedBySubStance;
	}
	return true;
}

UT4BaseAnimInstance* AT4MovableCharacterObject::GetAnimInstance() const
{
	check(nullptr != SkeletalMeshComponent);
	UT4BaseAnimInstance* AnimInstance = Cast<UT4BaseAnimInstance>(
		SkeletalMeshComponent->GetAnimInstance()
	);
	if (nullptr == AnimInstance)
	{
		return nullptr;
	}
	return AnimInstance;
}

USceneComponent* AT4MovableCharacterObject::GetAttachParentComponent()
{
	return Cast<USceneComponent>(SkeletalMeshComponent);
}

UCapsuleComponent* AT4MovableCharacterObject::GetCapsuleComponent() const
{
	return Cast<UCapsuleComponent>(CapsuleComponent);
}

USkeletalMeshComponent* AT4MovableCharacterObject::GetSkeletalMeshComponent() const
{
	return Cast<USkeletalMeshComponent>(SkeletalMeshComponent);
}

bool AT4MovableCharacterObject::IsLockOn() const
{
	if (nullptr == AnimControl)
	{
		return false;
	}
	const FT4GameObjectProperty& Property = GetPropertyConst();
	return Property.bIsLockOn;
}

bool AT4MovableCharacterObject::IsFalling() const
{
	if (nullptr == MovementComponent)
	{
		return false;
	}
	return MovementComponent->IsFalling();
}

bool AT4MovableCharacterObject::IsFlying() const
{
	if (nullptr == MovementComponent)
	{
		return false;
	}
	return MovementComponent->IsFlying();
}

bool AT4MovableCharacterObject::IsCombat() const // #109
{
	return (T4Const_CombatSubStanceName == SubStanceNameSelected) ? true : false;
}

float AT4MovableCharacterObject::GetMoveSpeedBySubStance() const // #109
{
	check(CharacterEntityPtr.IsValid());
	const FT4EntityCharacterPhysicalAttribute& PhysicalAttribute = CharacterEntityPtr->Physical;
	if (SubStanceNameSelected == T4Const_CombatSubStanceName)
	{
		return PhysicalAttribute.CombatSpeed;
	}
	else if (SubStanceNameSelected == T4Const_CrouchSubStanceName)
	{
		return PhysicalAttribute.CrouchSpeed; // #109
	}
	return PhysicalAttribute.DefaultSpeed;
}

/**	Change the Pawn's base. */
void AT4MovableCharacterObject::SetBase(
	UPrimitiveComponent* NewBaseComponent,
	const FName InBoneName,
	bool bNotifyPawn
)
{
	// If NewBaseComponent is nullptr, ignore bone name.
	const FName BoneName = (NewBaseComponent ? InBoneName : NAME_None);

	// See what changed.
	const bool bBaseChanged = (NewBaseComponent != BasedMovement.MovementBase);
	const bool bBoneChanged = (BoneName != BasedMovement.BoneName);

	if (bBaseChanged || bBoneChanged)
	{
		// Verify no recursion.
		APawn* Loop = (NewBaseComponent ? Cast<APawn>(NewBaseComponent->GetOwner()) : nullptr);
		while (Loop)
		{
			if (Loop == this)
			{
				T4_LOG(
					Warning,
					TEXT("Recursion detected. Pawn %s already based on %s."),
					*GetName().ToString(),
					*NewBaseComponent->GetName()
				); // -V595
				return;
			}
			if (UPrimitiveComponent* LoopBase = Loop->GetMovementBase())
			{
				Loop = Cast<APawn>(LoopBase->GetOwner());
			}
			else
			{
				break;
			}
		}

		// Set base.
		UPrimitiveComponent* OldBase = BasedMovement.MovementBase;
		BasedMovement.MovementBase = NewBaseComponent;
		BasedMovement.BoneName = BoneName;

		if (nullptr != MovementComponent)
		{
			const bool bBaseIsSimulating = NewBaseComponent && NewBaseComponent->IsSimulatingPhysics();
			if (bBaseChanged)
			{
				T4MovementUtil::RemoveTickDependency(MovementComponent->PrimaryComponentTick, OldBase);
				// We use a special post physics function if simulating, otherwise add normal tick prereqs.
				if (!bBaseIsSimulating)
				{
					T4MovementUtil::AddTickDependency(MovementComponent->PrimaryComponentTick, NewBaseComponent);
				}
			}

			if (NewBaseComponent)
			{
				// Update OldBaseLocation/Rotation as those were referring to a different base
				// ... but not when handling replication for proxies (since they are going to copy this data from the replicated values anyway)
				const bool bInBaseReplication = false; // #31 : check it!
				if (!bInBaseReplication)
				{
					// Force base location and relative position to be computed since we have a new base or bone so the old relative offset is meaningless.
					MovementComponent->SaveBaseLocation();
				}

				// Enable PostPhysics tick if we are standing on a physics object, as we need to to use post-physics transforms
				MovementComponent->PostPhysicsTickFunction.SetTickFunctionEnable(bBaseIsSimulating);
			}
			else
			{
				BasedMovement.BoneName = NAME_None; // None, regardless of whether user tried to set a bone name, since we have no base component.
				BasedMovement.bRelativeRotation = false;
				MovementComponent->CurrentFloor.Clear();
				MovementComponent->PostPhysicsTickFunction.SetTickFunctionEnable(false);
			}

			if (GetLocalRole() == ROLE_Authority || GetLocalRole() == ROLE_AutonomousProxy)
			{
				BasedMovement.bServerHasBaseComponent = (BasedMovement.MovementBase != nullptr); // Also set on proxies for nicer debugging.
				T4_LOG(
					Verbose,
					TEXT("Setting base on %s for '%s' to '%s'"),
					GetLocalRole() == ROLE_Authority ? TEXT("Server") : TEXT("AutoProxy"),
					*GetName().ToString(),
					*GetFullNameSafe(NewBaseComponent)
				);
			}
			else
			{
				T4_LOG(
					Verbose,
					TEXT("Setting base on Client for '%s' to '%s'"),
					*GetName().ToString(),
					*GetFullNameSafe(NewBaseComponent)
				);
			}
		}
	}
}

void AT4MovableCharacterObject::SaveRelativeBasedMovement(
	const FVector& NewRelativeLocation,
	const FRotator& NewRotation,
	bool bRelativeRotation
)
{
	checkSlow(BasedMovement.HasRelativeLocation());
	BasedMovement.Location = NewRelativeLocation;
	BasedMovement.Rotation = NewRotation;
	BasedMovement.bRelativeRotation = bRelativeRotation;
}

#if !UE_BUILD_SHIPPING
void AT4MovableCharacterObject::DebugDrawObjectInfo()
{
	check(nullptr != CapsuleComponent);
	FT4GameObjectDebugInfo& CurrentDebugInfo = GetDebugInfo();

	const bool bShowCapsule 
		= (CurrentDebugInfo.DebugBitFlags & ET4EngineDebugFlag::Debug_Object_Capsule_Bit) ? true : false;

	if (bShowCapsule || 0 != T4MovableCharacterObjectCVars::DebugCharCapsule)
	{
		DrawDebugCapsule(
			GetWorld(),
			GetActorLocation(),
			CapsuleComponent->GetScaledCapsuleHalfHeight(),
			CapsuleComponent->GetScaledCapsuleRadius(),
			FQuat::Identity,
			FColor(100, 255, 100),
			false
		);
	}
}

void AT4MovableCharacterObject::DebugDrawServerLocation() // #52
{
	check(nullptr != CapsuleComponent);
	if (0 != T4MovableCharacterObjectCVars::DebugServerLocation)
	{
#if WITH_EDITOR
		const float DefaultRadius = CapsuleComponent->GetScaledCapsuleRadius() * 0.5f;
		const FColor DebugColor1 = FColor(255, 255, 10);
		const FColor DebugColor2 = FColor(255, 10, 10);
		FColor BoxColor = DebugColor1;
		FColor ArrowColor = DebugColor2;
		if (HasPlayer())
		{
			BoxColor = DebugColor2;
			ArrowColor = DebugColor1;
		}
		DrawDebugBox(
			GetWorld(),
			SyncLocationForServer,
			FVector(DefaultRadius),
			SyncDirectionForServer.ToOrientationQuat(),
			BoxColor,
			false
		);
		const FVector StartLine = SyncLocationForServer + (DefaultRadius * SyncDirectionForServer);
		const FVector EndLine = SyncLocationForServer + ((DefaultRadius * 3.0f) * SyncDirectionForServer);
		DrawDebugDirectionalArrow(
			GetWorld(),
			StartLine,
			EndLine,
			25.0f,
			ArrowColor,
			false
		);
		DrawDebugDirectionalArrow(
			GetWorld(),
			StartLine + FVector(0.0f, 0.0f, DefaultRadius),
			EndLine + FVector(0.0f, 0.0f, DefaultRadius),
			25.0f,
			ArrowColor,
			false
		);
#endif
	}
}
#endif // !UE_BUILD_SHIPPING

bool AT4MovableCharacterObject::ExecuteMoveAsyncToAction(const FT4MoveAsyncAction& InAction)
{
	check(ET4ActionType::MoveAsync == InAction.ActionType);
	check(nullptr != MovementComponent);
	if (IsMovementLocked())
	{
		return false; // #76
	}
	{
		// #111 : Stance/SubStance 전환시 이동이 오면 즉시 적용 처리해준다. (즉, 키입력이 없을때만 보여주는 것!)
		FT4ActionTaskControl& ActionTaskControlRef = GetActionTaskControl();
		ActionTaskControlRef.Flush(ET4ActionType::SubStance);
		ActionTaskControlRef.Flush(ET4ActionType::Stance);
	}
#if USES_ASYNC_MOVE_ISSUE_52
	// #33 : 선이동!
	AddMovementInput(InAction.MoveDirection);
	if (T4Const_EmptyYawAngle != InAction.HeadYawAngle)
	{
		// #44 : LockOn 일 경우 이동방향과 틀려진다.
		FT4TurnAction RotationAction;
		RotationAction.bTransient = true; // Replay 녹화에서 제외. 상위 Action 이 녹화되어서 저장할 필요가 없다.
		RotationAction.ActionKey = T4ActionOverrideRotationPrimaryKey;
		RotationAction.TargetType = ET4TargetType::TargetCustom;
		RotationAction.RotationYawRate = GetPropertyConst().RotationYawRate; // #46
		RotationAction.TargetYawAngle = InAction.HeadYawAngle;
		ExecuteActionNode(&RotationAction, nullptr);
	}
#endif
	return true;
}

bool AT4MovableCharacterObject::ExecuteMoveSyncToAction(const FT4MoveSyncAction& InAction)
{
	check(ET4ActionType::MoveSync == InAction.ActionType);
	check(nullptr != MovementComponent);

	// #111 : Stance/SubStance 전환시 이동이 오면 즉시 적용 처리해준다. (즉, 키입력이 없을때만 보여주는 것!)
	FT4ActionTaskControl& ActionTaskControlRef = GetActionTaskControl();
	ActionTaskControlRef.Flush(ET4ActionType::SubStance);
	ActionTaskControlRef.Flush(ET4ActionType::Stance);

	// #52
	const float UpdateMoveSpeed = InAction.MoveVelocity.Size();

#if WITH_EDITOR
	{
		// #52
		SyncLocationForServer = InAction.ServerNavPoint; // #52
		SyncDirectionForServer = InAction.ServerDirection; // #52
	}
#endif

	GetProperty().SetMoveSpeed(ET4MoveMode::Current, UpdateMoveSpeed); // #52

#if USES_ASYNC_MOVE_ISSUE_52
	if (HasPlayer())
	{
		// #33, #40 : Player 는 선이동을 하였음으로 이동은 제외. 추후, 검증에 사용해야 함!
		return true;
	}
#endif

	MovementComponent->RequestDirectMove(InAction.MoveVelocity, InAction.bForceMaxSpeed); // #50
	if (T4Const_EmptyYawAngle != InAction.HeadYawAngle)
	{
		// #44, #40
		FT4TurnAction RotationAction;
		RotationAction.bTransient = true; // Replay 녹화에서 제외. 상위 Action 이 녹화되어서 저장할 필요가 없다.
		RotationAction.ActionKey = T4ActionOverrideRotationPrimaryKey;
		RotationAction.TargetType = ET4TargetType::TargetCustom;
		RotationAction.RotationYawRate = GetPropertyConst().RotationYawRate; // #46
		RotationAction.TargetYawAngle = InAction.HeadYawAngle;
		ExecuteActionNode(&RotationAction, nullptr);
	}
	return true;
}

bool AT4MovableCharacterObject::ExecuteJumpToAction(const FT4JumpAction& InAction)
{
	check(ET4ActionType::Jump == InAction.ActionType);
	check(nullptr != MovementComponent);
	MovementComponent->DoJump(InAction.JumpVelocity.Z);
	if (nullptr != AnimControl)
	{
		AnimControl->DoJump(InAction.JumpVelocity);
	}
	return true;
}

bool AT4MovableCharacterObject::ExecuteRollToAction(const FT4RollAction& InAction) // #46
{
	check(ET4ActionType::Roll == InAction.ActionType);
	check(nullptr != MovementComponent);
	MovementComponent->DoRoll(InAction.RollVelocity.Z);
	if (nullptr != AnimControl)
	{
		AnimControl->DoRoll(InAction.RollVelocity);
	}
	return true;
}

bool AT4MovableCharacterObject::ExecuteTeleportToAction(const FT4TeleportAction& InAction)
{
	check(ET4ActionType::Teleport == InAction.ActionType);
	check(nullptr != MovementComponent);
	FVector TeleportLocation = InAction.TargetLocation;
	TeleportLocation.Z += GetPropertyConst().HalfHeight;
	SetActorLocation(TeleportLocation);
	return true;
}

bool AT4MovableCharacterObject::ExecuteMoveStopAction(const FT4MoveStopAction& InAction) // #52
{
	check(ET4ActionType::MoveStop == InAction.ActionType);
	check(nullptr != MovementComponent);
#if WITH_EDITOR
	if (T4Const_EmptyYawAngle != InAction.HeadYawAngle)
	{
		// #52
		SyncLocationForServer = InAction.StopLocation; // #52
		FRotator CurrRotation = GetRotation();
		CurrRotation.Yaw = InAction.HeadYawAngle;
		SyncDirectionForServer = CurrRotation.Vector(); // #52
	}
#endif
	if (InAction.bSyncLocation)
	{
		ensure(false); // TODO
	}
	else
	{
		MovementComponent->StopMovementKeepPathing();
	}
	if (T4Const_EmptyYawAngle != InAction.HeadYawAngle)
	{
		// #44, #40, #38
		FT4TurnAction RotationAction;
		RotationAction.bTransient = true; // Replay 녹화에서 제외. 상위 Action 이 녹화되어서 저장할 필요가 없다.
		RotationAction.ActionKey = T4ActionOverrideRotationPrimaryKey;
		RotationAction.TargetType = ET4TargetType::TargetCustom;
		RotationAction.RotationYawRate = GetPropertyConst().RotationYawRate; // #46
		RotationAction.TargetYawAngle = InAction.HeadYawAngle;
		ExecuteActionNode(&RotationAction, nullptr);
	}
	return true;
}

bool AT4MovableCharacterObject::ExecuteMoveSpeedSyncAction(
	const FT4MoveSpeedSyncAction& InAction
) // #52
{
	check(ET4ActionType::MoveSpeedSync == InAction.ActionType);
	check(nullptr != MovementComponent);
	MovementComponent->MaxMoveSpeed = InAction.MoveSpeed;
	GetProperty().SetMoveSpeed(ET4MoveMode::Sync, InAction.MoveSpeed); // #52
	return true;
}

bool AT4MovableCharacterObject::ExecuteAimAction(
	const FT4AimAction& InAction
) // #113
{
	check(ET4ActionType::Aim == InAction.ActionType);

	FT4GameObjectState& ObjState = GetState();
	if (InAction.bClear)
	{
		ObjState.bAiming = false;
		ObjState.AimTargetDirection = FVector::ZeroVector;
		return true;
	}

	if (nullptr != MovementComponent)
	{
		if (T4Const_EmptyYawAngle != InAction.HeadYawAngle)
		{
			// #44, #40, #38
			FT4TurnAction RotationAction;
			RotationAction.bTransient = true; // Replay 녹화에서 제외. 상위 Action 이 녹화되어서 저장할 필요가 없다.
			RotationAction.ActionKey = T4ActionOverrideRotationPrimaryKey;
			RotationAction.TargetType = ET4TargetType::TargetCustom;
			RotationAction.RotationYawRate = GetPropertyConst().RotationYawRate; // #46
			RotationAction.TargetYawAngle = InAction.HeadYawAngle;
			ExecuteActionNode(&RotationAction, nullptr);
		}
	}

	ObjState.bAiming = true;
	ObjState.AimTargetDirection = InAction.TargetDirection;

	return true;
}

bool AT4MovableCharacterObject::ExecuteLockOnAction(const FT4LockOnAction& InAction)
{
	check(ET4ActionType::LockOn == InAction.ActionType);

	FT4GameObjectProperty& Property = GetProperty();
	Property.bIsLockOn = InAction.bSetLocked;

#if 0
	// #44 : LockOn 시 View 방향 회전은 이동 Action 을 통해 직접 처리하도록 변경함 (FT4MoveAsyncAction)
	//bUseControllerRotationYaw = Property.bIsLockOn; // 카메라와 캐릭터 방향 일치!
#endif

	if (nullptr != MovementComponent)
	{
		if (T4Const_EmptyYawAngle != InAction.HeadYawAngle)
		{
			// #44, #40, #38
			FT4TurnAction RotationAction;
			RotationAction.bTransient = true; // Replay 녹화에서 제외. 상위 Action 이 녹화되어서 저장할 필요가 없다.
			RotationAction.ActionKey = T4ActionOverrideRotationPrimaryKey;
			RotationAction.TargetType = ET4TargetType::TargetCustom;
			RotationAction.RotationYawRate = GetPropertyConst().RotationYawRate; // #46
			RotationAction.TargetYawAngle = InAction.HeadYawAngle;
			ExecuteActionNode(&RotationAction, nullptr);
		}
	}

	if (!Property.bIsLockOn)
	{
		// #40 : LockOn 은 LockOn Reset 을 동일 Action 을 사용함으로 Clear 를 별도로 처리해준다.
		FT4StopAction NewAction;
		NewAction.bTransient = true; // Replay 녹화에서 제외. 상위 Action 이 녹화되어서 저장할 필요가 없다.
		NewAction.ActionKey = InAction.ActionKey;
		NewAction.StartTimeSec = 0.0f;
		ExecuteActionNode(&NewAction, nullptr);
	}
	return true;
}

bool AT4MovableCharacterObject::ExecuteStanceAction(
	const FT4StanceAction& InAction
) // #73
{
	GetActionTaskControl().Flush(ET4ActionType::SubStance); // Stance 가 바뀌면 SubStance 도 즉시 Flush
	GetActionTaskControl().Flush(ET4ActionType::Stance); // 이전 처리 Flush
	if (InAction.bOnlyFlush)
	{
		return true; // #116 : 스탠스 전환중이면 강제로 Flush 해준다. (이후 필요하다면 별도 Action 으로 뺄 것!)
	}
	if (InAction.bImmediate || !IsLoaded())
	{
		ChangeStance(InAction.StanceName, false); // #111
		return true;
	}
	bool bResult = GetActionTaskControl().Play(&InAction);
	return bResult;
}
 
bool AT4MovableCharacterObject::ExecuteSubStanceAction(
	const FT4SubStanceAction& InAction
) // #106
{
	GetActionTaskControl().Flush(ET4ActionType::SubStance); // 이전 처리 Flush
	const bool bSkipAction = GetActionTaskControl().IsPending(ET4ActionType::Stance); // #116 : 만약, stance 전환중이라면 연출은 생락한다.
	if (InAction.bImmediate || bSkipAction || !IsLoaded())
	{
		ChangeSubStance(InAction.SubStanceName, false); // #111
		return true;
	}
	bool bResult = GetActionTaskControl().Play(&InAction);
	return bResult;
}

bool AT4MovableCharacterObject::ExecuteCostumeAction(
	const FT4CostumeAction& InAction
) // #72
{
	check(ET4ActionType::Costume == InAction.ActionType);

	if (ET4EntityCharacterMeshType::Composite != MeshType)
	{
		T4_LOG(
			Error,
			TEXT("Exchange failed. CharacterMeshType Composite only")
		);
		return false;
	}

	FSoftObjectPath PartAssetPath;
	if (InAction.bClearDefault)
	{
		if (!CharacterEntityPtr.IsValid())
		{
			T4_LOG(
				Error,
				TEXT("CharacterEntityAsset Not Found")
			);
			return false;
		}
		if (MasterPosePartName == NAME_None)
		{
			MasterPosePartName = InAction.TargetPartsName; // WARN : bMaster 체크가 없다면 첫번째를 사용한다!
		}
		if (MasterPosePartName == InAction.TargetPartsName)
		{
#if WITH_EDITOR
			// TODO : MasterPose Part 가 삭제되면 재 생성해주어야 한다. 툴용이니, 일단 그냥 리로드 해주겠다.
			FT4EditorAction ReloadAction;
			ReloadAction.EditorActionType = ET4EditorAction::ReloadObject;
			ExecuteEditorAction(ReloadAction);
#endif
			return true;
		}
		const FT4EntityCharacterCompositeMeshData& MeshData = CharacterEntityPtr->CopmpositeMeshData;
		if (!MeshData.DefaultPartsData.Contains(InAction.TargetPartsName))
		{
#if WITH_EDITOR
			if (MasterPoseModularMeshes.Contains(InAction.TargetPartsName))
			{
				// 에디터 용, 삭제된 경우...
				USkinnedMeshComponent* RemoveMeshComponent = MasterPoseModularMeshes[InAction.TargetPartsName];
				check(nullptr != RemoveMeshComponent);
				RemoveMeshComponent->UnregisterComponent();
				RemoveMeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
				return true;
			}
#endif
			T4_LOG(
				Error,
				TEXT("TargetPartName '%s' Not Found"),
				*(InAction.TargetPartsName.ToString())
			);
			return false;
		}
		const FT4EntityCharacterCompositePartMeshData& CompositePartMeshData = MeshData.DefaultPartsData[InAction.TargetPartsName];
		if (CompositePartMeshData.CostumeEntityAsset.IsNull())
		{
			T4_LOG(
				Error,
				TEXT("CostumeEntityAsset (%s) Not Found"),
				*(CompositePartMeshData.CostumeEntityAsset.ToString())
			);
			return false;
		}
		// #35 에서 EntityAsset 은 초기에 모두 로딩시켜 두었기 때문에 믿고 LoadSynchronous 로 얻는다.
		const UT4CostumeEntityAsset* CostumeEntityAsset = CompositePartMeshData.CostumeEntityAsset.LoadSynchronous();
		check(nullptr != CostumeEntityAsset);

		AddCompositeOverrideMaterialAsyncLoading(
			InAction.TargetPartsName, 
			CostumeEntityAsset->MeshData.OverrideMaterialData, 
			TEXT("ExecuteCostumeAction")
		); // #80

		PartAssetPath = CostumeEntityAsset->MeshData.SkeletalMeshAsset.ToSoftObjectPath();
	}
	else
	{
		if (InAction.CostumeEntityAsset.IsNull())
		{
			T4_LOG(
				Error,
				TEXT("CostumeEntityAsset (%s) Not Found"),
				*(InAction.CostumeEntityAsset.ToString())
			);
			return false;
		}
		// #35 에서 EntityAsset 은 초기에 모두 로딩시켜 두었기 때문에 믿고 LoadSynchronous 로 얻는다.
		const UT4CostumeEntityAsset* CostumeEntityAsset = InAction.CostumeEntityAsset.LoadSynchronous();
		check(nullptr != CostumeEntityAsset);

		AddCompositeOverrideMaterialAsyncLoading(
			InAction.TargetPartsName,
			CostumeEntityAsset->MeshData.OverrideMaterialData,
			TEXT("ExecuteCostumeAction")
		); // #80

		PartAssetPath = CostumeEntityAsset->MeshData.SkeletalMeshAsset.ToSoftObjectPath();
	}

	FT4SkeletalMeshLoadInfo& NewMeshLoadInfo = PendingMeshLoadInfos.AddDefaulted_GetRef();
	NewMeshLoadInfo.PartName = InAction.TargetPartsName;
	NewMeshLoadInfo.SkeletalMeshLoader.Load(
		PartAssetPath,
		false,
		TEXT("ExecuteCostumeAction")
	);
	bMeshAsyncLoading = true;
	return true;
}

bool AT4MovableCharacterObject::ExecuteHitAction(const FT4HitAction& InAction) // #76
{
#if WITH_EDITOR
	if (ET4LayerType::LevelEditor == GetLayerType()) // Level Editor 에서는 물리를 사용하지 못한다.
	{
		return false;
	}
#endif
	bool bResult = GetActionTaskControl().Play(&InAction);
	return bResult;
}

bool AT4MovableCharacterObject::ExecuteDieAction(const FT4DieAction& InAction) // #76
{
#if WITH_EDITOR
	if (ET4LayerType::LevelEditor == GetLayerType()) // Level Editor 에서는 물리를 사용하지 못한다.
	{
		return false;
	}
#endif
	bool bResult = GetActionTaskControl().Play(&InAction);
	if (bResult)
	{
		if (nullptr != AnimControl)
		{
			AnimControl->DoVoid();
		}
	}
	return bResult;
}

bool AT4MovableCharacterObject::ExecuteResurrectAction(const FT4ResurrectAction& InAction) // #76
{
	bool bResult = GetActionTaskControl().Play(&InAction);
	if (bResult)
	{
		if (nullptr != AnimControl)
		{
			AnimControl->DoVoid();
		}
	}
	return bResult;
}

#if WITH_EDITOR
void AT4MovableCharacterObject::RecreateAll() // #76
{
	FT4SpawnObjectAction ReCreateAction;
	ReCreateAction.EntityType = EntityKey.Type;
	ReCreateAction.EntityAssetPath = TSoftObjectPtr<UT4CharacterEntityAsset>(
		CharacterEntityPtr.Get()
	).ToSoftObjectPath();
	ReCreateAction.StanceName = StanceNameSelected; // #73
	ReCreateAction.SubStanceName = SubStanceNameSelected; // #116

	ResetSimulatePhysics();
	ResetOpacityState();
	ResetDynamicMaterialInstances(); // #78
	ResetLoadComplated();
	Reset();

	bool bResult = Create(&ReCreateAction);
	if (bResult)
	{

	}
}

void AT4MovableCharacterObject::ReloadOverrideMaterialAsyncLoading() // #80
{
	if (ET4EntityCharacterMeshType::FullBody == MeshType)
	{
		SetFullbodyOverrideMaterialAsyncLoading();
	}
	else if (ET4EntityCharacterMeshType::Composite == MeshType)
	{
		check(!CharacterEntityPtr.IsValid());
		const FT4EntityCharacterCompositeMeshData& MeshData = CharacterEntityPtr->CopmpositeMeshData;
		for (TMap<FName, FT4EntityCharacterCompositePartMeshData>::TConstIterator It(MeshData.DefaultPartsData); It; ++It)
		{
			const FName PartName = It->Key;
			const FT4EntityCharacterCompositePartMeshData& CompositePartMeshData = It->Value;
			if (CompositePartMeshData.CostumeEntityAsset.IsNull())
			{
				continue;
			}

			// #35 에서 EntityAsset 은 초기에 모두 로딩시켜 두었기 때문에 믿고 LoadSynchronous 로 얻는다.
			UT4CostumeEntityAsset* CostumeEntityAsset = CompositePartMeshData.CostumeEntityAsset.LoadSynchronous();
			check(nullptr != CostumeEntityAsset);

			AddCompositeOverrideMaterialAsyncLoading(
				(MasterPosePartName == PartName) ? NAME_None : PartName,
				CostumeEntityAsset->MeshData.OverrideMaterialData, 
				TEXT("ReloadOverrideMaterialAsyncLoading")
			); // #80
		}
	}
}

bool AT4MovableCharacterObject::ExecuteEditorAction(const FT4EditorAction& InAction)
{
	// #37
	check(ET4ActionType::Editor == InAction.ActionType);

	switch (InAction.EditorActionType)
	{
		case ET4EditorAction::ReloadAttributes:
			SetEntityAttributes(); // #81
			break;

		case ET4EditorAction::ReloadAnimSetSkill: // #81
		case ET4EditorAction::ReloadAnimSetOverlay: // #81
		case ET4EditorAction::ReloadAnimSetDefault: // #81
		case ET4EditorAction::ReloadAnimSetBlendSpace: // #81
			// #81 : 필요하다면 해당 AnimSet 만 업데이트 처리 추가. 현재는 SaveAll 시 ReloadObject 가 불리게 되어 있다.
			break;

		case ET4EditorAction::ReloadObject: // #37
			RecreateAll();
			break;

		case ET4EditorAction::RestoreReaction: // #76
			GetActionTaskControl().EditorRestoreReaction(); 
			break;

		case ET4EditorAction::UpdateOverrideMaterials: // #80
			ReloadOverrideMaterialAsyncLoading();
			break;

		case ET4EditorAction::PlayTagSet: // #81
			ActivePlayTag(InAction.PlayTagName, InAction.PlayTagType, InAction.ActionKey); // #74, #81
			break;
			
		case ET4EditorAction::PlayTagClear: // #81
			InactivePlayTag(InAction.PlayTagType, InAction.ActionKey); // #74, #81
			break;

		default:
			{
				T4_LOG(
					Error,
					TEXT("Unknown Editor Action type '%u'"),
					uint8(InAction.EditorActionType)
				);
			}
			break;
	}

	return true;
}
#endif