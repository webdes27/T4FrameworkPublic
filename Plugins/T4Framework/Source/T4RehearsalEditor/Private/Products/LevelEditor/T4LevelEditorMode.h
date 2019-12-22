// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4LevelEditorViewportClient.h"

#include "EdMode.h"

/**
  *
 */
class FViewport;
class FEditorViewportClient;
class UT4EditorActionPlaybackController;
class FT4LevelEditorMode : public FEdMode
{
public:
	FT4LevelEditorMode();
	~FT4LevelEditorMode();

	static const FEditorModeID EM_T4LevelEditorMode;

	void Initialize() override;

	void AddReferencedObjects(FReferenceCollector& Collector) override; // #68

	bool UsesToolkits() const override { return true;  }
	void Enter() override;
	void Exit() override;

	void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;
	
	bool InputKey(
		FEditorViewportClient* ViewportClient,
		FViewport* Viewport, 
		FKey Key, 
		EInputEvent Event
	) override;

	bool InputAxis(
		FEditorViewportClient* InViewportClient, 
		FViewport* Viewport, 
		int32 ControllerId, 
		FKey Key, 
		float Delta, 
		float DeltaTime
	) override;

	void DrawHUD(
		FEditorViewportClient* ViewportClient, 
		FViewport* Viewport, 
		const FSceneView* View, 
		FCanvas* Canvas
	) override; // #68

public:
	UT4EditorActionPlaybackController* GetActionPlaybackController() const { return EditorActionPlaybackController; } // #68

	void HandleOnAssetEditorRequestedOpen(UObject* InWorldObject); // #104
	void HandleOnAssetEditorOpened(UObject* InWorldObject); // #104

private:
	void CreateEditorActionPlaybackController(); // #104
	void UpdateCameraGameControl(FEditorViewportClient* ViewportClient, float DeltaSeconds);

private:
	FDelegateHandle AssetEditorRequestOpenHandle; // #104
	FDelegateHandle AssetEditorOpenedHandle; // #104
	bool bGameViewCached;
	bool bRealtimeCached;
	FT4LevelEditorViewportClient LevelEditorViewportClinet;
	UT4EditorActionPlaybackController* EditorActionPlaybackController; // #68
};
