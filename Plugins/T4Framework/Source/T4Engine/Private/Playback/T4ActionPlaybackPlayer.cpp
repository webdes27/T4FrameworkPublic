// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ActionPlaybackPlayer.h"
#include "Public/Playback/T4ActionPlaybackAPI.h"

#if !UE_BUILD_SHIPPING

#include "World/T4GameWorld.h"

#include "Misc/PackageName.h"
#include "Public/T4Engine.h"
#include "Engine/World.h"
#include "Engine/WorldComposition.h"

#include "T4EngineInternal.h"

/**
  * #68
 */
FT4ActionPlaybackPlayer::FT4ActionPlaybackPlayer(ET4LayerType InLayerType, bool bInRepeat, bool bInPlayerPossessed)
	: FT4ActionPlaybackBase(InLayerType)
	, bPlaying(false)
	, bPaused(false)
	, bRepeat(bInRepeat)
	, bPlayerPossessed(bInPlayerPossessed)
	, PlayStartItemIndex(0)
	, PlayTimeSec(0.0f)
	, MaxPlayTimeSec(0.0f)
	, bWorldTravelPending(false) // #87
	, bWorldTraveling(false) // #87
	, WorldTravelPlaybackItemIndex(-1) // #87
	, WorldTravelStartLocation(FVector::ZeroVector) // #87
{
}

FT4ActionPlaybackPlayer::~FT4ActionPlaybackPlayer()
{
}

void FT4ActionPlaybackPlayer::Reset()
{
	//check(!bPlaying);
	PlayAssetName.Empty();
	PlayTimeSec = 0.0f;
	MaxPlayTimeSec = 0.0f;
	PlayStartItemIndex = 0;
	OriginalPlayerObjectID.SetNone();
	PlaybackPlayerObjectID.SetNone();
	bWorldTravelPending = false;
	bWorldTraveling = false;
	WorldTravelPlaybackItemIndex = -1;
	WorldTravelStartLocation = FVector::ZeroVector; // #87
}

void FT4ActionPlaybackPlayer::SetPause(bool bPause)
{
	if (!bPlaying)
	{
		return;
	}
	bPaused = bPause;
	FT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	GameWorld->DoPlaybackAllPause(bPause);
}

void FT4ActionPlaybackPlayer::ProcessPlayOutsideWorld(float InDeltaTime)
{
	// #87 : World Tick 밖에서 처리해야 할 Action. 예) WorldTravel
	if (!bPlaying || !bWorldTravelPending)
	{
		return;
	}
	check(bWorldTraveling);
	check(0 <= WorldTravelPlaybackItemIndex);
	FT4ActionPlaybackData& PlaybackData = GetPlaybackData();
	check(WorldTravelPlaybackItemIndex < PlaybackData.PlayActions.Num());
	const FT4ActionPlaybackItem& PlayAction = PlaybackData.PlayActions[WorldTravelPlaybackItemIndex];
	check(ET4ActionType::WorldTravel == PlayAction.ActionType);
	check(PlayAction.ActionArrayIndex < PlaybackData.WorldTravelActions.Num());
	FT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	FT4ActionParameters* ActionParameters = nullptr;
	if (INDEX_NONE != PlayAction.UniqueKey && PlaybackData.PlayActionParameters.Contains(PlayAction.UniqueKey))
	{
		ActionParameters = &PlaybackData.PlayActionParameters[PlayAction.UniqueKey];
	}
	const FSoftObjectPath CurrentWorldObjectPath = GameWorld->GetWorldObjectPath();
	FT4WorldTravelAction CloneAction = PlaybackData.WorldTravelActions[PlayAction.ActionArrayIndex];
	if (CloneAction.MapEntityOrLevelObjectPath != CurrentWorldObjectPath)
	{
		CloneAction.StartLocation = WorldTravelStartLocation; // #104 : 같은 레벨은 이동하지 않는다.
		GameWorld->DoExecuteAction(&CloneAction, ActionParameters);
	}
	bWorldTravelPending = false;
}

void FT4ActionPlaybackPlayer::ProcessPlayInsideWorld(float InDeltaTime)
{
	// #68, #87 : World Tick Pre 에서 호출됨. 일부 Action 은 ProcessOutsideWorld 에서 처리됨!
	if (!bPlaying)
	{
		return;
	}
	FT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	if (bWorldTraveling)
	{
		if (bWorldTravelPending)
		{
			return; // 아직 로딩전임으로 pass
		}
#if WITH_EDITOR
		if (!GameWorld->IsDisabledLevelStreaming()) // #104 : Level Streaming 을 사용할 경우만 동작!
#endif
		{
			// #87 : 아직 PlayerObject 가 스폰되지 않았음으로 다음 위치의 LevelStreaming 을 계속 업데이트 해준다.
			if (!WorldTravelStartLocation.IsNearlyZero())
			{
				UWorld* UnrealWorld = GameWorld->GetWorld();
				check(nullptr != UnrealWorld);
				if (nullptr != UnrealWorld->WorldComposition)
				{
					UnrealWorld->WorldComposition->UpdateStreamingState(WorldTravelStartLocation);
					UnrealWorld->UpdateLevelStreaming();
				}
			}
			// #87 : 월드 로딩이 끝나면 진입하도록 처리...
			if (!GameWorld->GetController()->CheckLevelLoadComplated())
			{
				return;
			}
		}
		bWorldTraveling = false;
		WorldTravelStartLocation = FVector::ZeroVector;
	}
	if (bPaused)
	{
		return;
	}
	PlayTimeSec += InDeltaTime;
	FT4ActionPlaybackData& PlaybackData = GetPlaybackData();
	uint32 NumActionPlaybackItems = PlaybackData.PlayActions.Num();
	if (PlayStartItemIndex >= NumActionPlaybackItems)
	{
		// 액션은 모두 플레이 완료. 시간까지 넘어가면 Repeat 체크
		if (PlayTimeSec >= MaxPlayTimeSec)
		{
			CheckRepeat();
		}
		return; 
	}
	for (uint32 PlayIdx = PlayStartItemIndex; PlayIdx < NumActionPlaybackItems; ++PlayIdx)
	{
		const FT4ActionPlaybackItem& PlayAction = PlaybackData.PlayActions[PlayIdx];
		if (PlayTimeSec < PlayAction.Time)
		{
			break;
		}
		IT4GameObject* OwnerObject = nullptr;
		if (PlayAction.ObjectID.IsValid())
		{
			OwnerObject = GameWorld->GetContainer()->FindGameObject(PlayAction.ObjectID);
		}
		FT4ActionParameters* ActionParameters = nullptr;
		if (INDEX_NONE != PlayAction.UniqueKey && PlaybackData.PlayActionParameters.Contains(PlayAction.UniqueKey))
		{
			ActionParameters = &PlaybackData.PlayActionParameters[PlayAction.UniqueKey];
		}
		switch (PlayAction.ActionType)
		{
			// #T4_ADD_ACTION_TAG_CODE

			case ET4ActionType::WorldTravel: // #87
				{
					check(PlayAction.ActionArrayIndex < PlaybackData.WorldTravelActions.Num());

					// #87 : 레벨 후 시작 좌표를 찾기 위해 첫번째 Player 오브젝트의 위치를 찾아 레벨 로딩 완료의 기준 좌표로 사용한다.
					WorldTravelStartLocation = FVector::ZeroVector;

					int32 NextPlayerSpawnIndex = -1;
					for (uint32 NextSpawnIdx = PlayIdx + 1; NextSpawnIdx < NumActionPlaybackItems; ++NextSpawnIdx)
					{
						const FT4ActionPlaybackItem& NextSpawnAction = PlaybackData.PlayActions[NextSpawnIdx];
						if (ET4ActionType::SpawnObject == NextSpawnAction.ActionType)
						{
							check(NextSpawnAction.ActionArrayIndex < PlaybackData.SpawnObjectActions.Num());
							const FT4SpawnObjectAction& NextSpawnObjectAction = PlaybackData.SpawnObjectActions[NextSpawnAction.ActionArrayIndex];
							if (NextSpawnObjectAction.bPlayer)
							{
								WorldTravelStartLocation = NextSpawnObjectAction.SpawnLocation;
								break;
							}
						}
					}

					// #87 : World 이동은 World Tick 에서 처리하면 World 삭제가 됨으로 관련 정보를 저장하고,
					//       ProcessOutsideWorld 에서 World 전환을 처리한 후 계속 동작하도록 조치한다.

					WorldTravelPlaybackItemIndex = PlayIdx;
					bWorldTravelPending = true;
					bWorldTraveling = true; // 월드 로딩이 끝나면 진입하도록 처리...

					OriginalPlayerObjectID.SetNone(); // 월드가 바뀌어서 이전 Player 는 의미 없음으로 Reset 처리한다.
				}
				break;

			case ET4ActionType::SpawnObject:
				{
					check(PlayAction.ActionArrayIndex < PlaybackData.SpawnObjectActions.Num());
					const FT4SpawnObjectAction& RecAction = PlaybackData.SpawnObjectActions[PlayAction.ActionArrayIndex];
					bool bResult = GameWorld->DoExecuteAction(&RecAction, ActionParameters);
					if (bResult)
					{
						if (RecAction.bPlayer)
						{
							PlaybackPlayerObjectID = RecAction.ObjectID;
							if (bPlayerPossessed)
							{
								IT4ObjectController* PlayerController = GameWorld->GetPlayerControl();
								check(nullptr != PlayerController);
								PlayerController->SetGameObject(RecAction.ObjectID);
							}
						}
					}
				}
				break;

			case ET4ActionType::DespawnObject:
				{
					check(PlayAction.ActionArrayIndex < PlaybackData.DespawnObjectActions.Num());
					const FT4DespawnObjectAction& RecAction = PlaybackData.DespawnObjectActions[PlayAction.ActionArrayIndex];
					if (GameWorld->IsPlayerObject(PlayAction.ObjectID))
					{
						GameWorld->GetPlayerControl()->ClearGameObject(true);
					}
					GameWorld->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::MoveAsync:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.MoveAsyncActions.Num());
					const FT4MoveAsyncAction& RecAction = PlaybackData.MoveAsyncActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::MoveSync:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.MoveSyncActions.Num());
					const FT4MoveSyncAction& RecAction = PlaybackData.MoveSyncActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Jump:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.JumpActions.Num());
					const FT4JumpAction& RecAction = PlaybackData.JumpActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Roll:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.RollActions.Num());
					const FT4RollAction& RecAction = PlaybackData.RollActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Teleport:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.TeleportActions.Num());
					const FT4TeleportAction& RecAction = PlaybackData.TeleportActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Turn:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.TurnActions.Num());
					const FT4TurnAction& RecAction = PlaybackData.TurnActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::MoveStop:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.MoveStopActions.Num());
					const FT4MoveStopAction& RecAction = PlaybackData.MoveStopActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::MoveSpeedSync:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.MoveSpeedSyncActions.Num());
					const FT4MoveSpeedSyncAction& RecAction = PlaybackData.MoveSpeedSyncActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::LockOn:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.LockOnActions.Num());
					const FT4LockOnAction& RecAction = PlaybackData.LockOnActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::ChangeStance: // #73
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.ChangeStanceActions.Num());
					const FT4ChangeStanceAction& RecAction = PlaybackData.ChangeStanceActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::EquipWeapon:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.EquipWeaponActions.Num());
					const FT4EquipWeaponAction& RecAction = PlaybackData.EquipWeaponActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::UnEquipWeapon:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.UnEquipWeaponActions.Num());
					const FT4UnEquipWeaponAction& RecAction = PlaybackData.UnEquipWeaponActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::ExchangeCostume:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.ExchangeCostumeActions.Num());
					const FT4ExchangeCostumeAction& RecAction = PlaybackData.ExchangeCostumeActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Hit: // #76
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.HitActions.Num());
					const FT4HitAction& RecAction = PlaybackData.HitActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Die: // #76
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.DieActions.Num());
					const FT4DieAction& RecAction = PlaybackData.DieActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Resurrect: // #76
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.ResurrectActions.Num());
					const FT4ResurrectAction& RecAction = PlaybackData.ResurrectActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Conti:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.ContiActions.Num());
					const FT4ContiAction& RecAction = PlaybackData.ContiActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Stop:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.StopActions.Num());
					const FT4StopAction& RecAction = PlaybackData.StopActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;

#if WITH_EDITOR
			case ET4ActionType::Editor: // #80
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < PlaybackData.EditorActions.Num());
					const FT4EditorAction& RecAction = PlaybackData.EditorActions[PlayAction.ActionArrayIndex];
					OwnerObject->DoExecuteAction(&RecAction, ActionParameters);
				}
				break;
#endif

			default:
				{
					UE_LOG(
						LogT4Engine,
						Error,
						TEXT("FT4ActionPlaybackPlayer '%u' failed. no implementation."),
						uint32(PlayAction.ActionType)
					);
				}
				break;
		}
		PlayStartItemIndex = PlayIdx + 1;

		if (bWorldTraveling)
		{
			break; // #87 : 월드 로딩이 끝나면 진입하도록 처리...
		}
	}
}

bool FT4ActionPlaybackPlayer::Play(const FSoftObjectPath& InPlayPath)
{
	OnReset();
	bool bResult = LoadAsset(InPlayPath);
	if (!bResult)
	{
		return false;
	}
	check(nullptr != PlaybackAsset);
	PlayAssetName = T4ActionPlaybackAPI::GetAssetNameFromObjectPath(InPlayPath.ToString());
	PlayFullPath = FPackageName::GetLongPackagePath(InPlayPath.ToString()) / PlayAssetName;
	FT4ActionPlaybackData& PlaybackData = GetPlaybackData();
	{
		MaxPlayTimeSec = PlaybackData.Header.TotalPlayTimeSec;
	}
	FT4GameWorld* GameWorld = GetGameWorld();
	if (nullptr != GameWorld && GameWorld->HasPlayerObject())
	{
		// Player 를 기록해둔다.
		OriginalPlayerObjectID = GameWorld->GetPlayerObject()->GetObjectID();
		check(ET4SpawnMode::All == OriginalPlayerObjectID.SpawnMode);
	}
	bPlaying = true;
	return true;
}

bool FT4ActionPlaybackPlayer::DoPlay(const FSoftObjectPath& InPlayPath)
{
	return Play(InPlayPath);
}

bool FT4ActionPlaybackPlayer::DoPlay(
	const FString& InPlayAssetName,
	const FString& InFolderName
)
{
	const FString LoadFullPath = T4ActionPlaybackAPI::GetLongPackageName(InPlayAssetName, InFolderName);
	return Play(LoadFullPath);
}

void FT4ActionPlaybackPlayer::SetPlayerPossessed(bool bPossess)
{
	bPlayerPossessed = bPossess;
	if (!bPlaying)
	{
		return;
	}
	if (!PlaybackPlayerObjectID.IsValid())
	{
		return;
	}
	FT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	IT4ObjectController* PlayerController = GameWorld->GetPlayerControl();
	check(nullptr != PlayerController);
	if (bPlayerPossessed)
	{
		PlayerController->SetGameObject(PlaybackPlayerObjectID);
	}
	else if (OriginalPlayerObjectID.IsValid() && GameWorld->GetContainer()->HasGameObject(OriginalPlayerObjectID))
	{
		PlayerController->SetGameObject(OriginalPlayerObjectID);
	}
	else
	{
		PlayerController->ClearGameObject(true);
	}
}

void FT4ActionPlaybackPlayer::Stop()
{
	if (!bPlaying)
	{
		return;
	}
	if (bPaused)
	{
		SetPause(false);
	}
	FT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	if (OriginalPlayerObjectID.IsValid())
	{
		check(ET4SpawnMode::All == OriginalPlayerObjectID.SpawnMode);
		if (GameWorld->GetContainer()->HasGameObject(OriginalPlayerObjectID) &&
			!GameWorld->IsPlayerObject(OriginalPlayerObjectID))
		{
			// 본래 Player 로 돌려준다.
			IT4ObjectController* PlayerController = GameWorld->GetPlayerControl();
			check(nullptr != PlayerController);
			PlayerController->SetGameObject(OriginalPlayerObjectID);
		}
	}
	else
	{
		if (GameWorld->HasPlayerObject())
		{
			// playback 용 player 를 해제해준다.
			IT4ObjectController* PlayerController = GameWorld->GetPlayerControl();
			check(nullptr != PlayerController);
			PlayerController->ClearGameObject(true);
		}
	}
	GameWorld->DoPlaybackSnapshotRestore();
	bPlaying = false;
}

void FT4ActionPlaybackPlayer::CheckRepeat()
{
	if (!bRepeat)
	{
		return;
	}
	Stop();
	PlayTimeSec = -1.1f;
	PlayStartItemIndex = 0;
	bPlaying = true;
}

#endif