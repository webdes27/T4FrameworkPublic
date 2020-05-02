// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/Action/T4ActionCommandStatus.h"
#include "Public/Action/T4ActionCommandMoves.h"
#include "Public/Action/T4ActionCommandCommons.h"
#include "Public/Action/T4ActionCommandWorlds.h"
#if WITH_EDITOR
#include "Public/Action/T4ActionCommandEditors.h"
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
	ET4ActionCommandType ActionCommandType;

	UPROPERTY(VisibleAnywhere)
	int32 ActionArrayIndex;
	
public:
	FT4ActionReplayItem()
		: Time(0.0f)
		, UniqueKey(INDEX_NONE)
		, ActionCommandType(ET4ActionCommandType::None)
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


	// #T4_ADD_ACTION_TAG_CMD

	// T4ActionCommandWorlds.h

	UPROPERTY(VisibleAnywhere)
	TArray<FT4WorldTravelActionCommand> WorldTravelActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4WorldSpawnActionCommand> SpawnActorActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4WorldDespawnActionCommand> DespawnActorActions;


	// T4ActionCommandMoves.h

	UPROPERTY(VisibleAnywhere)
	TArray<FT4MoveAsyncActionCommand> MoveAsyncActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4MoveSyncActionCommand> MoveSyncActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4JumpActionCommand> JumpActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4RollActionCommand> RollActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4TeleportActionCommand> TeleportActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4RotationActionCommand> RotationActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4MoveStopActionCommand> MoveStopActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4MoveSpeedSyncActionCommand> MoveSpeedSyncActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4LaunchActionCommand> LaunchActions;


	// T4ActionCommandStatus.h

	UPROPERTY(VisibleAnywhere)
	TArray<FT4AimActionCommand> AimActions; // #113

	UPROPERTY(VisibleAnywhere)
	TArray<FT4LockOnActionCommand> LockOnActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4StanceActionCommand> StanceActions; // #73

	UPROPERTY(VisibleAnywhere)
	TArray<FT4PostureActionCommand> PostureActions; // #106

	UPROPERTY(VisibleAnywhere)
	TArray<FT4EquipWeaponActionCommand> EquipWeaponActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4UnequipWeaponActionCommand> UnequipWeaponActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4CostumeActionCommand> CostumeActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4SkinActionCommand> SkinActions; // #130

	UPROPERTY(VisibleAnywhere)
	TArray<FT4HitActionCommand> HitActions; // #76

	UPROPERTY(VisibleAnywhere)
	TArray<FT4CrowdControlActionCommand> CrowdControlActions; // #131

	UPROPERTY(VisibleAnywhere)
	TArray<FT4DieActionCommand> DieActions; // #76

	UPROPERTY(VisibleAnywhere)
	TArray<FT4ResurrectActionCommand> ResurrectActions; // #76


	// T4ActionCommandCommons.h

	UPROPERTY(VisibleAnywhere)
	TArray<FT4ActionSetActionCommand> SetActions;

	UPROPERTY(VisibleAnywhere)
	TArray<FT4StopActionCommand> StopActions;


#if WITH_EDITORONLY_DATA
	// T4ActionCommandEditors.h
	UPROPERTY(VisibleAnywhere)
	TArray<FT4EditorActionCommand> EditorActions; // #80
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