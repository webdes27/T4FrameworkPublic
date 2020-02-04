// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "Public/T4Frame.h"

/**
  *
 */
struct FWorldContext;
class IT4GameWorld;
class FT4PacketHandlerSC;
class FT4PacketHandlerCS;
class FT4GameFrame : public IT4GameFrame
{
public:
	explicit FT4GameFrame();
	~FT4GameFrame();

public:
	// IT4Frame
	ET4LayerType GetLayerType() const override { return LayerType; }
	virtual ET4FrameType GetType() const override { return ET4FrameType::Frame_None; }

	void OnReset() override;
	void OnStartPlay() override;

	void OnProcessPre(float InDeltaTime) override; // #34
	void OnProcessPost(float InDeltaTime) override; // #34

	void OnDrawHUD(
		FViewport* InViewport, 
		FCanvas* InCanvas,
		FT4HUDDrawInfo& InOutDrawInfo
	) override; // #68 : Only Client

	bool HasBegunPlay() const override { return bBegunPlay; }

	UWorld* GetWorld() const override;
	IT4GameWorld* GetGameWorld() const override;

	void RegisterGameplayInstance(IT4GameplayInstance* InLayerInstance) override // #42
	{
		GameplayInstance = InLayerInstance;
	}
	IT4GameplayInstance* GetGameplayInstance() const override { return GameplayInstance; } // #42

	bool OnWorldTravel(const UT4MapEntityAsset* InMapEntityAsset); // #87

public:
	// subclass impl

	// Client
	virtual IT4PlayerController* GetPlayerController() const override { return nullptr; }

	virtual bool GetMousePositionToWorldRay(FVector& OutLocation, FVector& OutDirection) override { return false; } // #113
	
	virtual IT4GameObject* GetMousePickingObject() override { return nullptr; }
	virtual IT4GameObject* GetMousePickingObject(const FVector& InLocation, const FVector& InDirection, FVector& OutHitLocation) override { return nullptr; } // #111

	virtual bool GetMousePickingLocation(FVector& OutLocation) override { return false; }
	virtual bool GetMousePickingLocation(
		ET4CollisionChannel InCollisionChannel, // #117
		const FVector& InLocation,
		const FVector& InDirection,
		FVector& OutLocation
	) override { return false; } // #113

	virtual FViewport* GetViewport() const override { return nullptr; } // #68

	virtual void ClearOutline() override {} // #115
	virtual void SetOutlineTarget(const FT4ObjectID& InObjectID, const FLinearColor& InColor) override {} // #115

#if WITH_EDITOR
	virtual bool IsPreviewMode() const override { return false; } // #68

	virtual void SetGlboalTimeScale(float InTimeScale) override {} // #117
	virtual float GetGlboalTimeScale() const override { return 1.0f; } // #117

	virtual void SetInputControlLock(bool bLock) override {} // #30
	virtual void SetPlayerChangeDisable(bool bDisable) override {} // #72

	virtual void SetEditoAISystemPaused(bool bInPaused) override {} // #52 : only EditorFramework

	virtual IT4EditorGameplayHandler* GetEditorGameplayCustomHandler() const override { return nullptr; } // #60
	virtual void SetEditorGameplayCustomHandler(IT4EditorGameplayHandler* bInGameplayHandler) {} // #60

	virtual AT4PlayerController* GetEditorPlayerController() const override { return nullptr; } // #79
	virtual void SetEditorPlayerController(AT4PlayerController* InPlayerController) override {} // #42

	virtual IT4EditorViewportClient* GetEditorViewportClient() const override { return nullptr; } // #79
	virtual void SetEditorViewportClient(IT4EditorViewportClient* InViewportClient) override {} // #30
#endif

#if (WITH_EDITOR || WITH_SERVER_CODE)
	// Server
	virtual uint32 GenerateNetIDForServer() override { return 0; } // #41
	virtual FT4ObjectID GenerateObjectIDForServer() override { return T4InvalidGameObjectID; }

	virtual bool RegisterGameAIController(const FT4NetID& InUniqueID, IT4GameAIController* InAIController) override { return false; } // #31
	virtual void UnregisterGameAIController(const FT4NetID& InUniqueID) override {}; // #31

	virtual IT4GameAIController* FindGameAIController(const FT4NetID& InUniqueID) const override { return nullptr; } // #31
#endif

public:
	bool OnInitialize(const FT4WorldConstructionValues& InWorldConstructionValues); // #87
	void OnFinalize();

	void HandleOnGameWorldTravelPre(IT4GameWorld* InGameWorld); // #87 : 월드 이동 ActionReplay 지원
	void HandleOnGameWorldTravelPost(IT4GameWorld* InGameWorld); // #87 : 월드 이동 ActionReplay 지원

protected:
	virtual bool Initialize() { return true; }
	virtual void Finalize() {}

	virtual void ResetPre() {}
	virtual void ResetPost() {}

	virtual void StartPlay() {}
	
	virtual void ProcessPre(float InDeltaTime) {}
	virtual void ProcessPost(float InDeltaTime) {}

	// Client
	virtual void DrawHUD(FViewport* InViewport, FCanvas* InCanvas, FT4HUDDrawInfo& InOutDrawInfo) {} // #68 : Only Client

protected:
	ET4LayerType LayerType;
	bool bInitialized;
	bool bBegunPlay;
	IT4GameWorld* GameWorld;
	IT4GameplayInstance* GameplayInstance; // #42
};
