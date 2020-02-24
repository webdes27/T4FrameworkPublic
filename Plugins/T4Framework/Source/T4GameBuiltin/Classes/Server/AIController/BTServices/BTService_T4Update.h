// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_T4Update.generated.h"

/**
  * #50, #114
 */
class UBehaviorTreeComponent;
UCLASS()
class UBTService_T4Update : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_T4Update();

protected:
	void TickNode(
		UBehaviorTreeComponent& InOwnerComp, 
		uint8* InNodeMemory, 
		float InDeltaTime
	) override;
};
