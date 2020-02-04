// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4Engine.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Networking/Travelling/
 */
class AT4GameObject;
class FT4GameObjectFactory
{
public:
	FT4GameObjectFactory();
	virtual ~FT4GameObjectFactory();

	AT4GameObject* CreateGameObject(
		ET4LayerType InLayerType, 
		const FT4SpawnObjectAction* InAction
	);
	AT4GameObject* CreateWorldObject(
		ET4LayerType InLayerType,
		ET4ObjectType InWorldObjectType, // #63 : Only World Object
		const FName& InName,
		const FT4ObjectID& InObjectID,
		const FVector& InLocation,
		const FRotator& InRotation,
		const FVector& InScale
	); // #54

	void DestroyGameObject(AT4GameObject* InGameObject);

private:
	AT4GameObject* CreateCharacterObject(const FT4SpawnObjectAction* InAction, UWorld* InWorld);
	AT4GameObject* CreatePropObject(const FT4SpawnObjectAction* InAction, UWorld* InWorld);
	AT4GameObject* CreateItemObject(const FT4SpawnObjectAction* InAction, UWorld* InWorld); // #41
	AT4GameObject* CreateZoneObject(const FT4SpawnObjectAction* InAction, UWorld* InWorld); // #94
};

FT4GameObjectFactory& GetGameObjectFactory();
