// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Asset/Classes/Entity/T4MapEntityAsset.h" // #87
#include "T4WorldAsset.generated.h"

/**
  * #83
 */
struct FT4WorldCustomVersion
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
	FT4WorldCustomVersion() {}
};

// #84
USTRUCT()
struct T4ASSET_API FT4WorldTileThumbnailData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4WorldTileThumbnailData()
		: ImageWidth(0)
		, ImageHeight(0)
	{
	}

	UPROPERTY(EditAnywhere) /** Thumbnail width (serialized) */
	int32 ImageWidth;

	UPROPERTY(EditAnywhere) /** Thumbnail height (serialized) */
	int32 ImageHeight;

	UPROPERTY(EditAnywhere) /** Compressed image data (serialized) */
	TArray<uint8> CompressedImageData;

	UPROPERTY(Transient) /** Image data bytes */
	TArray<uint8> RawImageData;
};

// #90
class UT4MapEnvironmentAsset;
USTRUCT()
struct T4ASSET_API FT4WorldEditorTransientData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4WorldEditorTransientData()
//		: TransientDebugColor(FColor(255, 255, 255, 32))
	{
		Reset();
	}

	void Reset()
	{
#if WITH_EDITOR
		TransientMapZoneName = NAME_None;
		TransientTransform.SetIdentity();
		TransientBlendPriority = 0;
		TransientBlendInTimeSec = 1.0f;
		TransientBlendOutTimeSec = 1.0f;
		TransientDebugColor = FColor(255, 255, 255, 32);
		TransientMapEnvironmentAsset.Reset();
#endif
	}

	UPROPERTY(EditAnywhere, Transient, meta = (DisplayName = "Name"))
	FName TransientMapZoneName; // #92

	UPROPERTY(EditAnywhere, Transient, meta = (DisplayName = "Transform"))
	FTransform TransientTransform; // #92

	UPROPERTY(EditAnywhere, Transient, meta = (DisplayName = "Blend Priority", ClampMin = "0", UIMin = "0", UIMax = "5"))
	int32 TransientBlendPriority; // #92

	UPROPERTY(EditAnywhere, Transient, meta = (DisplayName = "Blend In TimeSec", ClampMin = "0.0", UIMin = "0.0", UIMax = "10.0"))
	float TransientBlendInTimeSec; // #92

	UPROPERTY(EditAnywhere, Transient, meta = (DisplayName = "Blend Out TimeSec", ClampMin = "0.0", UIMin = "0.0", UIMax = "10.0"))
	float TransientBlendOutTimeSec; // #92

	UPROPERTY(EditAnywhere, Transient, meta = (DisplayName = "DebugColor"))
	FColor TransientDebugColor; // #92

	UPROPERTY(EditAnywhere, Transient, meta = (DisplayName = "Map Environment Asset"))
	TSoftObjectPtr<UT4MapEnvironmentAsset> TransientMapEnvironmentAsset; // #90
};

class UTexture2D;
UCLASS(ClassGroup = Tech4Labs, Category = "Tech4Labs")
class T4ASSET_API UT4WorldAsset : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UObject interface
	void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	//~ End UObject interface

#if WITH_EDITOR
	DECLARE_MULTICAST_DELEGATE(FT4OnPropertiesChanged);
	FT4OnPropertiesChanged& OnPropertiesChanged() { return OnPropertiesChangedDelegate; }
#endif // WITH_EDITOR

public:
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Map Entity Asset"))
	TSoftObjectPtr<UT4MapEntityAsset> MapEntityAsset; // #87

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UTexture2D* ThumbnailImage; // Internal: The thumbnail image

	UPROPERTY()
	TMap<FName, FT4WorldTileThumbnailData> WorldTileThumbnailDatas; // #84

	// #71 : WARN : FT4WorldDetailCustomization 에서 사용하는 임시 프로퍼티! (저장되지 않는다!!)
	UPROPERTY(EditAnywhere, Transient)
	FT4WorldEditorTransientData EditorTransientData;
#endif

private:
#if WITH_EDITOR
	FT4OnPropertiesChanged OnPropertiesChangedDelegate;
#endif
};
