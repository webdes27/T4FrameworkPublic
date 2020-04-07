// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4GameDataTypes.h"
#include "Classes/Common/T4CommonAssetStructs.h" // #103
#include "T4ContentWorldAsset.generated.h"

/**
  * #118
 */
struct FT4ContentWorldVersion
{
	enum Type
	{
		InitializeVer = 0,

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1,
	};

	T4GAMEPLAY_API const static FGuid GUID;

private:
	FT4ContentWorldVersion() {}
};

class UT4ContentSpawnAsset;

USTRUCT()
struct T4GAMEPLAY_API FT4ContentSpawnLayerData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4ContentSpawnLayerData()
		: TimeTag(NAME_None)
#if WITH_EDITORONLY_DATA
		, ParentID(NAME_None) // #122
		, FolderName(NAME_None) // #122
#endif
	{
	}

	FORCEINLINE bool operator==(const FName& InKey) const
	{
		return (ID == InKey) ? true : false;
	}

	FORCEINLINE bool operator==(const FT4ContentSpawnLayerData& InRhs) const
	{
		return (ID == InRhs.ID) ? true : false;
	}

	UPROPERTY(VisibleAnywhere, Category = Common)
	FName ID;

	UPROPERTY(VisibleAnywhere, Category = ServerOnly)
	FGuid Guid;

	UPROPERTY(VisibleAnywhere, Category = ServerOnly)
	FName TimeTag;
	
	UPROPERTY(EditAnywhere, Category = ServerOnly)
	TSoftObjectPtr<UT4ContentSpawnAsset> SpawnAsset;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor)
	FString Description;

	UPROPERTY(VisibleAnywhere, Category = Hide)
	FName ParentID; // #122

	UPROPERTY(VisibleAnywhere, Category = Hide)
	FName FolderName; // #122
#endif
};

class UTexture2D;
UCLASS(ClassGroup = T4Framework, Category = "T4Framework")
class T4GAMEPLAY_API UT4ContentWorldAsset : public UObject
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
	FT4GameWorldDataID WorldDataID;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	TArray<FT4ContentSpawnLayerData> SpawnLayerArray;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor)
	FT4EditorTestAutomationData TestAutomation; // #100, #103

	UPROPERTY()
	UTexture2D* ThumbnailImage; // Internal: The thumbnail image
#endif

private:
#if WITH_EDITOR
	FT4OnPropertiesChanged OnPropertiesChangedDelegate;
#endif
};