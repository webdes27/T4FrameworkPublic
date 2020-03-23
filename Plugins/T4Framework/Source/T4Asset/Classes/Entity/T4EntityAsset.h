// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4AssetDefinitions.h"
#include "Public/Entity/T4EntityTypes.h"
#include "Classes/Common/T4CommonAssetStructs.h" // #103
#include "T4EntityAsset.generated.h"

/**
  * #35
 */
USTRUCT()
struct T4ASSET_API FT4EntityEditorThumbnailAttribute
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityEditorThumbnailAttribute()
#if WITH_EDITORONLY_DATA
		: Rotation(FRotator(0.0f, 180.0f, 0.0f))
		, Location(FVector(500.0f, 0.0f, 100.0f))
#endif
	{
	}

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	FRotator Rotation;

	UPROPERTY(EditAnywhere)
	FVector Location;
#endif
};

USTRUCT()
struct T4ASSET_API FT4EntityBasePhysicalAttribute
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityBasePhysicalAttribute()
		: bCollisionDisabled(false) // #118
		, CapsuleHeight(200.0f)
		, CapsuleRadius(25.0f)

	{
	}

	// CustomizeCharacterEntityDetails
	// CustomizeItemCommonEntityDetails

	UPROPERTY(EditAnywhere)
	bool bCollisionDisabled; // #118

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1000"))
	float CapsuleHeight;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "10.0", ClampMax = "500"))
	float CapsuleRadius;
};

USTRUCT()
struct T4ASSET_API FT4EntityBaseRenderingAttribute
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityBaseRenderingAttribute()
		: Scale(1.0f)
		, ImportRotationYaw(-90.0f)
		, bReceivesDecals(false) // #54
	{
	}

	// CustomizeCharacterEntityDetails
	// CustomizeItemCommonEntityDetails

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.1", ClampMax = "10"))
	float Scale;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "-360.0", ClampMax = "360.0"))
	float ImportRotationYaw;

	UPROPERTY(EditAnywhere)
	bool bReceivesDecals; // #54
};

// #80
class UMaterialInterface;
USTRUCT()
struct T4ASSET_API FT4EntityMaterialSlotData // #124
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityMaterialSlotData()
	{
	}

	FORCEINLINE bool operator==(const FName& InKey) const
	{
		return (SlotName == InKey) ? true : false;
	}

	FORCEINLINE bool operator==(const FT4EntityMaterialSlotData& InRhs) const
	{
		return (SlotName == InRhs.SlotName) ? true : false;
	}

	UPROPERTY(VisibleAnywhere, Category = ClientOnly)
	FName SlotName;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UMaterialInterface> MaterialAsset;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = Editor)
	TSoftObjectPtr<UMaterialInterface> OriginalMaterialAsset;
#endif
};

USTRUCT()
struct T4ASSET_API FT4EntityMaterialData // #124
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityMaterialData()
	{
	}

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TArray<FT4EntityMaterialSlotData> MaterialSlotDatas;
};

// #74
class UT4WeaponEntityAsset;
USTRUCT()
struct T4ASSET_API FT4EntityPlayTagAttachmentData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityPlayTagAttachmentData()
		: PlayTag(T4Const_DefaultPlayTagName)
	{
	}

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	FName PlayTag;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	FName EquipPoint;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UT4WeaponEntityAsset> WeaponEntityAsset;
};

// #74
class UT4ActionAsset;
USTRUCT()
struct T4ASSET_API FT4EntityPlayTagActionData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityPlayTagActionData()
		: PlayTag(T4Const_DefaultPlayTagName)
	{
	}

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	FName PlayTag;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UT4ActionAsset> ActionAsset;
};

// #80
USTRUCT()
struct T4ASSET_API FT4EntityPlayTagMaterialData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityPlayTagMaterialData()
	{
	}

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	FName PlayTag;

	UPROPERTY(EditAnywhere, Category = Hide)
	FT4EntityMaterialData OverrideMaterialData;
};

// #74
USTRUCT()
struct T4ASSET_API FT4EntityPlayTagData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityPlayTagData()
	{
	}

	UPROPERTY(EditAnywhere)
	TArray<FT4EntityPlayTagMaterialData> MaterialTags; // #80

	UPROPERTY(EditAnywhere)
	TArray<FT4EntityPlayTagAttachmentData> AttachmentTags;

	UPROPERTY(EditAnywhere)
	TArray<FT4EntityPlayTagActionData> ActionTags;
};

class UTexture2D;
class UStaticMesh; // #81
class USkeletalMesh; // #81
UCLASS(ClassGroup = T4Framework, Category = "T4Framework")
class T4ASSET_API UT4EntityAsset : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UObject interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	//~ End UObject interface

public:
	virtual ET4EntityType GetEntityType() const { return ET4EntityType::None; }
	FName GetEntityKeyPath() const; // #37 : Make FT4EntityKey, ObjectPath

	FString GetEntityDisplayName() const; // #87
	const TCHAR* GetEntityTypeString() const; // #87

#if WITH_EDITOR
	virtual UStaticMesh* GetPrimaryStaticMeshAsset() const { return nullptr; } // #81
	virtual USkeletalMesh* GetPrimarySkeletalMeshAsset() const { return nullptr; } // #81

	DECLARE_MULTICAST_DELEGATE(FT4OnPropertiesChanged);
	FT4OnPropertiesChanged& OnPropertiesChanged() { return OnPropertiesChangedDelegate; }
#endif

public:
	UPROPERTY(EditAnywhere)
	FT4EntityPlayTagData PlayTagData; // #74, #81

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor)
	FT4EditorTestAutomation TestAutomation; // #100, #103

	UPROPERTY(EditAnywhere, Category = Editor)
	FT4EntityEditorThumbnailAttribute ThumbnailCameraInfo;

	UPROPERTY()
	UTexture2D* ThumbnailImage; // Internal: The thumbnail image
#endif

private:
#if WITH_EDITOR
	FT4OnPropertiesChanged OnPropertiesChangedDelegate;
#endif
};
