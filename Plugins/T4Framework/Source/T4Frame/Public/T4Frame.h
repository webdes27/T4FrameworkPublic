// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4FrameTypes.h"
#include "T4FrameGameTypes.h" // #104
#include "T4FrameStructs.h"

#if WITH_EDITOR
#include "T4FrameEditorSupport.h" // #60
#endif

#include "T4Engine/Public/T4EngineTypes.h"
#include "T4Engine/Public/Action/T4ActionKey.h"
#include "T4Engine/Public/T4Engine.h" // #63 : IT4AController

#include "Engine/EngineBaseTypes.h"
#include "InputCoreTypes.h"
#include "GenericPlatform/ICursor.h"

#if WITH_EDITOR
#include "ICursor.h"
#endif

/**
  *
 */
class APawn; // #86
class UWorld;
class FCanvas;
class FViewport;
struct FWorldContext;
class AController;
class AAIController;
class UInputComponent;
class IT4WorldActor;
class IT4GameWorld;
class AT4PlayerController;
class IT4EditorViewportClient;
class UT4MapEntityAsset; // #87

// #30
enum ET4FrameType
{
	Frame_Client,
	Frame_Server,

	Frame_None
};

class IT4GameFrame;
class IT4WorldActor;

#if WITH_EDITOR
DECLARE_MULTICAST_DELEGATE_OneParam(FT4OnViewTargetChanged, IT4WorldActor*);
#endif

// #114
class T4FRAME_API IT4GameObject
{
public:
	virtual ~IT4GameObject() {}

	virtual ET4LayerType GetLayerType() const = 0;
	virtual const FT4ObjectID& GetObjectID() const = 0; // #114
};

class AAIController;
class T4FRAME_API IT4GameAIController : public IT4ActorController
{
public:
	virtual ~IT4GameAIController() {}

	virtual AAIController* GetAIController() = 0; // #104

	virtual ET4GameTribeType GetTribeType() const = 0; // #104 : TODO M5
	virtual ET4GameEnemyType GetEnemyType() const = 0; // #104 : TODO M5
};

class T4FRAME_API IT4PlayerController : public IT4ActorController
{
public:
	virtual ~IT4PlayerController() {}

	// #15 : Editor 환경에서 HasAuthority 를 명시적으로 구분하기 위해 도입
	//       동일 프로세스에서 Player 의 Role 이 바뀌며 C/S Player 로 바뀌기 때문에 혼란스러운 점이 있기 때문
	virtual bool CheckAuthority() const = 0; // return HasAuthority()

	virtual UInputComponent* NewInputComponent() = 0;
	virtual void SetInputComponent(UInputComponent* InInputComponent) = 0;
	virtual void OnSetInputMode(ET4InputMode InMode) = 0;

	virtual FViewport* GetViewport() const = 0; // #68

	virtual APawn* GetDefaultPawn() const = 0; // #86

	virtual FRotator GetViewControlRotation() const = 0;

	virtual ET4CameraType GetCameraType() const = 0;
	virtual float GetCameraFOV() const = 0; // #40
	virtual FVector GetCameraLocation() const = 0;
	virtual FRotator GetCameraRotation() const = 0;
	virtual FVector GetCameraLookAtLocation() const = 0; // #30

	virtual void SwitchCameraType(ET4CameraType InCameraType) = 0; // #40
	virtual void SetCameraZoom(float InAmount) = 0;
	virtual void SetCameraZoomMaxScale(float InScale) = 0; // #86
	virtual void SetCameraPitch(float InAmount) = 0;
	virtual void SetCameraYaw(float InAmount) = 0;

	virtual void SetFreeCameraMoveDirection(const FVector& InDirection) = 0;
	virtual void SetFreeCameraLocationAndRotation(const FVector& InLocation, const FRotator& InRotation) = 0; // #94, #86

	virtual void GetCameraInfoCached(FRotator& OutRotation, float& OutDistance) = 0; // #87
	virtual void SetCameraInfoCached(const FRotator& InRotation, const float& InDistance) = 0; // #87

	virtual bool GetMousePositionToWorldRay(FVector& OutStartPosition, FVector& OutStartDirection) = 0;

	virtual void SetMouseCursorLock(bool bInLock) = 0;
	virtual bool IsMouseCursorLocked() const = 0;

	virtual void SetMouseCursorType(EMouseCursor::Type InCursorType) = 0;
	virtual void ShowMouseCursor(bool InShow) = 0;

	virtual void SetMouseCursorPosition(const FVector2D& InPosition) = 0; // #30, #113
	virtual bool GetMouseCursorPosition(FVector2D& OutPosition) const = 0; // #30, #113

#if WITH_EDITOR
	virtual bool EditorInputKey(FKey InKey, EInputEvent InEvent, float InAmountDepressed, bool bInGamepad) = 0; // #30
	virtual bool EditorInputAxis(FKey InKey, float InDelta, float InDeltaTime, int32 InNumSamples, bool bInGamepad) = 0; // #30

	virtual void EditorSetViewportClient(IT4EditorViewportClient* InEditorViewportClient) = 0;

	virtual FT4OnViewTargetChanged& GetOnViewTargetChanged() = 0;
#endif
};

// #42
class T4FRAME_API IT4GameplayInstance
{
public:
	virtual ~IT4GameplayInstance() {}

	virtual bool OnInitialize(ET4LayerType InLayerType) = 0;
	virtual void OnFinalize() = 0;

	virtual void OnReset() = 0;
	virtual void OnStartPlay() = 0;
	virtual void OnPlayerSpawned(IT4PlayerController* InOwnerPC) = 0;

	virtual void OnProcess(float InDeltaTime) = 0;
	virtual void OnDrawHUD(FViewport* InViewport, FCanvas* InCanvas, FT4HUDDrawInfo& InOutDrawInfo) = 0; // #68 : Only Client

#if WITH_EDITOR
	virtual IT4EditorGameDatabase* GetEditorGameDatabase() = 0; // #60
	virtual IT4EditorGameplayCommand* GetEditorGameplayCommand() = 0; // #114

	virtual void SetInputControlLock(bool bLock) = 0; // #30
	virtual void SetPlayerChangeDisable(bool bDisable) = 0; // #72
#endif
};

class UT4GameObject;
class T4FRAME_API IT4GameFrame
{
public:
	virtual ~IT4GameFrame() {}

	virtual ET4LayerType GetLayerType() const = 0;
	virtual ET4FrameType GetType() const = 0;

	virtual void OnReset() = 0;
	virtual void OnStartPlay() = 0;

	virtual void OnProcessPre(float InDeltaTime) = 0; // #34 : OnWorldPreActorTick
	virtual void OnProcessPost(float InDeltaTime) = 0; // #34 : OnWorldPostActorTick

	virtual void OnDrawHUD(FViewport* InViewport, FCanvas* InCanvas, FT4HUDDrawInfo& InOutDrawInfo) = 0; // #68 : Only Client

	virtual bool HasBegunPlay() const = 0;

	virtual UWorld* GetWorld() const = 0;
	virtual IT4GameWorld* GetGameWorld() const = 0;

	virtual void RegisterGameplayInstance(IT4GameplayInstance* InLayerInstance) = 0; // #42
	virtual IT4GameplayInstance* GetGameplayInstance() const = 0; // #42

	virtual bool OnWorldTravel(const UT4MapEntityAsset* InMapEntityAsset) = 0; // #87

	// Client
	virtual UT4GameObject* GetPlayerClientObject() const = 0; // #114 : Only Client
	virtual IT4PlayerController* GetPlayerController() const = 0;

	virtual bool GetMousePositionToWorldRay(FVector& OutLocation, FVector& OutDirection) = 0; // #113

	virtual IT4WorldActor* GetMousePickingActor() = 0;
	virtual IT4WorldActor* GetMousePickingActor(const FVector& InLocation, const FVector& InDirection, FVector& OutHitLocation) = 0; // #111

	virtual bool GetMousePickingLocation(FVector& OutLocation) = 0;
	virtual bool GetMousePickingLocation(ET4CollisionChannel InChannel, const FVector& InLocation, const FVector& InDirection, FVector& OutLocation) = 0; // #117

	virtual FViewport* GetViewport() const = 0; // #68

	virtual void ClearOutline() = 0; // #115
	virtual void SetOutlineTarget(const FT4ActorID& InActorID, const FLinearColor& InColor) = 0; // #115

	virtual bool AddClientGameObject(const FT4ObjectID& InObjectID, UT4GameObject* InGameObject) = 0; // #114
	virtual void RemoveClientGameObject(const FT4ObjectID& InObjectID) = 0; // #114
	virtual void RemoveAllClientGameObjects() = 0; // #114
	virtual UT4GameObject* GetClientGameObject(const FT4ObjectID& InObjectID) const = 0; // #114

#if WITH_EDITOR
	virtual bool IsPreviewMode() const = 0; // #68

	virtual void SetGlboalTimeScale(float InTimeScale) = 0; // #117
	virtual float GetGlboalTimeScale() const = 0; // #117

	virtual void SetInputControlLock(bool bLock) = 0; // #30
	virtual void SetPlayerChangeDisable(bool bDisable) = 0; // #72
	virtual void SetEditoAISystemPaused(bool bInPaused) = 0; // #52

	virtual IT4EditorGameplayContoller* GetEditorGameplayController() const = 0; // #60
	virtual void SetEditorGameplayContoller(IT4EditorGameplayContoller* bInGameplayHandler) = 0; // #60

	virtual AT4PlayerController* GetEditorPlayerController() const = 0; // #79
	virtual void SetEditorPlayerController(AT4PlayerController* InPlayerController) = 0; // #42

	virtual IT4EditorViewportClient* GetEditorViewportClient() const = 0; // #79
	virtual void SetEditorViewportClient(IT4EditorViewportClient* InViewportClient) = 0; // #30
#endif

#if (WITH_EDITOR || WITH_SERVER_CODE)
	// Server
	virtual FT4ObjectID GenerateObjectIDForServer() = 0; // #41

	virtual bool RegisterGameAIController(const FT4ObjectID& InObjectID, IT4GameAIController* InAIController) = 0; // #31
	virtual void UnregisterGameAIController(const FT4ObjectID& InObjectID) = 0; // #31

	virtual IT4GameAIController* FindGameAIController(const FT4ObjectID& InObjectID) const = 0; // #31

	virtual bool AddServerGameObject(const FT4ObjectID& InObjectID, UT4GameObject* InGameObject) = 0; // #114
	virtual void RemoveServerGameObject(const FT4ObjectID& InObjectID) = 0; // #114
	virtual void RemoveAllServerGameObjects() = 0; // #114
	virtual UT4GameObject* GetServerGameObject(const FT4ObjectID& InObjectID) const = 0; // #114
#endif
};

T4FRAME_API IT4GameFrame* T4FrameCreate(
	ET4FrameType InFrameType,
	const FT4WorldConstructionValues& InWorldConstructionValues // #87
);
T4FRAME_API void T4FrameDestroy(IT4GameFrame* InFrame);

T4FRAME_API IT4GameFrame* T4FrameGet(ET4LayerType InLayerType);
