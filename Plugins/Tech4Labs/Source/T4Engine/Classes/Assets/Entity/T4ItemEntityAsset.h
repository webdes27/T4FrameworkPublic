// Copyright 2019 Tech4 Labs, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4EntityAsset.h"
#include "T4ItemEntityAsset.generated.h"

/**
  * #35
 */
class UStaticMesh;
class USkeletalMesh;

USTRUCT()
struct T4ENGINE_API FT4EntityItemPhysicalAttribute : public FT4EntityBasePhysicalAttribute
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityItemPhysicalAttribute()
	{
		CapsuleHeight = 50.0f;
		CapsuleRadius = 10.0f;
	}
};

USTRUCT()
struct T4ENGINE_API FT4EntityItemRenderingAttribute : public FT4EntityBaseRenderingAttribute
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityItemRenderingAttribute()
	{
	}
};

USTRUCT()
struct T4ENGINE_API FT4EntityItemDropMeshData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityItemDropMeshData()
		: MeshType(ET4EntityMeshType::StaticMesh)
	{
	}

	UPROPERTY(EditAnywhere, Category= Asset)
	ET4EntityMeshType MeshType;

	UPROPERTY(EditAnywhere, Category = Asset)
	TSoftObjectPtr<UStaticMesh> StaticMeshPath;

	UPROPERTY(EditAnywhere, Category = Asset)
	TSoftObjectPtr<USkeletalMesh> SkeletalMeshPath;
};

UCLASS(ClassGroup = Tech4Labs, Category = "Tech4Labs")
class T4ENGINE_API UT4ItemEntityAsset : public UT4EntityAsset
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UObject interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	//~ End UObject interface

public:
	UPROPERTY(EditAnywhere, Category=Attribute)
	FT4EntityItemPhysicalAttribute DropMeshPhysical;

	UPROPERTY(EditAnywhere, Category=Attribute)
	FT4EntityItemRenderingAttribute DropMeshRendering;

	UPROPERTY(EditAnywhere, Category=Data)
	FT4EntityItemDropMeshData DropMeshData;
};
