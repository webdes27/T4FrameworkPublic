// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "World/T4GameWorld.h"
#if WITH_EDITOR 
#include "Tickable.h"
#endif
#if !UE_BUILD_SHIPPING
#include "Replay/T4ActionReplaySystem.h" // #68
#endif

/**
  * http://api.unrealengine.com/KOR/Gameplay/Networking/Travelling/
 */
class IT4GameObject;
class APlayerController;
class UMaterialParameterCollection;
class UMaterialParameterCollectionInstance;
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

	bool IsClientRenderable() const override { return (nullptr != PlayerNetworkControlRef) ? true : false; } // #115 : PC가 스폰되어야 렌더링이 가능하다.
	bool SetClientNetworkControl(IT4NetworkControl* InNetworkControl) override;
	
	APlayerController* GetPlayerController() const override; // #114
	APlayerCameraManager* GetPlayerCameraManager() const override; // #114

	bool HasPlayerObject() const override;
	bool ComparePlayerObject(const FT4ObjectID& InObjectID) const override;
	bool ComparePlayerObject(IT4GameObject* InGameObject) const override;

	void ClearPlayerObject(bool bInDefaultPawn) override; // #114

	IT4GameObject* GetPlayerObject() const override;
	void SetPlayerObject(const FT4ObjectID& InObjectID) override; // #114

	bool SetMPCGlobalParameterScalar(FName InParameterName, const float InScalar) override; // #115
	bool SetMPCGlobalParameterColor(FName InParameterName, const FLinearColor& InValue) override; // #115

	FVector GetCameraLocation() const override;
	FRotator GetCameraRotation() const override;

	IT4GameObject* GetIndicatorObject() override; // #117

#if !UE_BUILD_SHIPPING
	// #68
	IT4ActionReplayPlayer* GetActionReplayPlayer() const override;
	IT4ActionReplayRecorder* GetActionReplayRecorder() const override;
	IT4ActionReplaySystem* GetActionReplaySystem() override;

	void SaveActionReplaySnapshot() override;
	void RestoreActionReplaySnapshot() override;
	void SetActionReplayPause(bool bPause) override;
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

	void TryInitializePostProcessMaterials(); // #115

#if WITH_EDITOR
	void DestroyAllEditorCameraActors(); // #58
#endif

private:
	IT4NetworkControl* PlayerNetworkControlRef;
	IT4GameObject* IndicatorObjectRef; // #117

	TWeakObjectPtr<UMaterialParameterCollection> OwnerMPCGlobalPtr; // #115
	TWeakObjectPtr<UMaterialParameterCollectionInstance> MPCGlobalInstancePtr; // #115

#if !UE_BUILD_SHIPPING
	FT4ActionReplaySystem ActionReplaySystem; // #68
#endif

#if WITH_EDITOR
	TMap<uint32, TWeakObjectPtr<AT4EditorCameraActor>> EditorCameraActorMap; // #58 : Only Client
#endif
};
