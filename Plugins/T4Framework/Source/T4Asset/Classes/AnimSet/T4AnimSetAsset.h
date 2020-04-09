// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4AssetDefinitions.h"
#include "T4AnimSetAsset.generated.h"

/**
  * #39
 */
struct FT4AnimSetCustomVersion
{
	enum Type
	{
		InitializeVer = 0,

		CommonPropertyNameChanged, // #124
		CommonPropertyNameV2Changed, // #126

		SequenceAndStateLayerAdded, // #131
		PostureNameAdded, // #131

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1,
	};

	T4ASSET_API const static FGuid GUID;

private:
	FT4AnimSetCustomVersion() {}
};

class UTexture2D;
class USkeleton;
class UAnimSequence;
class UAnimMontage;
class UBlendSpaceBase;
class UT4EntityAsset;

USTRUCT()
struct T4ASSET_API FT4AnimSetAnimSequenceData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4AnimSetAnimSequenceData()
		: Name(NAME_None)
		, PostureName(T4Const_DefaultPostureName) // #131
		, DurationSec(0.0f)
	{
	}

	FORCEINLINE bool operator==(const FName& InKey) const
	{
		return (Name == InKey) ? true : false;
	}

	FORCEINLINE bool operator==(const FT4AnimSetAnimSequenceData& InRhs) const
	{
		return (Name == InRhs.Name) ? true : false;
	}

	UPROPERTY(VisibleAnywhere, Category = Common)
	FName Name;

	UPROPERTY(VisibleAnywhere, Category = Common)
	FName PostureName; // #131

	UPROPERTY(VisibleAnywhere, Category = Common)
	float DurationSec;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor)
	TSoftObjectPtr<UAnimSequence> AnimSequenceAsset;
#endif
};

// #131
USTRUCT()
struct T4ASSET_API FT4AnimSetSequenceLayerData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4AnimSetSequenceLayerData()
		: bAnimMontageAutoGen(true)
	{
	}

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	bool bAnimMontageAutoGen; // #69

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UAnimMontage> AnimMontageAsset; // #69

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TArray<FT4AnimSetAnimSequenceData> AnimSequenceArray;
};

// #131
USTRUCT()
struct T4ASSET_API FT4AnimSetBlendSpaceSetData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4AnimSetBlendSpaceSetData()
		: PostureName(NAME_None)
	{
	}

	FORCEINLINE bool operator==(const FName& InKey) const
	{
		return (PostureName == InKey) ? true : false;
	}

	FORCEINLINE bool operator==(const FT4AnimSetBlendSpaceSetData& InRhs) const
	{
		return (PostureName == InRhs.PostureName) ? true : false;
	}

	UPROPERTY()
	FName Name_DEPRECATED;

	UPROPERTY(VisibleAnywhere, Category = ClientOnly)
	FName PostureName;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UBlendSpaceBase> MoveBlendSpaceAsset;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UBlendSpaceBase> AimBlendSpaceAsset;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UBlendSpaceBase> FallBlendSpaceAsset;
};

// #131
USTRUCT()
struct T4ASSET_API FT4AnimSetStateLayerData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4AnimSetStateLayerData()
	{
	}

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TArray<FT4AnimSetBlendSpaceSetData> BlendSpaceSetDataArray;
};

// #111
class UT4WeaponEntityAsset;
USTRUCT()
struct T4ASSET_API FT4AnimSetTestSettings
{
	GENERATED_USTRUCT_BODY()

public:
	FT4AnimSetTestSettings()
	{
		Reset();
	}

	void Reset()
	{
#if WITH_EDITOR
		bAutoMounting = false; // #111
#endif
	}

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Testing)
	TSoftObjectPtr<UT4EntityAsset> PreviewEntityAsset;

	UPROPERTY(EditAnywhere, Category = Testing)
	TSoftObjectPtr<UT4WeaponEntityAsset> EquipWeaponEntityAsset; // #111

	UPROPERTY(EditAnywhere, Category = Testing)
	bool bAutoMounting; // #111
#endif
};

UCLASS(ClassGroup = T4Framework, Category = "T4Framework")
class T4ASSET_API UT4AnimSetAsset : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UObject interface
	void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	//~ End UObject interface

#if WITH_EDITOR
	DECLARE_MULTICAST_DELEGATE(FT4OnPropertiesChanged);
	FT4OnPropertiesChanged& OnPropertiesChanged() { return OnPropertiesChangedDelegate; }

#endif // WITH_EDITOR

public:
	UPROPERTY(EditAnywhere, Category = Default)
	TSoftObjectPtr<USkeleton> SkeletonAsset;

	// CustomizeSkillLayerDetails

	UPROPERTY(EditAnywhere, Category = SkillLayer)
	FT4AnimSetSequenceLayerData SkillLayerData; // #131

	// CustomizeOverlayLayerDetails

	UPROPERTY(EditAnywhere, Category = OverlayLayer)
	FT4AnimSetSequenceLayerData OverlayLayerData; // #131

	// CustomizeDefaultLayerDetails

	UPROPERTY(EditAnywhere, Category = DefaultLayer)
	FT4AnimSetSequenceLayerData DefaultLayerData; // #131
	
	// CustomizeStateLayerDetails

	UPROPERTY(EditAnywhere, Category = StateLayer)
	FT4AnimSetStateLayerData StateLayerData; // #131

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UTexture2D* ThumbnailImage; // Internal: The thumbnail image

	UPROPERTY(EditAnywhere, Category = Editor)
	FT4AnimSetTestSettings TestSettings; // #111 : 에디터 세팅 옵션
#endif

private:
#if WITH_EDITOR
	FT4OnPropertiesChanged OnPropertiesChangedDelegate;
#endif
};
