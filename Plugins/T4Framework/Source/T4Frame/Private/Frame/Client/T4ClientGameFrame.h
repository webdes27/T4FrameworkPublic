// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#if (WITH_EDITOR || WITH_SERVER_CODE)
#include "Frame/Server/T4ServerGameFrame.h"
#else
#include "Frame/T4GameFrame.h"
#endif

#if WITH_EDITOR
#include "UnrealEdMisc.h"
#endif

/**
  *
 */
#if WITH_EDITOR
class AAIController;
class AT4PlayerController;
class IT4EditorGameplayHandler;
#endif

#if (WITH_EDITOR || WITH_SERVER_CODE)
class FT4ClientGameFrame : public FT4ServerGameFrame
#else
class FT4ClientGameFrame : public FT4GameFrame
#endif
{
public:
	explicit FT4ClientGameFrame();
	~FT4ClientGameFrame();

	// IT4Frame
	ET4FrameType GetType() const override { return ET4FrameType::Frame_Client; }

	IT4PlayerController* GetPlayerController() const override;

	bool GetMousePositionToWorldRay(FVector& OutLocation, FVector& OutDirection) override; // #113
	
	IT4GameObject* GetMousePickingObject() override;
	IT4GameObject* GetMousePickingObject(const FVector& InLocation, const FVector& InDirection, FVector& OutHitLocation) override; // #111

	bool GetMousePickingLocation(FVector& OutLocation) override;
	bool GetMousePickingLocation(
		ET4CollisionChannel InCollisionChannel, // #117
		const FVector& InLocation,
		const FVector& InDirection,
		FVector& OutLocation
	) override; // #113

	FViewport* GetViewport() const; // #68

	void ClearOutline() override; // #115
	void SetOutlineTarget(const FT4ObjectID& InObjectID, const FLinearColor& InColor) override; // #115

#if WITH_EDITOR
	bool IsPreviewMode() const override; // #68

	void SetGlboalTimeScale(float InTimeScale) override; // #117
	float GetGlboalTimeScale() const override; // #117

	void SetInputControlLock(bool bLock) override; // #30
	void SetPlayerChangeDisable(bool bDisable) override; // #72

	void SetEditoAISystemPaused(bool bInPaused) override; // #52

	IT4EditorGameplayHandler* GetEditorGameplayCustomHandler() const override { return EditorGameplayHandler; } // #60
	void SetEditorGameplayCustomHandler(IT4EditorGameplayHandler* InGameplayAIHandler) override // #60
	{ 
		EditorGameplayHandler = InGameplayAIHandler;
	} 

	AT4PlayerController* GetEditorPlayerController() const override { return EditorPlayerControllerPtr.Get(); } // #79
	void SetEditorPlayerController(AT4PlayerController* InPlayerController) override; // #42

	IT4EditorViewportClient* GetEditorViewportClient() const override { return EditorViewportClient; } // #79
	void SetEditorViewportClient(IT4EditorViewportClient* InViewportClient) override { EditorViewportClient = InViewportClient; }// #30
#endif

protected:
	bool Initialize() override;
	void Finalize() override;

	void ResetPre() override;
	void ResetPost() override;

	void StartPlay() override;

	void ProcessPre(float InDeltaTime) override;
	void ProcessPost(float InDeltaTime) override;

	void DrawHUD(
		FViewport* InViewport,
		FCanvas* InCanvas,
		FT4HUDDrawInfo& InOutDrawInfo
	) override; // #68 : Only Client

private:
	bool InitializeDeferred();

	void ProcessInput(float InDeltaTime);

	void TryCheckPlayerSpawned();

#if WITH_EDITOR
	void StartPlayPreview();
	void StartPlayLevelEditor();
#endif

private:
	bool bInitializeDataLoaded;
	bool bPlayerSpawned; // #42

	FT4ObjectID OutlineTargetObjectID; // #115
	FLinearColor OutlineTargetColor; // #115

#if WITH_EDITOR
	float GlboalTimeScale; // #117
	bool bInputControlLocked; // #30
	bool bPlayerChangeDisabled; // #72
	IT4EditorGameplayHandler* EditorGameplayHandler; // #60
	IT4EditorViewportClient* EditorViewportClient;
	TWeakObjectPtr<AT4PlayerController> EditorPlayerControllerPtr;
#endif
};
