// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "T4WorldPreviewFloorMeshActor.generated.h"

/**
  * 
 */
class UStaticMeshComponent;
UCLASS()
class AT4WorldPreviewFloorMeshActor : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY()
	UStaticMeshComponent* FloorMeshComponent;
};
