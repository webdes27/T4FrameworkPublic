// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/T4GameObject.h"
#include "T4WorldProjectileObject.generated.h"

/**
  * #63
 */
class UT4SceneComponent;
class UT4SphereComponent;
class UT4ProjectileMovementComponent;
struct FT4ActionParameters;
UCLASS()
class AT4WorldProjectileObject : public AT4GameObject
{
	GENERATED_UCLASS_BODY()

	// Disable compiler-generated deprecation warnings by implementing our own destructor
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	~AT4WorldProjectileObject();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

public:
	// APawn
	void PostInitializeComponents() override;

public:
	// IT4GameObject
	ET4ObjectType GetObjectType() const override { return ET4ObjectType::World_Projectile; }

#if WITH_EDITOR
	void SetDebugPause(bool bInPause) override; // #102
#endif

protected:
	void Reset() override;

	void UpdateTickActorBefore(const FT4UpdateTime& InUpdateTime) override;
	void UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime) override;

	bool IsDestroyable() override; // #63

	// AT4GameObject
	USceneComponent* GetAttachParentComponent() override; // #54

protected:
	void SetArrived(bool bPlayEndConti);

	UFUNCTION()
	void HandleOnHit(
		UPrimitiveComponent* InHitComponent, 
		AActor* InOtherActor,
		UPrimitiveComponent* InOtherComponent,
		FVector InNormalImpulse,
		const FHitResult& InHit
	);

	bool ExecuteLaunchAction(
		const FT4LaunchAction& InAction,
		const FT4ActionParameters* InActionParam
	) override; // #63 : only Projectile

public:
	UPROPERTY(Category = World, VisibleAnywhere)
	UT4SphereComponent* CollisionComponent;

	UPROPERTY(Category = World, VisibleAnywhere)
	UT4ProjectileMovementComponent* ProjectileComponent;

	UPROPERTY(Category=World, VisibleAnywhere)
	UT4SceneComponent* AttachRootComponent;

public:
	bool bArrived;

	float PlayingTimeSec;
	float MaxPlayTimeSec;
	FVector GoalLocation;

	FT4ObjectID TargetObjectID;

	FSoftObjectPath EndContiAssetPath;
};
