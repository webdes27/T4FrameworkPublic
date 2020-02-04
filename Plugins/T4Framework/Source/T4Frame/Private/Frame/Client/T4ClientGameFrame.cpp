// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ClientGameFrame.h"

#include "Classes/Controller/Player/T4PlayerController.h"

#if WITH_EDITOR
#include "Engine/LocalPlayer.h"
#endif

#include "T4Asset/Public/Entity/T4Entity.h" // #35
#include "T4Engine/Public/T4Engine.h"
#include "T4Engine/Public/T4EngineConstants.h" // #115
#include "T4Frame/Classes/Controller/Player/T4PlayerDefaultPawn.h" // #86

#if WITH_EDITOR
#include "GameFramework/WorldSettings.h"
#endif

#include "T4FrameInternal.h"

/**
  *
 */
FT4ClientGameFrame::FT4ClientGameFrame()
	: bInitializeDataLoaded(false)
	, bPlayerSpawned(false)
	, OutlineTargetColor(FLinearColor::White) // #115
#if WITH_EDITOR
	, GlboalTimeScale(1.0f) // #117
	, bInputControlLocked(false)
	, bPlayerChangeDisabled(false) // #72
	, EditorGameplayHandler(nullptr) // #60
	, EditorViewportClient(nullptr) // #30
#endif
{
}

FT4ClientGameFrame::~FT4ClientGameFrame()
{
}

bool FT4ClientGameFrame::Initialize()
{
	// #17 : 레벨 에디터에서는 LdMode 변경시 테이블을 읽어들인다. 지연로딩!
	if (!T4EngineLayer::IsLevelEditor(LayerType))
	{
		bool bResult = InitializeDeferred();
		if (!bResult)
		{
			return false;
		}
	}
	return true;
}

bool FT4ClientGameFrame::InitializeDeferred()
{
	// #35, #9
	TArray<FString> AssetPaths;
	AssetPaths.Add(TEXT("/Game"));
	if (!T4AssetEntityManagerGet()->Initialize(AssetPaths))
	{
		return false;
	}
	bInitializeDataLoaded = true;
	return true;
}

void FT4ClientGameFrame::Finalize()
{
#if WITH_EDITOR
	check(!EditorPlayerControllerPtr.IsValid());
#endif
	if (bInitializeDataLoaded)
	{
		T4AssetEntityManagerGet()->Finalize(); // #35, #9
	}
#if (WITH_EDITOR || WITH_SERVER_CODE)
	FT4ServerGameFrame::Finalize();
#endif
}

void FT4ClientGameFrame::ResetPre()
{
#if WITH_EDITOR
	if (EditorPlayerControllerPtr.IsValid())
	{
		EditorPlayerControllerPtr->RemoveFromRoot();
		EditorPlayerControllerPtr->Destroy();
		EditorPlayerControllerPtr.Reset();
	}
	FT4ServerGameFrame::ResetPre();
#endif
}

void FT4ClientGameFrame::ResetPost()
{
	bPlayerSpawned = false;
#if (WITH_EDITOR || WITH_SERVER_CODE)
	FT4ServerGameFrame::ResetPost();
#endif
}

void FT4ClientGameFrame::StartPlay()
{
#if WITH_EDITOR
	if (T4EngineLayer::IsPreview(LayerType))
	{
		StartPlayPreview();
	}
	else if (T4EngineLayer::IsLevelEditor(LayerType))
	{
		StartPlayLevelEditor();
	}
#endif
#if (WITH_EDITOR || WITH_SERVER_CODE)
	FT4ServerGameFrame::StartPlay();
#endif
}

void FT4ClientGameFrame::ProcessPre(float InDeltaTime)
{
#if (WITH_EDITOR || WITH_SERVER_CODE)
	FT4ServerGameFrame::ProcessPre(InDeltaTime);
#endif
	ProcessInput(InDeltaTime);
}

void FT4ClientGameFrame::ProcessPost(float InDeltaTime)
{
#if (WITH_EDITOR || WITH_SERVER_CODE)
	FT4ServerGameFrame::ProcessPost(InDeltaTime);
#endif
}

void FT4ClientGameFrame::DrawHUD(
	FViewport* InViewport, 
	FCanvas* InCanvas,
	FT4HUDDrawInfo& InOutDrawInfo
) // #68 : Only Client
{
	if (nullptr != GameplayInstance)
	{
		GameplayInstance->OnDrawHUD(InViewport, InCanvas, InOutDrawInfo);
	}
}

void FT4ClientGameFrame::ProcessInput(float InDeltaTime)
{
	TryCheckPlayerSpawned();
}

void FT4ClientGameFrame::TryCheckPlayerSpawned()
{
	if (bPlayerSpawned)
	{
		return;
	}
	check(nullptr != GameWorld);
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return;
	}
	if (nullptr != GameplayInstance)
	{
		GameplayInstance->OnPlayerSpawned(PlayerController);
#if WITH_EDITOR
		SetInputControlLock(bInputControlLocked);
		SetPlayerChangeDisable(bPlayerChangeDisabled); // #72
#endif
	}
	bPlayerSpawned = true;
}

IT4PlayerController* FT4ClientGameFrame::GetPlayerController() const
{
	if (nullptr == GameWorld)
	{
		return nullptr;
	}
	APlayerController* AController = GameWorld->GetPlayerController();
	if (nullptr == AController)
	{
		return nullptr;
	}
	AT4PlayerController* PlayerController = Cast<AT4PlayerController>(AController);
	if (nullptr == PlayerController)
	{
		return nullptr;
	}
	return static_cast<IT4PlayerController*>(PlayerController);
}

bool FT4ClientGameFrame::GetMousePositionToWorldRay(
	FVector& OutLocation,
	FVector& OutDirection
) // #113
{
	check(nullptr != GameWorld);
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return false;
	}
	bool bResult = PlayerController->GetMousePositionToWorldRay(OutLocation, OutDirection);
	return bResult;
}

IT4GameObject* FT4ClientGameFrame::GetMousePickingObject(
	const FVector& InLocation,
	const FVector& InDirection,
	FVector& OutHitLocation
) // #111
{
	check(nullptr != GameWorld);
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return nullptr;
	}

	IT4WorldCollisionSystem* CollisionSystem = GameWorld->GetCollisionSystem();
	check(nullptr != CollisionSystem);

	const AActor* IgnoreActor = (PlayerController->HasGameObject()) 
		? PlayerController->GetGameObject()->GetPawn() : nullptr;
	FCollisionQueryParams TraceMouseOverGameObjectParams = FCollisionQueryParams(
		FName(TEXT("MouseOverGameObject")),
		true,
		IgnoreActor
	);
	FT4HitSingleResult HitResult;
	bool bResult = CollisionSystem->QueryLineTraceSingle(
		ET4CollisionChannel::SpawnObject,
		InLocation,
		InDirection,
		T4Const_DefaultLineTraceMaxDistance,
		TraceMouseOverGameObjectParams,
		HitResult
	);
	if (!bResult)
	{
		return nullptr;
	}
	IT4GameObject* TargetObject = HitResult.ResultObject;
	if (nullptr == TargetObject)
	{
		return nullptr;
	}
	OutHitLocation = HitResult.ResultLocation;
	return TargetObject;
}

IT4GameObject* FT4ClientGameFrame::GetMousePickingObject()
{
	check(nullptr != GameWorld);
	IT4WorldContainer* WorldContainer = GameWorld->GetContainer();
	check(nullptr != WorldContainer);
	if (0 >= WorldContainer->GetNumGameObjects())
	{
		return nullptr; // 스폰이 없으니 더이상 진행할 필요가 없다.
	}
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return nullptr;
	}
	FVector WorldLocation;
	FVector WorldDirection;
	FT4HitSingleResult HitResult;
	bool bResult = PlayerController->GetMousePositionToWorldRay(WorldLocation, WorldDirection);
	if (!bResult)
	{
		return nullptr;
	}
	FVector HitLocation;
	IT4GameObject* TargetObject = GetMousePickingObject(WorldLocation, WorldDirection, HitLocation);
	if (nullptr == TargetObject)
	{
		return nullptr;
	}
	return TargetObject;
}

bool FT4ClientGameFrame::GetMousePickingLocation(
	ET4CollisionChannel InCollisionChannel, // #117
	const FVector& InLocation,
	const FVector& InDirection,
	FVector& OutLocation
) // #113
{
	check(nullptr != GameWorld);
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return false;
	}
	IT4WorldCollisionSystem* CollisionSystem = GameWorld->GetCollisionSystem();
	check(nullptr != CollisionSystem);
	const AActor* IgnoreActor = (PlayerController->HasGameObject())
		? PlayerController->GetGameObject()->GetPawn() : nullptr;
	FCollisionQueryParams TracePickingLocationParams = FCollisionQueryParams(
		FName(TEXT("PickingLocation")),
		true,
		IgnoreActor
	);
	TracePickingLocationParams.MobilityType = (ET4CollisionChannel::WorldStatic != InCollisionChannel) 
		? EQueryMobilityType::Any : EQueryMobilityType::Static; // #117 : TODO 정리
	TracePickingLocationParams.bTraceComplex = true;
	TracePickingLocationParams.bReturnPhysicalMaterial = false;
	FT4HitSingleResult HitResult;
	bool bResult = CollisionSystem->QueryLineTraceSingle(
		InCollisionChannel, // ET4CollisionChannel::CollisionVisibility,
		InLocation,
		InDirection,
		T4Const_DefaultLineTraceMaxDistance,
		TracePickingLocationParams,
		HitResult
	);
	if (!bResult)
	{
		return false;
	}
	OutLocation = HitResult.ResultLocation;
	return true;
}

bool FT4ClientGameFrame::GetMousePickingLocation(FVector& OutLocation)
{
	check(nullptr != GameWorld);
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return false;
	}
	FVector WorldRayLocation;
	FVector WorldRayDirection;
	bool bResult = PlayerController->GetMousePositionToWorldRay(WorldRayLocation, WorldRayDirection);
	if (!bResult)
	{
		return false;
	}
	bResult = GetMousePickingLocation(
		ET4CollisionChannel::CollisionVisibility,
		WorldRayLocation, 
		WorldRayDirection, 
		OutLocation
	);
	return bResult;
}

FViewport* FT4ClientGameFrame::GetViewport() const // #68
{
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return nullptr;
	}
	return PlayerController->GetViewport();
}

void FT4ClientGameFrame::ClearOutline() // #115
{
	if (!OutlineTargetObjectID.IsValid())
	{
		return;
	}
	check(nullptr != GameWorld);
	IT4WorldContainer* WorldContainer = GameWorld->GetContainer();
	check(nullptr != WorldContainer);
	IT4GameObject* OldGameObject = WorldContainer->FindGameObject(OutlineTargetObjectID);
	if (nullptr != OldGameObject)
	{
		OldGameObject->SetOutline(false);
	}
	OutlineTargetObjectID.Empty();
}

void FT4ClientGameFrame::SetOutlineTarget(
	const FT4ObjectID& InObjectID, 
	const FLinearColor& InColor
) // #115
{
	if (OutlineTargetObjectID != InObjectID)
	{
		if (OutlineTargetObjectID.IsValid())
		{
			ClearOutline();
		}
	}
	if (!InObjectID.IsValid())
	{
		return;
	}
	check(nullptr != GameWorld);
	IT4WorldContainer* WorldContainer = GameWorld->GetContainer();
	check(nullptr != WorldContainer);
	IT4GameObject* NewGameObject = WorldContainer->FindGameObject(InObjectID);
	if (nullptr == NewGameObject)
	{
		return;
	}
	NewGameObject->SetOutline(true);
	if (OutlineTargetColor != InColor)
	{
		GameWorld->SetMPCGlobalParameterColor(T4EngineConstant::GetMPCGlobalOutlineColorName(), InColor);
		OutlineTargetColor = InColor;
	}
	OutlineTargetObjectID = InObjectID;
}

#if WITH_EDITOR
bool FT4ClientGameFrame::IsPreviewMode() const
{
	return (nullptr != EditorViewportClient) ? EditorViewportClient->IsPreviewMode() : false;
} // #68

void FT4ClientGameFrame::SetGlboalTimeScale(float InTimeScale) // #117
{
	UWorld* UnrealWorld = GameWorld->GetWorld();
	check(nullptr != UnrealWorld);
	UnrealWorld->GetWorldSettings()->TimeDilation = InTimeScale; // TODO : 필요하면 T4Engine 으로 내린다.
	GlboalTimeScale = InTimeScale;
}

float FT4ClientGameFrame::GetGlboalTimeScale() const // #117
{
	return GlboalTimeScale;
}

void FT4ClientGameFrame::SetInputControlLock(bool bLock) // #30
{
	bInputControlLocked = bLock;
	if (nullptr != GameplayInstance)
	{
		GameplayInstance->SetInputControlLock(bInputControlLocked);
	}
}

void FT4ClientGameFrame::SetPlayerChangeDisable(bool bDisable) // #72
{
	bPlayerChangeDisabled = bDisable;
	if (nullptr != GameplayInstance)
	{
		GameplayInstance->SetPlayerChangeDisable(bPlayerChangeDisabled);
	}
}

void FT4ClientGameFrame::SetEditoAISystemPaused(bool bInPaused) // #52
{
	SetT4EditorModeAISystemPaused(bInPaused); // #52
}

void FT4ClientGameFrame::SetEditorPlayerController(AT4PlayerController* InPlayerController)
{
	// #42
	check(!EditorPlayerControllerPtr.IsValid());
	check(nullptr != InPlayerController);

	InPlayerController->SetAsLocalPlayerController();

	// Possess the newly-spawned player.
	InPlayerController->SetRole(ROLE_Authority); // #4.24 : ->Role
	InPlayerController->SetReplicates(false);

	// #30 : 로컬 Player 가 없으면 카메라 회전(APlayerController::UpdateRotation)이 무력화 된다.
	ULocalPlayer* NewPlayer = NewObject<ULocalPlayer>(GEngine, GEngine->LocalPlayerClass);
	if (nullptr == NewPlayer)
	{
		return;
	}
	InPlayerController->SetPlayer(NewPlayer);

	// #86 : ViewTarget 이 없을 경우 카메라 동작 및 Level Streaming 이 동작하지 않음으로
	//       Default Pawn 을 스폰처리해준다. AT4PlayerController 의 CachedDefaultPawn 를 Client 만 사용하다
	//       모두 사용하도록 함께 정리함
	check(nullptr != GameWorld);
	UWorld* UnrealWorld = GameWorld->GetWorld();
	check(nullptr != UnrealWorld);
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save default player pawns into a map
	AT4PlayerDefaultPawn* DefaultPawn = UnrealWorld->SpawnActor<AT4PlayerDefaultPawn>(
		AT4PlayerDefaultPawn::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnInfo
	);
	InPlayerController->SetPawn(DefaultPawn);

	check(nullptr != EditorViewportClient);
	InPlayerController->EditorSetViewportClient(EditorViewportClient); // #30
	InPlayerController->AddToRoot();
	EditorPlayerControllerPtr = InPlayerController;
}

void FT4ClientGameFrame::StartPlayPreview()
{
	check(T4EngineLayer::IsPreview(LayerType));
	check(nullptr != GameWorld);
	UWorld* UnrealWorld = GameWorld->GetWorld();
	check(nullptr != UnrealWorld);

	// #30 : Editor/Preview 의 경우 GameMode(PIE)가 없음 World BeginPlay 를 별도로 호출해준다.
	UnrealWorld->GetWorldSettings()->NotifyBeginPlay();

	// #30 : Editor/Preview 의 경우 GameMode(PIE)가 없음으로 PC Spawn 을 별도로 처리해준다.
	FT4FrameDelegates::OnCreateEditorPlayerController.ExecuteIfBound(this); // #30, #42
}

void FT4ClientGameFrame::StartPlayLevelEditor()
{
	check(T4EngineLayer::IsLevelEditor(LayerType));

	if (!bInitializeDataLoaded)
	{
		InitializeDeferred(); // #17 : 레벨 에디터에서는 컨텐츠 데이터를 최대한 지연해서 로드한다.
	}

	check(nullptr != GameWorld);
	UWorld* UnrealWorld = GameWorld->GetWorld();
	check(nullptr != UnrealWorld);

	// #17 : bActorsInitialized 를 강제로 호출해 PlayerController 가 스폰될 수 있도록 처리
	//       단, 불필요한 레벨 저장이 일어나지는 않는지 체크 필요!
	UnrealWorld->InitializeActorsForPlay(FURL()); // bActorsInitialized check

	// #30 : Editor/Preview 의 경우 GameMode(PIE)가 없음 World BeginPlay 를 별도로 호출해준다.
	UnrealWorld->GetWorldSettings()->NotifyBeginPlay();

	// #30 : Editor/Preview 의 경우 GameMode(PIE)가 없음으로 PC Spawn 을 별도로 처리해준다.
	FT4FrameDelegates::OnCreateEditorPlayerController.ExecuteIfBound(this); // #30, #42
}
#endif