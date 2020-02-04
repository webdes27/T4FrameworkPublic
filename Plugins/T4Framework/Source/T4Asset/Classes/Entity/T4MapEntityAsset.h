// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4EntityAsset.h"
#include "T4MapEntityAsset.generated.h"

/**
  * #35
 */
struct FT4MapEntityCustomVersion
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
	FT4MapEntityCustomVersion() {}
};

// #84, #104 : WorldAsset 의 Tile 을 MapEntity 로 이전!
USTRUCT()
struct T4ASSET_API FT4LevelThumbnailData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4LevelThumbnailData()
#if WITH_EDITOR
		: ImageWidth(0)
		, ImageHeight(0)
#endif
	{
	}

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere) /** Thumbnail width (serialized) */
	int32 ImageWidth;

	UPROPERTY(EditAnywhere) /** Thumbnail height (serialized) */
	int32 ImageHeight;

	UPROPERTY(EditAnywhere) /** Compressed image data (serialized) */
	TArray<uint8> CompressedImageData;

	UPROPERTY(Transient) /** Image data bytes */
	TArray<uint8> RawImageData;
#endif
};

USTRUCT()
struct T4ASSET_API FT4EntityMapData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityMapData()
	{
	}

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> LevelAsset;
};

UCLASS(ClassGroup = T4Framework, Category = "T4Framework")
class T4ASSET_API UT4MapEntityAsset : public UT4EntityAsset
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UObject interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;

	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	//~ End UObject interface

public:
	ET4EntityType GetEntityType() const override { return ET4EntityType::Map; }

public:
	UPROPERTY(EditAnywhere)
	FT4EntityMapData MapData;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TMap<FName, FT4LevelThumbnailData> LevelThumbnailDatas; // #84, #104 : WorldAsset 의 Tile 을 MapEntity 로 이전!
#endif
};
