// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4Frame.h" // #25, #42

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
class T4FRAME_API AT4GameAIController : public AAIController, public IT4GameAIController
{
	GENERATED_UCLASS_BODY()

public:
	void PostInitializeComponents();

	virtual void TickActor(
		float InDeltaTime,
		enum ELevelTick InTickType,
		FActorTickFunction& InThisTickFunction
	) override;

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
	bool HasPlayerController() const override { return false; } // #104

#if (WITH_EDITOR || WITH_SERVER_CODE)
	virtual void OnNotifyAIEvent(const FName& InEventName, const FT4ObjectID& InSenderObjectID) override {} // #63
#endif

	bool SetGameObject(const FT4ObjectID& InNewTargetID) override;
	void ClearGameObject(bool bInSetDefaultPawn) override;

	bool HasGameObject() const override  { return GameObjectID.IsValid(); }
	const FT4ObjectID& GetGameObjectID() const override { return GameObjectID; }
	IT4GameObject* GetGameObject() const override;

	bool HasObserverObject() const override { return false; } // #52
	bool SetObserverObject(const FT4ObjectID& InNewObserverID) override { return false; } // #52 : 서버는 필요없다!
	void ClearObserverObject() override {} // #52 : 서버는 필요없다!

	IT4GameWorld* GetGameWorld() const override; // #52

	bool HasPublicAction(const FT4ActionKey& InActionKey) const override; // #20
	bool IsPlayingPublicAction(const FT4ActionKey& InActionKey) const override; // #20 : Playing 중인지를 체크. Paused 면 False 가 리턴됨!

	AController* GetAController() override;
	APlayerCameraManager* GetCameraManager() const override { return nullptr; } // #100

public:
	// IT4GameAIController
	virtual AAIController* GetAIController() override; // #104

	virtual ET4GameTribeType GetTribeType() const override { return ET4GameTribeType::Neutral; } // #104 : TODO
	virtual ET4GameEnemyType GetEnemyType() const override { return ET4GameEnemyType::NoEnemy; } // #104 : TODO

public:
	void SetNetID(const FT4NetID& InNetID) { NetID = InNetID;}
	const FT4NetID& GetNetID() const { return NetID; }

	IT4GameObject* FindGameObject(const FT4ObjectID& InObjectID) const; // #104
	bool FindNearestGameObjects(float InMaxDistance, TArray<IT4GameObject*>& OutObjects); // #104

protected:
	virtual void NotifyAIReady() {} // #50
	virtual void NotifyAIStart() {} // #50
	virtual void NotifyAIEnd() {} // #50

	IT4GameObject* FindGameObjectForServer(const FT4ObjectID& InObjectID) const; // #49

	bool IsServerRunning() const; // #104 : check 편의를 위하 editor define 을 사용하지 않음
	bool HasServerGameplayCustomSettings() const; // #104 : check 편의를 위하 editor define 을 사용하지 않음

#if WITH_EDITOR
	IT4EditorGameplayHandler* GetEditorGameplayCustomHandler() const; // #60
#endif

protected:
	ET4LayerType LayerType;

	FT4NetID NetID; // #15
	FT4ObjectID GameObjectID;

	UPROPERTY(transient)
	UT4PathFollowingComponent* OverridePathFollowingComponent; // #34
};
