// Copyright 2019 SoonBo Noh. All Rights Reserved.

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

	IT4GameObject* GetMouseOverGameObject() override;
	bool GetMousePickingLocation(FVector& OutLocation) override;

	FViewport* GetViewport() const; // #68

#if WITH_EDITOR
	bool IsPreviewMode() const override; // #68

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

#if WITH_EDITOR
	bool bInputControlLocked; // #30
	bool bPlayerChangeDisabled; // #72
	IT4EditorGameplayHandler* EditorGameplayHandler; // #60
	IT4EditorViewportClient* EditorViewportClient;
	TWeakObjectPtr<AT4PlayerController> EditorPlayerControllerPtr;
#endif
};
