// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4GameObjectFactory.h"

#include "MovableObject/T4MovableCharacterObject.h"
#include "MovableObject/T4MovablePropSkeletalObject.h"
#include "MovableObject/T4MovablePropStaticObject.h"
#include "MovableObject/T4MovablePropParticleObject.h"
#include "MovableObject/T4MovableItemObject.h" // #41
#include "MovableObject/T4MovableZoneObject.h" // #94

#include "WorldObject/T4WorldProjectileObject.h" // #63
#include "WorldObject/T4WorldIndicatorObject.h" // #117
#include "WorldObject/T4WorldDefaultObject.h" // #54

#include "Public/T4Engine.h"

#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h"
#include "T4Asset/Classes/Entity/T4PropEntityAsset.h"
#include "T4Asset/Classes/Entity/T4WeaponEntityAsset.h" // #41
#include "T4Asset/Classes/Entity/T4ZoneEntityAsset.h" // #94

#include "T4Asset/Public/Entity/T4Entity.h"

#include "Engine/World.h"

#include "T4EngineInternal.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Networking/Travelling/
 */
FT4GameObjectFactory::FT4GameObjectFactory()
{
}

FT4GameObjectFactory::~FT4GameObjectFactory()
{
}

AT4GameObject* FT4GameObjectFactory::CreateCharacterObject(const FT4SpawnObjectAction* InAction, UWorld* InWorld)
{
	check(nullptr != InAction);
	const UT4CharacterEntityAsset* EntityAsset = T4AssetEntityManagerGet()->GetActorEntity(InAction->EntityAssetPath);
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.bDeferConstruction = true; // deley construction

	AT4GameObject* NewGameObject = nullptr;
	switch (EntityAsset->MeshType)
	{
		case ET4EntityCharacterMeshType::FullBody:
		case ET4EntityCharacterMeshType::Composite: // #37
			{
				AT4MovableCharacterObject* NewSpawnActor = InWorld->SpawnActor<AT4MovableCharacterObject>(
					InAction->SpawnLocation,
					InAction->SpawnRotation,
					SpawnInfo
				);
				if (nullptr != NewSpawnActor)
				{
					NewGameObject = Cast<AT4GameObject>(NewSpawnActor);
				}
			}
			break;

		default:
			{
				T4_LOG(Error, TEXT("Unknown Actor mesh type '%u'"), uint8(EntityAsset->MeshType));
			}
			break;
	}

	return NewGameObject;
}

AT4GameObject* FT4GameObjectFactory::CreatePropObject(const FT4SpawnObjectAction* InAction, UWorld* InWorld)
{
	check(nullptr != InAction);
	const UT4PropEntityAsset* EntityAsset = T4AssetEntityManagerGet()->GetPropEntity(InAction->EntityAssetPath);
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}

	check(ET4EntityPropMeshType::Normal == EntityAsset->MeshType); // #41 : 지금은 Normal 만 지원!
	const FT4EntityPropNormalMeshData& MeshData = EntityAsset->NormalMeshData;

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.bDeferConstruction = true; // deley construction

	AT4GameObject* NewGameObject = nullptr;
	switch (MeshData.MeshType)
	{
		case ET4EntityMeshType::StaticMesh:
			{
				AT4MovablePropStaticObject* NewSpawnActor = InWorld->SpawnActor<AT4MovablePropStaticObject>(
					InAction->SpawnLocation,
					InAction->SpawnRotation,
					SpawnInfo
				);
				if (nullptr != NewSpawnActor)
				{
					NewGameObject = Cast<AT4GameObject>(NewSpawnActor);
				}
			}
			break;

		case ET4EntityMeshType::SkeletalMesh:
			{
				AT4MovablePropSkeletalObject* NewSpawnActor = InWorld->SpawnActor<AT4MovablePropSkeletalObject>(
					InAction->SpawnLocation,
					InAction->SpawnRotation,
					SpawnInfo
				);
				if (nullptr != NewSpawnActor)
				{
					NewGameObject = Cast<AT4GameObject>(NewSpawnActor);
				}
			}
			break;

		case ET4EntityMeshType::ParticleSystem:
			{
				AT4MovablePropParticleObject* NewSpawnActor = InWorld->SpawnActor<AT4MovablePropParticleObject>(
					InAction->SpawnLocation,
					InAction->SpawnRotation,
					SpawnInfo
				);
				if (nullptr != NewSpawnActor)
				{
					NewGameObject = Cast<AT4GameObject>(NewSpawnActor);
				}
			}
			break;

		default:
			{
				T4_LOG(Error, TEXT("Unknown prop type '%u'"), uint8(MeshData.MeshType));
			}
			break;
	}

	return NewGameObject;
}

AT4GameObject* FT4GameObjectFactory::CreateItemObject(const FT4SpawnObjectAction* InAction, UWorld* InWorld)
{
	check(nullptr != InAction);

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.bDeferConstruction = true; // deley construction

	AT4GameObject* NewGameObject = Cast<AT4GameObject>(InWorld->SpawnActor<AT4MovableItemObject>(
		InAction->SpawnLocation,
		InAction->SpawnRotation,
		SpawnInfo
	));
	return NewGameObject;
}

AT4GameObject* FT4GameObjectFactory::CreateZoneObject(const FT4SpawnObjectAction* InAction, UWorld* InWorld) // #94
{
	check(nullptr != InAction);

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.bDeferConstruction = true; // deley construction

	AT4GameObject* NewGameObject = Cast<AT4GameObject>(InWorld->SpawnActor<AT4MovableZoneObject>(
		InAction->SpawnLocation,
		InAction->SpawnRotation,
		SpawnInfo
	));
	return NewGameObject;
}

AT4GameObject* FT4GameObjectFactory::CreateGameObject(
	ET4LayerType InLayerType,
	const FT4SpawnObjectAction* InAction
)
{
	check(nullptr != InAction);

	IT4GameWorld* GameWorld = T4EngineWorldGet(InLayerType);
	if (nullptr == GameWorld)
	{
		return nullptr;
	}

	UWorld* World = GameWorld->GetWorld();
	check(nullptr != World);
	check(InLayerType == T4EngineLayer::Get(World));

	AT4GameObject* NewGameObject = nullptr;
	switch (InAction->EntityType)
	{
		case ET4EntityType::Character:
			NewGameObject = CreateCharacterObject(InAction, World);
			break;

		case ET4EntityType::Prop:
			NewGameObject = CreatePropObject(InAction, World);
			break;

		case ET4EntityType::Weapon:
		case ET4EntityType::Costume:
			NewGameObject = CreateItemObject(InAction, World); // #41
			break;

		case ET4EntityType::Zone:
			NewGameObject = CreateZoneObject(InAction, World); // #94
			break;

		default:
			{
				T4_LOG(Error, TEXT("[SL:%u] Unknown EntityAsset type '%u'"), uint8(InLayerType), uint8(InAction->EntityType));
			}
			break;
	}

	if (nullptr == NewGameObject)
	{
		T4_LOG(
			Error,
			TEXT("Failed to create or spawn. EntityAsset '%s'"),
			*(InAction->EntityAssetPath.ToString())
		);
		return nullptr;
	}
	bool bSpawned = NewGameObject->OnCreate(
		InLayerType,
		InAction
	);
	if (!bSpawned)
	{
		DestroyGameObject(NewGameObject);
		return nullptr;
	}
	NewGameObject->AddToRoot();
	return NewGameObject;
}

AT4GameObject* FT4GameObjectFactory::CreateWorldObject(
	ET4LayerType InLayerType,
	ET4ObjectType InWorldObjectType, // #63 : Only World Object
	const FName& InName,
	const FT4ObjectID& InObjectID,
	const FVector& InLocation,
	const FRotator& InRotation,
	const FVector& InScale
) // #54
{
	IT4GameWorld* GameWorld = T4EngineWorldGet(InLayerType);
	if (nullptr == GameWorld)
	{
		return nullptr;
	}

	UWorld* World = GameWorld->GetWorld();
	check(nullptr != World);
	check(InLayerType == T4EngineLayer::Get(World));

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.bDeferConstruction = true; // deley construction

	AT4GameObject* NewGameObject = nullptr;

	switch (InWorldObjectType)
	{
		case ET4ObjectType::World_Projectile: // #63
			{
				AT4WorldProjectileObject* NewWorldObject = World->SpawnActor<AT4WorldProjectileObject>(
					InLocation,
					InRotation,
					SpawnInfo
				);
				NewGameObject = Cast<AT4GameObject>(NewWorldObject);
			}
			break;

		case ET4ObjectType::World_Indicator: // #117
			{
				AT4WorldIndicatorObject* NewWorldObject = World->SpawnActor<AT4WorldIndicatorObject>(
					InLocation,
					InRotation,
					SpawnInfo
				);
				NewGameObject = Cast<AT4GameObject>(NewWorldObject);
			}
			break;

		case ET4ObjectType::World_Default: // #54
			{
				AT4WorldDefaultObject* NewWorldObject = World->SpawnActor<AT4WorldDefaultObject>(
					InLocation,
					InRotation,
					SpawnInfo
				);
				NewGameObject = Cast<AT4GameObject>(NewWorldObject);
			}
			break;

		default:
			{
				T4_LOG(Error, TEXT("Unknown WorldObject type '%u'"), uint8(InWorldObjectType));
			}
			break;
	}

	if (nullptr == NewGameObject)
	{
		T4_LOG(
			Error,
			TEXT("Failed to Create WorldObject.")
		);
		return nullptr;
	}

	bool bSpawned = NewGameObject->OnCreate(
		InLayerType,
		InWorldObjectType,
		InName,
		InObjectID,
		InLocation,
		InRotation,
		InScale
	);
	if (!bSpawned)
	{
		DestroyGameObject(NewGameObject);
		return nullptr;
	}
	NewGameObject->AddToRoot();
	return NewGameObject;
}

void FT4GameObjectFactory::DestroyGameObject(AT4GameObject* InGameObject)
{
	check(nullptr != InGameObject);
	InGameObject->OnReset();
	InGameObject->RemoveFromRoot();
	InGameObject->Destroy();
}

static FT4GameObjectFactory GT4ObjectFactory;
FT4GameObjectFactory& GetGameObjectFactory()
{
	return GT4ObjectFactory;
}