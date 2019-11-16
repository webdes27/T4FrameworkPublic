// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ExponentialHeightFogComponent.h" // #90
#include "T4MapEnvironmentAsset.generated.h"

/**
  * #90
 */
struct FT4MapEnvironmentCustomVersion
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
	FT4MapEnvironmentCustomVersion() {}
};

// WARN : Envrionment Data 추가 시는 아래 테그를 찾아 추가 구현을 해주어야 함! // #93
// #T4_ADD_TOD_TAG

// #93
USTRUCT()
struct T4ASSET_API FT4MapDirectionalData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4MapDirectionalData()
		: bEnabled(false)
		, Rotation(FRotator::ZeroRotator)
	{
	}

	UPROPERTY(EditAnywhere)
	bool bEnabled;

	// #92 : 프로퍼티 추가시 FT4WorldEnvironmentBlender::BlendDirectional 구현 필요!!

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnabled"))
	FRotator Rotation;
};


// #90
USTRUCT()
struct T4ASSET_API FT4MapDirectionalLightData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4MapDirectionalLightData()
		: bEnabled(false)
		, Intensity(10.0f)
		, LightColor(FColor::White)
	{
	}

	UPROPERTY(EditAnywhere)
	bool bEnabled;

	// #92 : 프로퍼티 추가시 FT4WorldEnvironmentBlender::BlendDirectionalLight 구현 필요!!

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnabled"))
	float Intensity;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnabled"))
	FColor LightColor;
};

// #90
class UTextureCube;
USTRUCT()
struct T4ASSET_API FT4MapSkyLightData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4MapSkyLightData()
		: bEnabled(false)
		, CubemapResolution(1024)
		, Intensity(10.0f)
		, LightColor(FColor::White)
	{
	}

	UPROPERTY(EditAnywhere)
	bool bEnabled;

	// #92 : 프로퍼티 추가시 FT4WorldEnvironmentBlender::BlendSkyLight 구현 필요!!

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnabled"))
	TSoftObjectPtr<UTextureCube> CubemapPtr;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnabled"))
	int32 CubemapResolution;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnabled"))
	float Intensity;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnabled"))
	FColor LightColor;
};

// #90
// https://docs.unrealengine.com/ko/Engine/Actors/FogEffects/AtmosphericFog/index.html
USTRUCT()
struct T4ASSET_API FT4MapAtmosphericFogData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4MapAtmosphericFogData()
		: bEnabled(false)
		, SunMultiplier(0.0f)
		, FogMultiplier(0.0f)
		, DensityMultiplier(0.0f)
		, DensityOffset(0.0f)
	{
	}

	UPROPERTY(EditAnywhere)
	bool bEnabled;

	// #92 : 프로퍼티 추가시 FT4WorldEnvironmentBlender::BlendAtmosphericFog 구현 필요!!

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnabled"))
	float SunMultiplier;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnabled"))
	float FogMultiplier;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnabled"))
	float DensityMultiplier;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnabled"))
	float DensityOffset;
};

// #90
USTRUCT()
struct T4ASSET_API FT4MapExponentialHeightFogData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4MapExponentialHeightFogData()
		: bEnabled(false)
		, FogDensity(0.02f)
		, FogHeightFalloff(0.2f)
		, FogInscatteringColor(FLinearColor::White)
	{
	}

	UPROPERTY(EditAnywhere)
	bool bEnabled;

	// #92 : 프로퍼티 추가시 FT4WorldEnvironmentBlender::BlendExponentialHeightFog 구현 필요!!

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnabled"))
	float FogDensity;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnabled"))
	float FogHeightFalloff;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnabled"))
	FExponentialHeightFogData SecondFogData;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bEnabled"))
	FLinearColor FogInscatteringColor;
};

// #90
USTRUCT()
struct T4ASSET_API FT4MapTimeTagData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4MapTimeTagData()
		: Name(NAME_None)
	{
	}

	UPROPERTY(EditAnywhere)
	FName Name;

	UPROPERTY(EditAnywhere)
	FT4MapDirectionalData DirectionalData; // #93

	UPROPERTY(EditAnywhere)
	FT4MapDirectionalLightData DirectionalLightData;

	UPROPERTY(EditAnywhere)
	FT4MapSkyLightData SkyLightData;

	UPROPERTY(EditAnywhere)
	FT4MapAtmosphericFogData AtmosphericFogData;

	UPROPERTY(EditAnywhere)
	FT4MapExponentialHeightFogData ExponentialHeightFogData;
};

// #90
USTRUCT()
struct T4ASSET_API FT4MapTimeTagSetData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4MapTimeTagSetData()
	{
	}

	UPROPERTY(EditAnywhere)
	TMap<FName, FT4MapTimeTagData> TimeTagMap;
};

class UTexture2D;
class UT4WorldAsset;
UCLASS(ClassGroup = Tech4Labs, Category = "Tech4Labs")
class T4ASSET_API UT4MapEnvironmentAsset : public UObject
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
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Time of Day"))
	FT4MapTimeTagSetData TimeTagSetData; // #90 : 이후 별도 Asset 으로 분리!

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor)
	TSoftObjectPtr<UT4WorldAsset> PreviewWorldAsset;
#endif

private:
#if WITH_EDITOR
	FT4OnPropertiesChanged OnPropertiesChangedDelegate;
#endif
};
