// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4GameBuiltin_GameDataTypes.h"
#include "Classes/Common/T4CommonAssetStructs.h" // #103
#include "T4GameBuiltin_ContentAsset.generated.h"

/**
  * #118
 */
struct ET4GameBuiltin_ContentVersion
{
	enum Type
	{
		InitializeVer = 0,

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1,
	};

	T4GAMEBUILTIN_API const static FGuid GUID;

private:
	ET4GameBuiltin_ContentVersion() {}
};

class UT4GameBuiltin_ContentSpawnAsset;

USTRUCT()
struct T4GAMEBUILTIN_API FT4GameBuiltin_SpawnLayerInfo
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameBuiltin_SpawnLayerInfo()
	{
	}

	FORCEINLINE bool operator==(const FName& InKey) const
	{
		return (ID == InKey) ? true : false;
	}

	FORCEINLINE bool operator==(const FT4GameBuiltin_SpawnLayerInfo& InRhs) const
	{
		return (ID == InRhs.ID) ? true : false;
	}

	UPROPERTY(VisibleAnywhere, Category = Common)
	FName ID;

	UPROPERTY(VisibleAnywhere, Category = ServerOnly)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	TSoftObjectPtr<UT4GameBuiltin_ContentSpawnAsset> ContentSpawnAsset;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor)
	FString Description;
#endif
};

class UTexture2D;
UCLASS(ClassGroup = T4Framework, Category = "T4Framework")
class T4GAMEBUILTIN_API UT4GameBuiltin_ContentAsset : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UObject interface
	void Serialize(FArchive& Ar) override;
	void PostLoad() override;

#if WITH_EDITOR
	DECLARE_MULTICAST_DELEGATE(FT4OnPropertiesChanged);
	FT4OnPropertiesChanged& OnPropertiesChanged() { return OnPropertiesChangedDelegate; }
#endif // WITH_EDITOR

public:
	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameBuiltin_GameWorldDataID WorldDataID;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	TArray<FT4GameBuiltin_SpawnLayerInfo> SpawnLayerArray;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor)
	FT4EditorTestAutomation TestAutomation; // #100, #103

	UPROPERTY()
	UTexture2D* ThumbnailImage; // Internal: The thumbnail image
#endif

private:
#if WITH_EDITOR
	FT4OnPropertiesChanged OnPropertiesChangedDelegate;
#endif
};