// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/T4GameObject.h"
#include "T4MovableZoneObject.generated.h"

/**
  * #94
 */
class UT4EnvironmentZoneComponent;
class UT4ZoneEntityAsset;
UCLASS()
class AT4MovableZoneObject : public AT4GameObject
{
	GENERATED_UCLASS_BODY()

	// Disable compiler-generated deprecation warnings by implementing our own destructor
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	~AT4MovableZoneObject();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

public:
	// APawn
	void PostInitializeComponents() override;

public:
	ET4ObjectType GetObjectType() const override { return ET4ObjectType::Entity_Zone; }

protected:
	UT4EnvironmentZoneComponent* GetEnvironmentComponent() override { return EnvironmentZoneComponent; } // #99

	void Reset() override;

	bool Create(const FT4SpawnObjectAction* InAction) override;
	   
	void UpdateTickActorBefore(const FT4UpdateTime& InUpdateTime) override;
	void UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime) override;

	bool CheckAsyncLoading();

	void ApplyEntityAttributes();

protected:
#if WITH_EDITOR
	bool ExecuteEditorAction(const FT4EditorAction& InAction) override; // #37
#endif
	
private:
#if WITH_EDITOR
	void RecreateAll(); // #80
#endif

public:
	UPROPERTY(Category = Zone, VisibleAnywhere)
	UT4EnvironmentZoneComponent* EnvironmentZoneComponent;

private:
	ET4ZoneType ZoneType;
	TWeakObjectPtr<const UT4ZoneEntityAsset> EntityAssetPtr;
};
