// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Public/T4GameBuiltin_GameDataTypes.h"
#include "Classes/Common/T4CommonAssetStructs.h" // #103
#include "T4GameBuiltin_SpawnAsset.generated.h"

/**
  * #118
 */
struct ET4GameBuiltin_SpawnVersion
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
	ET4GameBuiltin_SpawnVersion() {}
};

// #114
USTRUCT()
struct T4GAMEBUILTIN_API FT4GameBuiltin_OverrideNPCBehaviorData // #114
{
	GENERATED_USTRUCT_BODY()

public:
	// #114
	// FT4GameBuiltin_AIBehaviorData : AIController (Runtime)
	// FT4GameBuiltin_NPCBehaviorData : NPC DataTable (Original Set)
	// FT4GameBuiltin_OverrideNPCBehaviorData : Spawn Asset (Instance Set)

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
	ET4GameBuiltin_EnemyType EnemyType; // #104

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
	FT4GameBuiltin_OverrideNPCBehaviorData()
		: bOverride_EnemyType(false)
		, bOverride_Aggressive(false)
		, bOverride_ActiveOrKeepAggroTimeSec(false)
		, bOverride_AgentRadius(false)
		, bOverride_SensoryRange(false)
		, bOverride_RomaingRange(false)
		, bOverride_RoamingRate(false)
		, EnemyType(ET4GameBuiltin_EnemyType::None) // #104
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

	UPROPERTY(EditAnywhere, Category = ServerOnly)
	FT4GameBuiltin_OverrideNPCBehaviorData OverrideNPCBehaviorData;

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
class T4GAMEBUILTIN_API UT4GameBuiltin_SpawnAsset : public UObject
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