// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionReplayPlayer.h"
#include "Public/Replay/T4ActionReplayUtility.h"

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
FT4ActionReplayPlayer::FT4ActionReplayPlayer(ET4LayerType InLayerType, bool bInRepeat, bool bInPlayerPossessed)
	: FT4ActionReplayBase(InLayerType)
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

FT4ActionReplayPlayer::~FT4ActionReplayPlayer()
{
}

void FT4ActionReplayPlayer::Reset()
{
	//check(!bPlaying);
	PlayAssetName.Empty();
	PlayTimeSec = 0.0f;
	MaxPlayTimeSec = 0.0f;
	PlayStartItemIndex = 0;
	OriginalPlayerObjectID.Empty();
	PlaybackPlayerObjectID.Empty();
	bWorldTravelPending = false;
	bWorldTraveling = false;
	WorldTravelPlaybackItemIndex = -1;
	WorldTravelStartLocation = FVector::ZeroVector; // #87
}

void FT4ActionReplayPlayer::SetPause(bool bPause)
{
	if (!bPlaying)
	{
		return;
	}
	bPaused = bPause;
	FT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	GameWorld->SetActionReplayPause(bPause);
}

void FT4ActionReplayPlayer::ProcessPlayOutsideWorld(float InDeltaTime)
{
	// #87 : World Tick 밖에서 처리해야 할 Action. 예) WorldTravel
	if (!bPlaying || !bWorldTravelPending)
	{
		return;
	}
	check(bWorldTraveling);
	check(0 <= WorldTravelPlaybackItemIndex);
	FT4ActionReplayData& ReplayData = GetReplayData();
	check(WorldTravelPlaybackItemIndex < ReplayData.PlayActions.Num());
	const FT4ActionReplayItem& PlayAction = ReplayData.PlayActions[WorldTravelPlaybackItemIndex];
	check(ET4ActionType::WorldTravel == PlayAction.ActionType);
	check(PlayAction.ActionArrayIndex < ReplayData.WorldTravelActions.Num());
	FT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	FT4ActionParameters* ActionParameters = nullptr;
	if (INDEX_NONE != PlayAction.UniqueKey && ReplayData.PlayActionParameters.Contains(PlayAction.UniqueKey))
	{
		ActionParameters = &ReplayData.PlayActionParameters[PlayAction.UniqueKey];
	}
	const FSoftObjectPath CurrentWorldObjectPath = GameWorld->GetWorldObjectPath();
	FT4WorldTravelAction CloneAction = ReplayData.WorldTravelActions[PlayAction.ActionArrayIndex];
	if (CloneAction.MapEntityOrLevelObjectPath != CurrentWorldObjectPath)
	{
		CloneAction.StartLocation = WorldTravelStartLocation; // #104 : 같은 레벨은 이동하지 않는다.
		GameWorld->OnExecuteAction(&CloneAction, ActionParameters);
	}
	bWorldTravelPending = false;
}

void FT4ActionReplayPlayer::ProcessPlayInsideWorld(float InDeltaTime)
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
	FT4ActionReplayData& ReplayData = GetReplayData();
	uint32 NumActionReplayItems = ReplayData.PlayActions.Num();
	if (PlayStartItemIndex >= NumActionReplayItems)
	{
		// 액션은 모두 플레이 완료. 시간까지 넘어가면 Repeat 체크
		if (PlayTimeSec >= MaxPlayTimeSec)
		{
			CheckRepeat();
		}
		return; 
	}
	for (uint32 PlayIdx = PlayStartItemIndex; PlayIdx < NumActionReplayItems; ++PlayIdx)
	{
		const FT4ActionReplayItem& PlayAction = ReplayData.PlayActions[PlayIdx];
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
		if (INDEX_NONE != PlayAction.UniqueKey && ReplayData.PlayActionParameters.Contains(PlayAction.UniqueKey))
		{
			ActionParameters = &ReplayData.PlayActionParameters[PlayAction.UniqueKey];
		}
		switch (PlayAction.ActionType)
		{
			// #T4_ADD_ACTION_TAG_CODE

			case ET4ActionType::WorldTravel: // #87
				{
					check(PlayAction.ActionArrayIndex < ReplayData.WorldTravelActions.Num());

					// #87 : 레벨 후 시작 좌표를 찾기 위해 첫번째 Player 오브젝트의 위치를 찾아 레벨 로딩 완료의 기준 좌표로 사용한다.
					WorldTravelStartLocation = FVector::ZeroVector;

					int32 NextPlayerSpawnIndex = -1;
					for (uint32 NextSpawnIdx = PlayIdx + 1; NextSpawnIdx < NumActionReplayItems; ++NextSpawnIdx)
					{
						const FT4ActionReplayItem& NextSpawnAction = ReplayData.PlayActions[NextSpawnIdx];
						if (ET4ActionType::SpawnObject == NextSpawnAction.ActionType)
						{
							check(NextSpawnAction.ActionArrayIndex < ReplayData.SpawnObjectActions.Num());
							const FT4SpawnObjectAction& NextSpawnObjectAction = ReplayData.SpawnObjectActions[NextSpawnAction.ActionArrayIndex];
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

					OriginalPlayerObjectID.Empty(); // 월드가 바뀌어서 이전 Player 는 의미 없음으로 Reset 처리한다.
				}
				break;

			case ET4ActionType::SpawnObject:
				{
					check(PlayAction.ActionArrayIndex < ReplayData.SpawnObjectActions.Num());
					const FT4SpawnObjectAction& RecAction = ReplayData.SpawnObjectActions[PlayAction.ActionArrayIndex];
					bool bResult = GameWorld->OnExecuteAction(&RecAction, ActionParameters);
					if (bResult)
					{
						if (RecAction.bPlayer)
						{
							PlaybackPlayerObjectID = RecAction.ObjectID;
							if (bPlayerPossessed)
							{
								GameWorld->SetPlayerObject(RecAction.ObjectID);
							}
						}
					}
				}
				break;

			case ET4ActionType::DespawnObject:
				{
					check(PlayAction.ActionArrayIndex < ReplayData.DespawnObjectActions.Num());
					const FT4DespawnObjectAction& RecAction = ReplayData.DespawnObjectActions[PlayAction.ActionArrayIndex];
					if (GameWorld->ComparePlayerObject(PlayAction.ObjectID))
					{
						GameWorld->ClearPlayerObject(true);
					}
					GameWorld->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::MoveAsync:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.MoveAsyncActions.Num());
					const FT4MoveAsyncAction& RecAction = ReplayData.MoveAsyncActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::MoveSync:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.MoveSyncActions.Num());
					const FT4MoveSyncAction& RecAction = ReplayData.MoveSyncActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Jump:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.JumpActions.Num());
					const FT4JumpAction& RecAction = ReplayData.JumpActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Roll:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.RollActions.Num());
					const FT4RollAction& RecAction = ReplayData.RollActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Teleport:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.TeleportActions.Num());
					const FT4TeleportAction& RecAction = ReplayData.TeleportActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Turn:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.TurnActions.Num());
					const FT4TurnAction& RecAction = ReplayData.TurnActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::MoveStop:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.MoveStopActions.Num());
					const FT4MoveStopAction& RecAction = ReplayData.MoveStopActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::MoveSpeedSync:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.MoveSpeedSyncActions.Num());
					const FT4MoveSpeedSyncAction& RecAction = ReplayData.MoveSpeedSyncActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Aim: // #113
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.AimActions.Num());
					const FT4AimAction& RecAction = ReplayData.AimActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::LockOn:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.LockOnActions.Num());
					const FT4LockOnAction& RecAction = ReplayData.LockOnActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Stance: // #73
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.StanceActions.Num());
					const FT4StanceAction& RecAction = ReplayData.StanceActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::SubStance: // #106
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.SubStanceActions.Num());
					const FT4SubStanceAction& RecAction = ReplayData.SubStanceActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::EquipWeapon:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.EquipWeaponActions.Num());
					const FT4EquipWeaponAction& RecAction = ReplayData.EquipWeaponActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::UnequipWeapon:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.UnequipWeaponActions.Num());
					const FT4UnequipWeaponAction& RecAction = ReplayData.UnequipWeaponActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Costume:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.CostumeActions.Num());
					const FT4CostumeAction& RecAction = ReplayData.CostumeActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Hit: // #76
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.HitActions.Num());
					const FT4HitAction& RecAction = ReplayData.HitActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Die: // #76
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.DieActions.Num());
					const FT4DieAction& RecAction = ReplayData.DieActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Resurrect: // #76
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.ResurrectActions.Num());
					const FT4ResurrectAction& RecAction = ReplayData.ResurrectActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Conti:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.ContiActions.Num());
					const FT4ContiAction& RecAction = ReplayData.ContiActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

			case ET4ActionType::Stop:
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.StopActions.Num());
					const FT4StopAction& RecAction = ReplayData.StopActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;

#if WITH_EDITOR
			case ET4ActionType::Editor: // #80
				{
					check(nullptr != OwnerObject);
					check(PlayAction.ActionArrayIndex < ReplayData.EditorActions.Num());
					const FT4EditorAction& RecAction = ReplayData.EditorActions[PlayAction.ActionArrayIndex];
					OwnerObject->OnExecuteAction(&RecAction, ActionParameters);
				}
				break;
#endif

			default:
				{
					T4_LOG(
						Error,
						TEXT("No implementation Type '%u'"),
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

bool FT4ActionReplayPlayer::Play(const FSoftObjectPath& InPlayPath)
{
	OnReset();
	bool bResult = LoadAsset(InPlayPath);
	if (!bResult)
	{
		return false;
	}
	check(nullptr != ActionReplayAsset);
	PlayAssetName = T4ActionReplayUtility::GetAssetNameFromObjectPath(InPlayPath.ToString());
	PlayFullPath = FPackageName::GetLongPackagePath(InPlayPath.ToString()) / PlayAssetName;
	FT4ActionReplayData& ReplayData = GetReplayData();
	{
		MaxPlayTimeSec = ReplayData.Header.TotalPlayTimeSec;
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

bool FT4ActionReplayPlayer::DoPlay(const FSoftObjectPath& InPlayPath)
{
	return Play(InPlayPath);
}

bool FT4ActionReplayPlayer::DoPlay(
	const FString& InPlayAssetName,
	const FString& InFolderName
)
{
	const FString LoadFullPath = T4ActionReplayUtility::GetLongPackageName(InPlayAssetName, InFolderName);
	return Play(LoadFullPath);
}

void FT4ActionReplayPlayer::SetPlayerPossessed(bool bPossess)
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
	if (bPlayerPossessed)
	{
		GameWorld->SetPlayerObject(PlaybackPlayerObjectID);
	}
	else if (OriginalPlayerObjectID.IsValid() && GameWorld->GetContainer()->HasGameObject(OriginalPlayerObjectID))
	{
		GameWorld->SetPlayerObject(OriginalPlayerObjectID);
	}
	else
	{
		GameWorld->ClearPlayerObject(true);
	}
}

void FT4ActionReplayPlayer::Stop()
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
			!GameWorld->ComparePlayerObject(OriginalPlayerObjectID))
		{
			GameWorld->SetPlayerObject(OriginalPlayerObjectID); // 본래 Player 로 돌려준다.
		}
	}
	else
	{
		if (GameWorld->HasPlayerObject())
		{
			GameWorld->ClearPlayerObject(true); // playback 용 player 를 해제해준다.
		}
	}
	GameWorld->RestoreActionReplaySnapshot();
	bPlaying = false;
}

void FT4ActionReplayPlayer::CheckRepeat()
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