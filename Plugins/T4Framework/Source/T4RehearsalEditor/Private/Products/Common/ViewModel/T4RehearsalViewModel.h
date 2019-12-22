// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4Engine/Public/T4EngineTypes.h"

/**
  *
 */
enum class ET4ViewModelEditMode
{
	Preview,
	Conti,
	Entity,

	World, // #83
	WorldPreview, // #83

	None,
};

class AActor; // #94
class FViewport;
class FCanvas;
class UWorld;
struct FT4HUDDrawInfo;
class IT4GameWorld;
class IT4GameFrame;
class FT4RehearsalViewportClient;
class UT4EnvironmentDetailObject; // #94
class IT4RehearsalViewModel {
public:
	virtual ~IT4RehearsalViewModel() {}

	virtual ET4LayerType GetLayerType() const = 0;

	virtual ET4ViewModelEditMode GetEditMode() const = 0;
	virtual bool IsEditWidgetMode() const = 0; // #94

	virtual const FString GetAssetPath() = 0; // #79

	virtual UT4EnvironmentDetailObject* GetEnvironmentDetailObject() = 0; // #90, #94

	virtual void OnReset() = 0; // #79
	virtual void OnStartPlay(FT4RehearsalViewportClient* InViewportClient) = 0;

	virtual void OnDrawHUD(FViewport* InViewport, FCanvas* InCanvas, FT4HUDDrawInfo* InOutDrawInfo) = 0; // #59, #83

	virtual IT4GameFrame* CreateGameFrame() = 0; // #87
	virtual IT4GameFrame* GetGameFrame() const = 0; // #79
	virtual IT4GameWorld* GetGameWorld() const = 0; // #93

	virtual AActor* GetEditWidgetModeTarget() const = 0; // #94

	virtual void ChangeWorldEnvironment(FName InTimeTagName) = 0; // #94

	virtual void SetViewportShowOptionCapsule(bool bShow) = 0; // #76
	virtual bool IsShownViewportShowOptionCapsule() const = 0; //#76

	virtual void SetGameWorldTimeStop(bool bPause) = 0; // #94
	virtual bool IsGameWorldTimeStopped() const = 0; // #94

	virtual void SetGameWorldTimelapseScale(float InScale) = 0; // #93
	virtual float GetGameWorldTimelapseScale() const = 0; // #93

	virtual void SetGameWorldTimeHour(float InHour) = 0; // #93
	virtual float GetGameWorldTimeHour() const = 0; // #93

	virtual void NotifyActionPlaybackRec() = 0; // #104
	virtual void NotifyActionPlaybackPlay() = 0; // #104
};