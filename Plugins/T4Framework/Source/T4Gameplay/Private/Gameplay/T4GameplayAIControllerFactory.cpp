// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameplayAIControllerFactory.h"

#include "Classes/Controller/AI/T4GameplayCreatureAIController.h" // #31
#include "Classes/Controller/AI/T4GameplayFOAIController.h" // #41
#include "Classes/Controller/AI/T4GameplayItemAIController.h" // #41

#include "T4Engine/Public/T4Engine.h"
#include "T4Frame/Public/T4Frame.h"

#if (WITH_EDITOR || WITH_SERVER_CODE)

// #41
#include "Engine/World.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"

#include "T4GameplayInternal.h"

/**
  * #31, #41, #68
 */
FT4GameplayAIControllerFactory::FT4GameplayAIControllerFactory()
{
}

FT4GameplayAIControllerFactory::~FT4GameplayAIControllerFactory()
{
}

FT4NetID FT4GameplayAIControllerFactory::CreateCreatureAIController(
	ET4LayerType InLayerType,
	const FT4GameDataID& InGameDataID,
	const FVector& InSpawnLocation,
	const FRotator& InSpawnRotation
) // #41
{
	check(ET4LayerType::Max > InLayerType);
	check(InGameDataID.CheckType(ET4GameDataType::NPC));
	IT4GameFrame* GameFrame = T4FrameGet(InLayerType);
	check(nullptr != GameFrame);

	FT4NetID NewNetID; // #41

	// #31 : CreatureAI :
	AAIController* NewAI = BeginSpawnAIController(
		InLayerType,
		AT4GameplayCreatureAIController::StaticClass(),
		InSpawnLocation,
		InSpawnRotation
	);
	check(nullptr != NewAI);

	{
		AT4GameplayCreatureAIController* CreatureController = Cast<AT4GameplayCreatureAIController>(NewAI);
		if (nullptr == CreatureController)
		{
			check(false); // ???
			return NewNetID;
		}

		// #31 : Creature는 Spawn 시점에, Player 는 StartPlay 시점에 UniqueID 발급!
		NewNetID = GameFrame->GenerateNetIDForServer();
		CreatureController->SetNetID(NewNetID);

		bool bBinded = CreatureController->Bind(InGameDataID); // #50
		if (!bBinded)
		{
			check(false); // ???
			return NewNetID;
		}

		FinishSpawningAIController(NewAI, InSpawnLocation, InSpawnRotation);
	}

	NewAI->AddToRoot();
	return NewNetID;
}

void FT4GameplayAIControllerFactory::DestroyCreatureAIController(
	ET4LayerType InLayerType,
	AController* InAIController
) // #68
{
	check(ET4LayerType::Max > InLayerType);
	check(nullptr != InAIController);
	IT4GameFrame* GameFrame = T4FrameGet(InLayerType);
	check(nullptr != GameFrame);
	AT4GameplayCreatureAIController* CreatureController = Cast<AT4GameplayCreatureAIController>(
		InAIController
	);
	check(nullptr != CreatureController);
	const FT4NetID NetID = CreatureController->GetNetID();
	check(NetID.IsValid());
	check(CreatureController == GameFrame->FindGameAIController(NetID));
	CreatureController->ClearGameObject(false);
	CreatureController->RemoveFromRoot();
	CreatureController->Destroy();
}

FT4NetID FT4GameplayAIControllerFactory::CreateFOAIController(
	ET4LayerType InLayerType,
	const FT4GameDataID& InGameDataID,
	const FVector& InSpawnLocation,
	const FRotator& InSpawnRotation
) // #41
{
	check(ET4LayerType::Max > InLayerType);
	check(InGameDataID.CheckType(ET4GameDataType::FO));
	IT4GameFrame* GameFrame = T4FrameGet(InLayerType);
	check(nullptr != GameFrame);

	FT4NetID NewNetID; // #41

	// #31 : FOAI :
	AAIController* NewAI = BeginSpawnAIController(
		InLayerType,
		AT4GameplayFOAIController::StaticClass(),
		InSpawnLocation,
		InSpawnRotation
	);
	check(nullptr != NewAI);

	{
		AT4GameplayFOAIController* FOController = Cast<AT4GameplayFOAIController>(NewAI);
		if (nullptr == FOController)
		{
			check(false); // ???
			return NewNetID;
		}

		// #31 : FO는 Spawn 시점에, Player 는 StartPlay 시점에 UniqueID 발급!
		NewNetID = GameFrame->GenerateNetIDForServer();
		FOController->SetNetID(NewNetID);

		bool bBinded = FOController->Bind(InGameDataID); // #50
		if (!bBinded)
		{
			check(false); // ???
			return NewNetID;
		}

		FinishSpawningAIController(NewAI, InSpawnLocation, InSpawnRotation);
	}

	NewAI->AddToRoot();
	return NewNetID;
}

void FT4GameplayAIControllerFactory::DestroyFOAIController(
	ET4LayerType InLayerType,
	AController* InAIController
) // #68
{
	check(ET4LayerType::Max > InLayerType);
	check(nullptr != InAIController);
	IT4GameFrame* GameFrame = T4FrameGet(InLayerType);
	check(nullptr != GameFrame);
	AT4GameplayFOAIController* FOController = Cast<AT4GameplayFOAIController>(
		InAIController
	);
	check(nullptr != FOController);
	const FT4NetID NetID = FOController->GetNetID();
	check(NetID.IsValid());
	check(FOController == GameFrame->FindGameAIController(NetID));
	FOController->ClearGameObject(false);
	FOController->RemoveFromRoot();
	FOController->Destroy();
}

FT4NetID FT4GameplayAIControllerFactory::CreateItemAIController(
	ET4LayerType InLayerType,
	const FT4GameDataID& InGameDataID,
	const FVector& InSpawnLocation,
	const FRotator& InSpawnRotation
) // #41
{
	check(ET4LayerType::Max > InLayerType);
	check(InGameDataID.CheckType(ET4GameDataType::Item_Weapon) || InGameDataID.CheckType(ET4GameDataType::Item_Costume));
	IT4GameFrame* GameFrame = T4FrameGet(InLayerType);
	check(nullptr != GameFrame);

	FT4NetID NewNetID; // #41

	// #31 : ItemAI :
	AAIController* NewAI = BeginSpawnAIController(
		InLayerType,
		AT4GameplayItemAIController::StaticClass(),
		InSpawnLocation,
		InSpawnRotation
	);
	check(nullptr != NewAI);

	{
		AT4GameplayItemAIController* ItemController = Cast<AT4GameplayItemAIController>(NewAI);
		if (nullptr == ItemController)
		{
			check(false); // ???
			return NewNetID;
		}

		// #31 : Item는 Spawn 시점에, Player 는 StartPlay 시점에 UniqueID 발급!
		NewNetID = GameFrame->GenerateNetIDForServer();
		ItemController->SetNetID(NewNetID);

		bool bBinded = ItemController->Bind(InGameDataID); // #50
		if (!bBinded)
		{
			check(false); // ???
			return NewNetID;
		}

		FinishSpawningAIController(NewAI, InSpawnLocation, InSpawnRotation);
	}

	NewAI->AddToRoot();
	return NewNetID;
}

void FT4GameplayAIControllerFactory::DestroyItemAIController(
	ET4LayerType InLayerType,
	AController* InAIController
) // #68
{
	check(ET4LayerType::Max > InLayerType);
	check(nullptr != InAIController);
	IT4GameFrame* GameFrame = T4FrameGet(InLayerType);
	check(nullptr != GameFrame);
	AT4GameplayItemAIController* ItemController = Cast<AT4GameplayItemAIController>(
		InAIController
	);
	check(nullptr != ItemController);
	const FT4NetID NetID = ItemController->GetNetID();
	check(NetID.IsValid());
	check(ItemController == GameFrame->FindGameAIController(NetID));
	ItemController->ClearGameObject(false);
	ItemController->RemoveFromRoot();
	ItemController->Destroy();
}

AAIController* FT4GameplayAIControllerFactory::BeginSpawnAIController(
	ET4LayerType InLayerType,
	TSubclassOf<AAIController> InAIControllerClass,
	const FVector& InSpawnLocation,
	const FRotator& InSpawnRotation
)
{
	// #31, #41
	check(ET4LayerType::Max > InLayerType);
	IT4GameWorld* GameWorld = T4EngineWorldGet(InLayerType);
	check(nullptr != GameWorld);
	UWorld* UnrealWorld = GameWorld->GetWorld();
	if (nullptr == UnrealWorld)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnInItem;
	SpawnInItem.ObjectFlags |= RF_Transient;	// We never want to save AI controllers into a map
	SpawnInItem.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInItem.bDeferConstruction = true;
	AAIController* NewAI = UnrealWorld->SpawnActor<AAIController>(
		InAIControllerClass,
		InSpawnLocation,
		InSpawnRotation,
		SpawnInItem
	);
	if (nullptr == NewAI)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("FT4GameplayAIControllerFactory : Failed to spawn AAIController. LayerType '%u'"),
			uint32(InLayerType)
		);
		return nullptr;
	}

	NewAI->SetReplicates(false); // #34 : PC와 달리 Creature는 서버만 AIController 를 가지도록 설계한다.
	return NewAI;
}

void FT4GameplayAIControllerFactory::FinishSpawningAIController(
	AAIController* InAIController,
	const FVector& InSpawnLocation,
	const FRotator& InSpawnRotation
)
{
	// #31, #41
	const FTransform SpawnTransItemrm(InSpawnRotation, InSpawnLocation);
	UGameplayStatics::FinishSpawningActor(InAIController, SpawnTransItemrm);
}

static FT4GameplayAIControllerFactory GT4GameplayAIControllerFactory;
FT4GameplayAIControllerFactory& GetGameplayAIControllerFactory()
{
	return GT4GameplayAIControllerFactory;
}

#endif