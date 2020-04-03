// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/Action/T4ActionStatusCommands.h"
#include "Public/Action/T4ActionMoveCommands.h"
#include "Public/Action/T4ActionCommonCommands.h"
#include "Public/Action/T4ActionWorldCommands.h"
#if WITH_EDITOR
#include "Public/Action/T4ActionEditorCommands.h"
#endif

#include "Public/Action/T4ActionParameters.h"

#include "T4ActionReplayAsset.generated.h"

/**
  * #68
 */
struct ET4ActionReplayVersion
{
	enum Type
	{
		InitializeVer = 0,

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1,
	};

	T4ENGINE_API const static FGuid GUID;

private:
	ET4ActionReplayVersion() {}
};

USTRUCT()
struct T4ENGINE_API FT4ActionReplayHeader
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	float TotalPlayTimeSec;
	
public:
	FT4ActionReplayHeader()
		: TotalPlayTimeSec(0.0f)
	{
	}
};

USTRUCT()
struct T4ENGINE_API FT4ActionReplayItem
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	float Time;

	UPROPERTY(VisibleAnywhere)
	uint32 UniqueKey;

	UPROPERTY(VisibleAnywhere)
	FT4ActorID ActorID;

	UPROPERTY(VisibleAnywhere)
	ET4ActionType ActionType;

	UPROPERTY(VisibleAnywhere)
	int32 ActionArrayIndex;
	
public:
	FT4ActionReplayItem()
		: Time(0.0f)
		, UniqueKey(INDEX_NONE)
		, ActionType(ET4ActionType::None)
		, ActionArrayIndex(INDEX_NONE)
	{
	}
};

USTRUCT()
struct T4ENGINE_API FT4ActionReplayData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ActionReplayHeader Header;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4ActionReplayItem> PlayActions;

	UPROPERTY(VisibleAnywhere)
	TMap<uint32, FT4ActionParameters> PlayActionParameters; // PlayUniqueKey


	// #T4_ADD_ACTION_TAG_CODE

	// T4ActionWorldCommands.h

	UPROPERTY(VisibleAnywhere)
	TArray<FT4WorldTravelAction> WorldTravelActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4SpawnActorAction> SpawnActorActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4DespawnActorAction> DespawnActorActions;


	// T4ActionMoveCommands.h

	UPROPERTY(VisibleAnywhere)
	TArray<FT4MoveAsyncAction> MoveAsyncActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4MoveSyncAction> MoveSyncActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4JumpAction> JumpActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4RollAction> RollActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4TeleportAction> TeleportActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4TurnAction> TurnActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4MoveStopAction> MoveStopActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4MoveSpeedSyncAction> MoveSpeedSyncActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4LaunchAction> LaunchActions;


	// T4ActionStatusCommands.h

	UPROPERTY(VisibleAnywhere)
	TArray<FT4AimAction> AimActions; // #113

	UPROPERTY(VisibleAnywhere)
	TArray<FT4LockOnAction> LockOnActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4StanceAction> StanceActions; // #73

	UPROPERTY(VisibleAnywhere)
	TArray<FT4SubStanceAction> SubStanceActions; // #106

	UPROPERTY(VisibleAnywhere)
	TArray<FT4EquipWeaponAction> EquipWeaponActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4UnequipWeaponAction> UnequipWeaponActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4CostumeAction> CostumeActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4HitAction> HitActions; // #76

	UPROPERTY(VisibleAnywhere)
	TArray<FT4DieAction> DieActions; // #76

	UPROPERTY(VisibleAnywhere)
	TArray<FT4ResurrectAction> ResurrectActions; // #76


	// T4ActionCommonCommands.h

	UPROPERTY(VisibleAnywhere)
	TArray<FT4SetAction> SetActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4StopAction> StopActions;


#if WITH_EDITORONLY_DATA
	// T4ActionEditorCommands.h
	UPROPERTY(VisibleAnywhere)
	TArray<FT4EditorAction> EditorActions; // #80
#endif
};

UCLASS(ClassGroup = T4Framework, Category = "T4Framework")
class T4ENGINE_API UT4ActionReplayAsset : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UObject interface
	void Serialize(FArchive& Ar) override;

public:
	UPROPERTY(VisibleAnywhere)
	FT4ActionReplayData ReplayData;
};