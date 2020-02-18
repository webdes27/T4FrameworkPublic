// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4GameBuiltin_GameDataTypes.h"
#include "Classes/Common/T4CommonAssetStructs.h" // #103
#include "T4GameBuiltin_ContentSpawnAsset.generated.h"

/**
  * #118
 */
struct ET4GameBuiltin_ContentSpawnVersion
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
	ET4GameBuiltin_ContentSpawnVersion() {}
};

USTRUCT()
struct T4GAMEBUILTIN_API FT4GameBuiltin_SpawnEntityInfo
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameBuiltin_SpawnEntityInfo()
	{
	}

	FORCEINLINE bool operator==(const FName& InKey) const
	{
		return (ID == InKey) ? true : false;
	}

	FORCEINLINE bool operator==(const FT4GameBuiltin_SpawnEntityInfo& InRhs) const
	{
		return (ID == InRhs.ID) ? true : false;
	}

	// FT4ContentSpawnEntityDetails::CustomizeDetails

	UPROPERTY(VisibleAnywhere, Category = Common)
	FName ID;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameBuiltin_GameNPCDataID NPCDataID;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor)
	FString Description;
#endif
};

USTRUCT()
struct T4GAMEBUILTIN_API FT4GameBuiltin_SpawnActorInfo
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameBuiltin_SpawnActorInfo()
		: SpawnEntityID(NAME_None)
	{
	}

	FORCEINLINE bool operator==(const FName& InKey) const
	{
		return (ID == InKey) ? true : false;
	}

	FORCEINLINE bool operator==(const FT4GameBuiltin_SpawnActorInfo& InRhs) const
	{
		return (ID == InRhs.ID) ? true : false;
	}

	// FT4ContentSpawnActorDetails::CustomizeDetails

	UPROPERTY(VisibleAnywhere, Category = Common)
	FName ID;

	UPROPERTY(VisibleAnywhere, Category = ServerOnly)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FName SpawnEntityID;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FTransform Transform;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor)
	FString Description;
#endif
};

class UTexture2D;
class UT4GameBuiltin_ContentAsset;
UCLASS(ClassGroup = T4Framework, Category = "T4Framework")
class T4GAMEBUILTIN_API UT4GameBuiltin_ContentSpawnAsset : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UObject interface
	void Serialize(FArchive& Ar) override;
	void PostLoad() override;

public:
	UPROPERTY(EditAnywhere, Category = ServerOnly)
	TArray<FT4GameBuiltin_SpawnEntityInfo> SpawnEntityArray;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	TArray<FT4GameBuiltin_SpawnActorInfo> SpawnActorArray;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor, AssetRegistrySearchable)
	TSoftObjectPtr<UT4GameBuiltin_ContentAsset> ParentContentAsset;

	UPROPERTY()
	UTexture2D* ThumbnailImage; // Internal: The thumbnail image
#endif
};