// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Frame/Public/T4Frame.h"

#include "EditorViewportClient.h"

/**
  *
 */
class IT4RehearsalViewModel;
class IT4PlayerController;
class IT4GameObject;
class FViewport;
struct FWorldContext;
class UT4RehearsalGameInstance; // #79
class ST4RehearsalViewport;
class FT4RehearsalPreviewNullPlane;
struct FPostProcessSettings; // #100
class FT4RehearsalViewportClient
	: public FEditorViewportClient
	, public IT4EditorViewportClient // #30
	, public TSharedFromThis<FT4RehearsalViewportClient>
{
public:
	DECLARE_MULTICAST_DELEGATE(FT4OnDestroy); // #79
	DECLARE_DELEGATE_TwoParams(FT4OnScreenShotCaptured, UObject*, UTexture2D*);

public:
	FT4RehearsalViewportClient(
		const TSharedRef<ST4RehearsalViewport>& InViewport,
		IT4RehearsalViewModel* InViewModel,
		FT4OnScreenShotCaptured InOnScreenShotCaptured
	);
	~FT4RehearsalViewportClient();

	void Tick(float DeltaSeconds) override;
	void Draw(FViewport* Viewport, FCanvas* Canvas) override;

	/** FViewportClient interface */
	bool InputKey(FViewport* InViewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed = 1.f, bool bGamepad = false) override;
	bool InputAxis(FViewport* InViewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples = 1, bool bGamepad = false) override;

	// #48 : Widget Mode 가 Disable 되었음으로 Mouse 컨트롤을 직접해주기 위해 동작하지 않도록 처리
	void ReceivedFocus(FViewport* InViewport) override; 
	void MouseEnter(FViewport* InViewport, int32 x, int32 y) override; // #94
	void MouseMove(FViewport* InViewport, int32 x, int32 y) override; // #94
	void MouseLeave(FViewport* InViewport) override; // #94
	EMouseCursor::Type GetCursor(FViewport* InViewport, int32 X, int32 Y) override;
	void CapturedMouseMove(FViewport* InViewport, int32 InMouseX, int32 InMouseY) override;
	void LostFocus(FViewport* InViewport) override;
	// ~#48

	void TrackingStarted(const struct FInputEventState& InInputState, bool bIsDragging, bool bNudge) override; // #94
	void TrackingStopped() override; // #94

	void SetWidgetMode(FWidget::EWidgetMode NewMode) override {} // #30 : Widget모드 Disable

	// FEditorViewportClient
	UWorld* GetWorld() const override; // #79

	void OverridePostProcessSettings(FSceneView& View) override; // #100

public:
	void OnReset(); // #79

	void SetShowGrid(bool bShowGrid);
	void SetUpdateViewport(float InRuntimeDurationSec); // #59 : 섬네일 뷰포트는 Realtime 이 꺼져 있어 강제로 업데이트 해준다.

	void SetToolkitHost(TSharedRef<class IToolkitHost> InHost); // #94

	FT4OnDestroy& GetOnDestroy() { return OnDestroy; } // #79

public:
	// IT4EditorViewportClient
	FViewport* GetViewport() const override; // #68

	bool IsPreviewMode() const override { return bPreviewMode; }

	void SetUpdateCameraForPlayer(bool bEnable) override; // #79

	void SetMouseLocation(const int InX, const int InY) override;
	bool GetMousePosition(float& InLocationX, float& InLocationY) override;
	bool GetMousePositionToWorldRay(FVector& OutStartPosition, FVector& OutStartDirection) override;

	void ShowMouseCursor(bool InShow) override;
	void SetMouseCursorType(EMouseCursor::Type InMouseCursorType) override;

	void SetInitialLocationAndRotation(const FVector& InLocation, const FRotator& InRotation) override; // #86

private:
	void InitializeWidgetMode(); // #94

	void UpdateCameraFromPlayer(float DeltaSeconds);

	void DrawEmulateGameViewportClient(); // WARN : GameViewportClient 에만 있는 처리를 구현해준다.
	void DrawHUDWarning(FViewport* InViewport, FCanvas* InCanvas, FT4HUDDrawInfo& InOutDrawInfo);
	void DrawCaptureScreenShot(FViewport* InViewport);

	IT4PlayerController* GetPlayerController() const;
	IT4GameObject* GetPlayerObject() const;

private:
	TWeakPtr<ST4RehearsalViewport> ViewportPtr;
	IT4RehearsalViewModel* ViewModelRef;

	bool bPreviewMode;
	bool bUpdateCameraForPlayer; // #79
	bool bViewportFocused;
	bool bShowMouseCursor; // #48
	EMouseCursor::Type OverrideMouseCursorType; // #48

	float RealtimeDisableTimeLeft; // #59 : 섬네일 뷰포트는 Realtime 이 꺼져 있어 강제로 업데이트 해준다.

	FT4OnDestroy OnDestroy; // #79
	FT4OnScreenShotCaptured OnScreenShotCaptured;

	bool bWidgetModeEnabled; // #94

public:
	bool bCaptureScreenShot;
	TWeakObjectPtr<UObject> ScreenShotOwner;
};
