// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "T4BTTask_T4Approach.generated.h"

/**
  * #50, #114
 */
class UBehaviorTreeComponent;
UCLASS()
class UT4BTTask_T4Approach : public UBTTaskNode
{
	GENERATED_UCLASS_BODY()

public:
	EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& InOwnerComp,
		uint8* InNodeMemory
	) override;
};
