// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Controller/AI/BTTasks/BTTask_T4FindPointNearEnemy.h"
#include "Classes/Controller/AI/T4GameplayCreatureAIController.h"

#include "T4GameplayInternal.h"

/**
  *
 */
UBTTask_T4FindPointNearEnemy::UBTTask_T4FindPointNearEnemy(
	const FObjectInitializer& ObjectInitializer
)	: Super(ObjectInitializer)
{
}

EBTNodeResult::Type UBTTask_T4FindPointNearEnemy::ExecuteTask(
	UBehaviorTreeComponent& InOwnerComp,
	uint8* InNodeMemory
)
{
	AT4GameplayCreatureAIController* NPCController = Cast<AT4GameplayCreatureAIController>(InOwnerComp.GetAIOwner());
	if (nullptr == NPCController)
	{
		return EBTNodeResult::Failed;
	}
	if (!NPCController->HasGameObject())
	{
		return EBTNodeResult::Failed;
	}
	IT4GameObject* NPCGameObject = NPCController->GetGameObject();
	if (nullptr == NPCGameObject)
	{
		return EBTNodeResult::Failed;
	}
	IT4GameObject* AttackTargetObject = nullptr;
	if (NPCController->IsCurrentAggressive())
	{
		const FT4NPCAIMemory& AIMemory = NPCController->GetAIMemoryConst();
		FT4ObjectID AttackTargetObjectID = AIMemory.AttackTargetObjectID; // #50
		if (AttackTargetObjectID.IsValid())
		{
			// #104 : 나중에 별도 Sequence 로 빼자. 지금은 여기서 처리
			AttackTargetObject = NPCController->FindGameObject(AttackTargetObjectID);
		}
	}
	if (NPCController->IsOriginAggressive() && nullptr == AttackTargetObject)
	{
		// #104 : 선공 몹이면... 공격 기회를 준다.
		AttackTargetObject = NPCController->FindNearestEnemyBySensoryRange();
	} 
	if (nullptr == AttackTargetObject)
	{
		return EBTNodeResult::Failed;
	}
	const FT4GameObjectProperty& TargetObjectProperty = AttackTargetObject->GetPropertyConst();
	FVector MoveTargetPosition = FVector::ZeroVector;
	if (!NPCController->GetMoveTargetLocationByAttackRange(
		NPCGameObject->GetRootLocation(),
		AttackTargetObject->GetRootLocation(),
		TargetObjectProperty.CapsuleRadius,
		MoveTargetPosition
	))
	{
		return EBTNodeResult::Failed;
	}
	NPCController->DoUpdateMoveSpeed(ET4MoveSpeed::MoveSpeed_Run); // #52
	NPCController->SetMoveTargetLocation(MoveTargetPosition); // #50
	return EBTNodeResult::Succeeded;
}
