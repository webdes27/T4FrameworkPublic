// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ClientGameWorld.h"

#include "Object/T4GameObject.h" // #42
#include "Object/T4GameObjectFactory.h" // #54

#include "Public/T4Engine.h"

#include "Classes/Camera/T4CameraModifier.h" // #100
#include "Classes/Camera/T4EditorCameraActor.h" // #58

#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Controller.h" // #42
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

#include "T4EngineInternal.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Networking/Travelling/
 */
FT4ClientGameWorld::FT4ClientGameWorld()
	: PlayerControl(nullptr)
{
}

FT4ClientGameWorld::~FT4ClientGameWorld()
{
}

void FT4ClientGameWorld::Create()
{
#if !UE_BUILD_SHIPPING
	check(ET4LayerType::Max != LayerType);
	ActionPlaybackController.SetLayerType(LayerType);
#endif
}

void FT4ClientGameWorld::Reset()
{
#if WITH_EDITOR
	DestroyAllEditorCameraActors(); // #58
#endif
	PlayerControl = nullptr;
}

void FT4ClientGameWorld::CleanUp() // #87
{
#if !UE_BUILD_SHIPPING
	ActionPlaybackController.Reset();  // #68, #87 : Play 는 World Tick 과 분리한다. 
#endif
}

#if WITH_EDITOR // #87
bool FT4ClientGameWorld::IsTickable() const
{
	return ActionPlaybackController.IsPlaying();
}

void FT4ClientGameWorld::Tick(float InDeltaTime) 
{
	// FTickableGameObject::Tick
	// #87 : World Tick 밖에서 처리해야 할 Action. Only Play 예) WorldTravel
	//       일반적인 게임에서는 필요없고, Playback 에서만 필요할 것으로 보임
#if !UE_BUILD_SHIPPING
	ActionPlaybackController.ProcessOutsideWorld(InDeltaTime);
#endif
}

TStatId FT4ClientGameWorld::GetStatId() const
{
	return TStatId();
}
#endif

void FT4ClientGameWorld::ProcessPre(float InDeltaTime)
{
	if (nullptr != PlayerControl)
	{
		// #100 : PostProcess Action 에서 사용할 CameraModifier 를 초기화 해준다.
		//        매프레임 PostProcess Action 에서 설정한 값으로 다시 채워준다.
		APlayerCameraManager* PlayerCameraManager = PlayerControl->GetCameraManager();
		check(nullptr != PlayerCameraManager);
		UT4CameraModifier* CameraModifier = Cast<UT4CameraModifier>(
			PlayerCameraManager->FindCameraModifierByClass(
				UT4CameraModifier::StaticClass()
			)
		);
		check(nullptr != CameraModifier);
		CameraModifier->Reset();
	}
#if !UE_BUILD_SHIPPING
	// #68, #87 : World Tick Pre 에서 호출됨. 일부 Action 은 ProcessOutsideWorld 에서 처리됨!
	ActionPlaybackController.Process(InDeltaTime);
#endif
}

void FT4ClientGameWorld::ProcessPost(float InDeltaTime)
{
}

FVector FT4ClientGameWorld::GetCameraLocation() const
{
	if (nullptr == PlayerControl)
	{
		return FVector::ZeroVector;
	}
	APlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return FVector::ZeroVector;
	}
	check(nullptr != PlayerController->PlayerCameraManager);
	return PlayerController->PlayerCameraManager->GetCameraLocation();
}

FRotator FT4ClientGameWorld::GetCameraRotation() const
{
	if (nullptr == PlayerControl)
	{
		return FRotator::ZeroRotator;
	}
	APlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return FRotator::ZeroRotator;
	}
	check(nullptr != PlayerController->PlayerCameraManager);
	return PlayerController->PlayerCameraManager->GetCameraRotation();
}

IT4ObjectController* FT4ClientGameWorld::GetPlayerControl()
{
	return PlayerControl;
}

bool FT4ClientGameWorld::SetPlayerControl(
	IT4ObjectController* InPlayerControl
)
{
	PlayerControl = InPlayerControl;

#if WITH_EDITOR
	{
		// #86 : Rehearsal Preview 처리는 GameInstance 를 통한 PlayerContoller 처리를 하지 않음으로 임시로 LocalPlayer 를 설정해준다.
		// #87 : Rehearsal ViewModel 에 있던 처리를 이곳으로 옮김
		WorldController.SetPlayerController(GetPlayerController());
	}
#endif
	return true;
}

bool FT4ClientGameWorld::HasPlayerObject() const
{
	if (nullptr == PlayerControl)
	{
		return false;
	}
	IT4GameObject* PlayerObject = GetPlayerObject();
	return (nullptr != PlayerObject) ? true : false;
}

bool FT4ClientGameWorld::IsPlayerObject(const FT4ObjectID& InObjectID) const
{
	if (nullptr == PlayerControl || !InObjectID.IsValid())
	{
		return false;
	}
	IT4GameObject* PlayerObject = GetPlayerObject();
	if (nullptr == PlayerObject)
	{
		return false;
	}
	const FT4ObjectID& PlayerObjectID = PlayerObject->GetObjectID();
	return (InObjectID == PlayerObjectID) ? true : false;
}

bool FT4ClientGameWorld::IsPlayerObject(IT4GameObject* InGameObject) const
{
	if (nullptr == PlayerControl || nullptr == InGameObject)
	{
		return false;
	}
	IT4GameObject* PlayerObject = GetPlayerObject();
	if (nullptr == PlayerObject)
	{
		return false;
	}
	return (InGameObject == PlayerObject) ? true : false;
}

IT4GameObject* FT4ClientGameWorld::GetPlayerObject() const
{
	if (nullptr == PlayerControl)
	{
		return nullptr;
	}
	AController* AController = PlayerControl->GetAController();
	check(nullptr != AController);
	APawn* PlayerPawn = AController->GetPawn();
	if (nullptr == PlayerPawn)
	{
		return nullptr;
	}
	AT4GameObject* PlayerGameObject = Cast<AT4GameObject>(PlayerPawn);
	if (nullptr == PlayerGameObject)
	{
		return nullptr;
	}
	return PlayerGameObject;
}

APlayerController* FT4ClientGameWorld::GetPlayerController() const // #87
{
	if (nullptr == PlayerControl)
	{
		return nullptr;
	}
	return Cast<APlayerController>(PlayerControl->GetAController());
}

#if !UE_BUILD_SHIPPING
// #68
IT4ActionPlaybackPlayer* FT4ClientGameWorld::GetActionPlaybackPlayer() const
{
	return ActionPlaybackController.GetPlayer();
}

IT4ActionPlaybackRecorder* FT4ClientGameWorld::GetActionPlaybackRecorder() const
{
	return ActionPlaybackController.GetRecorder();
}

IT4ActionPlaybackController* FT4ClientGameWorld::GetActionPlaybackController()
{
	return &ActionPlaybackController;
}

void FT4ClientGameWorld::DoPlaybackSnapshotSave()
{
	// #87 : 현재 로드된 월드 저장...
	{
		IT4ActionPlaybackRecorder* ActionPlaybackRecorder = GetActionPlaybackRecorder();
		check(nullptr != ActionPlaybackRecorder);
		FT4WorldTravelAction WorldTravelAction;
		WorldTravelAction.MapEntityOrLevelObjectPath = WorldObjectPath;
#if WITH_EDITOR
		if (ET4LayerType::LevelEditor == GetLayerType())
		{
			WorldTravelAction.MapEntityOrLevelObjectPath = GetWorld();
		}
#endif
		WorldTravelAction.bPreveiwScene = WorldObjectPath.IsNull();
		ActionPlaybackRecorder->RecWorldAction(&WorldTravelAction, nullptr);
	}

	// #68 : 현 시점에 스폰된 오브젝트 정보를 기록해주어야 한다.
	if (0 < WorldContainer.GetNumGameObjects())
	{
		for (AT4GameObject* GameObject : WorldContainer.GetSpawnedGameObjects())
		{
			const FT4ObjectID& CurrentObjectID = GameObject->GetObjectID();
			if (ET4SpawnMode::All != CurrentObjectID.SpawnMode)
			{
				continue;
			}
			GameObject->DoActionPlaybackStartRecording();
		}
	}
}

void FT4ClientGameWorld::DoPlaybackSnapshotRestore()
{
	if (0 < WorldContainer.GetNumGameObjects())
	{
		TArray<FT4ObjectID> PlaybackObjectIDs;
		for (AT4GameObject* GameObject : WorldContainer.GetSpawnedGameObjects())
		{
			FT4ObjectID ObjectID = GameObject->GetObjectID();
			if (ET4SpawnMode::Playback != ObjectID.SpawnMode)
			{
				continue;
			}
			PlaybackObjectIDs.Add(ObjectID);
		}
		for (FT4ObjectID& ObjectID : PlaybackObjectIDs)
		{
			WorldContainer.ProcessDespawnObjectAction(ObjectID, T4ObjectWorldLeaveTimeSec);
		}
	}
}

void FT4ClientGameWorld::DoPlaybackAllPause(bool bInPause)
{
	if (0 < WorldContainer.GetNumGameObjects())
	{
		for (AT4GameObject* GameObject : WorldContainer.GetSpawnedGameObjects())
		{
			FT4ObjectID ObjectID = GameObject->GetObjectID();
			if (ET4SpawnMode::Playback != ObjectID.SpawnMode)
			{
				continue;
			}
			GameObject->SetDebugPause(bInPause);
		}
	}
}
// ~#68
#endif

#if WITH_EDITOR
AT4EditorCameraActor* FT4ClientGameWorld::FindOrCreateEditorCameraActor(
	uint32 InKey, 
	bool bInCreate,
	bool bInEmulMode
) // #58 : Only Client
{
	if (EditorCameraActorMap.Contains(InKey))
	{
		return EditorCameraActorMap[InKey].Get();
	}
	if (!bInCreate)
	{
		return nullptr;
	}
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save these temp actors into a map
	TWeakObjectPtr<AT4EditorCameraActor> NewEditorCameraActorPtr = GetWorld()->SpawnActor<AT4EditorCameraActor>(SpawnInfo);
	check(NewEditorCameraActorPtr.IsValid());
	NewEditorCameraActorPtr->Initialize(InKey, bInEmulMode);
	EditorCameraActorMap.Add(InKey, NewEditorCameraActorPtr);
	return NewEditorCameraActorPtr.Get();
}

void FT4ClientGameWorld::DestroyEditorCameraActor(uint32 InKey) // #58 : Only Client
{
	if (!EditorCameraActorMap.Contains(InKey))
	{
		return;
	}
	TWeakObjectPtr<AT4EditorCameraActor> EditorCameraActorPtr = EditorCameraActorMap[InKey];
	EditorCameraActorMap.Remove(InKey);
	if (EditorCameraActorPtr.IsValid())
	{
		EditorCameraActorPtr->RemoveFromRoot();
		EditorCameraActorPtr->Destroy();
		EditorCameraActorPtr.Reset();
	}
}

void FT4ClientGameWorld::DestroyAllEditorCameraActors() // #58
{
	for (TMap<uint32, TWeakObjectPtr<AT4EditorCameraActor>>::TConstIterator It(EditorCameraActorMap); It; ++It)
	{
		TWeakObjectPtr<AT4EditorCameraActor> EditorCameraActorPtr = It.Value();
		if (EditorCameraActorPtr.IsValid())
		{
			EditorCameraActorPtr->RemoveFromRoot();
			EditorCameraActorPtr->Destroy();
			EditorCameraActorPtr.Reset();
		}
	}
	EditorCameraActorMap.Empty();
}
#endif
