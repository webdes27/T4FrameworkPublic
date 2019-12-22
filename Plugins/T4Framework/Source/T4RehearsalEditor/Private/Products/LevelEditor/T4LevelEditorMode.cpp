// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4LevelEditorMode.h"
#include "T4LevelEditorToolkit.h"

#include "Products/Common/Helper/T4EditorActionPlaybackController.h"

#include "Products/T4RehearsalEditorUtils.h"

#include "T4Engine/Public/T4Engine.h"
#include "Public/T4Frame.h"

#include "LevelEditorViewport.h"
#include "EditorViewportClient.h"
#include "EditorModeManager.h"
#include "Toolkits/ToolkitManager.h"
#include "Subsystems/AssetEditorSubsystem.h"

#include "T4RehearsalEditorInternal.h"

/**
  *
 */
static const TCHAR* DefaultLevelEditorActionPlaybackFolderName = TEXT("LevelEditor"); // #68
const FEditorModeID FT4LevelEditorMode::EM_T4LevelEditorMode = TEXT("EM_T4LevelEditorMode");

FT4LevelEditorMode::FT4LevelEditorMode()
	: bGameViewCached(false)
	, bRealtimeCached(false)
	, EditorActionPlaybackController(nullptr) // #68
{
}

FT4LevelEditorMode::~FT4LevelEditorMode()
{
}

void FT4LevelEditorMode::Initialize()
{
	if (!AssetEditorRequestOpenHandle.IsValid())
	{
		UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>(); // #104
		check(nullptr != AssetEditorSubsystem);
		AssetEditorRequestOpenHandle = AssetEditorSubsystem->OnAssetEditorRequestedOpen().AddSP(
			this,
			&FT4LevelEditorMode::HandleOnAssetEditorRequestedOpen
		);
		AssetEditorOpenedHandle = AssetEditorSubsystem->OnAssetEditorOpened().AddSP(
			this,
			&FT4LevelEditorMode::HandleOnAssetEditorOpened
		);
	}
	CreateEditorActionPlaybackController(); // #104
}

void FT4LevelEditorMode::AddReferencedObjects(FReferenceCollector& Collector) // #68
{
	Collector.AddReferencedObject(EditorActionPlaybackController);
}

void FT4LevelEditorMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid())
	{
		Toolkit = MakeShareable(new FT4LevelEditorToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}

	IT4GameFrame* EditorFramework = T4FrameGet(ET4LayerType::LevelEditor);
	check(nullptr != EditorFramework);
	if (!EditorFramework->HasBegunPlay())
	{
		// #52 : 맵 이동을 했을 경우 다시 재설정을 해주어야 한다.
		EditorFramework->SetEditorViewportClient(&LevelEditorViewportClinet);
		EditorFramework->OnStartPlay();
	}
	EditorFramework->SetInputControlLock(false);
	EditorFramework->SetEditoAISystemPaused(false); // #52

	UWorld* UnrealWorld = EditorFramework->GetWorld();
	check(nullptr != UnrealWorld);
	if (nullptr == UnrealWorld->GetAISystem())
	{
		UnrealWorld->CreateAISystem(); // #31 : ai 동작을 위해 강제로 켜준다!
	}

	GetModeManager()->SetWidgetMode(FWidget::WM_None);
	GetModeManager()->SetHideViewportUI(true);

	if (nullptr != GCurrentLevelEditingViewportClient)
	{
		bGameViewCached = GCurrentLevelEditingViewportClient->IsInGameView();
		GCurrentLevelEditingViewportClient->SetGameView(true);

		bRealtimeCached = GCurrentLevelEditingViewportClient->IsRealtime();
		GCurrentLevelEditingViewportClient->SetRealtime(true); // #68 : 버그로 오해해서 강제로 켜고 복구해준다.
	}

	// #68
	IT4GameWorld* GameWorld = EditorFramework->GetGameWorld();
	if (nullptr != GameWorld && nullptr != GameWorld->GetActionPlaybackPlayer())
	{
		IT4ActionPlaybackPlayer* ActionPlaybackPlayer = GameWorld->GetActionPlaybackPlayer();
		check(nullptr != ActionPlaybackPlayer);
		if (ActionPlaybackPlayer->IsPaused())
		{
			ActionPlaybackPlayer->SetPause(false);
		}
	}
}

void FT4LevelEditorMode::Exit()
{
	IT4GameFrame* EditorFramework = T4FrameGet(ET4LayerType::LevelEditor);
	check(nullptr != EditorFramework);
	EditorFramework->SetInputControlLock(true);
	EditorFramework->SetEditoAISystemPaused(true); // #52

	GetModeManager()->SetWidgetMode(FWidget::WM_Translate);
	GetModeManager()->SetHideViewportUI(false);

	if (nullptr != GCurrentLevelEditingViewportClient)
	{
		GCurrentLevelEditingViewportClient->SetGameView(bGameViewCached);
		GCurrentLevelEditingViewportClient->SetRealtime(bRealtimeCached); // #68 : 버그로 오해해서 강제로 켜고 복구해준다.
	}

	// #68
	IT4GameWorld* GameWorld = EditorFramework->GetGameWorld();
	if (nullptr != GameWorld && nullptr != GameWorld->GetActionPlaybackPlayer())
	{
		IT4ActionPlaybackPlayer* ActionPlaybackPlayer = GameWorld->GetActionPlaybackPlayer();
		check(nullptr != ActionPlaybackPlayer);
		if (!ActionPlaybackPlayer->IsPaused())
		{
			ActionPlaybackPlayer->SetPause(true);
		}
	}

	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	FEdMode::Exit();
}

void FT4LevelEditorMode::Tick(
	FEditorViewportClient* ViewportClient, 
	float DeltaTime
)
{
	FEdMode::Tick(ViewportClient, DeltaTime);
	UpdateCameraGameControl(ViewportClient, DeltaTime); // #40 : PlayerTick 이후 카메라 설정을 변경해주어야 한다!
}

void FT4LevelEditorMode::UpdateCameraGameControl(
	FEditorViewportClient* ViewportClient,
	float DeltaSeconds
)
{
	IT4GameFrame* EditorFramework = T4FrameGet(ET4LayerType::LevelEditor);
	check(nullptr != EditorFramework);
	IT4PlayerController* PlayerController = EditorFramework->GetPlayerController();
	if (nullptr == PlayerController || !PlayerController->HasGameObject())
	{
		return;
	}
	IT4GameObject* ViewTargetObject = PlayerController->GetGameObject();
	check(nullptr != ViewTargetObject);

	ViewportClient->ViewFOV = ViewportClient->FOVAngle = PlayerController->GetCameraFOV(); // #40

	FRotator ViewportRotation = PlayerController->GetCameraRotation();
	ViewportRotation.Yaw = -ViewportRotation.Yaw; // #30 : 카메라 yaw 방향이 반대다.
	// #30 : preview 는 +X축이 전방임으로 게임 설정과는 +90도 차이가 있어 다시 돌려준다.
	ViewportRotation.Yaw -= ViewTargetObject->GetPropertyConst().MeshImportRotation.Yaw;

	const FVector CameraLocation = PlayerController->GetCameraLocation(); // #40
	const FVector CameraLookAtLocation = PlayerController->GetCameraLookAtLocation(); // #40

	ViewportClient->SetViewRotation(ViewportRotation);
	ViewportClient->SetViewLocation(CameraLocation);

	// #17 : bRecalculateView 를 true 로 설정한 이유는 EditorViewportClinet 의 카메라 Mat 가
	//       이미 업데이트 되었기 때문에 무력화하기 위해 강제로 업데이트 해주는 것이다.
	ViewportClient->SetLookAtLocation(CameraLookAtLocation, true);
}

bool FT4LevelEditorMode::InputKey(
	FEditorViewportClient* ViewportClient, 
	FViewport* Viewport, 
	FKey Key, 
	EInputEvent Event
)
{
	bool bResult = true;
	IT4GameFrame* EditorFramework = T4FrameGet(ET4LayerType::LevelEditor);
	check(nullptr != EditorFramework);
	IT4PlayerController* PlayerController = EditorFramework->GetPlayerController();
	if (nullptr != PlayerController)
	{
		bResult = PlayerController->EditorInputKey(Key, Event, 1.0f/*AmountDepressed*/, false/*bGamepad*/);
	}
	if (!bResult)
	{
		if (EKeys::G != Key && EKeys::F11 != Key && EKeys::Tilde != Key)
		{
			bResult = true; // viewport 특수 키는 제외!
		}
	}
	else
	{
		bResult = (nullptr != PlayerController) ? PlayerController->HasGameObject() : false;
	}
	return bResult; // WARN : 모드 이하로 키 정보를 보내지 않는다!
}

bool FT4LevelEditorMode::InputAxis(
	FEditorViewportClient* InViewportClient,
	FViewport* Viewport,
	int32 ControllerId,
	FKey Key,
	float Delta,
	float DeltaTime
)
{
	bool bResult = true;
	IT4GameFrame* EditorFramework = T4FrameGet(ET4LayerType::LevelEditor);
	check(nullptr != EditorFramework);
	IT4PlayerController* PlayerController = EditorFramework->GetPlayerController();
	if (nullptr != PlayerController)
	{
		bResult = PlayerController->EditorInputAxis(Key, Delta, DeltaTime, 1/*NumSamples*/, false/*bGamepad*/);
	}
	else
	{
		bResult = false;
	}
	return bResult; // WARN : 모드 이하로 키 정보를 보내지 않는다!
}

void FT4LevelEditorMode::DrawHUD(
	FEditorViewportClient* ViewportClient,
	FViewport* Viewport,
	const FSceneView* View,
	FCanvas* Canvas
) // #68
{
	FEdMode::DrawHUD(ViewportClient, Viewport, View, Canvas);
	IT4GameFrame* EditorFramework = T4FrameGet(ET4LayerType::LevelEditor);
	check(nullptr != EditorFramework);
	FT4HUDDrawInfo HUDDrawInfo;
	EditorFramework->OnDrawHUD(Viewport, Canvas, HUDDrawInfo);
}

void FT4LevelEditorMode::CreateEditorActionPlaybackController() // #104
{
	if (nullptr == EditorActionPlaybackController)
	{
		EditorActionPlaybackController = NewObject<UT4EditorActionPlaybackController>();
		check(nullptr != EditorActionPlaybackController);
		EditorActionPlaybackController->SetFlags(RF_Transactional); // Undo, Redo
	}

	UWorld* EditorWorld = T4EditorUtil::GetWorld(ET4LayerType::LevelEditor); // #68
	if (nullptr != EditorWorld)
	{
		const FString CurrentLevelName = EditorWorld->GetName();
		EditorActionPlaybackController->Set(
			ET4LayerType::LevelEditor,
			CurrentLevelName,
			DefaultLevelEditorActionPlaybackFolderName
		);
	}
}

void FT4LevelEditorMode::HandleOnAssetEditorRequestedOpen(UObject* InWorldObject) // #104
{
	if (nullptr == Cast<UWorld>(InWorldObject)) // 월드가 아니면 처리하지 않는다.
	{
		return;
	}
	EditorActionPlaybackController = nullptr;
}

void FT4LevelEditorMode::HandleOnAssetEditorOpened(UObject* InWorldObject) // #104
{
	CreateEditorActionPlaybackController();
}

