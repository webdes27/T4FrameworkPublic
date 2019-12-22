// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Products/T4RehearsalEditorUtils.h"

#include "T4Asset/Classes/Conti/T4ContiAsset.h"
#include "T4Asset/Classes/Entity/T4EntityAsset.h" // #36
#include "T4Engine/Public/Action/T4ActionCodeMinimal.h"
#include "T4Engine/Public/T4Engine.h"
#include "T4Frame/Public/T4Frame.h" // #30

#include "Engine/Level.h" // #91

#include "T4RehearsalEditorInternal.h"

/**
  * 
 */
namespace T4EditorUtil
{

IT4GameObject* GetPlayerGameObject(ET4LayerType InLayerType)
{
	IT4GameFrame* GameFrame = T4FrameGet(InLayerType);
	if (nullptr == GameFrame)
	{
		return nullptr;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	if (nullptr == GameWorld || !GameWorld->HasPlayerObject())
	{
		return nullptr;
	}
	IT4PlayerController* PlayerController = GameFrame->GetPlayerController();
	if (nullptr == PlayerController || !PlayerController->HasGameObject())
	{
		return nullptr;
	}
	return PlayerController->GetGameObject();
}

UWorld* GetWorld(ET4LayerType InLayerType)
{
	IT4GameFrame* GameFrame = T4FrameGet(InLayerType);
	if (nullptr == GameFrame)
	{
		return nullptr;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return nullptr;
	}
	return GameWorld->GetWorld();
}

void ServerDespawnAll(
	ET4LayerType InLayerType,
	bool bClearPlayerObject
) // #68
{
	IT4GameFrame* GameFrame = T4FrameGet(InLayerType);
	if (nullptr == GameFrame)
	{
		return;
	}
	IT4GameplayInstance* GameplayInstance = GameFrame->GetGameplayInstance();
	if (nullptr == GameplayInstance)
	{
		return;
	}
	IT4EditorGameData* EditorGameData = GameplayInstance->GetEditorGameData();
	if (nullptr == EditorGameData)
	{
		return;
	}
	EditorGameData->DoDespawnAll(bClearPlayerObject);
}

FBox CalculateLevelBounds(ULevel* InLevel) // #91 : World Single
{
	FBox LevelBounds(ForceInit);

	// #91 : World Single 에서 불필요한 Actor 를 제외하고, BBox 를 계산하기 위한 조치. 해상도와 관련이 있다.
	//       refer : ALevelBounds::CalculateLevelBounds
	if (nullptr != InLevel)
	{
		// Iterate over all level actors
		for (int32 ActorIndex = 0; ActorIndex < InLevel->Actors.Num(); ++ActorIndex)
		{
			AActor* Actor = InLevel->Actors[ActorIndex];
			if (Actor && Actor->IsLevelBoundsRelevant())
			{
				FString ActorClassName = Actor->GetClass()->GetName();
				if (ActorClassName.Contains(TEXT("Sky")))
				{
					continue; // Sky 제외. 볼륨을 비정상적으로 키운다.
				}
				else if (ActorClassName.Contains(TEXT("Light")))
				{
					continue; // 비정상적인 Light 키워드도 제외!
				}
				// Sum up components bounding boxes
				FBox ActorBox = Actor->GetComponentsBoundingBox(true);
				if (ActorBox.IsValid)
				{
					LevelBounds += ActorBox;
				}
			}
		}
	}

	return LevelBounds;
}

}