// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4RehearsalViewportClient.h"
#include "T4RehearsalViewport.h"

#include "Products/Common/ViewModel/T4RehearsalViewModel.h"
#include "Products/Common/Helper/T4EditorObjectSelectionEditMode.h" // #94

#include "T4Engine/Public/T4Engine.h"
#include "T4Frame/Public/T4Frame.h" // #30

#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "ImageUtils.h"

#include "EditorModes.h" // #94
#include "EditorModeManager.h" // #94

#include "Components/SkyLightComponent.h"
#include "Components/SphereReflectionCaptureComponent.h"

#include "Camera/PlayerCameraManager.h"

#include "T4RehearsalEditorInternal.h"

/**
  * 
 */
FT4RehearsalViewportClient::FT4RehearsalViewportClient(
	const TSharedRef<ST4RehearsalViewport>& InViewport,
	IT4RehearsalViewModel* InViewModel,
	FT4OnScreenShotCaptured InOnScreenShotCaptured
)	: FEditorViewportClient(nullptr, nullptr, StaticCastSharedRef<SEditorViewport>(InViewport))
	, ViewportPtr(InViewport)
	, ViewModelRef(InViewModel) // #59
	, bPreviewMode(false)
	, bUpdateCameraForPlayer(false)
	, bViewportFocused(false)
	, bShowMouseCursor(true) // #48
	, OverrideMouseCursorType(EMouseCursor::None) // #48
	, RealtimeDisableTimeLeft(0.0f) // #59
	, OnScreenShotCaptured(InOnScreenShotCaptured)
	, bWidgetModeEnabled(false) // #94
	, bCaptureScreenShot(false)
{
	check(nullptr != ViewModelRef);

	bPreviewMode = (ET4ViewModelEditMode::Preview == InViewModel->GetEditMode()) ? true : false;

	IT4GameFrame* GameFrame = InViewModel->CreateGameFrame(); // #87
	check(nullptr != GameFrame);

	OverrideNearClipPlane(1.0f);
	SetUpdateCameraForPlayer(false);

	if (bPreviewMode)
	{
		ViewFOV = FOVAngle = 30.0f; // #40
		SetViewRotation(FRotator(0.0f, 180.0f, 0.0f));
		SetViewLocation(FVector(500.0f, 0.0f, 100.0f));
		SetGameView(true);
		SetShowGrid(false);
		ShowWidget(false);
		SetRealtime(false);
	}
	else
	{
		SetViewRotation(FRotator(-25.0f, -180.0f, 0.0f));
		SetViewLocation(FVector(-500.0f, 0.0f, 200.0f));
		SetRealtime(true);
	}

	if (InViewModel->IsEditWidgetMode()) // #94
	{
		InitializeWidgetMode();
	}
}

FT4RehearsalViewportClient::~FT4RehearsalViewportClient()
{
	ViewModelRef = nullptr;
}

void FT4RehearsalViewportClient::OnReset() // #79
{
	GetOnDestroy().Broadcast();
	ViewModelRef = nullptr;
}

void FT4RehearsalViewportClient::Tick(float DeltaSeconds)
{
	if (bPreviewMode && IsRealtime())
	{
		// #59 : 섬네일 뷰포트는 Realtime 이 꺼져 있어 강제로 업데이트 해준다.
		RealtimeDisableTimeLeft -= DeltaSeconds;
		if (RealtimeDisableTimeLeft <= 0.0f)
		{
			IT4GameObject* PlayerObject = GetPlayerObject();
			if (nullptr == PlayerObject || (PlayerObject->IsLoaded() && 0.9f < PlayerObject->GetOpacity()))
			{
				SetRealtime(false); // #78 : 모델 로딩전과 WorldEnter 상황이 아니면 RealTime 을 끄지 않는다.
			}
		}
	}
	FEditorViewportClient::Tick(DeltaSeconds);   
	{
		// #79 : UEditorEngine::Tick( float DeltaSeconds, bool bIdleMode )
		// #87
		UWorld* UnrealWorld = GetWorld();
		if (nullptr != UnrealWorld)
		{
			UWorld* SaveWorld = GWorld; // #79
			{
				GWorld = UnrealWorld;
				UnrealWorld->Tick(ELevelTick::LEVELTICK_All, DeltaSeconds);
				{
					// Update sky light first because sky diffuse will be visible in reflection capture indirect specular
					USkyLightComponent::UpdateSkyCaptureContents(UnrealWorld);
					UReflectionCaptureComponent::UpdateReflectionCaptureContents(UnrealWorld);
				}
			}
			GWorld = SaveWorld;
		}
	}
	UpdateCameraFromPlayer(DeltaSeconds); // #40 : Player 업데이트가 되어야 정상적인 View 설정이 가능해진다.
}

void FT4RehearsalViewportClient::Draw(FViewport* InViewport, FCanvas* InCanvas)
{
	FEditorViewportClient::Draw(InViewport, InCanvas);

	DrawEmulateGameViewportClient(); // WARN : GameViewportClient 에만 있는 처리를 구현해준다.(EditorViewportClient 에는 없는)

	DrawCaptureScreenShot(InViewport);

	FT4HUDDrawInfo HUDDrawInfo;
	DrawHUDWarning(InViewport, InCanvas, HUDDrawInfo);

	if (nullptr != ViewModelRef)
	{
		ViewModelRef->OnDrawHUD(InViewport, InCanvas, &HUDDrawInfo); // #59, #83
	}
}

void FT4RehearsalViewportClient::InitializeWidgetMode() // #94
{
	{
		FEditorModeTools* EditorModeTools = GetModeTools();
		check(nullptr != EditorModeTools);

		Widget->SetUsesEditorModeTools(EditorModeTools);

		EditorModeTools->RemoveDefaultMode(FBuiltinEditorModes::EM_Default);
		EditorModeTools->AddDefaultMode(FT4EditorObjectSelectionEditMode::EM_T4EditorObjectSelectionEditMode);
		EditorModeTools->DeactivateAllModes();
		EditorModeTools->ActivateDefaultMode();

		EditorModeTools->SetWidgetMode(FWidget::EWidgetMode::WM_Translate);

		FEdMode* DefaultEditMode = EditorModeTools->GetActiveMode(FT4EditorObjectSelectionEditMode::EM_T4EditorObjectSelectionEditMode);
		check(nullptr != DefaultEditMode);
		FT4EditorObjectSelectionEditMode* GameObjectSelectionEditMode = static_cast<FT4EditorObjectSelectionEditMode*>(DefaultEditMode);
		check(nullptr != GameObjectSelectionEditMode);
		GameObjectSelectionEditMode->SetViewModel(ViewModelRef);

		bWidgetModeEnabled = true;

		if (Viewport)
		{
			Viewport->InvalidateHitProxy();
		}
	}
}

void FT4RehearsalViewportClient::UpdateCameraFromPlayer(float DeltaSeconds)
{
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return;
	}
	if (!bUpdateCameraForPlayer)
	{
		// #86 : ViewTarget 이 없을 경우 카메라 동작 및 Level Streaming 이 동작하지 않음으로 Default Pawn 을 스폰처리해준다.
		//       함께, AT4PlayerController 의 CachedDefaultPawn 를 Client 만 사용하다 모두 사용하도록 함께 정리함
		const FVector& CameraLocation = GetViewLocation();
		const FRotator& CameraRotation = GetViewRotation();
		PlayerController->SetFreeCameraLocationAndRotation(CameraLocation, CameraRotation); // #86, #94
		return;
	}
	if (!bPreviewMode)
	{
		bool bHasPlayerObject = PlayerController->HasGameObject();
		if (bHasPlayerObject && !IsCameraLocked())
		{
			EnableCameraLock(true);
		}
		else if (!bHasPlayerObject && IsCameraLocked())
		{
			EnableCameraLock(false);
		}
		if (!bHasPlayerObject)
		{
			return;
		}
	}
	if (!bPreviewMode)
	{
		ViewFOV = FOVAngle = PlayerController->GetCameraFOV(); // #40
	}
	const FVector CameraLocation = PlayerController->GetCameraLocation(); // #40
	FRotator CameraRotation = PlayerController->GetCameraRotation();
	const FVector CameraLookAtLocation = PlayerController->GetCameraLookAtLocation(); // #40
	SetViewLocation(CameraLocation);
	SetViewRotation(CameraRotation);
	SetLookAtLocation(CameraLookAtLocation, true); // Preview 는 +X 이 전방으로 Matrix 를 설정함으로 true 를 통해 업데이트를 해주고 있다.
}

void FT4RehearsalViewportClient::DrawEmulateGameViewportClient()
{
	// WARN : GameViewportClient 에만 있는 처리를 구현해준다.
	{
		// #86 : UGameViewportClient 처리 참고. 
		//       EditorViewportClient 에서는 LevelStreaming 을 사용하지 않기 때문에 별도로 처리해주는 것
		//       엔진 업데이트 시 매번 체크 필요!
		UWorld* UnrealWorld = GetWorld();
		if (nullptr != UnrealWorld)
		{
			UWorld* SaveWorld = GWorld; // #79
			{
				GWorld = UnrealWorld;
				UnrealWorld->UpdateLevelStreaming();
			}
			GWorld = SaveWorld;
		}
	}
}

inline void DrawHUDWarningInternal(
	FViewport* InViewport,
	FCanvas* InCanvas,
	const FString& InMessages,
	const FLinearColor& InColor,
	FT4HUDDrawInfo& InOutDrawInfo
)
{
	int32 XL;
	int32 YL;
	StringSize(GEngine->GetLargeFont(), XL, YL, *InMessages);
	const float DrawX = FMath::FloorToFloat(InViewport->GetSizeXY().X - XL - 10.0f);
	const float DrawY = InOutDrawInfo.NoticeLineOffset + YL;
	InCanvas->DrawShadowedString(DrawX, DrawY, *InMessages, GEngine->GetLargeFont(), InColor);
	InOutDrawInfo.NoticeLineOffset += YL + 2.0f;
}

void FT4RehearsalViewportClient::DrawHUDWarning(
	FViewport* InViewport, 
	FCanvas* InCanvas,
	FT4HUDDrawInfo& InOutDrawInfo
)
{
	InOutDrawInfo.NoticeLineOffset = 25.0f;

	if (!bPreviewMode && !IsRealtime())
	{
		// #59 : 툴 동작 중 PIE 를 실행하면 모든 ViewportClinet 에 대한 Runtime 을 꺼버리는 처리로 인해
		//       화면 멈춤이 있어 툴 화면에 Runtime 이 꺼졌다는 표시를 추가해준다.
		const FString DrawString = TEXT("* Realtime update disabled");
		DrawHUDWarningInternal(InViewport, InCanvas, DrawString, FLinearColor::Red, InOutDrawInfo);
	}
}

void FT4RehearsalViewportClient::DrawCaptureScreenShot(FViewport* InViewport)
{
	if (!bCaptureScreenShot)
	{
		return;
	}
	if (!ScreenShotOwner.IsValid() || !OnScreenShotCaptured.IsBound())
	{
		return;
	}

	int32 SrcWidth = InViewport->GetSizeXY().X;
	int32 SrcHeight = InViewport->GetSizeXY().Y;
	// Read the contents of the viewport into an array.
	TArray<FColor> OrigBitmap;
	if (InViewport->ReadPixels(OrigBitmap))
	{
		check(OrigBitmap.Num() == SrcWidth * SrcHeight);

		// Resize image to enforce max size.
		TArray<FColor> ScaledBitmap;
		int32 ScaledWidth = 512;
		int32 ScaledHeight = 512;
		FImageUtils::ImageResize(SrcWidth, SrcHeight, OrigBitmap, ScaledWidth, ScaledHeight, ScaledBitmap, true);

		// Compress.
		FCreateTexture2DParameters Params;
		Params.bDeferCompression = true;

		UTexture2D* ThumbnailImage = FImageUtils::CreateTexture2D(
			ScaledWidth,
			ScaledHeight,
			ScaledBitmap,
			ScreenShotOwner.Get(),
			TEXT("ThumbnailTexture"),
			RF_NoFlags,
			Params
		);

		OnScreenShotCaptured.Execute(ScreenShotOwner.Get(), ThumbnailImage);
	}

	bCaptureScreenShot = false;
	ScreenShotOwner.Reset();
}

bool FT4RehearsalViewportClient::InputKey(
	FViewport* InViewport,
	int32 ControllerId,
	FKey Key,
	EInputEvent Event,
	float AmountDepressed,
	bool bGamepad
)
{
	bool bResult = true;
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr != PlayerController)
	{
		bResult = PlayerController->EditorInputKey(Key, Event, AmountDepressed, bGamepad);
	}
	if (!bUpdateCameraForPlayer || bWidgetModeEnabled)
	{
		bResult = FEditorViewportClient::InputKey(
			InViewport, 
			ControllerId, 
			Key, 
			Event, 
			AmountDepressed, 
			bGamepad
		);
	}
	return bResult;
}

bool FT4RehearsalViewportClient::InputAxis(
	FViewport* InViewport,
	int32 ControllerId,
	FKey Key,
	float Delta,
	float DeltaTime,
	int32 NumSamples,
	bool bGamepad
)
{
	bool bResult = true;
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr != PlayerController)
	{
		bResult = PlayerController->EditorInputAxis(Key, Delta, DeltaTime, NumSamples, bGamepad);
	}
	if (!bUpdateCameraForPlayer || bWidgetModeEnabled)
	{
		bResult = FEditorViewportClient::InputAxis(
			InViewport, 
			ControllerId, 
			Key, 
			Delta, 
			DeltaTime, 
			NumSamples,
			bGamepad
		);
	}
	return bResult;
}

// #48 : Widget Mode 가 Disable 되었음으로 Mouse 컨트롤을 직접해주기 위해 동작하지 않도록 처리
void FT4RehearsalViewportClient::ReceivedFocus(FViewport* InViewport)
{
	bViewportFocused = true;
	if (bWidgetModeEnabled)
	{
		FEditorViewportClient::ReceivedFocus(InViewport);
	}
}

void FT4RehearsalViewportClient::MouseEnter(FViewport* InViewport, int32 x, int32 y) // #94
{
	if (bWidgetModeEnabled)
	{
		FEditorViewportClient::MouseEnter(InViewport, x, y);
	}
}

void FT4RehearsalViewportClient::MouseMove(FViewport* InViewport, int32 x, int32 y) // #94
{
	if (bWidgetModeEnabled)
	{
		FEditorViewportClient::MouseMove(InViewport, x, y);
	}
}

void FT4RehearsalViewportClient::MouseLeave(FViewport* InViewport) // #94
{
	if (bWidgetModeEnabled)
	{
		FEditorViewportClient::MouseLeave(InViewport);
	}
}

EMouseCursor::Type FT4RehearsalViewportClient::GetCursor(
	FViewport* InViewport, 
	int32 X, 
	int32 Y
)
{
	if (!bShowMouseCursor)
	{
		return EMouseCursor::None;
	}
	if (bWidgetModeEnabled)
	{
		FEditorViewportClient::GetCursor(InViewport, X, Y);
	}
	EMouseCursor::Type MouseCursor = EMouseCursor::Default;
	if (EMouseCursor::None != OverrideMouseCursorType)
	{
		MouseCursor = OverrideMouseCursorType;
	}
	return MouseCursor;
}

void FT4RehearsalViewportClient::CapturedMouseMove(FViewport* InViewport, int32 InMouseX, int32 InMouseY)
{
	if (bWidgetModeEnabled)
	{
		FEditorViewportClient::CapturedMouseMove(InViewport, InMouseX, InMouseY);
	}
}

void FT4RehearsalViewportClient::LostFocus(FViewport* InViewport)
{
	bViewportFocused = false;
	if (bWidgetModeEnabled)
	{
		FEditorViewportClient::LostFocus(InViewport);
	}
}
// ~#48

UWorld* FT4RehearsalViewportClient::GetWorld() const // #79
{
	if (nullptr == ViewModelRef)
	{
		return nullptr;
	}
	return ViewModelRef->GetGameFrame()->GetWorld();
}

void FT4RehearsalViewportClient::OverridePostProcessSettings(FSceneView& View) // #100
{
	if (nullptr == ViewModelRef)
	{
		return;
	}
	IT4GameWorld* GameWorld = ViewModelRef->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return;
	}
	IT4ObjectController* GameplayControl = GameWorld->GetPlayerControl();
	if (nullptr == GameplayControl)
	{
		return;
	}
	APlayerCameraManager* PlayerCameraManager = GameplayControl->GetCameraManager();
	if (nullptr == PlayerCameraManager)
	{
		return;
	}
	// #100 : GameViewportClient 에서만 호출되어 별도 구현함
	//        refer FSceneView * ULocalPlayer::CalcSceneView(class FSceneViewFamily* ViewFamily,
	{
		TArray<FPostProcessSettings> const* CameraAnimPPSettings;
		TArray<float> const* CameraAnimPPBlendWeights;
		PlayerCameraManager->GetCachedPostProcessBlends(CameraAnimPPSettings, CameraAnimPPBlendWeights);

		for (int32 PPIdx = 0; PPIdx < CameraAnimPPBlendWeights->Num(); ++PPIdx)
		{
			View.OverridePostProcessSettings((*CameraAnimPPSettings)[PPIdx], (*CameraAnimPPBlendWeights)[PPIdx]);
		}
	}
}

FViewport* FT4RehearsalViewportClient::GetViewport() const // #68
{
	return Viewport;
}

void FT4RehearsalViewportClient::SetMouseLocation(const int InX, const int InY)
{
	if (nullptr != Viewport)
	{
		Viewport->SetMouse(InX, InY);
	}
}

bool FT4RehearsalViewportClient::GetMousePosition(float& InLocationX, float& InLocationY)
{
	FViewportCursorLocation CusorLocation = GetCursorWorldLocationFromMousePos();
	InLocationX = CusorLocation.GetCursorPos().X;
	InLocationY = CusorLocation.GetCursorPos().Y;
	return true;
}

bool FT4RehearsalViewportClient::GetMousePositionToWorldRay(
	FVector& OutStartPosition,
	FVector& OutStartDirection
)
{
	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
		Viewport,
		GetWorld()->Scene,
		EngineShowFlags)
		.SetRealtimeUpdate(true));

	FSceneView* SceneView = CalcSceneView(&ViewFamily);
	if (nullptr != SceneView)
	{
		FIntPoint MousePosition;
		FVector WorldOrigin;
		FVector WorldDirection;
		Viewport->GetMousePos(MousePosition);
		SceneView->DeprojectFVector2D(MousePosition, OutStartPosition, OutStartDirection);
		return true;
	}
	return true;
}

void FT4RehearsalViewportClient::SetToolkitHost(TSharedRef<class IToolkitHost> InHost) // #94
{
	GetModeTools()->SetToolkitHost(InHost);
}

void FT4RehearsalViewportClient::TrackingStarted(
	const struct FInputEventState& InInputState, 
	bool bIsDraggingWidget, 
	bool bNudge
) // #94
{
	ModeTools->StartTracking(this, Viewport);
}

void FT4RehearsalViewportClient::TrackingStopped() // #94
{
	ModeTools->EndTracking(this, Viewport);

	Invalidate();
}

void FT4RehearsalViewportClient::ShowMouseCursor(bool InShow)
{
	bShowMouseCursor = InShow;
}

void FT4RehearsalViewportClient::SetMouseCursorType(EMouseCursor::Type InMouseCursorType)
{
	OverrideMouseCursorType = InMouseCursorType; // #48
}

void FT4RehearsalViewportClient::SetInitialLocationAndRotation(
	const FVector& InLocation, 
	const FRotator& InRotation
) // #86
{ 
	SetViewLocation(InLocation);
	SetViewRotation(InRotation);
}

void FT4RehearsalViewportClient::SetShowGrid(bool bShowGrid)
{
	if (bShowGrid != IsSetShowGridChecked())
	{
		FEditorViewportClient::SetShowGrid();
	}
}

void FT4RehearsalViewportClient::SetUpdateViewport(float InRuntimeDurationSec)
{
	if (!bPreviewMode)
	{
		return;
	}
	// #59 : 섬네일 뷰포트는 Realtime 이 꺼져 있어 강제로 업데이트 해준다.
	SetRealtime(true);
	RealtimeDisableTimeLeft = InRuntimeDurationSec;
}

void FT4RehearsalViewportClient::SetUpdateCameraForPlayer(bool bEnable)
{
	bUpdateCameraForPlayer = bEnable;
	if (bUpdateCameraForPlayer)
	{
		EnableCameraLock(true);
	}
	else
	{
		EnableCameraLock(false);
	}
}

IT4PlayerController* FT4RehearsalViewportClient::GetPlayerController() const
{
	if (nullptr == ViewModelRef)
	{
		return nullptr;
	}
	IT4GameFrame* GameFrame = ViewModelRef->GetGameFrame();
	check(nullptr != GameFrame);
	IT4PlayerController* PlayerController = GameFrame->GetPlayerController();
	if (nullptr == PlayerController)
	{
		return nullptr;
	}
	return PlayerController;
}

IT4GameObject* FT4RehearsalViewportClient::GetPlayerObject() const
{
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return nullptr;
	}
	return PlayerController->GetGameObject();
}
