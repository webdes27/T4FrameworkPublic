// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4FrameworkGameplay.h" // #114
#include "Public/T4Framework.h" // #25, #42

#include "T4Engine/Public/T4Engine.h"

#include "AIController.h"
#include "T4GameAIController.generated.h"

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
class T4FRAMEWORK_API AT4GameAIController : public AAIController, public IT4GameAIController
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

	virtual FName GetClassTypeName() const override { return NAME_None; } // #104 : Object type 을 Enum 이 아니라 FName 으로 처리. N개가 될 수 있음을 가정하겠음

	bool HasPlayer() const override { return false; } // #104

#if (WITH_EDITOR || WITH_SERVER_CODE)
	virtual void OnNotifyAIEvent(const FName& InEventName, const FT4ObjectID& InSenderObjectID) override {} // #63
#endif

	bool SetWorldActor(const FT4ActorID& InNewTargetID) override;
	void ResetWorldActor(bool bInSetDefaultPawn) override;

	bool HasWorldActor() const override  { return WorldActorID.IsValid(); }
	const FT4ActorID& GetWorldActorID() const override { return WorldActorID; }
	IT4WorldActor* GetWorldActor() const override;

	bool HasObserverActor() const override { return false; } // #52
	bool SetObserverActor(const FT4ActorID& InNewObserverID) override { return false; } // #52 : 서버는 필요없다!
	void ClearObserverActor() override {} // #52 : 서버는 필요없다!

	IT4GameWorld* GetGameWorld() const override; // #52

	bool HasAction(const FT4ActionKey& InActionKey) const override; // #20
	bool IsPlayingAction(const FT4ActionKey& InActionKey) const override; // #20 : Playing 중인지를 체크. Paused 면 False 가 리턴됨!

	AController* GetAController() override;
	APlayerCameraManager* GetCameraManager() const override { return nullptr; } // #100

public:
	// IT4GameAIController
	const FT4ObjectID& GetObjectID() const override { return ObjectID; }

	virtual AAIController* GetAIController() override; // #104

	virtual ET4GameplayTribeType GetTribeType() const override { return ET4GameplayTribeType::Neutral; } // #104 : TODO
	virtual ET4GameplayEnemyType GetEnemyType() const override { return ET4GameplayEnemyType::NoEnemy; } // #104 : TODO

public:
	void SetObjectID(const FT4ObjectID& InObjectID) { ObjectID = InObjectID;}

	IT4WorldActor* FindWorldActor(const FT4ActorID& InActorID) const; // #104
	bool FindNearestWorldActors(float InMaxDistance, TArray<IT4WorldActor*>& OutActors); // #104

protected:
	virtual void NotifyAdvance(float InDeltaTime) {} // #114
	virtual void NotifyBeginPlay() {} // #50
	virtual void NotifyAIStart() {} // #50
	virtual void NotifyAIEnd() {} // #50

	bool IsServerRunning() const; // #104 : check 편의를 위하 editor define 을 사용하지 않음
	bool HasServerGameplayCustomSettings() const; // #104 : check 편의를 위하 editor define 을 사용하지 않음

#if WITH_EDITOR
	IT4EditorGameplayContoller* GetEditorGameplayController() const; // #60
#endif

protected:
	ET4LayerType LayerType;

	FT4ObjectID ObjectID; // #15
	FT4ActorID WorldActorID;

	UPROPERTY(transient)
	UT4PathFollowingComponent* OverridePathFollowingComponent; // #34
};
