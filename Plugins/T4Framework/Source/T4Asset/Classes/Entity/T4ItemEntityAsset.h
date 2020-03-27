// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4EntityAsset.h"
#include "T4ItemEntityAsset.generated.h"

/**
  * #35
 */
class UStaticMesh;
class USkeletalMesh;
class UParticleSystem;
class UMaterialInterface;

USTRUCT()
struct T4ASSET_API FT4EntityItemPhysicalData : public FT4EntityPhysicalData
{
	GENERATED_USTRUCT_BODY()

	// CustomizeItemCommonEntityDetails

public:
	FT4EntityItemPhysicalData()
	{
		CapsuleHeight = 50.0f;
		CapsuleRadius = 10.0f;
	}
};

USTRUCT()
struct T4ASSET_API FT4EntityItemRenderingData : public FT4EntityRenderingData
{
	GENERATED_USTRUCT_BODY()

	// CustomizeItemCommonEntityDetails

public:
	FT4EntityItemRenderingData()
	{
	}
};

USTRUCT()
struct T4ASSET_API FT4EntityItemDropMeshData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityItemDropMeshData()
		: MeshType(ET4EntityMeshType::StaticMesh)
		, RelativeLocation(FVector::ZeroVector) // #106
		, RelativeRotation(FRotator::ZeroRotator) // #106
	{
	}

	UPROPERTY(EditAnywhere)
	ET4EntityMeshType MeshType;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UStaticMesh> StaticMeshAsset;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Override Material Data"))
	FT4EntityMaterialData StaticMeshOverrideMaterialData; // #80

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<USkeletalMesh> SkeletalMeshAsset;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Override Material Data"))
	FT4EntityMaterialData SkeletalMeshOverrideMaterialData; // #80

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UParticleSystem> ParticleSystemAsset;

	UPROPERTY(EditAnywhere)
	FVector RelativeLocation; // #106

	UPROPERTY(EditAnywhere)
	FRotator RelativeRotation; // #106
};

// #107
USTRUCT()
struct T4ASSET_API FT4EditorTestItemData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EditorTestItemData()
#if WITH_EDITOR
		: ItemSpawnType(ET4EntityEditorViewportItemSpawn::DropMesh)
		, ParentStanceName(NAME_None)
		, ParentSubStanceName(NAME_None)
#endif
	{
	}

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	ET4EntityEditorViewportItemSpawn ItemSpawnType;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UT4EntityAsset> ParentEntityAsset;

	UPROPERTY(EditAnywhere)
	FName ParentStanceName;

	UPROPERTY(EditAnywhere)
	FName ParentSubStanceName;
#endif
};

UCLASS(ClassGroup = T4Framework, Category = "T4Framework")
class T4ASSET_API UT4ItemEntityAsset : public UT4EntityAsset
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UObject interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	//~ End UObject interface

public:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Mesh Data"))
	FT4EntityItemDropMeshData DropMeshData;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Physical"))
	FT4EntityItemPhysicalData DropMeshPhysical;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Rendering"))
	FT4EntityItemRenderingData DropMeshRendering;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Test"))
	FT4EditorTestItemData EditorTestItemData; // #107
#endif
};
