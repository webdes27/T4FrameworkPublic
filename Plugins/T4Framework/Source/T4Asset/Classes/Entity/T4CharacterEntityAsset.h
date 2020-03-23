// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActorEntityAsset.h"
#include "Public/T4AssetDefinitions.h" // #74
#include "T4CharacterEntityAsset.generated.h"

/**
  * #35
 */
struct FT4CharacterEntityCustomVersion
{
	enum Type
	{
		InitializeVer = 0,

		CommonPropertyNameChanged, // #124

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1,
	};

	T4ASSET_API const static FGuid GUID;

private:
	FT4CharacterEntityCustomVersion() {}
};

class USkeleton;
class UPhysicsAsset; // #76
class UMaterialInterface; // #80
class USkeletalMesh;
class UAnimBlueprint;
class UAnimMontage;
class UBlendSpace;
class UT4AnimSetAsset; // #39
class UT4ActionAsset; // #74
class UT4WeaponEntityAsset; // #74
class UT4CostumeEntityAsset;

USTRUCT()
struct T4ASSET_API FT4EntityCharacterPhysicalAttribute : public FT4EntityBasePhysicalAttribute
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityCharacterPhysicalAttribute()
		: DefaultSpeed(500.0f) // #108
		, CombatSpeed(300.0f) // #109
		, CrouchSpeed(200.0f) // #109
		, JumpZVelocity(550.0f) // #46
		, RollZVelocity(250.0f) // #46
		, RotationYawRate(520.0f)
	{
	}

	// CustomizeCharacterEntityDetails

	UPROPERTY(EditAnywhere, Category = Physical, meta = (ClampMin = "10.0", ClampMax = "1000"))
	float DefaultSpeed; // #108

	UPROPERTY(EditAnywhere, Category = Physical, meta = (ClampMin = "10.0", ClampMax = "1000"))
	float CombatSpeed; // #109

	UPROPERTY(EditAnywhere, Category = Physical, meta = (ClampMin = "10.0", ClampMax = "1000"))
	float CrouchSpeed; // #109

	UPROPERTY(EditAnywhere, Category = Physical, meta = (ClampMin = "10.0", ClampMax = "1000"))
	float JumpZVelocity;

	UPROPERTY(EditAnywhere, Category = Physical, meta = (ClampMin = "5.0", ClampMax = "500"))
	float RollZVelocity; // #46

	UPROPERTY(EditAnywhere, Category = Physical, meta = (ClampMin = "10.0", ClampMax = "1500"))
	float RotationYawRate;
};

USTRUCT()
struct T4ASSET_API FT4EntityCharacterRenderingAttribute : public FT4EntityBaseRenderingAttribute
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityCharacterRenderingAttribute()
	{
	}

	// CustomizeCharacterEntityDetails
};

USTRUCT()
struct T4ASSET_API FT4EntityCharacterFullBodyMeshData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityCharacterFullBodyMeshData()
	{
	}

	UPROPERTY(EditAnywhere, Category = Asset)
	TSoftObjectPtr<USkeletalMesh> SkeletalMeshAsset;

	UPROPERTY(EditAnywhere, Category = Asset)
	FT4EntityMaterialData OverrideMaterialData; // #80

	UPROPERTY(EditAnywhere, Category = Asset)
	TSoftObjectPtr<UPhysicsAsset> OverridePhysicsAsset; // #76 : Fullbody SK 라면 기본 세팅된 PhsycisAsset 을 그대로 사용하고, Override 할 경우만 재설정한다.
};

// #37
USTRUCT()
struct T4ASSET_API FT4EntityCharacterCompositePartMeshData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityCharacterCompositePartMeshData()
		: PartName(NAME_None)
	{
	}

	FORCEINLINE bool operator==(const FName& InKey) const
	{
		return (PartName == InKey) ? true : false;
	}

	FORCEINLINE bool operator==(const FT4EntityCharacterCompositePartMeshData& InRhs) const
	{
		return (PartName == InRhs.PartName) ? true : false;
	}

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	FName PartName;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UT4CostumeEntityAsset> CostumeEntityAsset;
};

// #37
USTRUCT()
struct T4ASSET_API FT4EntityCharacterCompositeMeshData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityCharacterCompositeMeshData()
		: ModularType(ET4EntityCharacterModularType::MasterPose)
	{
	}

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	ET4EntityCharacterModularType ModularType; // #72

	UPROPERTY()
	TMap<FName, FT4EntityCharacterCompositePartMeshData> DefaultPartsData_DEPRECATED; // #37, #124

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TArray<FT4EntityCharacterCompositePartMeshData> DefaultPartsDatas; // #37, #124
};

// #73
USTRUCT()
struct T4ASSET_API FT4EntityCharacterStanceData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityCharacterStanceData()
		: StanceName(NAME_None)
		, ActivePlayTag(NAME_None)
	{
	}

	FORCEINLINE bool operator==(const FName& InKey) const
	{
		return (StanceName == InKey) ? true : false;
	}

	FORCEINLINE bool operator==(const FT4EntityCharacterStanceData& InRhs) const
	{
		return (StanceName == InRhs.StanceName) ? true : false;
	}

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	FName StanceName;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UT4AnimSetAsset> AnimSetAsset; // #39

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	FName ActivePlayTag; // #74, #73
};

// #73
USTRUCT()
struct T4ASSET_API FT4EntityCharacterStanceSetData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityCharacterStanceSetData()
	{
	}

	UPROPERTY()
	TMap<FName, FT4EntityCharacterStanceData> StanceMap_DEPRECATED; // #124; // #39, #73

	UPROPERTY(EditAnywhere, Category = Asset)
	TArray<FT4EntityCharacterStanceData> StanceDatas; // #39, #73, #124
};

// #76
USTRUCT()
struct T4ASSET_API FT4EntityCharacterReactionPhysicsBlendData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityCharacterReactionPhysicsBlendData()
		: TargetWeight(1.0f)
		, BlendInTimeSec(0.0f)
		, BlendOutTimeSec(0.0f)
	{
	}

	UPROPERTY(EditAnywhere, Category = Property)
	float TargetWeight;

	UPROPERTY(EditAnywhere, Category = Property)
	float BlendInTimeSec;

	UPROPERTY(EditAnywhere, Category = Property)
	float BlendOutTimeSec;
};

// #76
USTRUCT()
struct T4ASSET_API FT4EntityCharacterReactionPhysicsStartData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityCharacterReactionPhysicsStartData()
		: DelayTimeSec(0.0f)
		, ImpulseMainActionPoint(NAME_None)
		, ImpulseSubActionPoint(NAME_None)
		, ImpulsePower(0.0f)
		, CenterOfMass(FVector::ZeroVector)
		, MassOverrideInKg(100.0f)
		, bSimulateBodiesBelow(false)
	{
	}

	UPROPERTY(EditAnywhere, Category = Property)
	float DelayTimeSec;

	UPROPERTY(EditAnywhere, Category = Property)
	FName ImpulseMainActionPoint;

	UPROPERTY(EditAnywhere, Category = Property)
	FName ImpulseSubActionPoint;

	UPROPERTY(EditAnywhere, Category = Property)
	float ImpulsePower;

	UPROPERTY(EditAnywhere, Category = Property)
	FVector CenterOfMass;

	UPROPERTY(EditAnywhere, Category = Property)
	float MassOverrideInKg;

	UPROPERTY(EditAnywhere, Category = Property)
	bool bSimulateBodiesBelow;

	UPROPERTY(EditAnywhere, Category = Property)
	FT4EntityCharacterReactionPhysicsBlendData BlendData;
};

// #76
USTRUCT()
struct T4ASSET_API FT4EntityCharacterReactionPhysicsStopData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityCharacterReactionPhysicsStopData()
		: DelayTimeSec(0.0f)
	{
	}

	UPROPERTY(EditAnywhere, Category = Property)
	float DelayTimeSec;
};

// #76
USTRUCT()
struct T4ASSET_API FT4EntityCharacterReactionAnimationData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityCharacterReactionAnimationData()
		: DelayTimeSec(0.0f)
		, StartAnimSectionName(NAME_None)
		, LoopAnimSectionName(NAME_None)
		, BlendInTimeSec(T4Const_DefaultAnimBlendTimeSec)
		, BlendOutTimeSec(T4Const_DefaultAnimBlendTimeSec)
	{
	}

	UPROPERTY(EditAnywhere, Category = Property)
	float DelayTimeSec;

	UPROPERTY(EditAnywhere, Category = Property)
	FName StartAnimSectionName; // only locomotion layer

	UPROPERTY(EditAnywhere, Category = Property)
	FName LoopAnimSectionName; // only locomotion layer

	UPROPERTY(EditAnywhere, Category = Property)
	float BlendInTimeSec;

	UPROPERTY(EditAnywhere, Category = Property)
	float BlendOutTimeSec;
};

// #76
USTRUCT()
struct T4ASSET_API FT4EntityCharacterReactionData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityCharacterReactionData()
		: ReactionName(NAME_None)
		, ReactionType(ET4EntityReactionType::None)
		, MaxPlayTimeSec(0.0f)
		, bUsePhysicsStart(false)
		, bUsePhysicsStop(false)
		, bUseAnimation(false)
#if WITH_EDITOR
		, TestShotDirection(FVector::UpVector) // #76
#endif
	{
	}

	FORCEINLINE bool operator==(const FName& InKey) const
	{
		return (ReactionName == InKey) ? true : false;
	}

	FORCEINLINE bool operator==(const FT4EntityCharacterReactionData& InRhs) const
	{
		return (ReactionName == InRhs.ReactionName) ? true : false;
	}

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	FName ReactionName;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	ET4EntityReactionType ReactionType;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	float MaxPlayTimeSec;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	bool bUsePhysicsStart;

	UPROPERTY(EditAnywhere, Category = ClientOnly, meta = (EditCondition = "bUsePhysicsStart"))
	FT4EntityCharacterReactionPhysicsStartData PhysicsStartData;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	bool bUsePhysicsStop;

	UPROPERTY(EditAnywhere, Category = ClientOnly, meta = (EditCondition = "bUsePhysicsStop"))
	FT4EntityCharacterReactionPhysicsStopData PhysicsStopData;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	bool bUseAnimation;

	UPROPERTY(EditAnywhere, Category = ClientOnly, meta = (EditCondition = "bUseAnimation"))
	FT4EntityCharacterReactionAnimationData AnimationData;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor)
	FVector TestShotDirection; // #76
#endif
};

USTRUCT()
struct T4ASSET_API FT4EntityCharacterReactionSetData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityCharacterReactionSetData()
	{
	}

	UPROPERTY()
	TMap<FName, FT4EntityCharacterReactionData> ReactionMap_DEPRECATED; // #124;

	UPROPERTY(EditAnywhere)
	TArray<FT4EntityCharacterReactionData> ReactionDatas; // #124;
};

UCLASS(ClassGroup = T4Framework, Category = "T4Framework")
class T4ASSET_API UT4CharacterEntityAsset : public UT4ActorEntityAsset
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UObject interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;

	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	//~ End UObject interface

public:
	ET4EntityType GetEntityType() const override { return ET4EntityType::Character; }

#if WITH_EDITOR
	virtual USkeletalMesh* GetPrimarySkeletalMeshAsset() const override // #81
	{
		if (ET4EntityCharacterMeshType::FullBody != MeshType)
		{
			return nullptr;
		}
		if (FullBodyMeshData.SkeletalMeshAsset.IsNull())
		{
			return nullptr;
		}
		return FullBodyMeshData.SkeletalMeshAsset.LoadSynchronous();
	}
#endif

public:
	UPROPERTY(EditAnywhere, Category=Default, AssetRegistrySearchable)
	TSoftObjectPtr<USkeleton> SkeletonAsset; // #39

	UPROPERTY(EditAnywhere, Category = Default)
	TSoftObjectPtr<UAnimBlueprint> AnimBlueprintAsset;

	UPROPERTY(EditAnywhere, Category= Default)
	ET4EntityCharacterMeshType MeshType;

	UPROPERTY(EditAnywhere, Category=FullbodyMesh)
	FT4EntityCharacterFullBodyMeshData FullBodyMeshData;

	UPROPERTY(EditAnywhere, Category=CompositeMesh)
	FT4EntityCharacterCompositeMeshData CopmpositeMeshData; // #37

	UPROPERTY(EditAnywhere, Category=Stance)
	FT4EntityCharacterStanceSetData StanceSetData; // #73

	UPROPERTY(EditAnywhere, Category=Reaction)
	FT4EntityCharacterReactionSetData ReactionSetData; // #76

	UPROPERTY(EditAnywhere, Category= Physical)
	FT4EntityCharacterPhysicalAttribute Physical;

	UPROPERTY(EditAnywhere, Category= Rendering)
	FT4EntityCharacterRenderingAttribute Rendering;
};
