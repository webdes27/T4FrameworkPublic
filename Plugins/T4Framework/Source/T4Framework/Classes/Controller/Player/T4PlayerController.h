// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4Framework.h" // #25, #42

#include "T4Engine/Public/Action/T4ActionKey.h"

#include "GameFramework/PlayerController.h"

#include "T4PlayerController.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Framework/Controller/PlayerController/index.html
 */
class UT4SpringArmComponent;
class UT4CameraComponent;
class IT4WorldActor;

UCLASS()
class T4FRAMEWORK_API AT4PlayerController : public APlayerController, public IT4PlayerController
{
	GENERATED_UCLASS_BODY()

public:
	void SetupInputComponent() override;
	void PostInitializeComponents() override;

	void TickActor(
		float InDeltaTime, 
		enum ELevelTick InTickType,
		FActorTickFunction& InThisTickFunction
	) override;

	/** If true, actor is ticked even if TickType==LEVELTICK_ViewportsOnly	 */
	bool ShouldTickIfViewportsOnly() const override; // #17

	void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;
	
	// #116 : 마우스 우클릭으로 카메라 조정시 bShowMouseCursor flag 를 켜주면 자연스러운 움직임을 캡처할 수가 없어
	//        bShowMouseCursor flag 를 끄고, bMouseMoveLocked flag 로 MouseCursor 를 사용하도록 제어 
	EMouseCursor::Type GetMouseCursor() const override;

protected:
	void BeginPlay() override;

	void OnPossess(APawn* InPawn) override;
	void OnUnPossess() override;

public:
	// IT4ObjectController
	ET4LayerType GetLayerType() const override { return LayerType; }

	FName GetClassTypeName() const override { return DefaultPlayerClassName; } // #104 : Object type 을 Enum 이 아니라 FName 으로 처리. N개가 될 수 있음을 가정하겠음

	bool HasPlayer() const override { return true; } // #104

#if (WITH_EDITOR || WITH_SERVER_CODE)
	void OnNotifyAIEvent(const FName& InEventName, const FT4ObjectID& InSenderObjectID) override {} // #63
#endif

	APawn* GetDefaultPawn() const override; // #86

	bool SetWorldActor(const FT4ActorID& InNewTargetID) override;
	void ResetWorldActor(bool bInSetDefaultPawn) override;

	bool HasWorldActor() const override { return WorldActorID.IsValid(); }
	const FT4ActorID& GetWorldActorID() const override { return WorldActorID; }
	IT4WorldActor* GetWorldActor() const override;

	bool HasObserverActor() const override { return ObserverActorID.IsValid(); } // #52
	bool SetObserverActor(const FT4ActorID& InNewObserverID) override; // #52
	void ClearObserverActor() override; // #52

	IT4GameWorld* GetGameWorld() const override; // #52

	bool HasAction(const FT4ActionKey& InActionKey) const override; // #20
	bool IsPlayingAction(const FT4ActionKey& InActionKey) const override; // #20 : Playing 중인지를 체크. Paused 면 False 가 리턴됨!

	AController* GetAController() override;
	APlayerCameraManager* GetCameraManager() const override; // #100

public:
	void SetObjectID(const FT4ObjectID& InObjectID) { ObjectID = InObjectID; }

	// IT4PlayerController
	const FT4ObjectID& GetObjectID() const override { return ObjectID; }

	bool CheckAuthority() const override { return HasAuthority(); }

	UInputComponent* NewInputComponent() override;
	void SetInputComponent(UInputComponent* InInputComponent) override;
	void OnSetInputMode(ET4InputMode InMode) override;

	FViewport* GetViewport() const override; // #68

	FRotator GetViewControlRotation() const override;

	ET4CameraType GetCameraType() const override { return CameraTypeSelected; } // #40
	float GetCameraFOV() const override { return CameraFOV; } // #40
	FVector GetCameraLocation() const override;
	FRotator GetCameraRotation() const override;
	FVector GetCameraLookAtLocation() const override; // #30

	void SwitchCameraType(ET4CameraType InCameraType) override; // #40

	void SetCameraZoom(float InAmount) override;
	void SetCameraZoomMaxScale(float InScale) override { CameraZoomMaxScale = InScale; } // #86
	void SetCameraPitch(float InAmount) override;
	void SetCameraYaw(float InAmount) override;

	void SetFreeCameraMoveDirection(const FVector& InLocation) override;
	void SetFreeCameraLocationAndRotation(const FVector& InLocation, const FRotator& InRotation) override; // #94, #86

	void GetCameraInfoCached(FRotator& OutRotation, float& OutDistance) override; // #87
	void SetCameraInfoCached(const FRotator& InRotation, const float& InDistance) override; // #87

	bool GetMousePositionToWorldRay(FVector& OutStartPosition, FVector& OutStartDirection) override;

	void SetMouseCursorLock(bool bInLock) override;
	bool IsMouseCursorLocked() const override { return bMouseCursorLocked; }

	void SetMouseCursorType(EMouseCursor::Type InCursorType) override;
	void ShowMouseCursor(bool InShow) override;

	void SetMouseCursorPosition(const FVector2D& InPosition) override; // #30, #113 : ScreenSpace
	bool GetMouseCursorPosition(FVector2D& OutLocation) const override; // #30, #113

#if WITH_EDITOR
	// see UGameViewportClient::InputKey
	bool EditorInputKey(FKey InKey, EInputEvent InEvent, float InAmountDepressed, bool bInGamepad) override; // #30

	// see UGameViewportClient::InputAxis
	bool EditorInputAxis(FKey InKey, float InDelta, float InDeltaTime, int32 InNumSamples, bool bInGamepad) override; // #30

	void EditorSetViewportClient(IT4EditorViewportClient* InEditorViewportClient) override
	{
		EditorViewportClient = InEditorViewportClient;
	}

	FT4OnViewTargetChanged& GetOnViewTargetChanged() override { return OnViewTargetChanged; } // #39
#endif

protected:
	virtual void NotifyAdvance(float InDeltaTime) {} // #49
	virtual void NotifyBeginPlay() {} // #114
	virtual void NotifyPossess(IT4WorldActor* InNewWorldActor) {} // #49
	virtual void NotifyUnPossess(IT4WorldActor* InOldWorldActor) {} // #49

	IT4WorldActor* FindWorldActor(const FT4ActorID& InActorID) const; // #49

#if WITH_EDITOR
	IT4EditorGameplayContoller* GetEditorGameplayController() const; // #60
#endif

private:
	void AttachCameraComponent(APawn* InOuter);
	void DetachCameraComponent();

	APawn* GetTargetPawnSelected(); // #52

protected:
	ET4LayerType LayerType;
	FT4ObjectID ObjectID; // #114

private:
	FT4ActorID WorldActorID;
	FT4ActorID ObserverActorID; // #52

	TWeakObjectPtr<APawn> CachedDefaultPawn;

	bool bCameraTypeDirty; // #40
	ET4CameraType CameraTypeSelected; // #40

	bool bMouseCursorLocked; // #116
	FVector2D SaveMousePosition;

	float CachedCameraSpringTargetArmLength;
	FRotator CachedCameraRotation;

	// #40
	float CameraFOV;
	float CameraZoomSpeed;
	float CameraZoomDistanceMin;
	float CameraZoomDistanceMax;
	float CameraZoomMaxScale; // #86
	// ~#40

	UPROPERTY(Transient)
	UT4SpringArmComponent* CameraSpringArmComponent;

	UPROPERTY(Transient)
	UT4CameraComponent* CameraComponent;

#if WITH_EDITOR
	IT4EditorViewportClient* EditorViewportClient; // #30
	FT4OnViewTargetChanged OnViewTargetChanged; // #39
#endif
};
