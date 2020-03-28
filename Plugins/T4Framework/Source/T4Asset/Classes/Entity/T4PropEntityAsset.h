// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActorEntityAsset.h"
#include "T4PropEntityAsset.generated.h"

/**
  * #35
 */
struct FT4PropEntityCustomVersion
{
	enum Type
	{
		InitializeVer = 0,

		CommonPropertyNameChanged, // #126

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1,
	};

	T4ASSET_API const static FGuid GUID;

private:
	FT4PropEntityCustomVersion() {}
};

class UStaticMesh;
class USkeletalMesh;
class UParticleSystem;

USTRUCT()
struct T4ASSET_API FT4EntityPropPhysicalData : public FT4EntityPhysicalData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityPropPhysicalData()
		: bCanEverAffectNavigation(true)
	{
		BoundHeight = 200.0f;
		BoundRadius = 50.0f;
	}

	// CustomizePropEntityDetails // #126
	UPROPERTY(EditAnywhere, Category = Asset)
	bool bCanEverAffectNavigation; // #126 : Nav Burn 사용 여부
};

USTRUCT()
struct T4ASSET_API FT4EntityPropRenderingData : public FT4EntityRenderingData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityPropRenderingData()
	{
	}

	// CustomizePropEntityDetails // #126
};

USTRUCT()
struct T4ASSET_API FT4EntityPropMeshData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityPropMeshData()
		: MeshType(ET4EntityMeshType::StaticMesh)
		, RelativeLocation(FVector::ZeroVector) // #126
		, RelativeRotation(FRotator::ZeroRotator) // #108
		, bOverlapEvent(false) // #106
	{
	}

	UPROPERTY(EditAnywhere, Category= Asset)
	ET4EntityMeshType MeshType;

	UPROPERTY(EditAnywhere, Category = Asset)
	TSoftObjectPtr<UStaticMesh> StaticMeshAsset;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Override Material Data"))
	FT4EntityMaterialData StaticMeshOverrideMaterialData; // #80

	UPROPERTY(EditAnywhere, Category = Asset)
	TSoftObjectPtr<USkeletalMesh> SkeletalMeshAsset;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Override Material Data"))
	FT4EntityMaterialData SkeletalMeshOverrideMaterialData; // #80

	UPROPERTY(EditAnywhere, Category = Asset)
	TSoftObjectPtr<UParticleSystem> ParticleSystemAsset;

	UPROPERTY(EditAnywhere)
	FVector RelativeLocation; // #126

	UPROPERTY(EditAnywhere)
	FRotator RelativeRotation; // #108

	UPROPERTY(EditAnywhere)
	bool bOverlapEvent; // #106
};

// #126
USTRUCT()
struct T4ASSET_API FT4EntityPropAnimationData : public FT4EntityAnimationData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityPropAnimationData()
	{
	}
};

UCLASS(ClassGroup = T4Framework, Category = "T4Framework")
class T4ASSET_API UT4PropEntityAsset : public UT4ActorEntityAsset
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UObject interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;

	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	//~ End UObject interface

public:
	ET4EntityType GetEntityType() const override { return ET4EntityType::Prop; }

public:
	UPROPERTY(EditAnywhere, Category=Data)
	FT4EntityPropMeshData MeshData;

	UPROPERTY(EditAnywhere)
	FT4EntityPropAnimationData AnimationData; // #126

	UPROPERTY(EditAnywhere, Category= Physical)
	FT4EntityPropPhysicalData Physical;

	UPROPERTY(EditAnywhere, Category= Rendering)
	FT4EntityPropRenderingData Rendering;
};
