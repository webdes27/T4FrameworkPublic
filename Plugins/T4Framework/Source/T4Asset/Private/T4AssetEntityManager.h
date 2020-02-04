// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/Entity/T4Entity.h"

/**
  * #35 
 */
class FT4EntityManager : public IT4EntityManager
{
public:
	FT4EntityManager();
	virtual ~FT4EntityManager();

	bool Initialize(const TArray<FString>& InEntityPaths) override;
	void Finalize() override;

	void Reset() override;

	bool GetEntities(ET4EntityType InEntityType, TArray<UT4EntityAsset*>& OutEntities) override; // #87

	// #T4_ADD_ENTITY_TAG
	const UT4EntityAsset* GetEntity(const FT4EntityKey& InKey) override;
	const UT4EntityAsset* GetEntity(const FSoftObjectPath& InPath) override;

	const UT4MapEntityAsset* GetMapEntity(const FT4EntityKey& InKey) override;
	const UT4MapEntityAsset* GetMapEntity(const FSoftObjectPath& InPath) override;

	const UT4CharacterEntityAsset* GetActorEntity(const FT4EntityKey& InKey) override;
	const UT4CharacterEntityAsset* GetActorEntity(const FSoftObjectPath& InPath) override;
	
	const UT4PropEntityAsset* GetPropEntity(const FT4EntityKey& InKey) override;
	const UT4PropEntityAsset* GetPropEntity(const FSoftObjectPath& InPath) override;

	const UT4ItemEntityAsset* GetItemEntity(const FT4EntityKey& InKey) override; // #37
	const UT4ItemEntityAsset* GetItemEntity(const FSoftObjectPath& InPath) override; // #37

	const UT4CostumeEntityAsset* GetCostumeEntity(const FT4EntityKey& InKey) override; // #37
	const UT4CostumeEntityAsset* GetCostumeEntity(const FSoftObjectPath& InPath) override; // #37

	const UT4WeaponEntityAsset* GetWeaponEntity(const FT4EntityKey& InKey) override;
	const UT4WeaponEntityAsset* GetWeaponEntity(const FSoftObjectPath& InPath) override;

	const UT4ZoneEntityAsset* GetZoneEntity(const FT4EntityKey& InKey) override; // #94
	const UT4ZoneEntityAsset* GetZoneEntity(const FSoftObjectPath& InPath) override; // #94

private:
	template <class T>
	bool LoadEntityAssets(const TArray<FString>& InEntityPaths);

	const UT4EntityAsset* LoadEntityAsset(const FSoftObjectPath& InPath);

private:
	bool bInitialized;
	int32 InitCount;

	TMap<FSoftObjectPath, UT4EntityAsset*> EntityPathMap;
	TMap<FT4EntityKey, UT4EntityAsset*> EntityKeyMap;
	TArray<UT4EntityAsset*> CachedEntityAssets;
};
