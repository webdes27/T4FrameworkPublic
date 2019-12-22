// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Public/Asset/T4AssetLoader.h"
#include "Public/Asset/T4AssetManager.h"

#include "T4Asset/Classes/Conti/T4ContiAsset.h" // #24
#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h" // #39

#include "Components/DecalComponent.h" // #54
#include "Animation/BlendSpace.h"
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "PhysicsEngine/PhysicsAsset.h" // #76
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "BehaviorTree/BehaviorTree.h" // #50
#include "BehaviorTree/BlackboardData.h" // #50

#include "T4EngineInternal.h"

/**
  *
 */
FT4AssetLoader::FT4AssetLoader()
	: bLoadStart(false)
	, bSyncLoad(false)
	, bBindComplated(false)
	, LoadHandle(nullptr)
	, DebugToken(NAME_None)
{
}

FT4AssetLoader::~FT4AssetLoader()
{
	Reset();
}

void FT4AssetLoader::Reset()
{
	if (nullptr != LoadHandle)
	{
		LoadHandle->OnDestroy();
		LoadHandle = nullptr;
	}
}

void FT4AssetLoader::Load(
	const FSoftObjectPath& InAssetPath,
	bool bInSyncLoad,
	const TCHAR* InDebugString
)
{
	bSyncLoad = bInSyncLoad;
	IT4AssetManager* AssetLoader = T4AssetManagerGet();
	LoadHandle = AssetLoader->RequestAsync(InAssetPath);
	bBindComplated = false;
	DebugToken = InDebugString;
	bLoadStart = true;
}

bool FT4AssetLoader::IsLoadFailed() const
{
	if (nullptr == LoadHandle)
	{
		return true;
	}
	return LoadHandle->IsLoadFailed();
}

bool FT4AssetLoader::IsLoadCompleted() const
{
	if (nullptr == LoadHandle)
	{
		return false;
	}
	return LoadHandle->IsLoadCompleted();
}

bool FT4AssetLoader::IsBinded() const
{
	return bBindComplated;
}

void FT4AssetLoader::SetBinded()
{
	Reset();
	bBindComplated = true;
}

bool FT4AssetLoader::CheckReset() const
{
	return (nullptr == LoadHandle) ? true : false;
}

bool FT4MaterialLoader::Process(UDecalComponent* InDecalComponent)
{
	check(nullptr != LoadHandle);
	check(nullptr != InDecalComponent);
	if (!IsLoadCompleted())
	{
		return false;
	}
	UMaterialInterface* MaterialInterface = GetMaterialInterface();
	if (nullptr == MaterialInterface)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4MaterialLoader : Failed to convert UMaterialInterface. (%s, %s)"),
			*(DebugToken.ToString()),
			*(LoadHandle->GetObjectPath().ToString())
		);
		return false;
	}
	InDecalComponent->SetDecalMaterial(MaterialInterface);
	return true;
}

UMaterialInterface* FT4MaterialLoader::GetMaterialInterface() // #56
{
	check(nullptr != LoadHandle);
	if (!IsLoadCompleted())
	{
		return nullptr;
	}
	UObject* LoadedObject = LoadHandle->GetLoadedAsset();
	if (nullptr == LoadedObject)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4MaterialLoader : Failed to load Material. (%s, %s)"),
			*(DebugToken.ToString()),
			*(LoadHandle->GetObjectPath().ToString())
		);
		return nullptr;
	}
	UMaterialInterface* MaterialInterface = Cast<UMaterialInterface>(LoadedObject);
	if (nullptr == MaterialInterface)
	{
		return nullptr;
	}
	return MaterialInterface;
}

bool FT4AnimBlueprintClassLoader::Process(USkeletalMeshComponent* InMeshComponent)
{
	check(nullptr != LoadHandle);
	check(nullptr != InMeshComponent);
	if (!IsLoadCompleted())
	{
		return false;
	}
	UObject* LoadedBPObject = LoadHandle->GetLoadedAsset();
	if (nullptr == LoadedBPObject)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4AnimBlueprintClassLoader : Failed to load Anim Blueprint. (%s, %s)"),
			*(DebugToken.ToString()),
			*(LoadHandle->GetObjectPath().ToString())
		);
		return false;
	}
	UAnimBlueprintGeneratedClass* AnimBPClass = Cast<UAnimBlueprintGeneratedClass>(LoadedBPObject);
	if (nullptr == AnimBPClass)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4AnimBlueprintClassLoader : Convert to AnimBlueprint (%s, %s) failed."),
			*(DebugToken.ToString()),
			*(LoadHandle->GetObjectPath().ToString())
		);
		return false;
	}

	// #6 : 확인 필요. 원본의 dirty가 일어나는가?
	check(nullptr != InMeshComponent->SkeletalMesh);
	AnimBPClass->TargetSkeleton = InMeshComponent->SkeletalMesh->Skeleton;

	InMeshComponent->SetAnimInstanceClass(Cast<UClass>(AnimBPClass));
	return true;
}

UAnimMontage* FT4AnimMontageLoader::GetAnimMontage() const
{
	if (!IsLoadCompleted())
	{
		return nullptr;
	}
	return Cast<UAnimMontage>(LoadHandle->GetLoadedAsset());
}

UBlendSpaceBase* FT4BlendSpaceLoader::GetBlendSpace() const
{
	if (!IsLoadCompleted())
	{
		return nullptr;
	}
	return Cast<UBlendSpaceBase>(LoadHandle->GetLoadedAsset());
}

bool FT4PhysicsAssetLoader::Process(USkeletalMeshComponent* InMeshComponent) // #76
{
	check(nullptr != LoadHandle);
	check(nullptr != InMeshComponent);
	if (!IsLoadCompleted())
	{
		return false;
	}
	UObject* LoadedObject = LoadHandle->GetLoadedAsset();
	if (nullptr == LoadedObject)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4PhysicsAssetLoader : Failed to load Physics asset. (%s, %s)"),
			*(DebugToken.ToString()),
			*(LoadHandle->GetObjectPath().ToString())
		);
		return false;
	}
	UPhysicsAsset* PhysicsAsset = Cast<UPhysicsAsset>(LoadedObject);
	if (nullptr == PhysicsAsset)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4PhysicsAssetLoader : Failed to convert UPhysicsAsset. (%s, %s)"),
			*(DebugToken.ToString()),
			*(LoadHandle->GetObjectPath().ToString())
		);
		return false;
	}
	InMeshComponent->SetPhysicsAsset(PhysicsAsset);
	return true;
}


bool FT4StaticMeshLoader::Process(UStaticMeshComponent* InMeshComponent)
{
	check(nullptr != LoadHandle);
	check(nullptr != InMeshComponent);
	if (!IsLoadCompleted())
	{
		return false;
	}
	UObject* LoadedObject = LoadHandle->GetLoadedAsset();
	if (nullptr == LoadedObject)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4StaticMeshLoader : Failed to load Static mesh. (%s, %s)"),
			*(DebugToken.ToString()),
			*(LoadHandle->GetObjectPath().ToString())
		);
		return false;
	}
	UStaticMesh* StaticMesh = Cast<UStaticMesh>(LoadedObject);
	if (nullptr == StaticMesh)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4StaticMeshLoader : Failed to convert UStaticMesh. (%s, %s)"),
			*(DebugToken.ToString()),
			*(LoadHandle->GetObjectPath().ToString())
		);
		return false;
	}
	InMeshComponent->SetStaticMesh(StaticMesh);
	return true;
}

bool FT4SkeletalMeshLoader::Process(USkinnedMeshComponent* InMeshComponent)
{
	check(nullptr != LoadHandle);
	check(nullptr != InMeshComponent);
	if (!IsLoadCompleted())
	{
		return false;
	}
	UObject* LoadedObject = LoadHandle->GetLoadedAsset();
	if (nullptr == LoadedObject)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4SkeletalMeshLoader : Failed to load Skeletal mesh. (%s, %s)"),
			*(DebugToken.ToString()),
			*(LoadHandle->GetObjectPath().ToString())
		);
		return false;
	}
	USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(LoadedObject);
	if (nullptr == SkeletalMesh)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4SkeletalMeshLoader : Failed to convert USkeletalMesh. (%s, %s)"),
			*(DebugToken.ToString()),
			*(LoadHandle->GetObjectPath().ToString())
		);
		return false;
	}
	InMeshComponent->SetSkeletalMesh(SkeletalMesh);
	return true;
}

bool FT4ParticleSystemLoader::Process(UParticleSystemComponent* InParticleSystemComponent)
{
	check(nullptr != LoadHandle);
	check(nullptr != InParticleSystemComponent);
	if (!IsLoadCompleted())
	{
		return false;
	}
	UParticleSystem* ParticleSystem = GetParticleSystem();
	if (nullptr == ParticleSystem)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4ParticleSystemLoader : Failed to convert UParticleSystem. (%s, %s)"),
			*(DebugToken.ToString()),
			*(LoadHandle->GetObjectPath().ToString())
		);
		return false;
	}
	InParticleSystemComponent->SetTemplate(ParticleSystem);
	return true;
}

UParticleSystem* FT4ParticleSystemLoader::GetParticleSystem() // #56
{
	check(nullptr != LoadHandle);
	if (!IsLoadCompleted())
	{
		return nullptr;
	}
	UObject* LoadedObject = LoadHandle->GetLoadedAsset();
	if (nullptr == LoadedObject)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4ParticleSystemLoader : Failed to load ParticleSystem. (%s, %s)"),
			*(DebugToken.ToString()),
			*(LoadHandle->GetObjectPath().ToString())
		);
		return nullptr;
	}
	UParticleSystem* ParticleSystem = Cast<UParticleSystem>(LoadedObject);
	if (nullptr == ParticleSystem)
	{
		return nullptr;
	}
	return ParticleSystem;
}

// #24
UT4ContiAsset* FT4ContiAssetLoader::GetContiAsset() const
{
	if (!IsLoadCompleted())
	{
		return nullptr;
	}
	return Cast<UT4ContiAsset>(LoadHandle->GetLoadedAsset());
}

// #39
bool FT4AnimSetAssetLoader::Process()
{
	check(nullptr != LoadHandle);
	if (!IsLoadCompleted())
	{
		return false;
	}
	return true;
}

UT4AnimSetAsset* FT4AnimSetAssetLoader::GetAnimSetAsset() const
{
	if (!IsLoadCompleted())
	{
		return nullptr;
	}
	return Cast<UT4AnimSetAsset>(LoadHandle->GetLoadedAsset());
}

// #50
UBlackboardData* FT4BlackboardAssetLoader::GetBlackboardData() const
{
	if (!IsLoadCompleted())
	{
		return nullptr;
	}
	return Cast<UBlackboardData>(LoadHandle->GetLoadedAsset());
}

// #50
UBehaviorTree* FT4BehaviorTreeAssetLoader::GetBehaviorTree() const
{
	if (!IsLoadCompleted())
	{
		return nullptr;
	}
	return Cast<UBehaviorTree>(LoadHandle->GetLoadedAsset());
}
