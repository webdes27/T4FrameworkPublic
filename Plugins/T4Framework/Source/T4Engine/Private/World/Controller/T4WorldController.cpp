// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4WorldController.h"

#include "World/Context/T4WorldContextGame.h"
#if WITH_EDITOR
#include "World/Context/T4WorldContextPreviewWorld.h"
#include "World/Context/T4WorldContextPreviewScene.h"
#endif

#include "World/T4GameWorld.h"

#include "Public/Action/T4ActionWorldCommands.h"
#include "T4Asset/Classes/Entity/T4MapEntityAsset.h" // #87
#include "T4Asset/Public/Entity/T4Entity.h"

#include "Engine/Engine.h"
#include "Engine/LevelStreaming.h"
#include "GameFramework/PlayerController.h"

#include "T4EngineInternal.h"

static const float DefaultTimeTagDayHour = 12.0f; // #93

/**
  * #87
 */
FT4WorldController::FT4WorldController(FT4GameWorld* InGameWorld)
	: GameWorldType(ET4GameWorldType::None) // #87
	, GameWorldRef(InGameWorld)
	, WorldContextRef(nullptr)
	, WorldContextImplement(nullptr)
	, WorldTimeControl(this)
	, WorldEnvironmentControl(this) // #93
#if WITH_EDITOR
	, SaveWorldContextName(NAME_None) // #87 : ContextHandle 로 LayerType 을 만들기 때문에 보존
#endif
{
}

FT4WorldController::~FT4WorldController()
{
}

void FT4WorldController::Initialize(
	const FT4WorldConstructionValues& InWorldConstructionValues // #87
)
{
	check(nullptr == WorldContextRef);
	GameWorldType = InWorldConstructionValues.GameWorldType;
	CreateGameWorldContext(InWorldConstructionValues);

	FSoftObjectPath InitializeWorldObjectPath;
	if (ET4GameWorldType::Preview == GameWorldType)
	{
		FSoftObjectPath WorldTravelAssetPath = SelectWorldTravelObjectPath(
			InWorldConstructionValues.MapEntityOrLevelObjectPath
		);
		if (!WorldTravelAssetPath.IsNull())
		{
			WorldContextImplement->WorldTravel(WorldTravelAssetPath, FVector::ZeroVector);
		}
		InitializeWorldObjectPath = WorldTravelAssetPath;
	}
	else
	{
		check(nullptr != WorldContextRef);
		check(nullptr != WorldContextRef->World());
		InitializeWorldObjectPath = WorldContextRef->World();
#if WITH_EDITOR
		if (EWorldType::PIE == WorldContextRef->WorldType)
		{
			// #104 : PIE 일 경우는 Editor World 의 Original WorldObjectPath 를 넣어준다. (ActionReplay 대응)
			FWorldContext& EditorContext = GEditor->GetEditorWorldContext();
			InitializeWorldObjectPath = EditorContext.World();
		}
#endif
	}

	GameWorldRef->SetWorldObjectPath(InitializeWorldObjectPath);

	WorldTimeControl.StartPlay(DefaultTimeTagDayHour); // #93
	WorldEnvironmentControl.StartPlay(); // #115

#if WITH_EDITOR
	check(nullptr != WorldContextRef);
	SaveWorldContextName = WorldContextRef->ContextHandle; // #87 : ContextHandle 로 LayerType 을 만들기 때문에 보존해준다.
#endif
}

void FT4WorldController::Finalize()
{
	if (nullptr != WorldContextImplement)
	{
		delete WorldContextImplement;
		WorldContextImplement = nullptr;
	}
	WorldEnvironmentControl.Reset();
	WorldTimeControl.Reset(DefaultTimeTagDayHour); // #93
	WorldContextRef = nullptr;
	GameWorldRef = nullptr;
}

void FT4WorldController::Reset()
{
	if (nullptr != WorldContextImplement)
	{
		WorldContextImplement->Reset();
	}
	WorldEnvironmentControl.Reset();
	WorldTimeControl.Reset(DefaultTimeTagDayHour); // #93

#if WITH_EDITOR
	if (ET4GameWorldType::Preview == GameWorldType)
	{
		// #87 : Preview 는 WorldContext 의 Ownership 이 PreviewScene/World 에 있기 때문에 
		//       Reset 후 삭제됨으로 검증을 위하 null 로 만들어준다.
		WorldContextRef = nullptr;
	}
#endif
}

void FT4WorldController::ProcessPre(float InDeltaTime) // #34 : OnWorldPreActorTick
{
	if (nullptr != WorldContextImplement)
	{
		WorldContextImplement->ProcessPre(InDeltaTime);
	}
	WorldTimeControl.Process(InDeltaTime); // #93
	WorldEnvironmentControl.Process(InDeltaTime); // #92
}

void FT4WorldController::ProcessPost(float InDeltaTime) // #34 : OnWorldPostActorTick
{
	if (nullptr != WorldContextImplement)
	{
		WorldContextImplement->ProcessPost(InDeltaTime);
	}
}

bool FT4WorldController::CheckLevelLoadComplated() // #87
{
	if (nullptr == WorldContextRef)
	{
		return false;
	}
	UWorld* UnrealWorld = WorldContextRef->World();
	if (nullptr == UnrealWorld)
	{
		return false;
	}
	const TArray<ULevelStreaming*>& StreamingLevels = UnrealWorld->GetStreamingLevels();
	for (ULevelStreaming* StreamingLevel : StreamingLevels)
	{
		if (StreamingLevel->GetShouldBeVisibleFlag())
		{
			if (!StreamingLevel->HasLoadedLevel())
			{
				return false;
			}
			ULevel* SubLevel = StreamingLevel->GetLoadedLevel();
			if (nullptr == SubLevel)
			{
				return false;
			}
		}
	}
	return true;
}

UWorld* FT4WorldController::GetWorld() const
{
	if (nullptr == WorldContextRef)
	{
		return nullptr; // #87 : 받는쪽에서 예외 처리해주자!
	}
	return WorldContextRef->World();
}

IT4GameWorld* FT4WorldController::GetGameWorld() const
{
	return static_cast<IT4GameWorld*>(GameWorldRef);
} // #93

#if WITH_EDITOR
bool FT4WorldController::IsPreviewScene() const // #87
{
	if (nullptr == WorldContextImplement)
	{
		return false;
	}
	return WorldContextImplement->IsPreviewScene();
}
#endif

void FT4WorldController::SetPlayerController(APlayerController* InPlayerController) // #86
{
	// #86 : GameInstance 를 통한 PlayerContoller 처리를 하지 않음으로 임시로 LocalPlayer 를 설정해준다.
	// #87 : Rehearsal ViewModel 에 있던 처리를 이곳으로 옮김
	check(nullptr != WorldContextImplement);
	WorldContextImplement->SetPlayerController(InPlayerController);
}

bool FT4WorldController::ProcessWorldTravelAction(const FT4WorldTravelAction& InAction)
{
	check(nullptr != GameWorldRef);
	check(ET4ActionType::WorldTravel == InAction.ActionType);
	if (nullptr == WorldContextImplement)
	{
		return false;
	}
	if (ET4GameWorldType::Game == GameWorldType && InAction.bPreveiwScene)
	{
		return false; // #87 : Game 월드에서 Preview 로 이동은 패스!
	}

	FSoftObjectPath WorldTravelObjectPath = SelectWorldTravelObjectPath(InAction.MapEntityOrLevelObjectPath);
#if WITH_EDITOR
	if (WorldTravelObjectPath.IsNull() && !InAction.bPreveiwScene)
#else
	if (WorldTravelObjectPath.IsNull())
#endif
	{
		return false;
	}

	// #87 : 녹화된 월드 이동 ActionReplay 지원을 위해 Delegate 를 통해 Framework Reset/StartPlay 가 동작하도록 수정함
	FT4EngineDelegates::OnGameWorldTravelPre.Broadcast(GameWorldRef);

#if WITH_EDITOR
	if (ET4GameWorldType::Game != GameWorldType &&
		InAction.bPreveiwScene != WorldContextImplement->IsPreviewScene())
	{
		check(nullptr == WorldContextRef); // #87 " Reset 에서 nullptr 만들어주었다.

		// #87 : PreviewScene 또는 PreviewWorld 간의 변경 처리...
		{
			WorldContextImplement->Reset();
			delete WorldContextImplement;
			WorldContextImplement = nullptr;
		}
		FT4WorldConstructionValues WorldConstructionValues;
		WorldConstructionValues.GameWorldType = ET4GameWorldType::Preview;
		WorldConstructionValues.bPreviewThumbnailMode = false;
		WorldConstructionValues.MapEntityOrLevelObjectPath = InAction.MapEntityOrLevelObjectPath;
		CreateGameWorldContext(WorldConstructionValues);
	}
#endif

	if (!WorldContextImplement->IsPreviewScene())
	{
		check(!WorldTravelObjectPath.IsNull());
		bool bResult = WorldContextImplement->WorldTravel(WorldTravelObjectPath, InAction.StartLocation);
		if (!bResult)
		{
			T4_LOG(
				Error,
				TEXT("WorldTravel '%s' failed"),
				*(WorldTravelObjectPath.ToString())
			);
			return false;
		}
	}

	GameWorldRef->SetWorldObjectPath(WorldTravelObjectPath);

#if WITH_EDITOR
	if (ET4GameWorldType::Preview == GameWorldType)
	{
		// #87 : 레벨 이동으로 WorldContext 가 바뀌었음으로 교체!!
		if (nullptr == WorldContextRef)
		{
			WorldContextRef = WorldContextImplement->GetOwnerWorldContext();
		}
		check(nullptr != WorldContextRef);
		WorldContextRef->ContextHandle = SaveWorldContextName; // #87 : ContextHandle 로 LayerType 을 만들기 때문에 보존
	}
#endif

	check(nullptr != WorldContextRef);
#if WITH_EDITOR
	check(WorldContextRef->ContextHandle == SaveWorldContextName);
#endif

	// #87 : 녹화된 월드 이동 ActionReplay 지원을 위해 Delegate 를 통해 Framework Reset/StartPlay 가 동작하도록 수정함
	FT4EngineDelegates::OnGameWorldTravelPost.Broadcast(GameWorldRef);
	return true;
}

bool FT4WorldController::IsDisabledLevelStreaming() const // #86, #104
{
	if (nullptr == WorldContextImplement)
	{
		return false;
	}
	return WorldContextImplement->IsLevelStreamingFrozen();
}

void FT4WorldController::SetDisableLevelStreaming(bool bInDisable) // #86
{
	// #86 : World 의 UpdateStreamingState 를 제어하기 위한 옵션 처리
	check(nullptr != WorldContextImplement);
	WorldContextImplement->SetLevelStreamingFrozen(bInDisable);
}

void FT4WorldController::SetDisableEnvironmentUpdating(bool bInDisable) // #92
{
	// #92 : Map Environemnt Update 제어 옵션 처리
	WorldEnvironmentControl.SetPause(bInDisable);
}

void FT4WorldController::SetDisableTimelapse(bool bInDisable) // #93
{
	// #93 : 시간 경과 옵션 처리
	WorldTimeControl.SetPause(bInDisable);
}

void FT4WorldController::TryCheckAndSpawnGlobalWorldZoneVolume() // #115
{
	// #115 : Global WorldZoneVolume 이 있는지 확인하고 없다면 강제로 생성해준다.
	//        Global WorldZoneVolume 에 기본 PP 효과(Outline) Material 설정이 추가된다.
	// #115 : World 는 무조건 Global WorldZoneVolume 추가하도록 강제한다면 이 처리는 필요가 없다.
	//        현재는 정책적으로 정해지지 않았기 때문에 툴과 클라는 결과가 같도록 유지하기 위해 Fallback Spawn 처리를 해준다.
	WorldEnvironmentControl.TryCheckAndSpawnGlobalWorldZoneVolume();
}

bool FT4WorldController::GetTimelapseDisabled() const // #94
{
	return WorldTimeControl.IsPaused();
}

void FT4WorldController::CreateGameWorldContext(
	const FT4WorldConstructionValues& InWorldConstructionValues
) // #87
{
	check(nullptr == WorldContextImplement);

#if WITH_EDITOR

	// #87
	if (ET4GameWorldType::Preview == GameWorldType)
	{
		if (!InWorldConstructionValues.MapEntityOrLevelObjectPath.IsNull()) // #79
		{
			FT4WorldContextPreviewWorld* NewGameWorldContext = new FT4WorldContextPreviewWorld(this);
			check(nullptr != NewGameWorldContext);
			check(nullptr == InWorldConstructionValues.WorldContextGameOrEditorOnly); // #87 : WorldContext 소유권은 PreviewContext 가 가지고 있다.
			WorldContextRef = NewGameWorldContext->GetOwnerWorldContext();
			WorldContextImplement = static_cast<IT4WorldContext*>(NewGameWorldContext);
		}
		else
		{
			FT4WorldContextPreviewScene* NewGameWorldContext = new FT4WorldContextPreviewScene(this, InWorldConstructionValues.bPreviewThumbnailMode);
			check(nullptr != NewGameWorldContext);
			check(nullptr == InWorldConstructionValues.WorldContextGameOrEditorOnly); // #87 : WorldContext 소유권은 PreviewContext 가 가지고 있다.
			WorldContextRef = NewGameWorldContext->GetOwnerWorldContext();
			WorldContextImplement = static_cast<IT4WorldContext*>(NewGameWorldContext);
		}
	}
	else
#endif
	{
		check(nullptr != InWorldConstructionValues.WorldContextGameOrEditorOnly);
		WorldContextRef = InWorldConstructionValues.WorldContextGameOrEditorOnly;
		WorldContextImplement = new FT4WorldContextGame(this);
		check(nullptr != WorldContextImplement);
	}
	check(nullptr != WorldContextRef);
}

FSoftObjectPath FT4WorldController::SelectWorldTravelObjectPath(
	const FSoftObjectPath& InEntityOrLevelObjectPath
)
{
	FSoftObjectPath WorldTravelAssetPath;
	const UT4MapEntityAsset* MapEntityAsset = nullptr;
	if (!InEntityOrLevelObjectPath.IsNull())
	{
		MapEntityAsset = T4AssetEntityManagerGet()->GetMapEntity(InEntityOrLevelObjectPath);
		if (nullptr != MapEntityAsset)
		{
			if (MapEntityAsset->MapData.LevelAsset.IsNull())
			{
				T4_LOG(
					Error,
					TEXT("LevelAsset Not found")
				);
				return WorldTravelAssetPath;
			}
			WorldTravelAssetPath = MapEntityAsset->MapData.LevelAsset.ToSoftObjectPath();

			check(nullptr != GameWorldRef);
			GameWorldRef->SetEntityKeyName(MapEntityAsset->GetEntityKeyPath()); // #100
		}
	}
	if (nullptr == MapEntityAsset)
	{
		WorldTravelAssetPath = InEntityOrLevelObjectPath;

		check(nullptr != GameWorldRef);
		GameWorldRef->SetEntityKeyName(NAME_None); // #100
	}
	return WorldTravelAssetPath;
}
