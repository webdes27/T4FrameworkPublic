// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/T4GameObject.h"
#include "Public/Asset/T4AssetLoader.h"
#include "T4MovablePropStaticObject.generated.h"

/**
  *
 */
class UT4PropEntityAsset;
class FT4PropAnimControl;
class UT4CapsuleComponent;
class UT4StaticMeshComponent;

UCLASS()
class AT4MovablePropStaticObject : public AT4GameObject
{
	GENERATED_UCLASS_BODY()

	// Disable compiler-generated deprecation warnings by implementing our own destructor
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	~AT4MovablePropStaticObject();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

public:
	// APawn
	void PostInitializeComponents() override;

public:
	ET4ObjectType GetObjectType() const override { return ET4ObjectType::Entity_PropStatic; }

	IT4AnimControl* GetAnimControl() const override; // #14

protected:
	void Reset() override;

	bool Create(const FT4SpawnObjectAction* InAction) override;

	void UpdateTickActorBefore(const FT4UpdateTime& InUpdateTime) override;
	void UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime) override;

	bool CheckAsyncLoading();

	void ApplyEntityAttributes();

public:
	UPROPERTY(Category=Prop, VisibleAnywhere)
	UT4CapsuleComponent* CapsuleComponent;

	UPROPERTY(Category=Prop, VisibleAnywhere)
	UT4StaticMeshComponent* StaticMeshComponent;

private:
	FT4StaticMeshLoader StaticMeshLoader;

	TWeakObjectPtr<const UT4PropEntityAsset> EntityAssetPtr;
	FT4PropAnimControl* AnimControl;
};
