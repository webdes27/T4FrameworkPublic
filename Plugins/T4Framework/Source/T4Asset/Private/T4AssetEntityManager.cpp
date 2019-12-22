// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4AssetEntityManager.h"

#include "Classes/Entity/T4EntityAssetMinimal.h"

#include "AssetData.h"
#include "Engine/ObjectLibrary.h"

#include "T4AssetInternal.h"

/**
  * #35
 */
FT4EntityManager::FT4EntityManager()
	: bInitialized(false)
	, InitCount(0)
{
}

FT4EntityManager::~FT4EntityManager()
{
}

bool FT4EntityManager::Initialize(const TArray<FString>& InEntityPaths)
{
	InitCount++;
	if (bInitialized)
	{
		return true;
	}
	{
		// #35 : 초기 로딩시 EntityAsset 은 모두 읽어들인다.
		//       메타 데이터라 크지 않고, 매번 로드하기엔 가성비가 떨어진다.
		//		 단, Editor 에서는 GetEntity 을 통해 새로 생성된 Entity 를 컨테이너에 추가하는 처리가 있다.
		// #T4_ADD_ENTITY_TAG
		LoadEntityAssets<UT4MapEntityAsset>(InEntityPaths);
		LoadEntityAssets<UT4CharacterEntityAsset>(InEntityPaths);
		LoadEntityAssets<UT4PropEntityAsset>(InEntityPaths);
		LoadEntityAssets<UT4CostumeEntityAsset>(InEntityPaths);
		LoadEntityAssets<UT4WeaponEntityAsset>(InEntityPaths);
		LoadEntityAssets<UT4ZoneEntityAsset>(InEntityPaths); // #94
		if (0 >= EntityPathMap.Num())
		{
			return false;
		}
	}
	bInitialized = true;
	return true;
}

void FT4EntityManager::Finalize()
{
	if (!bInitialized)
	{
		return;
	}
	InitCount--;
	if (0 < InitCount)
	{
		return;
	}
	Reset();
	bInitialized = false;
}

void FT4EntityManager::Reset()
{
	for (UT4EntityAsset* EntityAsset : CachedEntityAssets)
	{
		EntityAsset->RemoveFromRoot();
	}
	CachedEntityAssets.Empty();
	EntityPathMap.Empty();
	EntityKeyMap.Empty();
}

bool FT4EntityManager::GetEntities(
	ET4EntityType InEntityType,
	TArray<UT4EntityAsset*>& OutEntities
) // #87
{
	for (UT4EntityAsset* EntityAsset : CachedEntityAssets)
	{
		check(nullptr != EntityAsset);
		if (ET4EntityType::None == InEntityType || EntityAsset->GetEntityType() == InEntityType)
		{
			OutEntities.Add(EntityAsset);
		}
	}
	return (0 < CachedEntityAssets.Num()) ? true : false;
}

const UT4EntityAsset* FT4EntityManager::GetEntity(const FT4EntityKey& InKey)
{
	if (!InKey.IsValid())
	{
		return nullptr;
	}
	if (EntityKeyMap.Contains(InKey))
	{
		return EntityKeyMap[InKey];
	}
#if WITH_EDITOR
	const FSoftObjectPath LoadPath = InKey.Value.ToString();
	const UT4EntityAsset* LoadedAsset = LoadEntityAsset(LoadPath);
	if (nullptr == LoadedAsset)
	{
		UE_LOG(
			LogT4Asset,
			Error,
			TEXT("FT4EntityManager : Failed to load Entity by '%s'"),
			*(InKey.ToString())
		);
		return nullptr;
	}
	return LoadedAsset;
#else
	// #35 : 클라이언트 일 경우 초기화 시 모두 캐시해서 SyncLoad 할 수 있도록 처리하기 때문에
	//       여기에 걸리면 assert 조건이다!
	check(false);
	return nullptr;
#endif
}

const UT4EntityAsset* FT4EntityManager::GetEntity(const FSoftObjectPath& InPath)
{
	if (EntityPathMap.Contains(InPath))
	{
		return EntityPathMap[InPath];
	}
#if WITH_EDITOR
	const UT4EntityAsset* LoadedAsset = LoadEntityAsset(InPath);
	if (nullptr == LoadedAsset)
	{
		UE_LOG(
			LogT4Asset,
			Error,
			TEXT("FT4EntityManager : Failed to load Entity by '%s'"),
			*(InPath.ToString())
		);
		return nullptr;
	}
	return LoadedAsset;
#else
	// #35 : 클라이언트 일 경우 초기화 시 모두 캐시해서 SyncLoad 할 수 있도록 처리하기 때문에
	//       여기에 걸리면 assert 조건이다!
	check(false);
	return nullptr;
#endif
}

// #T4_ADD_ENTITY_TAG
const UT4MapEntityAsset* FT4EntityManager::GetMapEntity(const FT4EntityKey& InKey)
{
	const UT4EntityAsset* EntityAsset = GetEntity(InKey);
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	if (ET4EntityType::Map != EntityAsset->GetEntityType())
	{
		return nullptr;
	}
	return Cast<UT4MapEntityAsset>(EntityAsset);
}

const UT4MapEntityAsset* FT4EntityManager::GetMapEntity(const FSoftObjectPath& InPath)
{
	const UT4EntityAsset* EntityAsset = GetEntity(InPath);
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	if (ET4EntityType::Map != EntityAsset->GetEntityType())
	{
		return nullptr;
	}
	return Cast<UT4MapEntityAsset>(EntityAsset);
}

const UT4CharacterEntityAsset* FT4EntityManager::GetActorEntity(const FT4EntityKey& InKey)
{
	const UT4EntityAsset* EntityAsset = GetEntity(InKey);
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	if (ET4EntityType::Character != EntityAsset->GetEntityType())
	{
		return nullptr;
	}
	return Cast<UT4CharacterEntityAsset>(EntityAsset);
}

const UT4CharacterEntityAsset* FT4EntityManager::GetActorEntity(const FSoftObjectPath& InPath)
{
	const UT4EntityAsset* EntityAsset = GetEntity(InPath);
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	if (ET4EntityType::Character != EntityAsset->GetEntityType())
	{
		return nullptr;
	}
	return Cast<UT4CharacterEntityAsset>(EntityAsset);
}

const UT4PropEntityAsset* FT4EntityManager::GetPropEntity(const FT4EntityKey& InKey)
{
	const UT4EntityAsset* EntityAsset = GetEntity(InKey);
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	if (ET4EntityType::Prop != EntityAsset->GetEntityType())
	{
		return nullptr;
	}
	return Cast<UT4PropEntityAsset>(EntityAsset);
}

const UT4PropEntityAsset* FT4EntityManager::GetPropEntity(const FSoftObjectPath& InPath)
{
	const UT4EntityAsset* EntityAsset = GetEntity(InPath);
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	if (ET4EntityType::Prop != EntityAsset->GetEntityType())
	{
		return nullptr;
	}
	return Cast<UT4PropEntityAsset>(EntityAsset);
}

const UT4ItemEntityAsset* FT4EntityManager::GetItemEntity(const FT4EntityKey& InKey)
{
	const UT4EntityAsset* EntityAsset = GetEntity(InKey);
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	const ET4EntityType CurrEntityType = EntityAsset->GetEntityType();
	if (ET4EntityType::Costume != CurrEntityType && ET4EntityType::Weapon != CurrEntityType)
	{
		return nullptr;
	}
	return Cast<UT4ItemEntityAsset>(EntityAsset);
}

const UT4ItemEntityAsset* FT4EntityManager::GetItemEntity(const FSoftObjectPath& InPath)
{
	const UT4EntityAsset* EntityAsset = GetEntity(InPath);
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	const ET4EntityType CurrEntityType = EntityAsset->GetEntityType();
	if (ET4EntityType::Costume != CurrEntityType && ET4EntityType::Weapon != CurrEntityType)
	{
		return nullptr;
	}
	return Cast<UT4ItemEntityAsset>(EntityAsset);
}

const UT4CostumeEntityAsset* FT4EntityManager::GetCostumeEntity(const FT4EntityKey& InKey)
{
	const UT4EntityAsset* EntityAsset = GetEntity(InKey);
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	if (ET4EntityType::Costume != EntityAsset->GetEntityType())
	{
		return nullptr;
	}
	return Cast<UT4CostumeEntityAsset>(EntityAsset);
}

const UT4CostumeEntityAsset* FT4EntityManager::GetCostumeEntity(const FSoftObjectPath& InPath)
{
	const UT4EntityAsset* EntityAsset = GetEntity(InPath);
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	if (ET4EntityType::Costume != EntityAsset->GetEntityType())
	{
		return nullptr;
	}
	return Cast<UT4CostumeEntityAsset>(EntityAsset);
}

const UT4WeaponEntityAsset* FT4EntityManager::GetWeaponEntity(const FT4EntityKey& InKey)
{
	const UT4EntityAsset* EntityAsset = GetEntity(InKey);
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	if (ET4EntityType::Weapon != EntityAsset->GetEntityType())
	{
		return nullptr;
	}
	return Cast<UT4WeaponEntityAsset>(EntityAsset);
}

const UT4WeaponEntityAsset* FT4EntityManager::GetWeaponEntity(const FSoftObjectPath& InPath)
{
	const UT4EntityAsset* EntityAsset = GetEntity(InPath);
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	if (ET4EntityType::Weapon != EntityAsset->GetEntityType())
	{
		return nullptr;
	}
	return Cast<UT4WeaponEntityAsset>(EntityAsset);
}

const UT4ZoneEntityAsset* FT4EntityManager::GetZoneEntity(const FT4EntityKey& InKey) // #94
{
	const UT4EntityAsset* EntityAsset = GetEntity(InKey);
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	if (ET4EntityType::Zone != EntityAsset->GetEntityType())
	{
		return nullptr;
	}
	return Cast<UT4ZoneEntityAsset>(EntityAsset);
}

const UT4ZoneEntityAsset* FT4EntityManager::GetZoneEntity(const FSoftObjectPath& InPath) // #94
{
	const UT4EntityAsset* EntityAsset = GetEntity(InPath);
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	if (ET4EntityType::Zone != EntityAsset->GetEntityType())
	{
		return nullptr;
	}
	return Cast<UT4ZoneEntityAsset>(EntityAsset);
}

template <class T>
bool FT4EntityManager::LoadEntityAssets(const TArray<FString>& InEntityPaths)
{
	TArray<FAssetData> EntityAssetDatas;
	UObjectLibrary* ObjLibrary = UObjectLibrary::CreateLibrary(T::StaticClass(), false, true);
	ObjLibrary->LoadAssetDataFromPaths(InEntityPaths);
	ObjLibrary->GetAssetDataList(EntityAssetDatas);
	for (const FAssetData& AssetData : EntityAssetDatas)
	{
		FSoftObjectPath AssetObjectPath = AssetData.ToSoftObjectPath();
		if (!EntityPathMap.Contains(AssetObjectPath))
		{
			LoadEntityAsset(AssetObjectPath);
		}
	}
	return true;
}

const UT4EntityAsset* FT4EntityManager::LoadEntityAsset(const FSoftObjectPath& InPath)
{
	FSoftObjectPath LoadPath = InPath;
	check(!EntityPathMap.Contains(LoadPath));
	UT4EntityAsset* LoadedAsset = Cast<UT4EntityAsset>(LoadPath.TryLoad());
	if (nullptr == LoadedAsset)
	{
		UE_LOG(
			LogT4Asset,
			Error,
			TEXT("FT4EntityManager : Failed to load Entity by '%s'"),
			*(InPath.ToString())
		);
		return nullptr;
	}
	FName UniequeKeyPath = LoadedAsset->GetEntityKeyPath();
	FT4EntityKey NewEntityKey(LoadedAsset->GetEntityType(), UniequeKeyPath);
	check(!EntityKeyMap.Contains(NewEntityKey));
	LoadedAsset->AddToRoot();
	CachedEntityAssets.Add(LoadedAsset);
	EntityPathMap.Add(InPath, LoadedAsset);
	EntityKeyMap.Add(NewEntityKey, LoadedAsset);
	return LoadedAsset;
}

static FT4EntityManager GT4EntityManager;
IT4EntityManager* T4AssetEntityManagerGet()
{
	return &GT4EntityManager;
}
