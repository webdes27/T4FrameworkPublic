// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Object/T4GameObject.h"
#include "Public/Asset/T4AssetLoader.h"
#include "T4MovableItemObject.generated.h"

/**
  *
 */
class UT4ItemEntityAsset;
class FT4ItemAnimControl;
class UT4CapsuleComponent;
class UT4StaticMeshComponent;
class UT4SkeletalMeshComponent;
class UMeshComponent;

UCLASS()
class AT4MovableItemObject : public AT4GameObject
{
	GENERATED_UCLASS_BODY()

	// Disable compiler-generated deprecation warnings by implementing our own destructor
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	~AT4MovableItemObject();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS

public:
	// APawn
	void PostInitializeComponents() override;

public:
	ET4ObjectType GetObjectType() const override { return ET4ObjectType::Entity_ItemSkeletal; }

	IT4AnimControl* GetAnimControl() const override; // #14

protected:
	void Reset() override;

	bool Create(const FT4SpawnObjectAction* InAction) override;
	   
	void UpdateTickActorBefore(const FT4UpdateTime& InUpdateTime) override;
	void UpdateTickActorAfter(const FT4UpdateTime& InUpdateTime) override;

	bool CheckAsyncLoading();

	void ApplyEntityAttributes();

	void ClearOverrideMaterialLoader();  // #80
	bool CheckOverrideMaterialAsyncLoading(bool bInitialize); // #80
	void SetOverrideMaterialAsyncLoading(); // #80

protected:
#if WITH_EDITOR
	bool ExecuteEditorAction(const FT4EditorAction& InAction) override; // #37
#endif
	
private:
	bool CreateStaticMesh(); // #80
	bool CreateSkeletalMesh(); // #80

#if WITH_EDITOR
	void RecreateAll(); // #80
#endif

public:
	UPROPERTY(Category= Item, VisibleAnywhere)
	UT4CapsuleComponent* CapsuleComponent;

	UPROPERTY(Category=Item, VisibleAnywhere)
	UT4StaticMeshComponent* StaticMeshComponent;

	UPROPERTY(Category=Item, VisibleAnywhere)
	UT4SkeletalMeshComponent* SkeletalMeshComponent;
	   
private:
	ET4EntityMeshType MeshType;
	UMeshComponent* ActiveMeshComponent;

	FT4StaticMeshLoader StaticMeshLoader;
	FT4SkeletalMeshLoader SkeletalMeshLoader;

	bool bOverrideMaterialLoading; // #80
	TArray<FT4MaterialLoader> OverrideMaterialLoaders; // #80

	TWeakObjectPtr<const UT4ItemEntityAsset> EntityAssetPtr;
	FT4ItemAnimControl* AnimControl;
};
