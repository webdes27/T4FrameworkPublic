// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WorldContainer.h"

#include "World/T4GameWorld.h"

#include "Object/T4GameObject.h" // #54
#include "Object/T4GameObjectFactory.h" // #54

#include "Public/T4EngineUtility.h" // #94

#include "Engine/World.h"

#include "T4EngineInternal.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Networking/Travelling/
 */
FT4WorldContainer::FT4WorldContainer(FT4GameWorld* InGameWorld)
	: GameWorldRef(InGameWorld)
	, ClientObjectIDIncr(0, ET4SpawnMode::Client) // #54 : 클라이언트에서만 사용하는 WorldObject 관리
{
}

FT4WorldContainer::~FT4WorldContainer()
{
	check(0 == GhostGameObjectInfos.Num());
	check(0 == SpawnedGameObjects.Num());
}

void FT4WorldContainer::Reset()
{
	PendingRemoveClientObjectIDs.Empty();
	ResetGhostObjects();
	ResetSpawnedGameObjects();
}

void FT4WorldContainer::ResetGhostObjects()
{
	if (0 >= GhostGameObjectInfos.Num())
	{
		return;
	}
	FT4GameObjectFactory& GameObjectFactory = GetGameObjectFactory();
	for (AT4GameObject* GhostObject : GhostGameObjectInfos)
	{
		check(nullptr != GhostObject);
		GameObjectFactory.DestroyGameObject(GhostObject);
	}
	GhostGameObjectInfos.Empty();
}

void FT4WorldContainer::ResetSpawnedGameObjects()
{
	if (0 >= SpawnedGameObjects.Num())
	{
		return;
	}
	FT4GameObjectFactory& GameObjectFactory = GetGameObjectFactory();
	for (AT4GameObject* GameObejct : SpawnedGameObjects)
	{
		check(nullptr != GameObejct);
		GameObjectFactory.DestroyGameObject(GameObejct);
	}
	SpawnedGameObjects.Empty();
	SpawnedGameObjectMap.Empty();
}

void FT4WorldContainer::ProcessPre(float InDeltaTime) // #34 : OnWorldPreActorTick
{
	if (0 < PendingRemoveClientObjectIDs.Num())
	{
		// #54 : 삭제 될 ClientObjectID 를 기록하고 ProcessPre 에서 수행한다.
		//       Iter 중 *Iter 삭제시 "Array has changed during ranged-for iteration!" 유발
		for (const FT4ObjectID& ClientObjectID : PendingRemoveClientObjectIDs)
		{
			RemoveGameObject(ClientObjectID, T4ObjectWorldLeaveTimeSec);
		}
		PendingRemoveClientObjectIDs.Empty();
	}

	if (0 < GhostGameObjectInfos.Num())
	{
		FT4GameObjectFactory& GameObjectFactory = GetGameObjectFactory();
		for (TArray<AT4GameObject*>::TIterator It(GhostGameObjectInfos); It; ++It)
		{
			AT4GameObject* GhostObject = *It;
			check(nullptr != GhostObject);
			if (!GhostObject->IsDestroyable())
			{
				continue;
			}
			GameObjectFactory.DestroyGameObject(GhostObject);
			GhostGameObjectInfos.RemoveAt(It.GetIndex());
		}
	}
}

void FT4WorldContainer::ProcessPost(float InDeltaTime) // #34 : OnWorldPreActorTick
{

}

bool FT4WorldContainer::ProcessSpawnObjectAction(
	const FT4SpawnObjectAction& InAction
)
{
	check(ET4ActionType::SpawnObject == InAction.ActionType);
	check(nullptr != GameWorldRef);
	UWorld* UnrealWorld = GameWorldRef->GetWorld();
	if (nullptr == UnrealWorld)
	{
		return false;
	}
	if (!InAction.ObjectID.IsValid())
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4GameWorld : invalid ObjectID.")
		);
		return false;
	}
	else if (SpawnedGameObjectMap.Contains(InAction.ObjectID))
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4GameWorld : already spawned. ObjectID = '%u'"),
			*InAction.ObjectID
		);
		return false;
	}
	AT4GameObject* NewGameObject = GetGameObjectFactory().CreateGameObject(
		GameWorldRef->GetLayerType(),
		&InAction
	);
	if (nullptr == NewGameObject)
	{
		return false;
	}
	bool bResult = AddGameObject(NewGameObject); // #54
	return bResult;
}

bool FT4WorldContainer::ProcessDespawnObjectAction(
	const FT4ObjectID& InObjectID, 
	float InFadeOutTimeSec
)
{
	check(nullptr != GameWorldRef);
	UWorld* UnrealWorld = GameWorldRef->GetWorld();
	if (nullptr == UnrealWorld)
	{
		return false;
	}
	if (!InObjectID.IsValid())
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4WorldContainer : invalid ObjectID.")
		);
		return false;
	}
	else if (ET4SpawnMode::Client == InObjectID.SpawnMode)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4WorldContainer : incorrect Object Spawn Type.")
		);
		ensure(false);
		return false;
	}
	bool bResult = RemoveGameObject(InObjectID, InFadeOutTimeSec);
	return bResult;
}

uint32 FT4WorldContainer::GetNumGameObjects() const
{
	return SpawnedGameObjects.Num();
}

bool FT4WorldContainer::GetGameObjects(
	ET4SpawnMode InSpawnType,
	TArray<IT4GameObject*>& OutGameObjects
) // #68
{
	if (0 >= SpawnedGameObjects.Num())
	{
		return false;
	}
	for (AT4GameObject* GameObject : SpawnedGameObjects)
	{
		const FT4ObjectID& CurrentObjectID = GameObject->GetObjectID();
		if (ET4SpawnMode::All != InSpawnType &&
			InSpawnType != CurrentObjectID.SpawnMode)
		{
			continue;
		}
		OutGameObjects.Add(static_cast<IT4GameObject*>(GameObject));
	}
	return (0 < OutGameObjects.Num()) ? true : false;
}

bool FT4WorldContainer::HasGameObject(const FT4ObjectID& InObjectID) const
{
	return SpawnedGameObjectMap.Contains(InObjectID);
}

IT4GameObject* FT4WorldContainer::FindGameObject(const FT4ObjectID& InObjectID) const
{
	if (!HasGameObject(InObjectID))
	{
		return nullptr;
	}
	return static_cast<IT4GameObject*>(SpawnedGameObjectMap[InObjectID]);
}

bool FT4WorldContainer::QueryNearestGameObjects(
	const FVector& InOriginLocation,
	const float InMaxDistance,
	TArray<IT4GameObject*>& OutObjects
)
{
	if (0 >= GetNumGameObjects())
	{
		return false;
	}
	// #34 : TODO 서버용 자료구조 추가!!
	if (0 < SpawnedGameObjects.Num())
	{
		float MaxDistanceSQ = FMath::Square(InMaxDistance);
		for (IT4GameObject* GameObejct : SpawnedGameObjects)
		{
			check(nullptr != GameObejct);
			if (ET4SpawnMode::All != GameObejct->GetObjectID().SpawnMode)
			{
				continue; // #68 : 타입을 인자로 받아야 할 수도 있어 보임! TODO
			}
			float DistanceSQ = FVector::DistSquared(InOriginLocation, GameObejct->GetRootLocation());
			if (MaxDistanceSQ >= DistanceSQ)
			{
				OutObjects.Add(GameObejct);
			}
		}
	}
	// TODO : Sorting!!
	return (0 < OutObjects.Num()) ? true : false;
}

bool FT4WorldContainer::GetStaticMapZoneVolumes(
	TArray<AT4MapZoneVolume*>& OutZoneVolumes
) // #94
{
	check(nullptr != GameWorldRef);
	UWorld* UnrealWorld = GameWorldRef->GetWorld();
	if (nullptr == UnrealWorld)
	{
		return false;
	}
	return T4EngineUtility::GetMapZomeVolumesOnWorld(UnrealWorld, OutZoneVolumes);
}

bool FT4WorldContainer::GetEnvironmentZoneComponentsInSpawnObject(
	TArray<UT4EnvironmentZoneComponent*>& OutEnvironmentZoneComponents
) // #94
{
	for (AT4GameObject* GameObejct : SpawnedGameObjects)
	{
		check(nullptr != GameObejct);
		UT4EnvironmentZoneComponent* EnvironmentZoneComponent = GameObejct->GetEnvironmentComponent();
		if (nullptr != EnvironmentZoneComponent)
		{
			OutEnvironmentZoneComponents.Add(EnvironmentZoneComponent); // #99
		}
	}
	return (0 < OutEnvironmentZoneComponents.Num()) ? true : false;
}

const FT4ObjectID FT4WorldContainer::NewClientObjectID()
{
	FT4ObjectID ReturnWorldObjectID = ++ClientObjectIDIncr;
	check(ET4SpawnMode::Client == ReturnWorldObjectID.SpawnMode);
	while (HasGameObject(ReturnWorldObjectID))
	{
		// #54 : check it!!
		ReturnWorldObjectID = ++ClientObjectIDIncr;
	}
	check(ET4SpawnMode::Client == ReturnWorldObjectID.SpawnMode);
	return ReturnWorldObjectID;
}

AT4GameObject* FT4WorldContainer::CreateClientObjectInternal(
	ET4ObjectType InWorldObjectType,
	const FName& InName,
	const FVector& InLocation,
	const FRotator& InRotation,
	const FVector& InScale
)  // #68 : 소멸 조건이 되면 스스로 소멸한다.
{
	const FT4ObjectID NewWorldObjectID = NewClientObjectID();
	check(nullptr != GameWorldRef);
	check(ET4SpawnMode::Client == NewWorldObjectID.SpawnMode);
	AT4GameObject* NewClientObject = GetGameObjectFactory().CreateWorldObject(
		GameWorldRef->GetLayerType(),
		InWorldObjectType,
		InName,
		NewWorldObjectID,
		InLocation,
		InRotation,
		InScale
	);
	if (nullptr == NewClientObject)
	{
		return nullptr;
	}
	bool bResult = AddGameObject(NewClientObject); // #54
	if (!bResult)
	{
		return nullptr;
	}
	return NewClientObject;
}

IT4GameObject* FT4WorldContainer::PlayClientObject(
	ET4ObjectType InWorldObjectType,
	const FName& InName,
	const FVector& InLocation,
	const FRotator& InRotation,
	const FVector& InScale
)  // #68 : 소멸 조건이 되면 스스로 소멸한다.
{
	AT4GameObject* NewClientObject = CreateClientObjectInternal(
		InWorldObjectType,
		InName,
		InLocation,
		InRotation,
		InScale
	);
	if (nullptr != NewClientObject)
	{
		NewClientObject->SetAutoDestroy(); // #68 : WorldObject 만 받는다.
	}
	return static_cast<IT4GameObject*>(NewClientObject);
}

IT4GameObject* FT4WorldContainer::CreateClientObject(
	ET4ObjectType InWorldObjectType, // #63 : Only World Object
	const FName& InName,
	const FVector& InLocation,
	const FRotator& InRotation,
	const FVector& InScale
) // #54
{
	AT4GameObject* NewClientObject = CreateClientObjectInternal(
		InWorldObjectType,
		InName,
		InLocation,
		InRotation,
		InScale
	);
	if (nullptr == NewClientObject)
	{
		return nullptr;
	}
	return static_cast<IT4GameObject*>(NewClientObject);
}

bool FT4WorldContainer::DestroyClientObject(
	const FT4ObjectID& InObjectID
)
{
	// #54 : 현재는 ClientOnly
	if (!InObjectID.IsValid())
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4ClientGameWorld : invalid Client ObjectID.")
		);
		return false;
	}
	else if (ET4SpawnMode::Client != InObjectID.SpawnMode)
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4ClientGameWorld : incorrect Object Spawn Type.")
		);
		ensure(false);
		return false;
	}
	if (PendingRemoveClientObjectIDs.Contains(InObjectID))
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4ClientGameWorld : already Pending ClientObject. ObjectID = '%s'"),
			*(InObjectID.ToString())
		);
		return false;
	}
	// #54 : 삭제 될 ClientObjectID 를 기록하고 ProcessPre 에서 수행한다.
	//       Iter 중 *Iter 삭제시 "Array has changed during ranged-for iteration!" 유발
	PendingRemoveClientObjectIDs.Add(InObjectID);
	return true;
}

bool FT4WorldContainer::AddGameObject(AT4GameObject* InNewObject) // #54
{
	check(nullptr != InNewObject);
	const FT4ObjectID ObjectID = InNewObject->GetObjectID();
	if (SpawnedGameObjectMap.Contains(ObjectID))
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4GameWorld : already spawned. ObjectID = '%s'"),
			*(ObjectID.ToString())
		);
		return false;
	}
	SpawnedGameObjectMap.Add(ObjectID, InNewObject);
	SpawnedGameObjects.Add(InNewObject);
	return true;
}

bool FT4WorldContainer::RemoveGameObject(
	const FT4ObjectID& InObjectID,
	float InFadeOutTimeSec // #67, #78
) // #54
{
	if (!SpawnedGameObjectMap.Contains(InObjectID))
	{
		UE_LOG(
			LogT4Engine,
			Error,
			TEXT("FT4GameWorld : Spawn Object not found. ObjectID = '%s'"),
			*(InObjectID.ToString())
		);
		return false;
	}

	AT4GameObject* DespawnObject = SpawnedGameObjectMap[InObjectID];
	check(nullptr != DespawnObject);

	if (0.0f < InFadeOutTimeSec)
	{
		DespawnObject->OnWorldLeaveStart(InFadeOutTimeSec); // #36 : Leave 시의 Ghost 처리. Coll 충돌 제외 등...
		GhostGameObjectInfos.Add(DespawnObject);
	}
	else
	{
		FT4GameObjectFactory& GameObjectFactory = GetGameObjectFactory();
		GameObjectFactory.DestroyGameObject(DespawnObject);
	}

	SpawnedGameObjectMap.Remove(InObjectID);
	SpawnedGameObjects.Remove(DespawnObject);
	return true;
}