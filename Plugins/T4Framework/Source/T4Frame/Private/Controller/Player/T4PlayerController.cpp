// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Controller/Player/T4PlayerController.h"
#include "Classes/Controller/Player/T4PlayerDefaultPawn.h"
#include "Classes/T4GameHUD.h" // #68

#include "Public/T4Frame.h" // #30

#include "T4Engine/Classes/Camera/T4PlayerCameraManager.h" // #100
#include "T4Engine/Classes/Camera/T4SpringArmComponent.h" // #100
#include "T4Engine/Classes/Camera/T4CameraComponent.h" // #100

#include "T4Engine/Public/T4Engine.h"
#include "T4Engine/Public/T4EngineSettings.h"

#include "Engine/LocalPlayer.h"
#include "Net/UnrealNetwork.h"
#include "Components/ActorComponent.h" // #42

#if WITH_EDITOR
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#endif

#include "T4FrameInternal.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Framework/Controller/PlayerController/index.html
 */
AT4PlayerController::AT4PlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, LayerType(ET4LayerType::Max)
	, bCameraTypeDirty(false) // #40
	, CameraTypeSelected(ET4CameraType::None) // #40
	, bCameraMoveLocked(false)
	, SaveMouseLocation(FVector2D::ZeroVector)
	, CachedCameraSpringTargetArmLength(0.0f)
	, CachedCameraRotation(FRotator::ZeroRotator)
	, CameraFOV(90.0f) // #40
	, CameraZoomSpeed(100.0f) // #40
	, CameraZoomDistanceMin(100.0f) // #40
	, CameraZoomDistanceMax(1000.0f) // #40
	, CameraZoomMaxScale(1.0f) // #86
	, MainWeaponDataIDName(NAME_None) // #48
	, CameraSpringArmComponent(nullptr)
	, CameraComponent(nullptr)
#if WITH_EDITOR
	, EditorViewportClient(nullptr) // #30
#endif
{
	bReplicates = true;
	PlayerCameraManagerClass = AT4PlayerCameraManager::StaticClass();
}

void AT4PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	//InputComponent
}

void AT4PlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	SwitchCameraType(ET4CameraType::TPS); // #40
}

void AT4PlayerController::TickActor(
	float InDeltaTime,
	enum ELevelTick InTickType,
	FActorTickFunction& InThisTickFunction
)
{
	Super::TickActor(InDeltaTime, InTickType, InThisTickFunction);

	NotifyAdvance(InDeltaTime); // #49
}

bool AT4PlayerController::ShouldTickIfViewportsOnly() const
{
	return T4EngineLayer::IsLevelEditor(LayerType); // #17
}

void AT4PlayerController::EndPlay(const EEndPlayReason::Type InEndPlayReason)
{
	check(ET4LayerType::Max != LayerType);

	if (!T4EngineLayer::IsServer(LayerType))
	{
		// #15 : 서버가 아닐 경우만...
		IT4GameWorld* GameWorld = T4EngineWorldGet(LayerType);
		check(nullptr != GameWorld);
		GameWorld->SetPlayerControl(nullptr);
	}

	Super::EndPlay(InEndPlayReason);
}

void AT4PlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(ET4LayerType::Max == LayerType);
	LayerType = T4EngineLayer::Get(GetWorld()); // #12 : Support Multiple LayerType
	check(ET4LayerType::Max != LayerType);

	if (!T4EngineLayer::IsServer(LayerType))
	{
		// #15 : 서버가 아닐 경우만 GameWorld 에 Player 를 설정해준다!
		IT4GameWorld* GameWorld = T4EngineWorldGet(LayerType);
		check(nullptr != GameWorld);
		GameWorld->SetPlayerControl(static_cast<IT4ObjectController*>(this));

		// #86 : ViewTarget 이 없을 경우 카메라 동작 및 Level Streaming 이 동작하지 않음으로 Default Pawn 을 스폰처리해준다.
		//       함께, AT4PlayerController 의 CachedDefaultPawn 를 Client 만 사용하다 모두 사용하도록 함께 정리함
		APawn* DefaultPawn = GetPawn();
		if (nullptr != DefaultPawn)
		{
			// WARN : GameMode 가 있을 경우만 Default Pawn 이 존재한다.
			CachedDefaultPawn = DefaultPawn;
		}

		if (T4EngineLayer::IsClient(LayerType))
		{
			// #68 : Clinet 만 GameHUD 가 아니라면 재생성 해준다. T4GameHUD 를 상속받도록 가이드 필요
			AT4GameHUD* GameHUD = Cast<AT4GameHUD>(GetHUD());
			if (nullptr == GameHUD)
			{
				// #68 : HandleOnHUDPostRender
				ClientSetHUD(AT4GameHUD::StaticClass());
			}
		}
	}
}

void AT4PlayerController::OnPossess(APawn* InPawn)
{
	if (!HasAuthority())
	{
		// #15 : Authority 가 아니면 Possess 를 호출하면 warning 을 뱉고 실행이 안된다.
		//       즉, 이경우 OtherWin 에서 카메라 조정을 할 수 없게 됨으로 별도 처리를 해준다.
		//       TODO : ClientRetryClientRestart 를 호출하는 것이 맞는지 확인 필요...
		check(ET4LayerType::ServerMax < LayerType);
		ClientRetryClientRestart(InPawn);
		return;
	}
/*
#if WITH_EDITOR
	// #16 : ListenServer 사용시 ClientStart 에서 TearOff 를 체크해 Pawn = nullptr 로 만들어버린다. (컨트롤 불가)
	//       T4Framework 에서는 PlayerController 관련 액터 말고는 모두 tearoff = on 여서 강제로 Replication 을 켜주고
	//       Possess 후 다시 돌려 우회한다. (변수에 public 접근은 deprecated 예정...)
	bool bIsTearOff = InPawn->bTearOff;
	if (GetNetMode() == ENetMode::NM_ListenServer && bIsTearOff)
	{
		InPawn->bTearOff = false;
	}
#endif
*/

	Super::OnPossess(InPawn);

/*
#if WITH_EDITOR
	if (GetNetMode() == ENetMode::NM_ListenServer && bIsTearOff)
	{
		InPawn->bTearOff = bIsTearOff;
	}
#endif
*/
}

void AT4PlayerController::OnUnPossess()
{
	if (!HasAuthority())
	{
		// #15 : Authority 가 아니면 Possess 를 호출하면 warning 을 뱉고 실행이 안된다.
		//       즉, 이경우 OtherWin 에서 카메라 조정을 할 수 없게 됨으로 별도 처리를 해준다.
		//       TODO : ClientRetryClientRestart 를 호출하는 것이 맞는지 확인 필요...
		check(ET4LayerType::ServerMax < LayerType);
		ClientRetryClientRestart(nullptr);
		return;
	}

	bool bAuthorityCached = HasAuthority();
	if (bAuthorityCached)
	{
		// #15 : OnUnPossess 에서 Role 이 Authority 면 bReplicates 를 강제로 켜버린다.
		//       T4Framework 에서는 Pawn 은 모두 bReplicates = false 가 유지되어야 함으로 임시로 Role 을 변경
		//       bReplicates 가 켜지는 것을 방지한다.
		SwapRoles();
	}

	Super::OnUnPossess();

	if (bAuthorityCached)
	{
		// #15 : Role 복구!
		SwapRoles();
	}
}

UInputComponent* AT4PlayerController::NewInputComponent()
{
	return NewObject<UInputComponent>(this);
}

void AT4PlayerController::SetInputComponent(UInputComponent* InInputComponent)
{
	Super::PushInputComponent(InInputComponent);
}

void AT4PlayerController::OnSetInputMode(ET4InputMode InMode)
{
#if WITH_EDITOR
	if (T4EngineLayer::IsToolSide(LayerType))
	{
		return; // WARN : #29, #17 : Only Editor LayerType
	}
#endif
	if (GameAndUI == InMode)
	{
		FInputModeGameAndUI InputMode;
		Super::SetInputMode(InputMode);
	}
	else if (GameOnly == InMode)
	{
		FInputModeGameOnly InputMode;
		Super::SetInputMode(InputMode);
	}
}

APawn* AT4PlayerController::GetDefaultPawn() const // #86
{
	if (HasGameObject())
	{
		return nullptr;
	}
	return GetPawn();
}

bool AT4PlayerController::SetGameObject(const FT4ObjectID& InNewTargetID)
{
	if (!InNewTargetID.IsValid())
	{
		return false;
	}
	if (ObserverObjectID.IsValid())
	{
		ClearObserverObject();
	}
	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	IT4GameObject* NewTargetObject = GameWorld->GetContainer()->FindGameObject(InNewTargetID);
	if (nullptr == NewTargetObject)
	{
		UE_LOG(
			LogT4Frame,
			Error,
			TEXT("[SL:%u] PC::SetGameObject '%s' failed. object not found."),
			uint32(LayerType),
			*(InNewTargetID.ToString())
		);
		return false;
	}
	APawn* NewTargetPawn = NewTargetObject->GetPawn();
	if (nullptr == NewTargetPawn)
	{
		UE_LOG(
			LogT4Frame,
			Error,
			TEXT("[SL:%u] PC::SetGameObject '%s' failed. Pawn is null."),
			uint32(LayerType),
			*(InNewTargetID.ToString())
		);
		return false;
	}
	ClearGameObject(false);
	if (!T4EngineLayer::IsServer(LayerType))
	{
		// #15 : 서버가 아닐 경우만
		OnPossess(NewTargetPawn); 
		AttachCameraComponent(NewTargetPawn);
	}
	GameObjectID = InNewTargetID;
	// #34 : for Server All or Client Only Player
	check(nullptr != NewTargetObject);
	NewTargetObject->SetObjectController(this);
	NotifyPossess(NewTargetObject); // #49
#if WITH_EDITOR
	GetOnViewTargetChanged().Broadcast(NewTargetObject); // #39
#endif
	return true;
}

void AT4PlayerController::ClearGameObject(bool bInSetDefaultPawn)
{
	if (!GameObjectID.IsValid())
	{
		return;
	}
	if (!T4EngineLayer::IsServer(LayerType))
	{
		// #15 : 서버가 아닐 경우만
		DetachCameraComponent(); 
		OnUnPossess();
		if (bInSetDefaultPawn)
		{
			// #86 : ViewTarget 이 없을 경우 카메라 동작 및 Level Streaming 이 동작하지 않음으로 Default Pawn 을 스폰처리해준다.
			//       함께, AT4PlayerController 의 CachedDefaultPawn 를 Client 만 사용하다 모두 사용하도록 함께 정리함
			if (CachedDefaultPawn.IsValid())
			{
				OnPossess(CachedDefaultPawn.Get());
			}
		}
	}
	// #34 : for Server All or Client Only Player
	IT4GameObject* OldTargetObject = GetGameObject();
	if (nullptr != OldTargetObject)
	{
		OldTargetObject->SetObjectController(nullptr);
	}
	GameObjectID.SetNone();
	NotifyUnPossess(OldTargetObject); // #49
#if WITH_EDITOR
	GetOnViewTargetChanged().Broadcast(nullptr); // #39
#endif
}

IT4GameObject* AT4PlayerController::GetGameObject() const
{
	if (!GameObjectID.IsValid())
	{
		return nullptr;
	}
	return FindGameObject(GameObjectID);
}

bool AT4PlayerController::SetObserverObject(
	const FT4ObjectID& InNewObserverID
) // #52
{
	if (T4EngineLayer::IsServer(LayerType))
	{
		ensure(false);
		return false; // WARN : 서버는 동작하지 않는 옵션!
	}
	if (!InNewObserverID.IsValid())
	{
		return false;
	}
	if (InNewObserverID == GameObjectID || InNewObserverID == ObserverObjectID)
	{
		ClearObserverObject();
		return true;
	}
	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	IT4GameObject* NewObserverObject = GameWorld->GetContainer()->FindGameObject(InNewObserverID);
	if (nullptr == NewObserverObject)
	{
		UE_LOG(
			LogT4Frame,
			Error,
			TEXT("[SL:%u] SetObserverObject '%s' failed. object not found."),
			uint32(LayerType),
			*(InNewObserverID.ToString())
		);
		return false;
	}
	APawn* ObserverPawn = NewObserverObject->GetPawn();
	if (nullptr == ObserverPawn)
	{
		UE_LOG(
			LogT4Frame,
			Error,
			TEXT("[SL:%u] SetObserverObject '%s' failed. Pawn is null."),
			uint32(LayerType),
			*(InNewObserverID.ToString())
		);
		return false;
	}
	ClearObserverObject();
	{
		ObserverPawn->PossessedBy(this);
		ObserverPawn->SetReplicates(false); // #52 : check 필요. 강제로 켜버려서 다시 끈다. 툴 기능을 감안하자.
		// TODO : Tab 으로 카메라를 변경하면 회전이 되지 않는 문제가 있다 확인 필요!!
	}
	AttachToPawn(ObserverPawn);
	AddPawnTickDependency(ObserverPawn);
	AutoManageActiveCameraTarget(ObserverPawn);
	AttachCameraComponent(ObserverPawn);
	ObserverObjectID = InNewObserverID;
	return true;
}

void AT4PlayerController::ClearObserverObject() // #52
{
	if (T4EngineLayer::IsServer(LayerType))
	{
		ensure(false);
		return; // WARN : 서버는 동작하지 않는 옵션!
	}
	if (!ObserverObjectID.IsValid())
	{
		return;
	}
	DetachCameraComponent();
	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	IT4GameObject* OldObserverObject = GameWorld->GetContainer()->FindGameObject(ObserverObjectID);
	if (nullptr != OldObserverObject)
	{
		APawn* ObserverPawn = OldObserverObject->GetPawn();
		check(nullptr != ObserverPawn);
		ObserverPawn->UnPossessed();
	}
	APawn* const MyPawn = GetPawn();
	AttachToPawn(MyPawn);
	AddPawnTickDependency(MyPawn);
	AutoManageActiveCameraTarget(this);
	ObserverObjectID.SetNone();
}

IT4GameWorld* AT4PlayerController::GetGameWorld() const // #52
{
	return T4EngineWorldGet(LayerType);
}

bool AT4PlayerController::HasPublicAction(const FT4ActionKey& InActionKey) const
{
	// #20
	IT4GameObject* ViewTargetObject = GetGameObject();
	if (nullptr == ViewTargetObject)
	{
		return false;
	}
	IT4ActionControl* ViewTargetActionRoot = ViewTargetObject->GetActionControl();
	if (nullptr == ViewTargetActionRoot)
	{
		return false;
	}
	return ViewTargetActionRoot->HasAction(InActionKey);
}

bool AT4PlayerController::IsPlayingPublicAction(const FT4ActionKey& InActionKey) const
{
	// #20
	IT4GameObject* ViewTargetObject = GetGameObject();
	if (nullptr == ViewTargetObject)
	{
		return false;
	}
	IT4ActionControl* ViewTargetActionRoot = ViewTargetObject->GetActionControl();
	if (nullptr == ViewTargetActionRoot)
	{
		return false;
	}
	return ViewTargetActionRoot->IsPlaying(InActionKey);
}

AController* AT4PlayerController::GetAController()
{
	return Cast<AController>(this);
}

APlayerCameraManager* AT4PlayerController::GetCameraManager() const // #100
{
	return PlayerCameraManager;
}

FViewport* AT4PlayerController::GetViewport() const // #68
{
#if WITH_EDITOR
	if (T4EngineLayer::IsToolSide(LayerType))
	{
		check(nullptr != EditorViewportClient);
		return EditorViewportClient->GetViewport();
	}
#endif
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer)
	{
		UGameViewportClient* ViewportClient = LocalPlayer->ViewportClient;
		if (ViewportClient)
		{
			return ViewportClient->Viewport;
		}
	}
	return nullptr;
}

FRotator AT4PlayerController::GetViewControlRotation() const
{
	return GetControlRotation();
}

FVector AT4PlayerController::GetCameraLocation() const
{
	if (T4EngineLayer::IsServer(LayerType))
	{
		ensure(false);
		return FVector::ZeroVector; // WARN : 서버는 동작하지 않는 기능
	}
	if (nullptr == PlayerCameraManager)
	{
		return FVector::ZeroVector;
	}
	return PlayerCameraManager->GetCameraLocation();
}

FRotator AT4PlayerController::GetCameraRotation() const
{
	if (T4EngineLayer::IsServer(LayerType))
	{
		ensure(false);
		return FRotator::ZeroRotator; // WARN : 서버는 동작하지 않는 기능
	}
	if (nullptr == PlayerCameraManager)
	{
		return FRotator::ZeroRotator;
	}
	return PlayerCameraManager->GetCameraRotation();
}

FVector AT4PlayerController::GetCameraLookAtLocation() const // #30
{
	if (T4EngineLayer::IsServer(LayerType))
	{
		ensure(false);
		return FVector::ZeroVector; // WARN : 서버는 동작하지 않는 기능
	}
	if (nullptr == CameraSpringArmComponent)
	{
		return FVector::ZeroVector;
	}
	// #40 : ShoulderView 의 경우 SocketOffset 이 있어 TargetView 를 사용하지 않고, 
	//       CameraSpringArmComponent 에서 CameraType 으로 At Location 을 달리 사용한다.
	FVector CameraAtLocation = CameraSpringArmComponent->GetComponentLocation();
	if (ET4CameraType::ShoulderView == ShoulderView)
	{
		const FRotator CameraLocation = GetCameraRotation();
		CameraAtLocation += CameraLocation.RotateVector(CameraSpringArmComponent->SocketOffset);
	}
	return CameraAtLocation;
}

void AT4PlayerController::SwitchCameraType(ET4CameraType InCameraType)
{
	// #40
	if (T4EngineLayer::IsServer(LayerType))
	{
		ensure(false);
		return; // WARN : 서버는 동작하지 않는 기능
	}
	if (CameraTypeSelected == InCameraType)
	{
		return;
	}
	DetachCameraComponent();
	{
		CameraTypeSelected = InCameraType;
		bCameraTypeDirty = true;
	}
	APawn* ViewTargetPawn = GetTargetPawnSelected();
	if (nullptr == ViewTargetPawn)
	{
		UE_LOG(
			LogT4Frame,
			Verbose,
			TEXT("[SL:%u] SwitchCameraType failed. Pawn is null."),
			uint32(LayerType)
		);
		return;
	}
	AttachCameraComponent(ViewTargetPawn);
}

void AT4PlayerController::SetCameraZoom(float InAmount)
{
	if (T4EngineLayer::IsServer(LayerType))
	{
		ensure(false);
		return; // WARN : 서버는 동작하지 않는 기능
	}
	if (nullptr == CameraSpringArmComponent)
	{
		return;
	}
	float CurrentTargetArmLength = CameraSpringArmComponent->TargetArmLength;
	CurrentTargetArmLength = FMath::Clamp(
		CurrentTargetArmLength - (CameraZoomSpeed * InAmount),
		CameraZoomDistanceMin,
		CameraZoomDistanceMax * CameraZoomMaxScale // #86
	);
	CameraSpringArmComponent->TargetArmLength = CurrentTargetArmLength;
}

void AT4PlayerController::SetCameraPitch(float InAmount)
{
	if (T4EngineLayer::IsServer(LayerType))
	{
		ensure(false);
		return; // WARN : 서버는 동작하지 않는 기능
	}
	AddPitchInput(InAmount);
	if (bCameraMoveLocked)
	{
		T4SetMouseLocation(SaveMouseLocation.X, SaveMouseLocation.Y);
	}
}

void AT4PlayerController::SetCameraYaw(float InAmount)
{
	if (T4EngineLayer::IsServer(LayerType))
	{
		ensure(false);
		return; // WARN : 서버는 동작하지 않는 기능
	}
	AddYawInput(InAmount);
	if (bCameraMoveLocked)
	{
		T4SetMouseLocation(SaveMouseLocation.X, SaveMouseLocation.Y);
	}
}

void AT4PlayerController::SetFreeCameraMoveDirection(const FVector& InDirection)
{
	if (T4EngineLayer::IsServer(LayerType))
	{
		ensure(false);
		return; // WARN : 서버는 동작하지 않는 기능
	}
	if (HasGameObject())
	{
		return;
	}
	APawn* DefaultPawn = GetPawn();
	if (nullptr != DefaultPawn)
	{
		DefaultPawn->AddMovementInput(InDirection);
	}
}

void AT4PlayerController::SetFreeCameraLocationAndRotation(
	const FVector& InLocation,
	const FRotator& InRotation
) // #94, #86
{
	if (T4EngineLayer::IsServer(LayerType))
	{
		ensure(false);
		return; // WARN : 서버는 동작하지 않는 기능
	}
	if (HasGameObject())
	{
		return;
	}
	APawn* DefaultPawn = GetPawn();
	if (nullptr != DefaultPawn)
	{
		DefaultPawn->SetActorLocationAndRotation(InLocation, InRotation);
	}
}

void AT4PlayerController::SetCameraLock(bool bInLock)
{
	if (T4EngineLayer::IsServer(LayerType))
	{
		ensure(false);
		return; // WARN : 서버는 동작하지 않는 기능
	}
	bCameraMoveLocked = bInLock;
	if (bInLock)
	{
		T4GetMousePosition(SaveMouseLocation.X, SaveMouseLocation.Y);
	}
	else
	{
		T4SetMouseLocation(SaveMouseLocation.X, SaveMouseLocation.Y);
		SaveMouseLocation = FVector2D::ZeroVector;
	}
}

void AT4PlayerController::GetCameraInfoCached(
	FRotator& OutRotation,
	float& OutDistance
) // #87
{
	OutRotation = GetControlRotation();
	OutDistance = (nullptr != CameraSpringArmComponent) ? CameraSpringArmComponent->TargetArmLength : 500.0f;
}

void AT4PlayerController::SetCameraInfoCached(
	const FRotator& InRotation,
	const float& InDistance
) // #87
{
	SetControlRotation(InRotation);
	if (nullptr != CameraSpringArmComponent)
	{
		CameraSpringArmComponent->TargetArmLength = InDistance;
	}
	CachedCameraRotation = InRotation;
	CachedCameraSpringTargetArmLength = InDistance;
}

bool AT4PlayerController::GetMousePositionToWorldRay(
	FVector& OutStartPosition,
	FVector& OutStartDirection
)
{
#if WITH_EDITOR 
	if (T4EngineLayer::IsToolSide(LayerType))
	{
		// #30
		check(nullptr != EditorViewportClient);
		return EditorViewportClient->GetMousePositionToWorldRay(OutStartPosition, OutStartDirection);
	}
#endif
	DeprojectMousePositionToWorld(OutStartPosition, OutStartDirection);
	return true;
}

void AT4PlayerController::SetMouseCursorType(EMouseCursor::Type InMouseCursorType)
{
	CurrentMouseCursor = InMouseCursorType;
	// #30
#if WITH_EDITOR
	if (T4EngineLayer::IsToolSide(LayerType))
	{
		check(nullptr != EditorViewportClient);
		EditorViewportClient->SetMouseCursorType(InMouseCursorType);
	}
#endif
}

void AT4PlayerController::ShowMouseCursor(bool InShow)
{
	bShowMouseCursor = InShow;
	// #30
#if WITH_EDITOR
	if (T4EngineLayer::IsToolSide(LayerType))
	{
		check(nullptr != EditorViewportClient);
		EditorViewportClient->ShowMouseCursor(InShow);
	}
#endif
}

#if WITH_EDITOR
bool AT4PlayerController::EditorInputKey(
	FKey InKey,
	EInputEvent InEvent,
	float InAmountDepressed,
	bool bInGamepad
)
{
	return InputKey(InKey, InEvent, InAmountDepressed, bInGamepad); // #30
}

bool AT4PlayerController::EditorInputAxis(
	FKey InKey, 
	float InDelta, 
	float InDeltaTime, 
	int32 InNumSamples, 
	bool bInGamepad
)
{
	return InputAxis(InKey, InDelta, InDeltaTime, InNumSamples, bInGamepad); // #30
}
#endif

IT4GameObject* AT4PlayerController::FindGameObject(const FT4ObjectID& InObjectID) const
{
	check(ET4LayerType::Max > LayerType);
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	return GameWorld->GetContainer()->FindGameObject(InObjectID);
}

#if WITH_EDITOR
IT4EditorGameplayHandler* AT4PlayerController::GetEditorGameplayCustomHandler() const // #60
{
	check(ET4LayerType::Max > LayerType);
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	IT4EditorGameplayHandler* EditorGameplayHandler = GameFrame->GetEditorGameplayCustomHandler();
	return EditorGameplayHandler;
}
#endif

void AT4PlayerController::AttachCameraComponent(
	APawn* InOuter
)
{
	if (T4EngineLayer::IsServer(LayerType))
	{
		ensure(false);
		return; // WARN : 서버는 동작하지 않는 기능
	}
	check(nullptr == CameraSpringArmComponent);
	check(nullptr == CameraComponent);

	USceneComponent* PawnParentComponent = InOuter->GetRootComponent();
	check(nullptr != PawnParentComponent);

	CameraSpringArmComponent = NewObject<UT4SpringArmComponent>(InOuter);
	CameraComponent = NewObject<UT4CameraComponent>(InOuter);

	CameraSpringArmComponent->AttachToComponent(
		PawnParentComponent,
		FAttachmentTransformRules::SnapToTargetIncludingScale
	);
	CameraSpringArmComponent->RegisterComponent();

	CameraComponent->AttachToComponent(
		CameraSpringArmComponent, 
		FAttachmentTransformRules::SnapToTargetIncludingScale
	);
	CameraComponent->RegisterComponent();

	float ApplyTargetArmLength = 0.0f;
	FRotator ApplyCameraRotation = FRotator::ZeroRotator;
	bool bUsePawnControlRotation = true;

	// #40
	const UT4EngineSettings* EngineSettings = GetDefault<UT4EngineSettings>();
	check(nullptr != EngineSettings);

	switch (CameraTypeSelected)
	{
		case ET4CameraType::TPS:
			{
				CameraFOV = EngineSettings->TPS_FieldOfViewDegree;
				CameraComponent->SetFieldOfView(CameraFOV);
				CameraZoomDistanceMin = EngineSettings->TPS_MinZoomDistance;
				CameraZoomDistanceMax = EngineSettings->TPS_MaxZoomDistance;
				ApplyCameraRotation = EngineSettings->TPS_DefaultRotation;
				ApplyTargetArmLength = EngineSettings->TPS_DefaultZoomDistance;
			}
			break;

		case ET4CameraType::ShoulderView:
			{
				CameraFOV = EngineSettings->SV_FieldOfViewDegree;
				CameraComponent->SetFieldOfView(CameraFOV);
				CameraSpringArmComponent->SocketOffset = EngineSettings->SV_SocketOffset;
				CameraZoomDistanceMin = EngineSettings->SV_MinZoomDistance;
				CameraZoomDistanceMax = EngineSettings->SV_MaxZoomDistance;
				ApplyCameraRotation = FRotator::ZeroRotator;
				ApplyTargetArmLength = EngineSettings->SV_DefaultZoomDistance;
			}
			break;

		default:
			{
				UE_LOG(
					LogT4Frame,
					Error,
					TEXT("[SL:%u] SwitchCameraType '%u' failed. no implementation."),
					uint32(LayerType),
					uint32(CameraTypeSelected)
				);
			}
			break;
	};

	if (bCameraTypeDirty)
	{
		CameraSpringArmComponent->TargetArmLength = ApplyTargetArmLength;
		ApplyCameraRotation = ApplyCameraRotation;
		bCameraTypeDirty = false; // #40
	}
	else
	{
		CameraSpringArmComponent->TargetArmLength = CachedCameraSpringTargetArmLength;
		ApplyCameraRotation = CachedCameraRotation;
	}
	
	CameraSpringArmComponent->BackupSocketOffset = CameraSpringArmComponent->SocketOffset; // #58

	CameraSpringArmComponent->bEnableCameraLag = true;
	CameraSpringArmComponent->CameraLagSpeed = EngineSettings->CameraLagSpeed;
	CameraZoomSpeed = EngineSettings->CameraZoomSpeed;

	CameraSpringArmComponent->bUsePawnControlRotation = HasObserverObject() ? false : bUsePawnControlRotation; // #40 : Pawn 회전 사용

	CameraSpringArmComponent->SetRelativeLocation(FVector::ZeroVector);
	CameraSpringArmComponent->SetRelativeRotation(FRotator::ZeroRotator);

	SetControlRotation(ApplyCameraRotation);
}

void AT4PlayerController::DetachCameraComponent()
{
	if (T4EngineLayer::IsServer(LayerType))
	{
		ensure(false);
		return; // WARN : 서버는 동작하지 않는 기능
	}
	if (nullptr == CameraSpringArmComponent)
	{
		return;
	}
	check(nullptr != CameraComponent);

	CachedCameraSpringTargetArmLength = CameraSpringArmComponent->TargetArmLength;
	CachedCameraRotation = GetControlRotation();
		
	CameraComponent->UnregisterComponent();
	CameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	CameraComponent->DestroyComponent();

	CameraSpringArmComponent->UnregisterComponent();
	CameraSpringArmComponent->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	CameraSpringArmComponent->DestroyComponent();

	CameraComponent = nullptr;
	CameraSpringArmComponent = nullptr;
}

void AT4PlayerController::T4SetMouseLocation(const int InX, const int InY)
{
	// #30
#if WITH_EDITOR
	if (T4EngineLayer::IsToolSide(LayerType))
	{
		check(nullptr != EditorViewportClient);
		EditorViewportClient->SetMouseLocation(InX, InY);
		return;
	}
#endif
	SetMouseLocation(InX, InY);
}

bool AT4PlayerController::T4GetMousePosition(
	float& InLocationX,
	float& InLocationY
) const
{
	// #30
#if WITH_EDITOR
	if (T4EngineLayer::IsToolSide(LayerType))
	{
		check(nullptr != EditorViewportClient);
		return EditorViewportClient->GetMousePosition(InLocationX, InLocationY);
	}
#endif
	return GetMousePosition(InLocationX, InLocationY);
}

APawn* AT4PlayerController::GetTargetPawnSelected() // #52
{
	if (T4EngineLayer::IsServer(LayerType))
	{
		ensure(false);
		return nullptr; // WARN : 서버는 동작하지 않는 기능
	}
	if (HasGameObject())
	{
		IT4GameObject* GameObject = GetGameObject();
		if (nullptr != GameObject)
		{
			return GameObject->GetPawn();
		}
	}
	if (HasObserverObject())
	{
		IT4GameObject* ObserverObject = FindGameObject(ObserverObjectID);
		if (nullptr != ObserverObject)
		{
			return ObserverObject->GetPawn();
		}
	}
	return nullptr;
}
