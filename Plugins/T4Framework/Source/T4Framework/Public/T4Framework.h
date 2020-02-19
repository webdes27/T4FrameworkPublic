// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4FrameworkTypes.h"
#include "T4FrameworkGameplay.h" // #104
#include "T4FrameworkStructs.h"

#if WITH_EDITOR
#include "T4FrameworkEditor.h" // #60
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
class IT4WorldSystem;
class AT4PlayerController;
class IT4EditorViewportClient;
class UT4MapEntityAsset; // #87

// #30
enum ET4FrameworkType
{
	Frame_Client,
	Frame_Server,

	Frame_None
};

class IT4Framework;
class IT4WorldActor;

#if WITH_EDITOR
DECLARE_MULTICAST_DELEGATE_OneParam(FT4OnViewTargetChanged, IT4WorldActor*);
#endif

class IT4GameObject;
class AAIController;

// #34, #63, #114
class T4FRAMEWORK_API IT4ObjectController
{
public:
	virtual ~IT4ObjectController() {}

	virtual ET4LayerType GetLayerType() const = 0;
	virtual ET4ControllerType GetControllerType() const = 0; // #114

	virtual const FT4ObjectID& GetObjectID() const = 0; // #114 : GameObject and Controller ID (WARN : 서버는 모두, 클라는 Player 만 존재)

#if (WITH_EDITOR || WITH_SERVER_CODE)
	virtual void OnNotifyAIEvent(const FName& InEventName, const FT4ObjectID& InSenderObjectID) = 0; // #63
#endif

	virtual bool SetControlActor(const FT4ActorID& InNewTargetID) = 0;
	virtual void ResetControlActor(bool bInSetDefaultPawn) = 0;

	virtual bool HasControlActor() const = 0;
	virtual const FT4ActorID& GetControlActorID() const = 0;
	virtual IT4WorldActor* GetControlActor() const = 0;

	virtual bool HasObserverActor() const = 0; // #52
	virtual bool SetObserverActor(const FT4ActorID& InNewObserverID) = 0; // #52
	virtual void ClearObserverActor() = 0; // #52

	virtual bool HasAction(const FT4ActionKey& InActionKey) const = 0; // #102 : 존재만 해도 true 리턴
	virtual bool IsPlayingAction(const FT4ActionKey& InActionKey) const = 0; // #20 : Playing 중인지를 체크. Paused 면 False 가 리턴됨!

	virtual AController* GetAController() = 0;
	virtual APlayerCameraManager* GetCameraManager() const = 0; // #100

	virtual IT4WorldSystem* GetWorldSystem() const = 0; // #52
	virtual IT4GameObject* GetGameObject() const = 0; // #114
};

class T4FRAMEWORK_API IT4NPCAIController : public IT4ObjectController
{
public:
	virtual ~IT4NPCAIController() {}

	virtual AAIController* GetAIController() = 0; // #104
};

class T4FRAMEWORK_API IT4PlayerController : public IT4ObjectController
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

// #114
class T4FRAMEWORK_API IT4GameObject
{
public:
	virtual ~IT4GameObject() {}

	virtual ET4LayerType GetLayerType() const = 0;
	virtual const FT4ObjectID& GetObjectID() const = 0; // #114

	virtual IT4ObjectController* GetController() const = 0; // #114 : Server All, Client Player Only
};

// #42
class T4FRAMEWORK_API IT4GameplayInstance
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
class T4FRAMEWORK_API IT4Framework
{
public:
	virtual ~IT4Framework() {}

	virtual ET4LayerType GetLayerType() const = 0;
	virtual ET4FrameworkType GetType() const = 0;

	virtual void OnReset() = 0;
	virtual void OnStartPlay() = 0;

	virtual void OnProcessPre(float InDeltaTime) = 0; // #34 : OnWorldPreActorTick
	virtual void OnProcessPost(float InDeltaTime) = 0; // #34 : OnWorldPostActorTick

	virtual void OnDrawHUD(FViewport* InViewport, FCanvas* InCanvas, FT4HUDDrawInfo& InOutDrawInfo) = 0; // #68 : Only Client

	virtual bool HasBegunPlay() const = 0;

	virtual UWorld* GetWorld() const = 0;
	virtual IT4WorldSystem* GetWorldSystem() const = 0;

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
#if WITH_EDITOR
	virtual FT4ObjectID ReservedObjectIDForEditor() = 0; // #114 : 미리 잡아놓는다. (툴용)
#endif

	virtual bool AddObjectController(const FT4ObjectID& InObjectID, IT4NPCAIController* InAIController) = 0; // #31
	virtual void RemoveObjectController(const FT4ObjectID& InObjectID) = 0; // #31
	virtual IT4NPCAIController* GetObjectController(const FT4ObjectID& InObjectID) const = 0; // #31

	virtual bool AddServerGameObject(const FT4ObjectID& InObjectID, UT4GameObject* InGameObject) = 0; // #114
	virtual void RemoveServerGameObject(const FT4ObjectID& InObjectID) = 0; // #114
	virtual void RemoveAllServerGameObjects() = 0; // #114
	virtual UT4GameObject* GetServerGameObject(const FT4ObjectID& InObjectID) const = 0; // #114
#endif
};

namespace T4Framework
{
	T4FRAMEWORK_API IT4Framework* CreateFramework(
		ET4FrameworkType InFrameType,
		const FT4WorldConstructionValues& InWorldConstructionValues // #87
	);
	T4FRAMEWORK_API void DestroyFramework(IT4Framework* InFramework);

	T4FRAMEWORK_API IT4Framework* GetFramework(ET4LayerType InLayerType);
}