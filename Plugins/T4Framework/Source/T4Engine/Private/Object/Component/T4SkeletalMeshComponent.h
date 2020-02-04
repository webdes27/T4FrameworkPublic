// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4EngineTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "T4SkeletalMeshComponent.generated.h"

/**
  * https://docs.unrealengine.com/en-us/Engine/Content/Types/SkeletalMeshes
 */
UCLASS()
class UT4SkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_UCLASS_BODY()

public:
	void TickComponent(
		float InDeltaTime,
		enum ELevelTick InTickType,
		FActorComponentTickFunction* InThisTickFunction
	) override;

public:
	ET4LayerType GetLayerType() const { return LayerType; } // #111
	const FT4ObjectID& GetOwnerObjectID() const { return OwnerObjectID; } // #111

	void SetLayerType(ET4LayerType InLayerType) { LayerType = InLayerType; } // #111
	void SetOwnerObjectID(const FT4ObjectID& InObjectID) { OwnerObjectID = InObjectID; } // #111

protected:
	void BeginPlay() override;

private:
	ET4LayerType LayerType; // #111
	FT4ObjectID OwnerObjectID; // #111
};
