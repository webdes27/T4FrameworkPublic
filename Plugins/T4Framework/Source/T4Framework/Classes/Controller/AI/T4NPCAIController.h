// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4FrameworkGameplay.h" // #114
#include "Public/T4Framework.h" // #25, #42

#include "T4Engine/Public/T4Engine.h"

#include "AIController.h"
#include "T4NPCAIController.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Framework/Controller/AIController/
 */

enum ET4AIDataLoadState // #50
{
	AIDataLoad_Ready,
	AIDataLoad_Loading,
	AIDataLoad_Loaded,
	AIDataLoad_NoData,
};

class UT4PathFollowingComponent;

UCLASS()
class T4FRAMEWORK_API AT4NPCAIController : public AAIController, public IT4NPCAIController
{
	GENERATED_UCLASS_BODY()

public:
	void PostInitializeComponents();

	void TickActor(float InDeltaTime, enum ELevelTick InTickType, FActorTickFunction& InThisTickFunction) override;

	/** If true, actor is ticked even if TickType==LEVELTICK_ViewportsOnly	 */
	bool ShouldTickIfViewportsOnly() const override; // #17

	void EndPlay(const EEndPlayReason::Type InEndPlayReason) override;

	void HandleOnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result); // #34

protected:
	void BeginPlay() override;

	// Begin AController interface
	void GameHasEnded(class AActor* EndGameFocus = NULL, bool bIsWinner = false) override;
	void OnPossess(class APawn* InPawn) override;
	void OnUnPossess() override;
	void BeginInactiveState() override;
	// End AController interface

public:
	// IT4ObjectController
	ET4LayerType GetLayerType() const override { return LayerType; }
	ET4ControllerType GetControllerType() const override { return ET4ControllerType::Controller_NPCAI; } // #114

#if (WITH_EDITOR || WITH_SERVER_CODE)
	virtual void OnNotifyAIEvent(const FName& InEventName, const FT4ObjectID& InSenderObjectID) override {} // #63
#endif

	bool SetControlActor(const FT4ActorID& InNewTargetID) override;
	void ResetControlActor(bool bInSetDefaultPawn) override;

	bool HasControlActor() const override  { return ControlActorID.IsValid(); }
	const FT4ActorID& GetControlActorID() const override { return ControlActorID; }
	IT4WorldActor* GetControlActor() const override;

	bool HasObserverActor() const override { return false; } // #52
	bool SetObserverActor(const FT4ActorID& InNewObserverID) override { return false; } // #52 : 서버는 필요없다!
	void ClearObserverActor() override {} // #52 : 서버는 필요없다!

	bool HasAction(const FT4ActionKey& InActionKey) const override; // #20
	bool IsPlayingAction(const FT4ActionKey& InActionKey) const override; // #20 : Playing 중인지를 체크. Paused 면 False 가 리턴됨!

	AController* GetAController() override;
	APlayerCameraManager* GetCameraManager() const override { return nullptr; } // #100

	IT4WorldSystem* GetWorldSystem() const override; // #52
	IT4GameObject* GetGameObject() const override; // #114

public:
	// IT4NPCAIController
	const FT4ObjectID& GetObjectID() const override { return ObjectID; }

	virtual AAIController* GetAIController() override; // #104

public:
	void SetObjectID(const FT4ObjectID& InObjectID) { ObjectID = InObjectID;}

	IT4WorldActor* FindWorldActor(const FT4ActorID& InActorID) const; // #104

protected:
	virtual void NotifyAdvance(float InDeltaTime) {} // #114
	virtual void NotifyBeginPlay() {} // #50
	virtual void NotifyEndPlay() {} // #50
	virtual void NotifyAIStart() {} // #50
	virtual void NotifyAIEnd() {} // #50

protected:
	ET4LayerType LayerType;

	FT4ObjectID ObjectID; // #15
	FT4ActorID ControlActorID;

	UPROPERTY(transient)
	UT4PathFollowingComponent* OverridePathFollowingComponent; // #34
};
