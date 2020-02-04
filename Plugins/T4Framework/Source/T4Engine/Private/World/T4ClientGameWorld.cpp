// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ClientGameWorld.h"

#include "Object/T4GameObject.h" // #42
#include "Object/T4GameObjectFactory.h" // #54

#include "Public/T4EngineConstants.h" // #115
#include "Public/T4Engine.h"

#include "Classes/Camera/T4CameraModifier.h" // #100
#include "Classes/Camera/T4EditorCameraActor.h" // #58

#include "Materials/MaterialParameterCollection.h" // #115
#include "Materials/MaterialParameterCollectionInstance.h" // #115

#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Controller.h" // #42
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

#include "T4EngineInternal.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Networking/Travelling/
 */
FT4ClientGameWorld::FT4ClientGameWorld()
	: PlayerNetworkControlRef(nullptr)
	, IndicatorObjectRef(nullptr) // #117
{
}

FT4ClientGameWorld::~FT4ClientGameWorld()
{
}

void FT4ClientGameWorld::Create()
{
	{
		// #115
		check(!OwnerMPCGlobalPtr.IsValid());
		check(!MPCGlobalInstancePtr.IsValid());
		OwnerMPCGlobalPtr = Cast<UMaterialParameterCollection>(T4EngineConstant::GetMPCGlobalObject());
		check(OwnerMPCGlobalPtr.IsValid());
	}
#if !UE_BUILD_SHIPPING
	check(ET4LayerType::Max != LayerType);
	ActionReplaySystem.SetLayerType(LayerType);
#endif
}

void FT4ClientGameWorld::Reset()
{
#if WITH_EDITOR
	DestroyAllEditorCameraActors(); // #58
#endif
	PlayerNetworkControlRef = nullptr;
	IndicatorObjectRef = nullptr; // #117
}

void FT4ClientGameWorld::CleanUp() // #87
{
#if !UE_BUILD_SHIPPING
	ActionReplaySystem.Reset(); // #68, #87 : Play 는 World Tick 과 분리한다. 
#endif
	if (OwnerMPCGlobalPtr.IsValid()) // #115
	{
		MPCGlobalInstancePtr.Reset();
		OwnerMPCGlobalPtr.Reset();
	}
}

#if WITH_EDITOR // #87
bool FT4ClientGameWorld::IsTickable() const
{
	return ActionReplaySystem.IsPlaying();
}

void FT4ClientGameWorld::Tick(float InDeltaTime) 
{
	// FTickableGameObject::Tick
	// #87 : World Tick 밖에서 처리해야 할 Action. Only Play 예) WorldTravel
	//       일반적인 게임에서는 필요없고, ActionReplay 에서만 필요할 것으로 보임
#if !UE_BUILD_SHIPPING
	ActionReplaySystem.ProcessOutsideWorld(InDeltaTime);
#endif
}

TStatId FT4ClientGameWorld::GetStatId() const
{
	return TStatId();
}
#endif

void FT4ClientGameWorld::ProcessPre(float InDeltaTime)
{
	if (nullptr != PlayerNetworkControlRef)
	{
		// #100 : PostProcess Action 에서 사용할 CameraModifier 를 초기화 해준다.
		//        매프레임 PostProcess Action 에서 설정한 값으로 다시 채워준다.
		APlayerCameraManager* PlayerCameraManager = PlayerNetworkControlRef->GetCameraManager();
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
	ActionReplaySystem.Process(InDeltaTime);
#endif
}

void FT4ClientGameWorld::ProcessPost(float InDeltaTime)
{
}

bool FT4ClientGameWorld::SetClientNetworkControl(
	IT4NetworkControl* InNetworkControl
)
{
	PlayerNetworkControlRef = InNetworkControl;

#if WITH_EDITOR
	{
		// #86 : Rehearsal Preview 처리는 GameInstance 를 통한 PlayerContoller 처리를 하지 않음으로 임시로 LocalPlayer 를 설정해준다.
		// #87 : Rehearsal ViewModel 에 있던 처리를 이곳으로 옮김
		WorldController.SetPlayerController(GetPlayerController());
	}
#endif
	return true;
}

APlayerController* FT4ClientGameWorld::GetPlayerController() const // #87, #114
{
	if (nullptr == PlayerNetworkControlRef)
	{
		return nullptr;
	}
	return Cast<APlayerController>(PlayerNetworkControlRef->GetAController());
}

APlayerCameraManager* FT4ClientGameWorld::GetPlayerCameraManager() const // #114
{
	if (nullptr == PlayerNetworkControlRef)
	{
		return nullptr;
	}
	return PlayerNetworkControlRef->GetCameraManager();
}

bool FT4ClientGameWorld::HasPlayerObject() const
{
	IT4GameObject* PlayerObject = GetPlayerObject();
	return (nullptr != PlayerObject) ? true : false;
}

bool FT4ClientGameWorld::ComparePlayerObject(const FT4ObjectID& InObjectID) const
{
	if (!InObjectID.IsValid())
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

bool FT4ClientGameWorld::ComparePlayerObject(IT4GameObject* InGameObject) const
{
	if (nullptr == PlayerNetworkControlRef || nullptr == InGameObject)
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

void FT4ClientGameWorld::ClearPlayerObject(bool bInDefaultPawn) // #114
{
	if (nullptr == PlayerNetworkControlRef)
	{
		return;
	}
	PlayerNetworkControlRef->ClearGameObject(bInDefaultPawn);
}

IT4GameObject* FT4ClientGameWorld::GetPlayerObject() const
{
	if (nullptr == PlayerNetworkControlRef)
	{
		return nullptr;
	}
	AController* AController = PlayerNetworkControlRef->GetAController();
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

void FT4ClientGameWorld::SetPlayerObject(const FT4ObjectID& InObjectID) // #114
{
	if (nullptr == PlayerNetworkControlRef)
	{
		return;
	}
	PlayerNetworkControlRef->SetGameObject(InObjectID);
}

bool FT4ClientGameWorld::SetMPCGlobalParameterScalar(FName InParameterName, const float InScalar) // #115
{
	TryInitializePostProcessMaterials(); // #115
	const bool bResult = MPCGlobalInstancePtr->SetScalarParameterValue(InParameterName, InScalar);
	if (!bResult)
	{
		T4_LOG(
			Warning,
			TEXT("Unable to find '%s' parameter in MPC Global"),
			*(InParameterName.ToString())
		);
	}
	return bResult;
}

bool FT4ClientGameWorld::SetMPCGlobalParameterColor(FName InParameterName, const FLinearColor& InValue) // #115
{
	TryInitializePostProcessMaterials(); // #115
	const bool bResult = MPCGlobalInstancePtr->SetVectorParameterValue(InParameterName, InValue);
	if (!bResult)
	{
		T4_LOG(
			Warning, 
			TEXT("Unable to find '%s' parameter in MPC Global"),
			*(InParameterName.ToString())
		);
	}
	return bResult;
}

FVector FT4ClientGameWorld::GetCameraLocation() const
{
	APlayerCameraManager* PlayerCameraManager = GetPlayerCameraManager();
	if (nullptr == PlayerCameraManager)
	{
		return FVector::ZeroVector;
	}
	return PlayerCameraManager->GetCameraLocation();
}

FRotator FT4ClientGameWorld::GetCameraRotation() const
{
	APlayerCameraManager* PlayerCameraManager = GetPlayerCameraManager();
	if (nullptr == PlayerCameraManager)
	{
		return FRotator::ZeroRotator;
	}
	return PlayerCameraManager->GetCameraRotation();
}

IT4GameObject* FT4ClientGameWorld::GetIndicatorObject() // #117
{
	if (nullptr == IndicatorObjectRef)
	{
		IndicatorObjectRef = GetContainer()->CreateClientObject(
			ET4ObjectType::World_Indicator,
			TEXT("WorldIndicatorObject"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			FVector::OneVector
		);
	}
	return IndicatorObjectRef;
}

void FT4ClientGameWorld::TryInitializePostProcessMaterials() // 3115
{
	if (MPCGlobalInstancePtr.IsValid())
	{
		return;
	}

	// #115 : Global WorldZoneVolume 이 있는지 확인하고 없다면 강제로 생성해준다.
	//        Global WorldZoneVolume 에 기본 PP 효과(Outline) Material 설정이 추가된다.
	// #115 : World 는 무조건 Global WorldZoneVolume 추가하도록 강제한다면 이 처리는 필요가 없다.
	//        현재는 정책적으로 정해지지 않았기 때문에 툴과 클라는 결과가 같도록 유지하기 위해 Fallback Spawn 처리를 해준다.
	// #115 : (TODO) 이후 명확히 월드 이동, 렌더링 가능한 상태를 알 수 있는 방법이 있다면 대체할 것!

	WorldController.TryCheckAndSpawnGlobalWorldZoneVolume();

	MPCGlobalInstancePtr = GetWorld()->GetParameterCollectionInstance(OwnerMPCGlobalPtr.Get());
	if (!MPCGlobalInstancePtr.IsValid())
	{
		T4_LOG(Warning, TEXT("Unable to find MPC Instance in MPC Global"));
	}
}

#if !UE_BUILD_SHIPPING
// #68
IT4ActionReplayPlayer* FT4ClientGameWorld::GetActionReplayPlayer() const
{
	return ActionReplaySystem.GetPlayer();
}

IT4ActionReplayRecorder* FT4ClientGameWorld::GetActionReplayRecorder() const
{
	return ActionReplaySystem.GetRecorder();
}

IT4ActionReplaySystem* FT4ClientGameWorld::GetActionReplaySystem()
{
	return &ActionReplaySystem;
}

void FT4ClientGameWorld::SaveActionReplaySnapshot()
{
	// #87 : 현재 로드된 월드 저장...
	{
		IT4ActionReplayRecorder* ActionReplayRecorder = GetActionReplayRecorder();
		check(nullptr != ActionReplayRecorder);
		FT4WorldTravelAction WorldTravelAction;
		WorldTravelAction.MapEntityOrLevelObjectPath = WorldObjectPath;
#if WITH_EDITOR
		if (ET4LayerType::LevelEditor == GetLayerType())
		{
			WorldTravelAction.MapEntityOrLevelObjectPath = GetWorld();
		}
#endif
		WorldTravelAction.bPreveiwScene = WorldObjectPath.IsNull();
		ActionReplayRecorder->RecWorldAction(&WorldTravelAction, nullptr);
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
			GameObject->SaveActionReplaySnapshot();
		}
	}
}

void FT4ClientGameWorld::RestoreActionReplaySnapshot()
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
			WorldContainer.ProcessDespawnObjectAction(ObjectID, T4Const_ObjectWorldLeaveTimeSec);
		}
	}
}

void FT4ClientGameWorld::SetActionReplayPause(bool bInPause)
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
