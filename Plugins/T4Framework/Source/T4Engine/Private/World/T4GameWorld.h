// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Controller/T4WorldController.h" // #87
#include "Container/T4WorldContainer.h" // #87
#include "System/T4WorldCollisionSystem.h" // #87
#include "System/T4WorldNavigationSystem.h" // #87
#include "Public/T4Engine.h"
#include "Public/Action/T4ActionCodeWorld.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Networking/Travelling/
 */
class UT4MapEntityAsset;
class AT4GameObject;
class FT4GameWorld : public IT4GameWorld
{
public:
	explicit FT4GameWorld();
	virtual ~FT4GameWorld();

	// IT4GameWorld
	ET4LayerType GetLayerType() const override { return LayerType; }
	virtual ET4WorldType GetType() const override { return ET4WorldType::Null; }

	void OnReset() override;

	void OnProcessPre(float InDeltaTime) override; // #34 : OnWorldPreActorTick
	void OnProcessPost(float InDeltaTime) override; // #34 : OnWorldPostActorTick

	bool DoExecuteAction(const FT4ActionStruct* InAction, const FT4ActionParameters* InActionParam) override;

	ET4GameWorldType GetGameWorldType() const override; // #87
	const FName GetEntityKeyName() const override { return EntityKeyName; } // #100 : 현재 로딩된 Entity KeyName 만약, 없다면 NAME_None (preview or Level 을 직접) 로 리턴됨

	UWorld* GetWorld() const override;
	const FSoftObjectPath& GetWorldObjectPath() const override { return WorldObjectPath; } // #104

	IT4WorldController* GetController() override { return static_cast<IT4WorldController*>(&WorldController); } // #87
	IT4WorldContainer* GetContainer() override { return static_cast<IT4WorldContainer*>(&WorldContainer); } // #87
	IT4WorldCollisionSystem* GetCollisionSystem() override { return static_cast<IT4WorldCollisionSystem*>(&WorldCollisionSystem); } // #87
	IT4WorldNavigationSystem* GetNavigationSystem() override { return static_cast<IT4WorldNavigationSystem*>(&WorldNavigationSystem); } // #87

public:
	// subclass impl
	// Client Only
	virtual FVector GetCameraLocation() const override { return FVector::ZeroVector; }
	virtual FRotator GetCameraRotation() const override  { return FRotator::ZeroRotator; }

	virtual IT4ObjectController* GetPlayerControl() override  { return nullptr; }
	virtual bool SetPlayerControl(IT4ObjectController* InPlayerControl) override { return false; }

	virtual bool HasPlayerObject() const override { return false; }
	virtual bool IsPlayerObject(const FT4ObjectID& InObjectID) const override { return false; }
	virtual bool IsPlayerObject(IT4GameObject* InGameObject) const override { return false; }
	virtual IT4GameObject* GetPlayerObject() const override { return nullptr; }

#if !UE_BUILD_SHIPPING
	virtual IT4ActionPlaybackPlayer* GetActionPlaybackPlayer() const override { return nullptr; } // #68
	virtual IT4ActionPlaybackRecorder* GetActionPlaybackRecorder() const override { return nullptr; }
	virtual IT4ActionPlaybackController* GetActionPlaybackController() override { return nullptr; }

	virtual void DoPlaybackSnapshotSave() {} // #68, #87
	virtual void DoPlaybackSnapshotRestore() {} // #87
	virtual void DoPlaybackAllPause(bool bPause) {}
#endif

#if WITH_EDITOR
	virtual AT4EditorCameraActor* FindOrCreateEditorCameraActor(
		uint32 InKey, 
		bool bInCreate, 
		bool bInEmulMode
	) override { return nullptr; } // #58 : Only Client
	virtual void DestroyEditorCameraActor(uint32 InKey) override {} // #58 : Only Client

	bool IsDisabledLevelStreaming() const override; // #86, #104
	void SetDisableLevelStreaming(bool bInDisable) override; // #86 : World 의 UpdateStreamingState 를 제어하기 위한 옵션 처리
	void SetDisableEnvironmentUpdating(bool bInDisable) override; // #92 : Map Environemnt Update 제어 옵션 처리
	void SetDisableTimelapse(bool bInDisable) override; // #93 : 시간 경과 옵션 처리
	bool GetTimelapseDisabled() const override; // #94
#endif

public:
	bool OnInitialize(const FT4WorldConstructionValues& InWorldConstructionValues); // #87
	void OnFinalize();

	void SetEntityKeyName(FName InName) { EntityKeyName = InName; } // #100
	void SetWorldObjectPath(const FSoftObjectPath& InWorldObjectPath); // #87

	FT4WorldController* GetControllerImpl() { return &WorldController; } // #94
	FT4WorldContainer* GetContainerImpl() { return &WorldContainer; } // #94

protected:
	virtual void Create() {}
	virtual void Reset() {}
	virtual void CleanUp() {} // #87

	virtual void ProcessPre(float InDeltaTime) {}
	virtual void ProcessPost(float InDeltaTime) {}

	bool ExecuteWorldTravelAction(const FT4WorldTravelAction& InAction);
	bool ExecuteSpawnObjectAction(const FT4SpawnObjectAction& InAction);
	bool ExecuteDespawnObjectAction(const FT4DespawnObjectAction& InAction);

protected:
	ET4LayerType LayerType;
	FName EntityKeyName; // #100
	FSoftObjectPath WorldObjectPath; // #87 : nullptr == PreviewScene

	FT4WorldController WorldController; // #87
	FT4WorldContainer WorldContainer; // #87
	FT4WorldCollisionSystem WorldCollisionSystem; // #87
	FT4WorldNavigationSystem WorldNavigationSystem; // #87
};
