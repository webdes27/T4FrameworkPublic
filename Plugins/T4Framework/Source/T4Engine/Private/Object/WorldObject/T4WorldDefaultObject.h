// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/T4GameObject.h"
#include "T4WorldDefaultObject.generated.h"

/**
  * #54
 */
class UT4SceneComponent;
UCLASS()
class AT4WorldDefaultObject : public AT4GameObject
{
	GENERATED_UCLASS_BODY()

	// Disable compiler-generated deprecation warnings by implementing our own destructor
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	~AT4WorldDefaultObject();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

public:
	// APawn
	void PostInitializeComponents() override;

public:
	// IT4GameObject
	ET4ObjectType GetObjectType() const override { return ET4ObjectType::World_Default; }

public:
	void SetAutoDestroy() override { bAutoDestroy = true; } // #68 : WorldObject 만 받는다.

protected:
	void Reset() override;

	void UpdateTickActorBefore(const FT4UpdateTime& InUpdateTime) override;
	void UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime) override;

	// AT4GameObject
	USceneComponent* GetAttachParentComponent() override; // #54

public:
	UPROPERTY(Category=World, VisibleAnywhere)
	UT4SceneComponent* SceneComponent;

private:
	bool bAutoDestroy; // #68 : WorldObject 만 받는다.
};
