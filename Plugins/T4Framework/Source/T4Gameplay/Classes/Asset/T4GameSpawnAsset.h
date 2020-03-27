// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4GameDataTypes.h"
#include "Classes/Common/T4CommonAssetStructs.h" // #103
#include "T4Engine/Public/T4EngineTypes.h" // #126
#include "T4GameSpawnAsset.generated.h"

/**
  * #118
 */
struct FT4GameSpawnVersion
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
	FT4GameSpawnVersion() {}
};

// #114
USTRUCT()
struct T4GAMEPLAY_API FT4GameOverrideBehaviorData // #114
{
	GENERATED_USTRUCT_BODY()

public:
	// #114
	// FT4GameAIBehaviorData : AIController (Runtime)
	// FT4GameNPCBehaviorData : NPC DataTable (Original Set)
	// FT4GameOverrideBehaviorData : Spawn Asset (Instance Set)

	// Override
	UPROPERTY(EditAnywhere, meta = (PinHiddenByDefault, InlineEditConditionToggle))
	bool bOverride_EnemyType;

	UPROPERTY(EditAnywhere, meta = (PinHiddenByDefault, InlineEditConditionToggle))
	bool bOverride_Aggressive;

	UPROPERTY(EditAnywhere, meta = (PinHiddenByDefault, InlineEditConditionToggle))
	bool bOverride_ActiveOrKeepAggroTimeSec;

	UPROPERTY(EditAnywhere, meta = (PinHiddenByDefault, InlineEditConditionToggle))
	bool bOverride_AgentRadius;

	UPROPERTY(EditAnywhere, meta = (PinHiddenByDefault, InlineEditConditionToggle))
	bool bOverride_SensoryRange;

	UPROPERTY(EditAnywhere, meta = (PinHiddenByDefault, InlineEditConditionToggle))
	bool bOverride_RomaingRange;

	UPROPERTY(EditAnywhere, meta = (PinHiddenByDefault, InlineEditConditionToggle))
	bool bOverride_RoamingRate;
	
	// Properties
	UPROPERTY(EditAnywhere, meta = (editcondition = "bOverride_EnemyType"))
	ET4GameEnemyType EnemyType; // #104

	UPROPERTY(EditAnywhere, meta = (editcondition = "bOverride_Aggressive"))
	bool bAggressive;

	UPROPERTY(EditAnywhere, meta = (editcondition = "bOverride_ActiveOrKeepAggroTimeSec"))
	float ActiveOrKeepAggroTimeSec;

	UPROPERTY(EditAnywhere, meta = (editcondition = "bOverride_AgentRadius"))
	float AgentRadius; // #114 : Agent 크기 및 Attack/Stop Distance 에서 겹치지 않기 위한 값으로 사용. WorldActor 의 CapsuleRadius 보다 커야함

	UPROPERTY(EditAnywhere, meta = (editcondition = "bOverride_SensoryRange"))
	float SensoryRange;

	UPROPERTY(EditAnywhere, meta = (editcondition = "bOverride_RomaingRange"))
	float RomaingRange;

	UPROPERTY(EditAnywhere, meta = (editcondition = "bOverride_RoamingRate", ClampMin = "0.0", ClampMax = "1.0"))
	float RoamingRate;

public:
	FT4GameOverrideBehaviorData()
		: bOverride_EnemyType(false)
		, bOverride_Aggressive(false)
		, bOverride_ActiveOrKeepAggroTimeSec(false)
		, bOverride_AgentRadius(false)
		, bOverride_SensoryRange(false)
		, bOverride_RomaingRange(false)
		, bOverride_RoamingRate(false)
		, EnemyType(ET4GameEnemyType::None) // #104
		, bAggressive(false)
		, ActiveOrKeepAggroTimeSec(0.0f)
		, AgentRadius(0.0f) // #114 : Agent 크기 및 Attack/Stop Distance 에서 겹치지 않기 위한 값으로 사용. WorldActor 의 CapsuleRadius 보다 커야함
		, SensoryRange(0.0f)
		, RomaingRange(0.0f)
		, RoamingRate(0.0f)
	{
	}
};

USTRUCT()
struct T4GAMEPLAY_API FT4GameSpawnObjectData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameSpawnObjectData()
#if WITH_EDITORONLY_DATA
		: ParentID(NAME_None) // #122
		, FolderName(NAME_None) // #122
#endif
	{
	}

	FORCEINLINE bool operator==(const FName& InKey) const
	{
		return (ID == InKey) ? true : false;
	}

	FORCEINLINE bool operator==(const FT4GameSpawnObjectData& InRhs) const
	{
		return (ID == InRhs.ID) ? true : false;
	}

	// FT4ContentSpawnObjectDetails::CustomizeDetails

	UPROPERTY(VisibleAnywhere, Category = Common)
	FName ID;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameNPCDataID NPCDataID;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameOverrideBehaviorData OverrideBehaviorData;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor)
	FString Description;

	UPROPERTY(VisibleAnywhere, Category = Hide)
	FName ParentID; // #122

	UPROPERTY(VisibleAnywhere, Category = Hide)
	FName FolderName; // #122
#endif
};

USTRUCT()
struct T4GAMEPLAY_API FT4GameSpawnActorData
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameSpawnActorData()
		: SpawnObjectID(NAME_None)
#if WITH_EDITORONLY_DATA
		, DebugColor(FColorList::White)
		, ParentID(NAME_None) // #122
		, FolderName(NAME_None) // #122
#endif
	{
	}

	FORCEINLINE bool operator==(const FName& InKey) const
	{
		return (ID == InKey) ? true : false;
	}

	FORCEINLINE bool operator==(const FT4GameSpawnActorData& InRhs) const
	{
		return (ID == InRhs.ID) ? true : false;
	}

	// FT4ContentSpawnActorDetails::CustomizeDetails

	UPROPERTY(VisibleAnywhere, Category = Common)
	FName ID;

	UPROPERTY(VisibleAnywhere, Category = ServerOnly)
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FName SpawnObjectID;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FTransform Transform;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor)
	FString Description;

	UPROPERTY(EditAnywhere, Category = Editor)
	FColor DebugColor;

	UPROPERTY(VisibleAnywhere, Category = Hide)
	FName ParentID; // #122

	UPROPERTY(VisibleAnywhere, Category = Hide)
	FName FolderName; // #122
#endif
};

// #126
USTRUCT()
struct T4GAMEPLAY_API FT4GameSpawnTestSettings
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameSpawnTestSettings()
#if WITH_EDITOR
		: DefaultCamera(ET4CameraType::TPS)
		, PlayeDataID(NAME_None)
		, WeaponNameID(NAME_None)
		, SubStanceName(NAME_None)
		, SpawnLocation(FVector::ZeroVector)
		, SpawnRotation(FRotator::ZeroRotator)
#endif
	{
	}

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor)
	ET4CameraType DefaultCamera;

	UPROPERTY(EditAnywhere, Category = Editor)
	FName PlayeDataID;

	UPROPERTY(EditAnywhere, Category = Editor)
	FName WeaponNameID;

	UPROPERTY(EditAnywhere, Category = Editor)
	FName SubStanceName;

	UPROPERTY(EditAnywhere, Category = Editor)
	FVector SpawnLocation;

	UPROPERTY(EditAnywhere, Category = Editor)
	FRotator SpawnRotation;
#endif
};

class UTexture2D;
class UT4GameContentAsset;
UCLASS(ClassGroup = T4Framework, Category = "T4Framework")
class T4GAMEPLAY_API UT4GameSpawnAsset : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UObject interface
	void Serialize(FArchive& Ar) override;
	void PostLoad() override;

public:
	UPROPERTY(EditAnywhere, Category = ServerOnly)
	TArray<FT4GameSpawnObjectData> SpawnObjectArray;

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	TArray<FT4GameSpawnActorData> SpawnActorArray;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Editor, AssetRegistrySearchable)
	TSoftObjectPtr<UT4GameContentAsset> ParentContentAsset;

	UPROPERTY(EditAnywhere, Category = Editor)
	FT4GameSpawnTestSettings TestSettings; // #126

	UPROPERTY()
	UTexture2D* ThumbnailImage; // Internal: The thumbnail image
#endif
};