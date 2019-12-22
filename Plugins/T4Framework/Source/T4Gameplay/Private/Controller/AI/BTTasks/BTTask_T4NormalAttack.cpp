// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Controller/AI/BTTasks/BTTask_T4NormalAttack.h"
#include "Classes/Controller/AI/T4GameplayCreatureAIController.h"

#include "T4GameplayInternal.h"

/**
  * #50
 */
UBTTask_T4NormalAttack::UBTTask_T4NormalAttack(
	const FObjectInitializer& ObjectInitializer
)	: Super(ObjectInitializer)
{
}

EBTNodeResult::Type UBTTask_T4NormalAttack::ExecuteTask(
	UBehaviorTreeComponent& InOwnerComp,
	uint8* InNodeMemory
)
{
	AT4GameplayCreatureAIController* NPCController = Cast<AT4GameplayCreatureAIController>(InOwnerComp.GetAIOwner());
	if (nullptr == NPCController)
	{
		return EBTNodeResult::Failed;
	}
	if (NPCController->IsAttacking())
	{
		return EBTNodeResult::Succeeded; // 공격중이라, 더이상 진행이 안되도록 처리!
	}
	if (!NPCController->IsCurrentAggressive())
	{
		return EBTNodeResult::Failed;
	}
	const FT4NPCAIMemory& AIMemory = NPCController->GetAIMemoryConst();
	FT4ObjectID AttackTargetObjectID = AIMemory.AttackTargetObjectID; // #50
	if (!AttackTargetObjectID.IsValid())
	{
		return EBTNodeResult::Failed;
	}
	if (!NPCController->CheckValidAttackByTarget())
	{
		return EBTNodeResult::Failed; // #104 : 공격 대상은 있지만, 공격 거리가 아님으로 다가가도록 다음 Task 로 이동
	}
	NPCController->DoUpdateMoveSpeed(ET4MoveSpeed::MoveSpeed_Stand); // #52
	bool bResult = NPCController->DoAttack(AttackTargetObjectID);
	if (!bResult)
	{
		return EBTNodeResult::Failed;
	}
	return EBTNodeResult::Succeeded;
}
