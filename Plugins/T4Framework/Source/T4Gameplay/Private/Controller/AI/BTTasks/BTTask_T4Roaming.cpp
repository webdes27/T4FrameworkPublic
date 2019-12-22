// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Controller/AI/BTTasks/BTTask_T4Roaming.h"
#include "Classes/Controller/AI/T4GameplayCreatureAIController.h"

#include "T4GameplayInternal.h"

/**
  * #50
 */
UBTTask_T4Roaming::UBTTask_T4Roaming(
	const FObjectInitializer& ObjectInitializer
)	: Super(ObjectInitializer)
{
}

EBTNodeResult::Type UBTTask_T4Roaming::ExecuteTask(
	UBehaviorTreeComponent& InOwnerComp,
	uint8* InNodeMemory
)
{
	AT4GameplayCreatureAIController* NPCController = Cast<AT4GameplayCreatureAIController>(InOwnerComp.GetAIOwner());
	if (nullptr == NPCController)
	{
		return EBTNodeResult::Failed;
	}
	FVector MoveTargetPosition = FVector::ZeroVector;
	if (!NPCController->DoRoaming(MoveTargetPosition))
	{
		return EBTNodeResult::Failed;
	}
	NPCController->DoUpdateMoveSpeed(ET4MoveSpeed::MoveSpeed_Walk); // #52
	NPCController->SetMoveTargetLocation(MoveTargetPosition); // #50
	return EBTNodeResult::Succeeded;
}
