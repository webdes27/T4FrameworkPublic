// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/T4GameObject.h"
#include "Public/Asset/T4AssetLoader.h"
#include "T4MovablePropParticleObject.generated.h"

/**
  *
 */
class AEmitter;
class UParticleSystem;
class UT4PropEntityAsset;
class FT4PropAnimControl;
class UT4CapsuleComponent;
class UT4ParticleSystemComponent;

UCLASS()
class AT4MovablePropParticleObject : public AT4GameObject
{
	GENERATED_UCLASS_BODY()

	// Disable compiler-generated deprecation warnings by implementing our own destructor
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	~AT4MovablePropParticleObject();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

public:
	void PostActorCreated() override;
	void PostInitializeComponents() override;

	void SetTemplate(UParticleSystem* NewTemplate);

public:
	ET4ObjectType GetObjectType() const override { return ET4ObjectType::Entity_PropParticle; }

	IT4AnimControl* GetAnimControl() const override; // #14

protected:
	void Reset() override;

	bool Create(const FT4SpawnObjectAction* InAction) override;

	void CreateFinished(
		const FVector& InSpawnLocation,
		const FRotator& InSpawnRotation,
		const FVector& InSpawnScale
	) override;

	void UpdateTickActorBefore(const FT4UpdateTime& InUpdateTime) override;
	void UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime) override;

	bool CheckAsyncLoading();

	void ApplyEntityAttributes();

	UFUNCTION()
	void OnParticleSystemFinished(UParticleSystemComponent* FinishedComponent);

public:
	UPROPERTY(Category=Prop, VisibleAnywhere)
	UT4CapsuleComponent* CapsuleComponent;

	UPROPERTY(Category=Prop, VisibleAnywhere)
	UT4ParticleSystemComponent* ParticleSystemComponent;

	UPROPERTY(Category=Prop, EditAnywhere)
	uint32 bPostUpdateTickGroup:1;

	UPROPERTY()
	uint32 bDestroyOnSystemFinish:1;

private:
	FT4ParticleSystemLoader ParticleSystemLoader;

	TWeakObjectPtr<const UT4PropEntityAsset> EntityAssetPtr;
	FT4PropAnimControl* AnimControl;
};
