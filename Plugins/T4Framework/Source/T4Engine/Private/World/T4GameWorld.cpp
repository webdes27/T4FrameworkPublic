// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4GameWorld.h"

#include "Object/T4GameObject.h" // #54
#include "Object/T4GameObjectFactory.h" // #54

#include "Public/Action/T4ActionParameters.h" // #28
#include "Public/Action/T4ActionWorldCommands.h"

#include "T4Asset/Classes/Entity/T4MapEntityAsset.h" // #87
#include "T4Asset/Public/Entity/T4Entity.h"

#include "T4EngineInternal.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Networking/Travelling/
 */
FT4GameWorld::FT4GameWorld()
	: LayerType(ET4LayerType::Max)
	, EntityKeyName(NAME_None) // #100
	, WorldController(this) // #87 
	, WorldContainer(this) // #87
	, WorldCollisionSystem(this) // #87
	, WorldNavigationSystem(this) // #87
{
}

FT4GameWorld::~FT4GameWorld()
{
}

bool FT4GameWorld::OnInitialize(
	const FT4WorldConstructionValues& InWorldConstructionValues // #87
)
{
	check(ET4LayerType::Max == LayerType);
	WorldController.Initialize(InWorldConstructionValues);

	// #87 : GameWorld class 내에서 World 관리를 위해 InWorldContext 가 nullptr 일 수도 있도록 디자인을 변경함
	FWorldContext* ExistOrNewWorldContext = WorldController.GetWorldContext();
	check(nullptr != ExistOrNewWorldContext);

#if WITH_EDITOR
	// #87 : WorldContext 생성 시점이 변경되어 EngineLayer 등록을 T4Frame 모듈에서 T4GameWorld 로 이전함
	T4EngineLayer::Add(ExistOrNewWorldContext);
#endif
	LayerType = T4EngineLayer::Get(ExistOrNewWorldContext); // #87
	check(ET4LayerType::Max != LayerType);

	Create();
	return true;
}

void FT4GameWorld::OnFinalize()
{
	OnReset();
	CleanUp(); // #87

#if WITH_EDITOR
	// #87 : WorldContext 생성 시점이 변경되어 EngineLayer 등록을 T4Frame 모듈에서 T4GameWorld 로 이전함
	T4EngineLayer::Remove(WorldController.GetWorldContextName()); // #30
#endif

	WorldController.Finalize(); // #87
	EntityKeyName = NAME_None;
	WorldObjectPath.Reset();
}

void FT4GameWorld::OnProcessPre(float InDeltaTime)
{
	ProcessPre(InDeltaTime);
	WorldContainer.ProcessPre(InDeltaTime); // #87
	WorldController.ProcessPre(InDeltaTime); // #87
}

void FT4GameWorld::OnProcessPost(float InDeltaTime)
{
	WorldContainer.ProcessPost(InDeltaTime); // #87
	WorldController.ProcessPost(InDeltaTime); // #87
	ProcessPost(InDeltaTime);
}

void FT4GameWorld::OnReset()
{
	WorldNavigationSystem.Reset(); // #87
	WorldCollisionSystem.Reset(); // #87
	WorldContainer.Reset(); // #87
	WorldController.Reset(); // #87
	Reset();
}

ET4GameWorldType FT4GameWorld::GetGameWorldType() const // #87
{
	return WorldController.GetGameWorldType();
}

UWorld* FT4GameWorld::GetWorld() const
{
	return WorldController.GetWorld();
}

void FT4GameWorld::SetWorldObjectPath(const FSoftObjectPath& InWorldObjectPath) // #87
{
	WorldObjectPath = InWorldObjectPath;
}

#if WITH_EDITOR
bool FT4GameWorld::IsDisabledLevelStreaming() const // #86, #104
{
	return WorldController.IsDisabledLevelStreaming();
}

void FT4GameWorld::SetDisableLevelStreaming(bool bInDisable)
{
	// #86 : World 의 UpdateStreamingState 를 제어하기 위한 옵션 처리
	WorldController.SetDisableLevelStreaming(bInDisable);
}

void FT4GameWorld::SetDisableEnvironmentUpdating(bool bInDisable)
{
	// #92 : Map Environemnt Update 제어 옵션 처리
	WorldController.SetDisableEnvironmentUpdating(bInDisable);
}

void FT4GameWorld::SetDisableTimelapse(bool bInDisable)
{
	// #93 : 시간 경과 옵션 처리
	WorldController.SetDisableTimelapse(bInDisable);
}

bool FT4GameWorld::GetTimelapseDisabled() const // #94
{
	return WorldController.GetTimelapseDisabled();
}
#endif

bool FT4GameWorld::OnExecuteAction(
	const FT4ActionCommand* InAction,
	const FT4ActionParameters* InActionParam
)
{
	check(nullptr != InAction);
	bool bResult = true;
	const ET4ActionType ActionType = InAction->ActionType;
	switch (ActionType)
	{
		case ET4ActionType::WorldTravel:
			bResult = ExecuteWorldTravelAction(*(static_cast<const FT4WorldTravelAction*>(InAction)));
			break;

		case ET4ActionType::SpawnObject:
			bResult = ExecuteSpawnObjectAction(*(static_cast<const FT4SpawnObjectAction*>(InAction)));
			break;

		case ET4ActionType::DespawnObject:
			bResult = ExecuteDespawnObjectAction(*(static_cast<const FT4DespawnObjectAction*>(InAction)));
			break;

		default:
		{
			T4_LOG(
				Error,
				TEXT("[SL:%u] No implementation Action '%s'"),
				uint32(LayerType),
				*(InAction->ToString())
			);
			bResult = false;
		}
		break;
	}
#if !UE_BUILD_SHIPPING
	if (bResult)
	{
		// #68 : 일단, 에디터에서만 동작하도록 처리해준다.
		IT4ActionReplayRecorder* ActionReplayRecorder = GetActionReplayRecorder();
		if (nullptr != ActionReplayRecorder)
		{
			ActionReplayRecorder->RecWorldAction(InAction, InActionParam);
		}
	}
#endif
	return bResult;
}

bool FT4GameWorld::ExecuteWorldTravelAction(
	const FT4WorldTravelAction& InAction
)
{
	check(ET4ActionType::WorldTravel == InAction.ActionType);
	bool bResult = WorldController.ProcessWorldTravelAction(InAction);
	if (!bResult)
	{
		T4_LOG(
			Error,
			TEXT("WorldTravel '%s' failed."), 
			*(InAction.MapEntityOrLevelObjectPath.ToString())
		);
		// TODO : 실패했을 경우에 대한 처리!
	}
	return bResult;
}

bool FT4GameWorld::ExecuteSpawnObjectAction(
	const FT4SpawnObjectAction& InAction
)
{
	check(ET4ActionType::SpawnObject == InAction.ActionType);
	bool bResult = WorldContainer.ProcessSpawnObjectAction(InAction);
	return bResult;
}

bool FT4GameWorld::ExecuteDespawnObjectAction(
	const FT4DespawnObjectAction& InAction
)
{
	check(ET4ActionType::DespawnObject == InAction.ActionType);
	bool bResult = WorldContainer.ProcessDespawnObjectAction(InAction.ObjectID, InAction.FadeOutTimeSec);
	return bResult;
}