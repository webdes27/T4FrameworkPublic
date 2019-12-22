// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "World/T4GameWorld.h"
#if WITH_EDITOR 
#include "Tickable.h"
#endif
#if !UE_BUILD_SHIPPING
#include "Playback/T4ActionPlaybackController.h" // #68
#endif

/**
  * http://api.unrealengine.com/KOR/Gameplay/Networking/Travelling/
 */
class IT4GameObject;
class APlayerController;
#if WITH_EDITOR
class FT4ClientGameWorld : public FT4GameWorld, public FTickableGameObject
#else
class FT4ClientGameWorld : public FT4GameWorld
#endif
{
public:
	explicit FT4ClientGameWorld();
	virtual ~FT4ClientGameWorld();

#if WITH_EDITOR // #87
	//~ FTickableObjectBase interface
	virtual bool IsTickable() const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	//~ FTickableGameObject interface
	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual bool IsTickableInEditor() const override { return true; }
#endif

public:
	// IT4GameWorld
	ET4WorldType GetType() const override { return ET4WorldType::Client; }

	FVector GetCameraLocation() const override;
	FRotator GetCameraRotation() const override;

	IT4ObjectController* GetPlayerControl() override;
	bool SetPlayerControl(IT4ObjectController* InPlayerControl) override;
	
	bool HasPlayerObject() const override;
	bool IsPlayerObject(const FT4ObjectID& InObjectID) const override;
	bool IsPlayerObject(IT4GameObject* InGameObject) const override;
	IT4GameObject* GetPlayerObject() const override;

#if !UE_BUILD_SHIPPING
	// #68
	IT4ActionPlaybackPlayer* GetActionPlaybackPlayer() const override;
	IT4ActionPlaybackRecorder* GetActionPlaybackRecorder() const override;
	IT4ActionPlaybackController* GetActionPlaybackController() override;

	void DoPlaybackSnapshotSave() override;
	void DoPlaybackSnapshotRestore() override;
	void DoPlaybackAllPause(bool bPause) override;
	// ~#68
#endif

#if WITH_EDITOR
	AT4EditorCameraActor* FindOrCreateEditorCameraActor(
		uint32 InKey, 
		bool bInCreate, 
		bool bInEmulMode
	) override; // #58 : Only Client
	void DestroyEditorCameraActor(uint32 InKey) override; // #58 : Only Client
#endif

protected:
	void Create() override;
	void Reset() override;
	void CleanUp() override; // #87

	void ProcessPre(float InDeltaTime);
	void ProcessPost(float InDeltaTime);

	APlayerController* GetPlayerController() const; // #87

#if WITH_EDITOR
	void DestroyAllEditorCameraActors(); // #58
#endif

private:
	IT4ObjectController * PlayerControl;

#if !UE_BUILD_SHIPPING
	FT4ActionPlaybackController ActionPlaybackController; // #68
#endif

#if WITH_EDITOR
	TMap<uint32, TWeakObjectPtr<AT4EditorCameraActor>> EditorCameraActorMap; // #58 : Only Client
#endif
};
