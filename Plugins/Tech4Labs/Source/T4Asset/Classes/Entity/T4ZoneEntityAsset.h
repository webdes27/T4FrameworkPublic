// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4EntityAsset.h"
#include "Classes/World/T4EnvironmentAsset.h"
#include "T4ZoneEntityAsset.generated.h"

/**
  * #94
 */
struct FT4ZoneEntityCustomVersion
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
	FT4ZoneEntityCustomVersion() {}
};

USTRUCT()
struct T4ASSET_API FT4EntityZoneDebugData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityZoneDebugData()
#if WITH_EDITOR
		: DebugColor(FColor::Blue)
		, DebugSegments(32)
#endif
	{
	}

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	FColor DebugColor; // #92

	UPROPERTY(EditAnywhere, meta = (ClampMin = "8", UIMin = "8", UIMax = "128"))
	int32 DebugSegments; // #94
#endif
};

USTRUCT()
struct T4ASSET_API FT4EntityZoneData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityZoneData()
		: BlendPriority(6) // #92, #93, #94 : Map 은 0~5 까지, Dynamic 은 6 이상
		, BlendInTimeSec(1.0f)
		, BlendOutTimeSec(1.0f)
		, BrushType(ET4EntityZoneBrushType::Cylinder)
		, HalfHeight(1000.0f)
		, Radius(2500.0f)
	{
	}

	UPROPERTY(EditAnywhere, meta = (ClampMin = "6", UIMin = "6", UIMax = "10"))
	int32 BlendPriority; // #92, #93, #94 : Map 은 0~5 까지, Dynamic 은 6 이상

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "10.0"))
	float BlendInTimeSec; // #92

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0", UIMax = "10.0"))
	float BlendOutTimeSec; // #92

	UPROPERTY(EditAnywhere)
	ET4EntityZoneBrushType BrushType;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "1000.0", UIMax = "100000.0"))
	float HalfHeight;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "1000.0", UIMax = "100000.0"))
	float Radius;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	FT4EntityZoneDebugData DebugData; // #92
#endif
};

USTRUCT()
struct T4ASSET_API FT4EntityZoneEnvironmentData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4EntityZoneEnvironmentData()
	{
	}

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UT4EnvironmentAsset> EnvironmentAsset; // #90, #92
};

UCLASS(ClassGroup = Tech4Labs, Category = "Tech4Labs")
class T4ASSET_API UT4ZoneEntityAsset : public UT4EntityAsset
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UObject interface
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;

	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	//~ End UObject interface

public:
	ET4EntityType GetEntityType() const override { return ET4EntityType::Zone; }

public:
	UPROPERTY(EditAnywhere, Category = Default)
	FT4EntityZoneData ZoneData;

	UPROPERTY(EditAnywhere, Category = Environment)
	FT4EntityZoneEnvironmentData ZoneEnvironmentData;
};
