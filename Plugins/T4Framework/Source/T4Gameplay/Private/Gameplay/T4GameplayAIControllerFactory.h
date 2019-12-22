// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Frame/Public/T4FrameNetwork.h"

/**
  * #31, #41, #68
 */
#if (WITH_EDITOR || WITH_SERVER_CODE)
class AAIController;
class FT4GameplayAIControllerFactory
{
public:
	FT4GameplayAIControllerFactory();
	virtual ~FT4GameplayAIControllerFactory();

	FT4NetID CreateCreatureAIController(
		ET4LayerType InLayerType,
		const FT4GameDataID& InGameDataID,
		const FVector& InSpawnLocation,
		const FRotator& InSpawnRotation
	); // #41

	void DestroyCreatureAIController(
		ET4LayerType InLayerType, 
		AController* InAIController
	); // #68

	FT4NetID CreateFOAIController(
		ET4LayerType InLayerType,
		const FT4GameDataID& InGameDataID,
		const FVector& InSpawnLocation,
		const FRotator& InSpawnRotation
	); // #41

	void DestroyFOAIController(
		ET4LayerType InLayerType,
		AController* InAIController
	); // #68

	FT4NetID CreateItemAIController(
		ET4LayerType InLayerType,
		const FT4GameDataID& InGameDataID,
		const FVector& InSpawnLocation,
		const FRotator& InSpawnRotation
	); // #41

	void DestroyItemAIController(
		ET4LayerType InLayerType,
		AController* InAIController
	); // #68

private:
	AAIController* BeginSpawnAIController(
		ET4LayerType InLayerType,
		TSubclassOf<AAIController> InAIControllerClass,
		const FVector& InSpawnLocation,
		const FRotator& InSpawnRotation
	); // #31, #41

	void FinishSpawningAIController(
		AAIController* InAIController,
		const FVector& InSpawnLocation,
		const FRotator& InSpawnRotation
	);
};

FT4GameplayAIControllerFactory& GetGameplayAIControllerFactory();
#endif