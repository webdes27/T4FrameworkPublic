// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Controller/AI/BTServices/BTService_T4DetectAttackable.h"
#include "Classes/Controller/AI/T4GameplayCreatureAIController.h"

#include "T4GameplayInternal.h"

/**
  * #50
 */
UBTService_T4DetectAttackable::UBTService_T4DetectAttackable()
{
	NodeName = TEXT("T4DetectAttackable");
	Interval = 1.0f;
}

void UBTService_T4DetectAttackable::TickNode(
	UBehaviorTreeComponent& InOwnerComp,
	uint8* InNodeMemory,
	float InDeltaTime
)
{
	Super::TickNode(InOwnerComp, InNodeMemory, InDeltaTime);
	AT4GameplayCreatureAIController* NPCController = Cast<AT4GameplayCreatureAIController>(InOwnerComp.GetAIOwner());
	if (nullptr == NPCController)
	{
		return;
	}
	FT4ObjectID TargetGameObjectID;
	if (NPCController->IsCurrentAggressive())
	{
		const FT4NPCAIMemory& AIMemory = NPCController->GetAIMemoryConst();
		FT4ObjectID AttackTargetObjectID = AIMemory.AttackTargetObjectID; // #50
		if (AttackTargetObjectID.IsValid())
		{
			if (NPCController->FindGameObject(AttackTargetObjectID))
			{
				// #104 : 타겟이 존재한다면 기존 타겟을 계속 유지하고, 다음 스탭으로 진행하도록 처리하자.
				return;
			}
		}
		IT4GameObject* NewTargetObject = NPCController->FindNearestEnemyByAttackRange();
		if (nullptr != NewTargetObject)
		{
			TargetGameObjectID = NewTargetObject->GetObjectID();
		}
	}
	NPCController->SetAttackTargetObjectID(TargetGameObjectID);
}