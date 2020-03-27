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
	float DurationSec;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TSoftObjectPtr<UAnimSequence> AnimSequnceAsset_DEPRECATED; // #124

	UPROPERTY(EditAnywhere, Category = Editor)
	TSoftObjectPtr<UAnimSequence> AnimSequenceAsset;
#endif
};

USTRUCT()
struct T4ASSET_API FT4AnimSetBlendSpaceData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4AnimSetBlendSpaceData()
		: Name(NAME_None)
	{
	}

	FORCEINLINE bool operator==(const FName& InKey) const
	{
		return (Name == InKey) ? true : false;
	}

	FORCEINLINE bool operator==(const FT4AnimSetBlendSpaceData& InRhs) const
	{
		return (Name == InRhs.Name) ? true : false;
	}

	UPROPERTY(VisibleAnywhere, Category = ClientOnly)
	FName Name;

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UBlendSpaceBase> BlendSpaceAsset;
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

	// CustomizeSkillAnimationDetails

	UPROPERTY(EditAnywhere, Category = SkillLayer, meta = (DisplayName = "bAutoGen"))
	bool bSkillAnimMontageAutoGen; // #69

	UPROPERTY(EditAnywhere, Category = SkillLayer, meta = (DisplayName = "Anim Montage Asset"))
	TSoftObjectPtr<UAnimMontage> SkillAnimMontageAsset; // #69

	UPROPERTY(EditAnywhere, Category = SkillLayer)
	TArray<FT4AnimSetAnimSequenceData> SkillAnimSequenceArray;


	// CustomizeOverlayAnimationDetails

	UPROPERTY(EditAnywhere, Category = OverlayLayer, meta = (DisplayName = "bAutoGen"))
	bool bOverlayAnimMontageAutoGen; // #69

	UPROPERTY(EditAnywhere, Category = OverlayLayer, meta = (DisplayName = "Anim Montage Asset"))
	TSoftObjectPtr<UAnimMontage> OverlayAnimMontageAsset; // #69

	UPROPERTY(EditAnywhere, Category = OverlayLayer)
	TArray<FT4AnimSetAnimSequenceData> OverlayAnimSequenceArray;


	// CustomizeDefaultAnimationDetails

	UPROPERTY(EditAnywhere, Category = DefaultLayer, meta = (DisplayName = "bAutoGen"))
	bool bDefaultAnimMontageAutoGen; // #69

	UPROPERTY(EditAnywhere, Category = DefaultLayer, meta = (DisplayName = "Anim Montage Asset"))
	TSoftObjectPtr<UAnimMontage> DefaultAnimMontageAsset; // #38, #69

	UPROPERTY(EditAnywhere, Category = DefaultLayer)
	TArray<FT4AnimSetAnimSequenceData> DefaultAnimSequenceArray;


	// CustomizeBlendSpaceDetails

	UPROPERTY(EditAnywhere, Category = BlendSpace)
	TArray<FT4AnimSetBlendSpaceData> BlendSpaceArray;

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UTexture2D* ThumbnailImage; // Internal: The thumbnail image

	UPROPERTY()
	FT4AnimSetTestSettings EditorSettings_DEPRECATED; // #111 : 에디터 세팅 옵션

	UPROPERTY(EditAnywhere, Category = Editor)
	FT4AnimSetTestSettings TestSettings; // #111 : 에디터 세팅 옵션
#endif

private:
#if WITH_EDITOR
	FT4OnPropertiesChanged OnPropertiesChangedDelegate;
#endif
};
