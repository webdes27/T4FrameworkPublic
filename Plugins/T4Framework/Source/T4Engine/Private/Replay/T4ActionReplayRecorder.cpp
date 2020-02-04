// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionReplayRecorder.h"
#include "Public/Replay/T4ActionReplayUtility.h"

#if !UE_BUILD_SHIPPING

#include "Misc/PackageName.h"

#include "T4EngineInternal.h"

/**
  * #68
 */
FT4ActionReplayRecorder::FT4ActionReplayRecorder(ET4LayerType InLayerType)
	: FT4ActionReplayBase(InLayerType)
	, bRecording(false)
	, RecTimeSec(0.0f)
{
}

FT4ActionReplayRecorder::~FT4ActionReplayRecorder()
{
}

void FT4ActionReplayRecorder::Reset()
{
	//check(!bRecording);
	RecAssetName.Empty();
	RecTimeSec = 0.0f;
}

void FT4ActionReplayRecorder::ProcessRecording(float InDeltaTime)
{
	if (!bRecording)
	{
		return;
	}
	RecTimeSec += InDeltaTime;
}

bool FT4ActionReplayRecorder::Rec(
	const FString& InAssetName,
	const FString& InPackagePath
)
{
	OnReset();
	bool bResult = NewAsset();
	if (bResult)
	{
		RecAssetName = InAssetName;
		RecPackagePath = InPackagePath;
		RecFullPath = InPackagePath / RecAssetName;
		bRecording = true;
	}
	return bResult;
}

bool FT4ActionReplayRecorder::DoRec(
	const FSoftObjectPath& InRecPath
)
{
	const FString AssetName = T4ActionReplayUtility::GetAssetNameFromObjectPath(InRecPath.ToString());
	const FString PackagePath = FPackageName::GetLongPackagePath(InRecPath.ToString());
	return Rec(AssetName, PackagePath);
}

bool FT4ActionReplayRecorder::DoRec(
	const FString& InRecAssetName,
	const FString& InFolderName
)
{
	const FString SavePackagePath = T4ActionReplayUtility::GetLongPackagePath(InFolderName);
	return Rec(InRecAssetName, SavePackagePath);
}

void FT4ActionReplayRecorder::Stop()
{
	if (!bRecording)
	{
		return;
	}
	FT4ActionReplayData& ReplayData = GetReplayData();
	ReplayData.Header.TotalPlayTimeSec = RecTimeSec;
	bool bResult = SaveAsset(RecAssetName, RecPackagePath);
	if (!bResult)
	{

	}
	bRecording = false;
}

bool FT4ActionReplayRecorder::RecWorldAction(
	const FT4ActionCommand* InAction,
	const FT4ActionParameters* InActionParam
)
{
	FT4ActionReplayData& ReplayData = GetReplayData();
	FT4ActionReplayItem& NewPlaybackItem = ReplayData.PlayActions.AddDefaulted_GetRef();
	NewPlaybackItem.Time = RecTimeSec;
	NewPlaybackItem.UniqueKey = ReplayData.PlayActions.Num();
	NewPlaybackItem.ActionType = InAction->ActionType;
	switch (InAction->ActionType)
	{
		// #T4_ADD_ACTION_TAG_CODE

		case ET4ActionType::WorldTravel: // #87
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.WorldTravelActions.Num();
				FT4WorldTravelAction& NewAction = ReplayData.WorldTravelActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4WorldTravelAction*>(InAction));
			}
			break;

		case ET4ActionType::SpawnObject:
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.SpawnObjectActions.Num();
				FT4SpawnObjectAction& NewAction = ReplayData.SpawnObjectActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4SpawnObjectAction*>(InAction));
				NewAction.ObjectID.SpawnMode = ET4SpawnMode::Playback; // ObjectID 의 SpawnMode 를 미리 변경해준다.
			}
			break;

		case ET4ActionType::DespawnObject:
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.DespawnObjectActions.Num();
				FT4DespawnObjectAction& NewAction = ReplayData.DespawnObjectActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4DespawnObjectAction*>(InAction));
				NewAction.ObjectID.SpawnMode = ET4SpawnMode::Playback; // ObjectID 의 SpawnMode 를 미리 변경해준다.
			}
			break;

		default:
			{
				T4_LOG(
					Error,
					TEXT("No implementation Action '%s'"),
					*(InAction->ToString())
				);
			}
			break;
	}
	if (nullptr != InActionParam)
	{
		if (InActionParam->bDirty)
		{
			FT4ActionParameters NewActionParameters = *InActionParam;
			if (NewActionParameters.CheckBits(ET4TargetParamBits::ObjectIDBit))
			{
				FT4ObjectID ConvTargetObjectID = NewActionParameters.GetTargetParams().TargetObjectID;
				ConvTargetObjectID.SpawnMode = ET4SpawnMode::Playback;
				NewActionParameters.SetTargetObjectID(ConvTargetObjectID);
			}
			ReplayData.PlayActionParameters.Add(NewPlaybackItem.UniqueKey, NewActionParameters);
		}
	}
	return true;
}

bool FT4ActionReplayRecorder::RecObjectAction(
	const FT4ObjectID& InObjectID,
	const FT4ActionCommand* InAction,
	const FT4ActionParameters* InActionParam
)
{
	FT4ActionReplayData& ReplayData = GetReplayData();
	FT4ActionReplayItem& NewPlaybackItem = ReplayData.PlayActions.AddDefaulted_GetRef();
	NewPlaybackItem.Time = RecTimeSec;
	NewPlaybackItem.UniqueKey = ReplayData.PlayActions.Num();
	NewPlaybackItem.ObjectID = InObjectID;
	NewPlaybackItem.ObjectID.SpawnMode = ET4SpawnMode::Playback; // ObjectID 의 SpawnMode 를 미리 변경해준다.
	NewPlaybackItem.ActionType = InAction->ActionType;
	switch (InAction->ActionType)
	{
		// #T4_ADD_ACTION_TAG_CODE

		case ET4ActionType::MoveAsync:
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.MoveAsyncActions.Num();
				FT4MoveAsyncAction& NewAction = ReplayData.MoveAsyncActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4MoveAsyncAction*>(InAction));
			}
			break;

		case ET4ActionType::MoveSync:
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.MoveSyncActions.Num();
				FT4MoveSyncAction& NewAction = ReplayData.MoveSyncActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4MoveSyncAction*>(InAction));
			}
			break;

		case ET4ActionType::Jump:
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.JumpActions.Num();
				FT4JumpAction& NewAction = ReplayData.JumpActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4JumpAction*>(InAction));
			}
			break;

		case ET4ActionType::Roll:
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.RollActions.Num();
				FT4RollAction& NewAction = ReplayData.RollActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4RollAction*>(InAction));
			}
			break;

		case ET4ActionType::Teleport:
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.TeleportActions.Num();
				FT4TeleportAction& NewAction = ReplayData.TeleportActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4TeleportAction*>(InAction));
			}
			break;

		case ET4ActionType::Turn:
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.TurnActions.Num();
				FT4TurnAction& NewAction = ReplayData.TurnActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4TurnAction*>(InAction));
			}
			break;

		case ET4ActionType::MoveStop:
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.MoveStopActions.Num();
				FT4MoveStopAction& NewAction = ReplayData.MoveStopActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4MoveStopAction*>(InAction));
			}
			break;

		case ET4ActionType::MoveSpeedSync:
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.MoveSpeedSyncActions.Num();
				FT4MoveSpeedSyncAction& NewAction = ReplayData.MoveSpeedSyncActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4MoveSpeedSyncAction*>(InAction));
			}
			break;

		case ET4ActionType::Aim: // #113
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.AimActions.Num();
				FT4AimAction& NewAction = ReplayData.AimActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4AimAction*>(InAction));
			}
			break;

		case ET4ActionType::LockOn:
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.LockOnActions.Num();
				FT4LockOnAction& NewAction = ReplayData.LockOnActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4LockOnAction*>(InAction));
			}
			break;

		case ET4ActionType::Stance: // #73
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.StanceActions.Num();
				FT4StanceAction& NewAction = ReplayData.StanceActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4StanceAction*>(InAction));
			}
			break;

		case ET4ActionType::SubStance: // #106
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.SubStanceActions.Num();
				FT4SubStanceAction& NewAction = ReplayData.SubStanceActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4SubStanceAction*>(InAction));
			}
			break;

		case ET4ActionType::EquipWeapon:
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.EquipWeaponActions.Num();
				FT4EquipWeaponAction& NewAction = ReplayData.EquipWeaponActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4EquipWeaponAction*>(InAction));
			}
			break;

		case ET4ActionType::UnequipWeapon:
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.UnequipWeaponActions.Num();
				FT4UnequipWeaponAction& NewAction = ReplayData.UnequipWeaponActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4UnequipWeaponAction*>(InAction));
			}
			break;

		case ET4ActionType::Costume:
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.CostumeActions.Num();
				FT4CostumeAction& NewAction = ReplayData.CostumeActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4CostumeAction*>(InAction));
			}
			break;

		case ET4ActionType::Hit: // #76
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.HitActions.Num();
				FT4HitAction& NewAction = ReplayData.HitActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4HitAction*>(InAction));
			}
			break;

		case ET4ActionType::Die: // #76
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.DieActions.Num();
				FT4DieAction& NewAction = ReplayData.DieActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4DieAction*>(InAction));
			}
			break;

		case ET4ActionType::Resurrect: // #76
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.ResurrectActions.Num();
				FT4ResurrectAction& NewAction = ReplayData.ResurrectActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4ResurrectAction*>(InAction));
			}
			break;

		case ET4ActionType::Conti:
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.ContiActions.Num();
				FT4ContiAction& NewAction = ReplayData.ContiActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4ContiAction*>(InAction));
			}
			break;

		case ET4ActionType::Stop:
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.StopActions.Num();
				FT4StopAction& NewAction = ReplayData.StopActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4StopAction*>(InAction));
			}
			break;

#if WITH_EDITOR
		case ET4ActionType::Editor: // #80
			{
				NewPlaybackItem.ActionArrayIndex = ReplayData.EditorActions.Num();
				FT4EditorAction& NewAction = ReplayData.EditorActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4EditorAction*>(InAction));
			}
			break;
#endif

		default:
			{
				T4_LOG(
					Error,
					TEXT("No implementation Action '%s'"),
					*(InAction->ToString())
				);
			}
			break;
	}
	if (nullptr != InActionParam)
	{
		if (InActionParam->bDirty)
		{
			FT4ActionParameters NewActionParameters = *InActionParam;
			if (NewActionParameters.CheckBits(ET4TargetParamBits::ObjectIDBit))
			{
				FT4ObjectID ConvTargetObjectID = NewActionParameters.GetTargetParams().TargetObjectID;
				ConvTargetObjectID.SpawnMode = ET4SpawnMode::Playback;
				NewActionParameters.SetTargetObjectID(ConvTargetObjectID);
			}
			ReplayData.PlayActionParameters.Add(NewPlaybackItem.UniqueKey, NewActionParameters);
		}
	}
	return true;
}

#endif