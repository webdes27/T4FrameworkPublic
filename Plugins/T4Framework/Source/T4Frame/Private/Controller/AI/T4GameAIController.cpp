// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Controller/AI/T4GameAIController.h"
#include "Classes/Controller/AI/Component/T4PathFollowingComponent.h"

#include "Public/T4Frame.h" // #30

#include "T4Engine/Public/T4Engine.h"

#include "Net/UnrealNetwork.h"

#include "T4FrameInternal.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Framework/Controller/AIController/
 */
AT4GameAIController::AT4GameAIController(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
	, LayerType(ET4LayerType::Max)
{
	bReplicates = false; // #15
	bWantsPlayerState = true;

	if (GetPathFollowingComponent())
	{
		GetPathFollowingComponent()->DestroyComponent();
	}

	OverridePathFollowingComponent = CreateDefaultSubobject<UT4PathFollowingComponent>(TEXT("T4PathFollowingComponent"));
	OverridePathFollowingComponent->OnRequestFinished.AddUObject(this, &AT4GameAIController::HandleOnMoveCompleted);
	SetPathFollowingComponent(OverridePathFollowingComponent);
}

void AT4GameAIController::PostInitializeComponents()
{
	//UT4PathFollowingComponent* NewPathFollowingComponent = NewObject<UT4PathFollowingComponent>(this);
	//NewPathFollowingComponent->OnRequestFinished.AddUObject(this, &AT4GameAIController::OnMoveCompleted);
	//SetPathFollowingComponent(NewPathFollowingComponent);

	Super::PostInitializeComponents();
}

void AT4GameAIController::TickActor(
	float InDeltaTime,
	enum ELevelTick InTickType,
	FActorTickFunction& InThisTickFunction
)
{
	Super::TickActor(InDeltaTime, InTickType, InThisTickFunction);

	// advance
}

bool AT4GameAIController::ShouldTickIfViewportsOnly() const
{
#if WITH_EDITOR
	if (HasT4EditorModeAISystemPaused()) // #52 : temp
	{
		return false;
	}
#endif
	return T4EngineLayer::IsLevelEditor(LayerType); // #17
}

void AT4GameAIController::EndPlay(const EEndPlayReason::Type InEndPlayReason)
{
	Super::EndPlay(InEndPlayReason);

	NotifyAIEnd(); // #50

	check(ET4LayerType::Max != LayerType);
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	check(NetID.IsValid());

	GameFrame->UnregisterGameAIController(NetID);
}

void AT4GameAIController::BeginPlay()
{
	Super::BeginPlay();

	check(ET4LayerType::Max == LayerType);
	LayerType = T4EngineLayer::Get(GetWorld()); // #12 : Support Multiple LayerType
	check(ET4LayerType::Max != LayerType);

	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);

	check(NetID.IsValid());
	GameFrame->RegisterGameAIController(NetID, this);

	NotifyAIReady(); // #50
}

void AT4GameAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AT4GameAIController::OnUnPossess()
{
	Super::OnUnPossess();
}

void AT4GameAIController::GameHasEnded(AActor* EndGameFocus, bool bIsWinner)
{
}

void AT4GameAIController::BeginInactiveState()
{
	Super::BeginInactiveState();
}

void AT4GameAIController::HandleOnMoveCompleted(
	FAIRequestID RequestID,
	const FPathFollowingResult& Result
) // #34
{
	Super::OnMoveCompleted(RequestID, Result);
}

bool AT4GameAIController::SetGameObject(const FT4ObjectID& InNewTargetID)
{
	if (!InNewTargetID.IsValid())
	{
		return false;
	}
	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	IT4GameObject* NewTargetObject = GameWorld->GetContainer()->FindGameObject(InNewTargetID);
	if (nullptr == NewTargetObject)
	{
		UE_LOG(
			LogT4Frame,
			Error,
			TEXT("[SL:%u] AT4GameAIController::SetGameObject '%s' failed. object not found."),
			uint32(LayerType),
			*(InNewTargetID.ToString())
		);
		return false;
	}
	APawn* TargetPawn = NewTargetObject->GetPawn();
	if (nullptr == TargetPawn)
	{
		UE_LOG(
			LogT4Frame,
			Error,
			TEXT("[SL:%u] AT4GameAIController::SetGameObject '%s' failed. Pawn is null."),
			uint32(LayerType),
			*(InNewTargetID.ToString())
		);
		return false;
	}
	ClearGameObject(false);
	OnPossess(TargetPawn);
	GameObjectID = InNewTargetID;
	if (nullptr != OverridePathFollowingComponent)
	{
		OverridePathFollowingComponent->SetGameObjectID(GameObjectID); // #34
	}
	// #34 : for Server All or Client Only Player
	check(nullptr != NewTargetObject);
	NewTargetObject->SetObjectController(this);
	NotifyAIStart(); // #50
	return true;
}

void AT4GameAIController::ClearGameObject(bool bInSetDefaultPawn)
{
	if (!GameObjectID.IsValid())
	{
		return;
	}
	OnUnPossess();
	// #34 : for Server All or Client Only Player
	IT4GameObject* OldTargetObject = GetGameObject();
	if (nullptr != OldTargetObject)
	{
		OldTargetObject->SetObjectController(nullptr);
	}
	if (nullptr != OverridePathFollowingComponent)
	{
		OverridePathFollowingComponent->ClearGameObjectID(); // #34
	}
	GameObjectID.SetNone();
}

IT4GameObject* AT4GameAIController::GetGameObject() const
{
	if (!GameObjectID.IsValid())
	{
		return nullptr;
	}
	return FindGameObjectForServer(GameObjectID);
}

IT4GameWorld* AT4GameAIController::GetGameWorld() const // #52
{
	return T4EngineWorldGet(LayerType);
}

bool AT4GameAIController::HasPublicAction(const FT4ActionKey& InActionKey) const
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

bool AT4GameAIController::IsPlayingPublicAction(const FT4ActionKey& InActionKey) const
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

AController* AT4GameAIController::GetAController()
{
	return Cast<AController>(this);
}

AAIController* AT4GameAIController::GetAIController() 
{ 
	return Cast<AAIController>(this); 
} // #104

IT4GameObject* AT4GameAIController::FindGameObject(const FT4ObjectID& InObjectID) const // #104
{
	return FindGameObjectForServer(InObjectID);
}

bool AT4GameAIController::FindNearestGameObjects(
	float InMaxDistance, 
	TArray<IT4GameObject*>& OutObjects
) // #104
{
	IT4GameObject* OwnerGameObject = GetGameObject();
	check(nullptr != OwnerGameObject);
	if (!OwnerGameObject->IsLoaded())
	{
		return false;
	}
	IT4GameWorld* GameWorld = T4EngineWorldGet(LayerType);
	check(nullptr != GameWorld);
	bool bResult = GameWorld->GetContainer()->QueryNearestGameObjects(
		OwnerGameObject->GetNavPoint(),
		InMaxDistance, // #50
		OutObjects
	);
	return bResult;
}

IT4GameObject* AT4GameAIController::FindGameObjectForServer(const FT4ObjectID& InObjectID) const
{
	check(ET4LayerType::Max > LayerType);
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	return GameWorld->GetContainer()->FindGameObject(InObjectID);
}

bool AT4GameAIController::IsServerRunning() const
{
	// #104 : check 편의를 위하 editor define 을 사용하지 않음
#if WITH_EDITOR
	{
		IT4EditorGameplayHandler* EditorGameplayHandler = GetEditorGameplayCustomHandler(); // #60
		if (nullptr != EditorGameplayHandler)
		{
			return EditorGameplayHandler->IsSimulating();
		}
	}
#endif
	return true;
}

bool AT4GameAIController::HasServerGameplayCustomSettings() const
{
	// #104 : check 편의를 위하 editor define 을 사용하지 않음
#if WITH_EDITOR
	{
		IT4EditorGameplayHandler* EditorGameplayHandler = GetEditorGameplayCustomHandler(); // #60
		if (nullptr != EditorGameplayHandler)
		{
			return EditorGameplayHandler->IsUsedGameplaySettings();
		}
	}
#endif
	return false;
}

#if WITH_EDITOR
IT4EditorGameplayHandler* AT4GameAIController::GetEditorGameplayCustomHandler() const // #60
{
	check(ET4LayerType::Max > LayerType);
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	IT4EditorGameplayHandler* EditorGameplayHandler = GameFrame->GetEditorGameplayCustomHandler();
	return EditorGameplayHandler;
}
#endif