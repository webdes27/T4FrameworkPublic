// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "T4StaticMeshComponent.generated.h"

/**
  * https://docs.unrealengine.com/en-us/Engine/Content/Types/StaticMeshes
 */
UCLASS()
class UT4StaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_UCLASS_BODY()

public:
	void TickComponent(
		float DeltaTime,
		enum ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

protected:
	void BeginPlay() override;
};
