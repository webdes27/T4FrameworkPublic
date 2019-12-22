// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Controller/AI/BTTasks/BTTask_T4Wait.h"
#include "Classes/Controller/AI/T4GameplayCreatureAIController.h"

#include "T4GameplayInternal.h"

/**
 * #50 refer BTTask_WaitBlackboardTime
 */
UBTTask_T4Wait::UBTTask_T4Wait(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "T4WaitTime";
}

EBTNodeResult::Type UBTTask_T4Wait::ExecuteTask(
	UBehaviorTreeComponent& InOwnerComp,
	uint8* InNodeMemory
)
{
	// Update wait time based on current NPCAIMemory value
	AT4GameplayCreatureAIController* NPCController = Cast<AT4GameplayCreatureAIController>(InOwnerComp.GetAIOwner());
	if (nullptr == NPCController)
	{
		return EBTNodeResult::Failed;
	}
	NPCController->DoUpdateMoveSpeed(ET4MoveSpeed::MoveSpeed_Stand); // #52
	const FT4NPCAIMemory& AIMemory = NPCController->GetAIMemoryConst();
	WaitTime = AIMemory.IdleWaitTime;
	return Super::ExecuteTask(InOwnerComp, InNodeMemory);
}

void UBTTask_T4Wait::TickTask(
	UBehaviorTreeComponent& InOwnerComp,
	uint8* InNodeMemory,
	float InDeltaSeconds
)
{
	Super::TickTask(InOwnerComp, InNodeMemory, InDeltaSeconds);
	AT4GameplayCreatureAIController* NPCController = Cast<AT4GameplayCreatureAIController>(InOwnerComp.GetAIOwner());
	if (nullptr != NPCController)
	{
		// 어그로가 끌리면 Wait 를 강제로 종료한다.
		if (NPCController->IsCurrentAggressive())
		{
			FinishLatentTask(InOwnerComp, EBTNodeResult::Succeeded);
		}
	}
}