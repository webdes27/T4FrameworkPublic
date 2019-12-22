// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionPlaybackRecorder.h"
#include "Public/Playback/T4ActionPlaybackAPI.h"

#if !UE_BUILD_SHIPPING

#include "Misc/PackageName.h"

#include "T4EngineInternal.h"

/**
  * #68
 */
FT4ActionPlaybackRecorder::FT4ActionPlaybackRecorder(ET4LayerType InLayerType)
	: FT4ActionPlaybackBase(InLayerType)
	, bRecording(false)
	, RecTimeSec(0.0f)
{
}

FT4ActionPlaybackRecorder::~FT4ActionPlaybackRecorder()
{
}

void FT4ActionPlaybackRecorder::Reset()
{
	//check(!bRecording);
	RecAssetName.Empty();
	RecTimeSec = 0.0f;
}

void FT4ActionPlaybackRecorder::ProcessRecording(float InDeltaTime)
{
	if (!bRecording)
	{
		return;
	}
	RecTimeSec += InDeltaTime;
}

bool FT4ActionPlaybackRecorder::Rec(
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

bool FT4ActionPlaybackRecorder::DoRec(
	const FSoftObjectPath& InRecPath
)
{
	const FString AssetName = T4ActionPlaybackAPI::GetAssetNameFromObjectPath(InRecPath.ToString());
	const FString PackagePath = FPackageName::GetLongPackagePath(InRecPath.ToString());
	return Rec(AssetName, PackagePath);
}

bool FT4ActionPlaybackRecorder::DoRec(
	const FString& InRecAssetName,
	const FString& InFolderName
)
{
	const FString SavePackagePath = T4ActionPlaybackAPI::GetLongPackagePath(InFolderName);
	return Rec(InRecAssetName, SavePackagePath);
}

void FT4ActionPlaybackRecorder::Stop()
{
	if (!bRecording)
	{
		return;
	}
	FT4ActionPlaybackData& PlaybackData = GetPlaybackData();
	PlaybackData.Header.TotalPlayTimeSec = RecTimeSec;
	bool bResult = SaveAsset(RecAssetName, RecPackagePath);
	if (!bResult)
	{

	}
	bRecording = false;
}

bool FT4ActionPlaybackRecorder::RecWorldAction(
	const FT4ActionStruct* InAction,
	const FT4ActionParameters* InActionParam
)
{
	FT4ActionPlaybackData& PlaybackData = GetPlaybackData();
	FT4ActionPlaybackItem& NewPlaybackItem = PlaybackData.PlayActions.AddDefaulted_GetRef();
	NewPlaybackItem.Time = RecTimeSec;
	NewPlaybackItem.UniqueKey = PlaybackData.PlayActions.Num();
	NewPlaybackItem.ActionType = InAction->ActionType;
	switch (InAction->ActionType)
	{
		// #T4_ADD_ACTION_TAG_CODE

		case ET4ActionType::WorldTravel: // #87
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.WorldTravelActions.Num();
				FT4WorldTravelAction& NewAction = PlaybackData.WorldTravelActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4WorldTravelAction*>(InAction));
			}
			break;

		case ET4ActionType::SpawnObject:
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.SpawnObjectActions.Num();
				FT4SpawnObjectAction& NewAction = PlaybackData.SpawnObjectActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4SpawnObjectAction*>(InAction));
				NewAction.ObjectID.SpawnMode = ET4SpawnMode::Playback; // ObjectID 의 SpawnMode 를 미리 변경해준다.
			}
			break;

		case ET4ActionType::DespawnObject:
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.DespawnObjectActions.Num();
				FT4DespawnObjectAction& NewAction = PlaybackData.DespawnObjectActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4DespawnObjectAction*>(InAction));
				NewAction.ObjectID.SpawnMode = ET4SpawnMode::Playback; // ObjectID 의 SpawnMode 를 미리 변경해준다.
			}
			break;

		default:
			{
				UE_LOG(
					LogT4Engine,
					Error,
					TEXT("FT4ActionPlaybackRecorder '%s' failed. no implementation."),
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
			PlaybackData.PlayActionParameters.Add(NewPlaybackItem.UniqueKey, NewActionParameters);
		}
	}
	return true;
}

bool FT4ActionPlaybackRecorder::RecObjectAction(
	const FT4ObjectID& InObjectID,
	const FT4ActionStruct* InAction,
	const FT4ActionParameters* InActionParam
)
{
	FT4ActionPlaybackData& PlaybackData = GetPlaybackData();
	FT4ActionPlaybackItem& NewPlaybackItem = PlaybackData.PlayActions.AddDefaulted_GetRef();
	NewPlaybackItem.Time = RecTimeSec;
	NewPlaybackItem.UniqueKey = PlaybackData.PlayActions.Num();
	NewPlaybackItem.ObjectID = InObjectID;
	NewPlaybackItem.ObjectID.SpawnMode = ET4SpawnMode::Playback; // ObjectID 의 SpawnMode 를 미리 변경해준다.
	NewPlaybackItem.ActionType = InAction->ActionType;
	switch (InAction->ActionType)
	{
		// #T4_ADD_ACTION_TAG_CODE

		case ET4ActionType::MoveAsync:
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.MoveAsyncActions.Num();
				FT4MoveAsyncAction& NewAction = PlaybackData.MoveAsyncActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4MoveAsyncAction*>(InAction));
			}
			break;

		case ET4ActionType::MoveSync:
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.MoveSyncActions.Num();
				FT4MoveSyncAction& NewAction = PlaybackData.MoveSyncActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4MoveSyncAction*>(InAction));
			}
			break;

		case ET4ActionType::Jump:
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.JumpActions.Num();
				FT4JumpAction& NewAction = PlaybackData.JumpActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4JumpAction*>(InAction));
			}
			break;

		case ET4ActionType::Roll:
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.RollActions.Num();
				FT4RollAction& NewAction = PlaybackData.RollActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4RollAction*>(InAction));
			}
			break;

		case ET4ActionType::Teleport:
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.TeleportActions.Num();
				FT4TeleportAction& NewAction = PlaybackData.TeleportActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4TeleportAction*>(InAction));
			}
			break;

		case ET4ActionType::Turn:
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.TurnActions.Num();
				FT4TurnAction& NewAction = PlaybackData.TurnActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4TurnAction*>(InAction));
			}
			break;

		case ET4ActionType::MoveStop:
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.MoveStopActions.Num();
				FT4MoveStopAction& NewAction = PlaybackData.MoveStopActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4MoveStopAction*>(InAction));
			}
			break;

		case ET4ActionType::MoveSpeedSync:
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.MoveSpeedSyncActions.Num();
				FT4MoveSpeedSyncAction& NewAction = PlaybackData.MoveSpeedSyncActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4MoveSpeedSyncAction*>(InAction));
			}
			break;

		case ET4ActionType::LockOn:
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.LockOnActions.Num();
				FT4LockOnAction& NewAction = PlaybackData.LockOnActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4LockOnAction*>(InAction));
			}
			break;

		case ET4ActionType::ChangeStance: // #73
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.ChangeStanceActions.Num();
				FT4ChangeStanceAction& NewAction = PlaybackData.ChangeStanceActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4ChangeStanceAction*>(InAction));
			}
			break;

		case ET4ActionType::EquipWeapon:
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.EquipWeaponActions.Num();
				FT4EquipWeaponAction& NewAction = PlaybackData.EquipWeaponActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4EquipWeaponAction*>(InAction));
			}
			break;

		case ET4ActionType::UnEquipWeapon:
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.UnEquipWeaponActions.Num();
				FT4UnEquipWeaponAction& NewAction = PlaybackData.UnEquipWeaponActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4UnEquipWeaponAction*>(InAction));
			}
			break;

		case ET4ActionType::ExchangeCostume:
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.ExchangeCostumeActions.Num();
				FT4ExchangeCostumeAction& NewAction = PlaybackData.ExchangeCostumeActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4ExchangeCostumeAction*>(InAction));
			}
			break;

		case ET4ActionType::Hit: // #76
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.HitActions.Num();
				FT4HitAction& NewAction = PlaybackData.HitActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4HitAction*>(InAction));
			}
			break;

		case ET4ActionType::Die: // #76
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.DieActions.Num();
				FT4DieAction& NewAction = PlaybackData.DieActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4DieAction*>(InAction));
			}
			break;

		case ET4ActionType::Resurrect: // #76
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.ResurrectActions.Num();
				FT4ResurrectAction& NewAction = PlaybackData.ResurrectActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4ResurrectAction*>(InAction));
			}
			break;

		case ET4ActionType::Conti:
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.ContiActions.Num();
				FT4ContiAction& NewAction = PlaybackData.ContiActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4ContiAction*>(InAction));
			}
			break;

		case ET4ActionType::Stop:
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.StopActions.Num();
				FT4StopAction& NewAction = PlaybackData.StopActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4StopAction*>(InAction));
			}
			break;

#if WITH_EDITOR
		case ET4ActionType::Editor: // #80
			{
				NewPlaybackItem.ActionArrayIndex = PlaybackData.EditorActions.Num();
				FT4EditorAction& NewAction = PlaybackData.EditorActions.AddDefaulted_GetRef();
				NewAction = *(static_cast<const FT4EditorAction*>(InAction));
			}
			break;
#endif

		default:
			{
				UE_LOG(
					LogT4Engine,
					Error,
					TEXT("FT4ActionPlaybackRecorder '%s' failed. no implementation."),
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
			PlaybackData.PlayActionParameters.Add(NewPlaybackItem.UniqueKey, NewActionParameters);
		}
	}
	return true;
}

#endif