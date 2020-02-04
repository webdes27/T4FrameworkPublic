// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4Engine.h"
#include "Public/Action/T4ActionWorldCommands.h"

/**
  * #87
 */
class FT4GameWorld;
class AT4GameObject;
class AT4WorldZoneVolume;
class UT4EnvironmentZoneComponent; // #99
class FT4WorldContainer : public IT4WorldContainer
{
public:
	explicit FT4WorldContainer(FT4GameWorld* InGameWorld);
	virtual ~FT4WorldContainer();

	// IT4WorldContainer
	uint32 GetNumGameObjects() const override;
	bool GetGameObjects(
		ET4SpawnMode InSpawnType,
		TArray<IT4GameObject*>& OutGameObjects
	) override; // #68

	bool HasGameObject(const FT4ObjectID& InObjectID) const override;
	IT4GameObject* FindGameObject(const FT4ObjectID& InObjectID) const override;

	bool QueryNearestGameObjects(
		const FVector& InOriginLocation,
		const float InMaxDistance,
		TArray<IT4GameObject*>& OutObjects
	) override; // #34

	// #54 : 현재는 ClientOnly
	IT4GameObject* PlayClientObject(
		ET4ObjectType InWorldObjectType,
		const FName& InName,
		const FVector& InLocation,
		const FRotator& InRotation,
		const FVector& InScale
	) override; // #68 : 소멸 조건이 되면 스스로 소멸한다.

	IT4GameObject* CreateClientObject(
		ET4ObjectType InWorldObjectType, // #63 : Only World Object
		const FName& InName,
		const FVector& InLocation,
		const FRotator& InRotation,
		const FVector& InScale
	) override;
	bool DestroyClientObject(const FT4ObjectID& InObjectID) override;
	// #54 : 현재는 ClientOnly

public:
	void Reset();

	void ProcessPre(float InDeltaTime); // #34 : OnWorldPreActorTick
	void ProcessPost(float InDeltaTime); // #34 : OnWorldPostActorTick

	bool ProcessSpawnObjectAction(const FT4SpawnObjectAction& InAction);
	bool ProcessDespawnObjectAction(const FT4ObjectID& InObjectID, float InFadeOutTimeSec);

	TArray<AT4GameObject*>& GetSpawnedGameObjects() { return SpawnedGameObjects; }
	bool GetStaticWorldZoneVolumes(TArray<AT4WorldZoneVolume*>& OutZoneVolumes); // #94
	bool GetEnvironmentZoneComponentsInSpawnObject(TArray<UT4EnvironmentZoneComponent*>& OutEnvironmentZoneComponents); // #94

private:
	void ResetGhostObjects();
	void ResetSpawnedGameObjects();

	bool AddGameObject(AT4GameObject* InNewObject); // #54
	bool RemoveGameObject(const FT4ObjectID& InObjectID, float InFadeOutTimeSec); // #54, #67, #78

	// #54 : 현재는 ClientOnly
	const FT4ObjectID NewClientObjectID();

	AT4GameObject* CreateClientObjectInternal(
		ET4ObjectType InWorldObjectType,
		const FName& InName,
		const FVector& InLocation,
		const FRotator& InRotation,
		const FVector& InScale
	); // #68
	// #54 : 현재는 ClientOnly

private:
	FT4GameWorld* GameWorldRef;

	TMap<FT4ObjectID, AT4GameObject*> SpawnedGameObjectMap;
	TArray<AT4GameObject*> SpawnedGameObjects;
	TArray<AT4GameObject*> GhostGameObjectInfos; // #54

	// #54 : 삭제 될 ClientObjectID 를 기록하고 ProcessPre 에서 수행한다.
	//       Iter 중 *Iter 삭제시 "Array has changed during ranged-for iteration!" 유발
	TArray<FT4ObjectID> PendingRemoveClientObjectIDs;
	FT4ObjectID ClientObjectIDIncr; // #54 : 클라이언트에서만 사용하는 WorldObject 관리
};
