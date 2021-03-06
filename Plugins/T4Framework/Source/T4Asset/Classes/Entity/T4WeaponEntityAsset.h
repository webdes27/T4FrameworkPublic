// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ItemEntityAsset.h"
#include "T4WeaponEntityAsset.generated.h"

/**
  * #35
 */
struct FT4WeaponEntityCustomVersion
{
	enum Type
	{
		InitializeVer = 0,

		CommonPropertyNameChanged, // #124
		CommonPropertyNameV2Changed, // #126

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1,
	};

	T4ASSET_API const static FGuid GUID;

private:
	FT4WeaponEntityCustomVersion() {}
};

class UStaticMesh;
class USkeletalMesh;
class UPhysicsAsset; // #76
class UMaterialInterface;
USTRUCT()
struct T4ASSET_API FT4EntityItemWeaponMeshData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityItemWeaponMeshData()
		: MeshType(ET4EntityMeshType::StaticMesh)
		, EquipPointName(NAME_None) // #106
		, RelativeRotation(FRotator::ZeroRotator) // #108
		, RelativeScale(1.0f)
		, StanceName(NAME_None) // #106
		, bOverlapEvent(false) // #106
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
	FName EquipPointName; // #106

	UPROPERTY(EditAnywhere)
	FRotator RelativeRotation; // #108

	UPROPERTY(EditAnywhere)
	float RelativeScale; // #108

	UPROPERTY(EditAnywhere)
	FName StanceName; // #106

	UPROPERTY(EditAnywhere)
	bool bOverlapEvent; // #106
};

// #107
USTRUCT()
struct T4ASSET_API FT4EntityWeaponAnimationData : public FT4EntityAnimationData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityWeaponAnimationData()
	{
	}
};

UCLASS(ClassGroup = T4Framework, Category = "T4Framework")
class T4ASSET_API UT4WeaponEntityAsset : public UT4ItemEntityAsset
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UObject interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;

	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	//~ End UObject interface

public:
	ET4EntityType GetEntityType() const override { return ET4EntityType::Weapon; }

#if WITH_EDITOR
	virtual UStaticMesh* GetPrimaryStaticMeshAsset() const override // #81
	{
		if (ET4EntityMeshType::StaticMesh != MeshData.MeshType)
		{
			return nullptr;
		}
		if (MeshData.StaticMeshAsset.IsNull())
		{
			return nullptr;
		}
		return MeshData.StaticMeshAsset.LoadSynchronous();
	}

	virtual USkeletalMesh* GetPrimarySkeletalMeshAsset() const override // #81
	{
		if (ET4EntityMeshType::SkeletalMesh != MeshData.MeshType)
		{
			return nullptr;
		}
		if (MeshData.SkeletalMeshAsset.IsNull())
		{
			return nullptr;
		}
		return MeshData.SkeletalMeshAsset.LoadSynchronous();
	}
#endif

public:
	UPROPERTY(EditAnywhere)
	FT4EntityItemWeaponMeshData MeshData;

	UPROPERTY(EditAnywhere)
	FT4EntityWeaponAnimationData AnimationData; // #107
};
