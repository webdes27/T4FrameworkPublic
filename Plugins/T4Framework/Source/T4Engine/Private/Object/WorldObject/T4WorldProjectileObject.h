// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

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
	void SetArrived(bool bInPlayEndConti);
	void SetArrivedForTarget(
		IT4GameObject* InTargetObject,
		const FVector& InHitLocation,
		const FVector& InShootDirection,
		FName InHitBone, 
		bool bInPlayEndConti
	); // #112

	void SetThisDestroy(); // #112

	void StopHeadConti(); // #112
	void ProjectileMoveStop(); // #112
	void PlayEndConti(bool bInPlayEndConti); // #112
	
	void AdvanceToTarget(const FT4UpdateTime& InUpdateTime, float InFlyingLifeTimeSec); // #112
	void AdvanceToInfinity(const FT4UpdateTime& InUpdateTime, float InFlyingLifeTimeSec); // #112

	bool CheckHit(
		ET4CollisionChannel InCollisionChnnel,
		const FVector& InShootDirection, 
		float InMaxDistance, 
		FT4HitSingleResult& OutHitResult
	); // #112

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
	bool bDestroying; // #112
	bool bArrived;
	bool bHitAttached; // #112

	float PlayingTimeSec;
	float MaxPlayTimeSec;
	FVector GoalLocation;

	FT4ObjectID OwnerObjectID; // #112
	FT4ObjectID TargetObjectID;
	
	bool bEnableHitAttached; // #112
	float HitAttachedTimeSec; // #112

	float ProjectileLength; // #112 : Projectile 의 길이, 충돌 계산에서 Offset 으로 사용. (원점 에서의 길이)
	float ProjectileLengthEnter; // #112
	FName ThrowTargetPoint; // #106

	int32 FlyingHitCount;

	FSoftObjectPath HeadContiAssetPath; // #112
	FSoftObjectPath EndContiAssetPath;
};
