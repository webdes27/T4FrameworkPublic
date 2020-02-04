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
struct T4ASSET_API FT4AnimSetSequenceInfo
{
	GENERATED_USTRUCT_BODY()

public:
	FT4AnimSetSequenceInfo()
		: Name(NAME_None)
		, DurationSec(0.0f)
	{
	}

	FORCEINLINE bool operator==(const FName& InKey) const
	{
		return (Name == InKey) ? true : false;
	}

	FORCEINLINE bool operator==(const FT4AnimSetSequenceInfo& InRhs) const
	{
		return (Name == InRhs.Name) ? true : false;
	}

	UPROPERTY(VisibleAnywhere, Category = AnimSequenceInfo)
	FName Name;

	UPROPERTY(VisibleAnywhere, Category = AnimSequenceInfo)
	float DurationSec;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = AnimSequenceInfo)
	TSoftObjectPtr<UAnimSequence> AnimSequnceAsset;
#endif
};

USTRUCT()
struct T4ASSET_API FT4AnimSetBlendSpaceInfo
{
	GENERATED_USTRUCT_BODY()

public:
	FT4AnimSetBlendSpaceInfo()
		: Name(NAME_None)
	{
	}

	FORCEINLINE bool operator==(const FName& InKey) const
	{
		return (Name == InKey) ? true : false;
	}

	FORCEINLINE bool operator==(const FT4AnimSetBlendSpaceInfo& InRhs) const
	{
		return (Name == InRhs.Name) ? true : false;
	}

	UPROPERTY(VisibleAnywhere, Category = BlendSpaceInfo)
	FName Name;

	UPROPERTY(EditAnywhere, Category = BlendSpaceInfo)
	TSoftObjectPtr<UBlendSpaceBase> BlendSpaceAsset;
};

// #111
class UT4WeaponEntityAsset;
USTRUCT()
struct T4ASSET_API FT4AnimSetEditorSettings
{
	GENERATED_USTRUCT_BODY()

public:
	FT4AnimSetEditorSettings()
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

// #39
USTRUCT()
struct T4ASSET_API FT4AnimSetEditorTransientData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4AnimSetEditorTransientData()
	{
		Reset();
	}

	void Reset()
	{
#if WITH_EDITOR
		TransientSelectAnimSectionName = NAME_None;
		TransientSelectBlendSpaceName = NAME_None;
#endif
	}

	// #39 : WARN : CustomDetails 에서 사용하는 임시 프로퍼티! (저장되지 않는다!!)
	UPROPERTY(EditAnywhere, Transient)
	FName TransientSelectAnimSectionName;

	UPROPERTY(EditAnywhere, Transient)
	TSoftObjectPtr<UAnimSequence> TransientAnimSequenceAsset;

	UPROPERTY(EditAnywhere, Transient)
	FName TransientSelectBlendSpaceName;

	UPROPERTY(EditAnywhere, Transient)
	TSoftObjectPtr<UBlendSpaceBase> TransientBlendSpaceAsset;
	// ~#39 : WARN : CustomDetails 에서 사용하는 임시 프로퍼티!
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
	TArray<FT4AnimSetSequenceInfo> SkillAnimSequenceArray;


	// CustomizeOverlayAnimationDetails

	UPROPERTY(EditAnywhere, Category = OverlayLayer, meta = (DisplayName = "bAutoGen"))
	bool bOverlayAnimMontageAutoGen; // #69

	UPROPERTY(EditAnywhere, Category = OverlayLayer, meta = (DisplayName = "Anim Montage Asset"))
	TSoftObjectPtr<UAnimMontage> OverlayAnimMontageAsset; // #69

	UPROPERTY(EditAnywhere, Category = OverlayLayer)
	TArray<FT4AnimSetSequenceInfo> OverlayAnimSequenceArray;


	// CustomizeDefaultAnimationDetails

	UPROPERTY(EditAnywhere, Category = DefaultLayer, meta = (DisplayName = "bAutoGen"))
	bool bDefaultAnimMontageAutoGen; // #69

	UPROPERTY(EditAnywhere, Category = DefaultLayer, meta = (DisplayName = "Anim Montage Asset"))
	TSoftObjectPtr<UAnimMontage> DefaultAnimMontageAsset; // #38, #69

	UPROPERTY(EditAnywhere, Category = DefaultLayer)
	TArray<FT4AnimSetSequenceInfo> DefaultAnimSequenceArray;


	// CustomizeBlendSpaceDetails

	UPROPERTY(EditAnywhere, Category = BlendSpace)
	TArray<FT4AnimSetBlendSpaceInfo> BlendSpaceArray;

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UTexture2D* ThumbnailImage; // Internal: The thumbnail image

	UPROPERTY(EditAnywhere, Category = Editor)
	FT4AnimSetEditorSettings EditorSettings; // #111 : 에디터 세팅 옵션

	// #39 : WARN : AnimSetCustomDetails 에서 사용하는 임시 프로퍼티! (저장되지 않는다!!)
	// TODO : Transient 설정으로 Editor Dirty 가 발생함으로 다른 방법 고려 필요
	UPROPERTY(EditAnywhere, Transient)
	FT4AnimSetEditorTransientData EditorTransientData;
#endif

private:
#if WITH_EDITOR
	FT4OnPropertiesChanged OnPropertiesChangedDelegate;
#endif
};
