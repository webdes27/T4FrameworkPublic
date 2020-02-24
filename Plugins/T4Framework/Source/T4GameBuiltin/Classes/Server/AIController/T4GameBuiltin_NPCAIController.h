// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4GameBuiltin_Types.h"
#include "Public/T4GameBuiltin_GameDataTypes.h" // #48
#include "Public/Server/T4GameBuiltin_AIStructs.h"

#include "T4Engine/Public/Asset/T4AssetLoader.h" // #50, #42
#include "T4Framework/Classes/Controller/AI/T4NPCAIController.h"
#include "T4GameBuiltin_NPCAIController.generated.h"

/**
  * WARN : AI Controller 는 서버에서만 사용하고, 클라리언트에서는 사용하지 않음에 유의할 것!
  * #114
 */
class UBehaviorTree;
class IT4WorldActor;
class UT4GameBuiltin_ServerObject;
UCLASS()
class T4GAMEBUILTIN_API AT4GameBuiltin_NPCAIController : public AT4NPCAIController
{
	GENERATED_UCLASS_BODY()

public:
	void AIUpdate(float InDeltaTime); // #114 : from BTTree

	void NotifyAITaskState(ET4GameBuiltin_AITaskState InAITaskState);
	
	bool CheckCancelWaitTask();

public:
	bool Bind(const FT4GameBuiltin_GameDataID& InNPCGameDataID); // #31, #50

	UT4GameBuiltin_ServerObject* GetServerObject() const; // #114
	IT4ObjectController* GetObjectController(const FT4ObjectID& InObjectID) const; // #114

protected:
	void NotifyAdvance(float InDeltaTime) override; // #114
	void NotifyBeginPlay() override; // #50
	void NotifyEndPlay() override; // #50
	void NotifyAIStart() override; // #50
	void NotifyAIEnd() override; // #50

	void HandleOnCallbackMoveTo(const FVector& InMoveVelocity, bool bForceMaxSpeed); // #42, #34, #52 : MovementComponet::MaxSpeed 를 사용할지에 대한 Flag, 기본값이 false 로 Velocity 에서 Speed 를 얻는다. 동기화 이슈!!
	void HandleOnCallbackMoveStop(); // #52

private:
	bool CheckAsyncLoading();

private:
	ET4AIDataLoadState AIDataLoadState; // #50
	FT4BehaviorTreeAssetLoader BehaviorTreeAssetLoader;

	/* Cached BT component */
	UPROPERTY(transient)
	UBehaviorTree* BehaviorTreeAsset;
};
