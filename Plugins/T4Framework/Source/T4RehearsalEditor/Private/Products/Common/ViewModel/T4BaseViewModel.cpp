// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Products/Common/ViewModel/T4BaseViewModel.h"
#include "Products/T4RehearsalEditorUtils.h"

#include "Products/Common/Viewport/T4RehearsalViewportClient.h"
#include "Products/Common/Helper/T4EditorActionPlaybackController.h" // #68

#include "T4Asset/Public/Action/T4ActionContiStructs.h" // #81
#include "T4Asset/Public/T4AssetUtils.h" // #39, #62
#include "T4Asset/Classes/Entity/T4EntityAsset.h"
#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h" // #39
#include "T4Asset/Classes/Entity/T4MapEntityAsset.h" // #39
#include "T4Asset/Public/Entity/T4Entity.h"
#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h"

#include "T4Engine/Public/T4Engine.h"
#include "T4Engine/Public/Action/T4ActionCodeMinimal.h"
#include "T4Frame/Public/T4Frame.h" // #30
#include "T4Frame/Classes/Controller/Player/T4PlayerController.h" // #86

#include "Animation/AnimSequence.h"
#include "Engine/LocalPlayer.h" // #86

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "FT4BaseViewModel"

/**
  * #76
 */
FT4BaseViewModel::FT4BaseViewModel()
	: LayerType(ET4LayerType::Max)
	, GameFrameOwner(nullptr)
	, ViewportClientRef(nullptr)
	, bCachedPlayerSettingsSaved(false) // #87
	, CachedCameraZoomDistance(0.0f) // #87
	, CachedCameraControlRotation(FRotator::ZeroRotator) // #87
	, CachedPlayerRotation(FRotator::ZeroRotator) // #87
	, bTestAutomation(false) // #100, #103
	, TestAutomationGameTimeHour(12.0f) // #100, #103
	, TestAutomationSpawnLocation(FVector::ZeroVector) // #100, #103
	, TestAutomationSpawnRotation(FRotator::ZeroRotator) // #100, #103
{
}

FT4BaseViewModel::~FT4BaseViewModel()
{
	check(nullptr == ViewportClientRef); // #85 : Subclass::CleanUp 에서 DestroyAll 을 호출해줄 것!
	check(nullptr == GameFrameOwner); 
}

void FT4BaseViewModel::OnCleanup() // #79
{
	OnReset();
	if (nullptr != ViewportClientRef)
	{
		if (ViewportClientResetHandle.IsValid())
		{
			ViewportClientRef->GetOnDestroy().Remove(ViewportClientResetHandle);
			ViewportClientResetHandle.Reset();
		}
		ViewportClientRef->OnReset();
		ViewportClientRef = nullptr;
	}
	if (nullptr != GameFrameOwner)
	{
		T4FrameDestroy(GameFrameOwner);
		GameFrameOwner = nullptr;
	}
	if (EditorActionPlaybackControllerPtr.IsValid())
	{
		EditorActionPlaybackControllerPtr->RemoveFromRoot();
		EditorActionPlaybackControllerPtr.Reset();
	}
	Cleanup();
}

void FT4BaseViewModel::OnReset() // #79
{
	Reset();
	if (nullptr != ViewportClientRef)
	{
		// #68 : 컨트롤 캐릭터가 변경되면 SetUpdateCameraForPlayer 를 제어해준다. 
		//       ViewportClient 가 EnableCameraLock 설정에 따라 Tick 여부가 결정된다.
		// #86 : Player 가 사라짐으로 ViewportClient Update 를 리셋해준다.
		ViewportClientRef->SetUpdateCameraForPlayer(false);
	}
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr != GameFrame)
	{
		GameFrame->OnReset();
	}
}

void FT4BaseViewModel::OnStartPlay(FT4RehearsalViewportClient* InViewportClient)
{
	check(ET4LayerType::Max != LayerType);
	check(nullptr != GameFrameOwner);
	check(nullptr == ViewportClientRef);
	check(nullptr != InViewportClient);
	ViewportClientRef = InViewportClient;
	ViewportClientResetHandle = ViewportClientRef->GetOnDestroy().AddRaw(
		this, 
		&FT4BaseViewModel::HandleOnDestroyViewportClient
	); // #79

	{
		// #87
		GameFrameOwner->SetEditorViewportClient(ViewportClientRef);
		GameFrameOwner->OnStartPlay();
		RegisterPlayerViewTargetChanged();
	}

	StartPlay(); // #76, #86
}

IT4GameFrame* FT4BaseViewModel::CreateGameFrame() // #87
{
	check(ET4LayerType::Max == LayerType);
	check(nullptr == GameFrameOwner);

	FT4WorldConstructionValues WorldConstructionValues; // #87
	{
		WorldConstructionValues.GameWorldType = ET4GameWorldType::Preview;
		WorldConstructionValues.bPreviewThumbnailMode = (ET4ViewModelEditMode::Preview == GetEditMode()) ? true : false;

	}
	SetupStartWorld(WorldConstructionValues); // #87

	check(ET4GameWorldType::None != WorldConstructionValues.GameWorldType);
	GameFrameOwner = T4FrameCreate(ET4FrameType::Frame_Client, WorldConstructionValues);
	check(nullptr != GameFrameOwner);
	LayerType = GameFrameOwner->GetLayerType();
	check(ET4LayerType::Max != LayerType);
	return GameFrameOwner;
}

IT4GameWorld* FT4BaseViewModel::GetGameWorld() const // #93
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return nullptr;
	}
	return GameFrame->GetGameWorld();
}

void FT4BaseViewModel::OnDrawHUD(
	FViewport* InViewport,
	FCanvas* InCanvas,
	FT4HUDDrawInfo* InOutDrawInfo
)  // #59, #83
{
	DrawHUD(InViewport, InCanvas, InOutDrawInfo);

	// #68 :  UGameViewportClient::Draw 처리를 대신해준다.
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr != GameFrame)
	{
		GameFrame->OnDrawHUD(InViewport, InCanvas, *InOutDrawInfo);
	}
}

//#76
void FT4BaseViewModel::SetViewportShowOptionCapsule(bool bShow)
{
	IT4GameObject* PlayerObject = GetPlayerObject();
	if (nullptr == PlayerObject)
	{
		return;
	}
	FT4GameObjectDebugInfo& CurrentDebugInfo = PlayerObject->GetDebugInfo();
	if (bShow)
	{
		CurrentDebugInfo.DebugBitFlags |= ET4EngineDebugFlag::Debug_Object_Capsule_Bit;
	}
	else
	{
		CurrentDebugInfo.DebugBitFlags &= ~ET4EngineDebugFlag::Debug_Object_Capsule_Bit;
	}
}

bool FT4BaseViewModel::IsShownViewportShowOptionCapsule() const
{ 
	IT4GameObject* PlayerObject = GetPlayerObject();
	if (nullptr == PlayerObject)
	{
		return false;
	}
	FT4GameObjectDebugInfo& CurrentDebugInfo = PlayerObject->GetDebugInfo();
	const bool bShowCapsule
		= (CurrentDebugInfo.DebugBitFlags & ET4EngineDebugFlag::Debug_Object_Capsule_Bit) ? true : false;
	return bShowCapsule;
} 
//#76

void FT4BaseViewModel::SetGameWorldTimeStop(bool bPause) // #94
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return;
	}
	GameWorld->SetDisableTimelapse(bPause);
	GameWorld->SetDisableEnvironmentUpdating(bPause);
}

bool FT4BaseViewModel::IsGameWorldTimeStopped() const // #94
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return false;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return false;
	}
	return GameWorld->GetTimelapseDisabled();
}

void FT4BaseViewModel::SetGameWorldTimelapseScale(float InScale) // #93
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return;
	}
	GameWorld->GetController()->SetGameTimeScale(InScale);
}

float FT4BaseViewModel::GetGameWorldTimelapseScale() const // #93
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return 1.0f;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return 1.0f;
	}
	return GameWorld->GetController()->GetGameTimeScale();
}

void FT4BaseViewModel::SetGameWorldTimeHour(float InHour) // #93
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return;
	}
	GameWorld->GetController()->SetGameTimeHour(InHour);
}

float FT4BaseViewModel::GetGameWorldTimeHour() const // #93
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return 0.0f;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return 0.0f;
	}
	return GameWorld->GetController()->GetGameTimeHour();
}

void FT4BaseViewModel::ServerDespawnAll(bool bClearPlayerObject) // #68
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return;
	}
	IT4GameplayInstance* GameplayInstance = GameFrame->GetGameplayInstance();
	if (nullptr == GameplayInstance)
	{
		return;
	}
	IT4EditorGameData* EditorGameData = GameplayInstance->GetEditorGameData();
	if (nullptr == EditorGameData)
	{
		return;
	}
	EditorGameData->DoDespawnAll(bClearPlayerObject);
}

UT4EditorActionPlaybackController* FT4BaseViewModel::GetActionPlaybackController() 
{ 
	// #68
	if (!EditorActionPlaybackControllerPtr.IsValid())
	{
		EditorActionPlaybackControllerPtr = NewObject<UT4EditorActionPlaybackController>(); // #68
		EditorActionPlaybackControllerPtr->SetFlags(RF_Transactional); // Undo, Redo
		EditorActionPlaybackControllerPtr->Set(
			GetLayerType(),
			GetActionPlaybackAssetName(),
			GetActionPlaybackFolderName()
		);
		EditorActionPlaybackControllerPtr->AddToRoot();
	}
	return EditorActionPlaybackControllerPtr.Get();
}

bool FT4BaseViewModel::GetValidSpawnLocation(
	const FVector& InOriginLocation, 
	const FVector2D& InRange,
	int32 InTryCount, 
	FVector& OutSpawnLocation
) // #76
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return false;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return false;
	}
	IT4WorldNavigationSystem* NavigationSystem = GameWorld->GetNavigationSystem();
	if (nullptr == NavigationSystem)
	{
		return false;
	}
	while (InTryCount)
	{
		if (NavigationSystem->GetRandomLocation(
			InOriginLocation, 
			FMath::RandRange(InRange.X, InRange.Y),
			OutSpawnLocation
		))
		{
			return true;
		}
		InTryCount--;
	}
	return false;
}

void FT4BaseViewModel::SelectPointOfInterest(int32 InIndex) // #100
{
	UObject* EditObject = GetEditObject();
	if (nullptr == EditObject)
	{
		return;
	}
	FT4EditorTestAutomation* TestAutomation = GetTestAutomation();
	if (nullptr == TestAutomation)
	{
		return;
	}
	FT4EditorPointOfInterest CurrentData;
	if (!T4AssetUtil::GetPointOfInterest(TestAutomation, InIndex, &CurrentData))
	{
		return;
	}
	TestAutomation->TransientName = CurrentData.Name;
}

void FT4BaseViewModel::UpdatePointOfInterest(int32 InIndex) // #100
{
	UObject* EditObject = GetEditObject();
	if (nullptr == EditObject)
	{
		return;
	}
	FT4EditorTestAutomation* TestAutomation = GetTestAutomation();
	if (nullptr == TestAutomation)
	{
		return;
	}
	FT4EditorPointOfInterest UpdatePOIData;
	if (!GetPOIInfo(&UpdatePOIData))
	{
		return;
	}
	T4AssetUtil::UpdatePointOfInterest(EditObject, TestAutomation, InIndex, &UpdatePOIData);
}

void FT4BaseViewModel::TravelPointOfInterest(int32 InIndex) // #100
{
	UObject* EditObject = GetEditObject();
	if (nullptr == EditObject)
	{
		return;
	}
	FT4EditorTestAutomation* TestAutomation = GetTestAutomation();
	if (nullptr == TestAutomation)
	{
		return;
	}
	FT4EditorPointOfInterest POIData;
	if (!T4AssetUtil::GetPointOfInterest(TestAutomation, InIndex, &POIData))
	{
		return;
	}
	TravelPOI(&POIData);
}

int32 FT4BaseViewModel::AddPointOfInterest() // #100
{
	UObject* EditObject = GetEditObject();
	if (nullptr == EditObject)
	{
		return -1;
	}
	FT4EditorTestAutomation* TestAutomation = GetTestAutomation();
	if (nullptr == TestAutomation)
	{
		return -1;
	}
	FT4EditorPointOfInterest NewPOIData;
	if (!GetPOIInfo(&NewPOIData))
	{
		return -1;
	}
	NewPOIData.Name = TestAutomation->TransientName;
	T4AssetUtil::AddPointOfInterest(EditObject, TestAutomation, &NewPOIData);
	return TestAutomation->PointOfInterests.Num() - 1;
}

void FT4BaseViewModel::RemovePointOfInterest(int32 InIndex) // #100
{
	UObject* EditObject = GetEditObject();
	if (nullptr == EditObject)
	{
		return;
	}
	FT4EditorTestAutomation* TestAutomation = GetTestAutomation();
	if (nullptr == TestAutomation)
	{
		return;
	}
	T4AssetUtil::RemovePointOfInterest(EditObject, TestAutomation, InIndex);
}

void FT4BaseViewModel::TravelPOI(FT4EditorPointOfInterest* InPOIData) // #100, #103
{
	check(nullptr != InPOIData)
	IT4GameWorld* GameWorld = GetGameWorld();
	if (nullptr == GameWorld)
	{
		return;
	}
	if (GameWorld->GetEntityKeyName() == InPOIData->MapEntityName)
	{
		IT4GameObject* PlayerObject = GetPlayerObject();
		if (nullptr != PlayerObject)
		{
			PlayerObject->GetPawn()->SetActorRotation(InPOIData->SpawnRotation); // TODO : Action
			FVector TeleportLocation = InPOIData->SpawnLocation;
			TeleportLocation.Z += 100.0f;
			ClientTeleport(InPOIData->SpawnLocation);
			GameWorld->GetController()->SetGameTimeHour(InPOIData->GameTimeHour);
			return;
		}
	}
	bTestAutomation = true;
	TestAutomationGameTimeHour = InPOIData->GameTimeHour; // #103
	TestAutomationSpawnLocation = InPOIData->SpawnLocation; // #100
	TestAutomationSpawnRotation = InPOIData->SpawnRotation; // #100
	if (InPOIData->MapEntityName != NAME_None)
	{
		ClientWorldTravel(FT4EntityKey(ET4EntityType::Map, InPOIData->MapEntityName));
	}
	else
	{
		ClientWorldTravel(nullptr); // preview
	}
}

bool FT4BaseViewModel::GetPOIInfo(FT4EditorPointOfInterest* OutPOIData) // #100, #103
{
	check(nullptr != OutPOIData);
	IT4GameWorld* GameWorld = GetGameWorld();
	if (nullptr == GameWorld)
	{
		return false;
	}
	IT4GameObject* PlayerObject = GetPlayerObject();
	if (nullptr == PlayerObject)
	{
		return false;
	}
	FT4EditorPointOfInterest NewData;
	OutPOIData->MapEntityName = GameWorld->GetEntityKeyName();
	OutPOIData->GameTimeHour = GameWorld->GetController()->GetGameTimeHour();
	OutPOIData->SpawnLocation = PlayerObject->GetRootLocation();
	OutPOIData->SpawnRotation = PlayerObject->GetRotation();
	return true;
}

bool FT4BaseViewModel::ServerSpawnObject(const FName& InGameDataID) // #60
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return false;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return false;
	}
	FVector SpawnLocation = FVector::ZeroVector;
	FVector OriginPosition = FVector::ZeroVector;
	IT4PlayerController* PlayerController = GameFrame->GetPlayerController();
	if (nullptr != PlayerController && PlayerController->HasGameObject())
	{
		OriginPosition = PlayerController->GetGameObject()->GetRootLocation();
	}
	if (!GetValidSpawnLocation(OriginPosition, FVector2D(150.0f, 500.0f), 5, SpawnLocation))
	{
		return false;
	}
	IT4GameplayInstance* GameplayInstance = GameFrame->GetGameplayInstance();
	if (nullptr == GameplayInstance)
	{
		return false;
	}
	IT4EditorGameData* EditorGameData = GameplayInstance->GetEditorGameData();
	if (nullptr == EditorGameData)
	{
		return false;
	}
	bool bResult = EditorGameData->DoNPCSpawn(InGameDataID, SpawnLocation);
	return bResult;
}

bool FT4BaseViewModel::ServerEquipWeapon(
	ET4LayerType InLayerType,
	const FName& InWeaponGameDataID,
	bool bInUnEquip
) // #60
{
	IT4GameFrame* FoundFramework = T4FrameGet(InLayerType);
	if (nullptr == FoundFramework)
	{
		return false;
	}
	IT4GameWorld* GameWorld = FoundFramework->GetGameWorld();
	if (nullptr == GameWorld || !GameWorld->HasPlayerObject())
	{
		return false;
	}
	IT4GameplayInstance* GameplayInstance = FoundFramework->GetGameplayInstance();
	if (nullptr == GameplayInstance)
	{
		return false;
	}
	IT4EditorGameData* EditorGameData = GameplayInstance->GetEditorGameData();
	if (nullptr == EditorGameData)
	{
		return false;
	}
	bool bResult = EditorGameData->DoEquipWeaponItem(InWeaponGameDataID, bInUnEquip);
	return bResult;
}

bool FT4BaseViewModel::ServerEquipWeapon(const FName& InWeaponGameDataID, bool bInUnEquip) // #60
{
	bool bResult = ServerEquipWeapon(LayerType, InWeaponGameDataID, bInUnEquip);
	return bResult;
}

bool FT4BaseViewModel::ClientIsPlayingAction(const FT4ActionKey& InActionKey) // #56
{
	IT4GameObject* PlayerGameObject = GetPlayerObject();
	if (nullptr == PlayerGameObject)
	{
		return false;
	}
	IT4ActionControl* ActionControl = PlayerGameObject->GetActionControl();
	if (nullptr == ActionControl)
	{
		return false;
	}
	return ActionControl->IsPlaying(InActionKey);
}

void FT4BaseViewModel::ClientSetPauseObject(
	ET4LayerType InLayerType,
	bool bInPause
) // #54
{
	IT4GameObject* PlayerGameObject = GetPlayerObject(InLayerType);
	if (nullptr == PlayerGameObject)
	{
		return;
	}
	PlayerGameObject->SetDebugPause(bInPause);
}

void FT4BaseViewModel::ClientSetPauseObject(
	bool bInPause
) // #54
{
	ClientSetPauseObject(LayerType, bInPause);
}

void FT4BaseViewModel::ClientStopAction(
	ET4LayerType InLayerType, 
	const FT4ActionKey& InActionKey
)
{
	IT4GameObject* PlayerGameObject = GetPlayerObject(InLayerType);
	if (nullptr == PlayerGameObject)
	{
		return;
	}
	FT4StopAction NewAction;
	NewAction.ActionKey = InActionKey;
	NewAction.StartTimeSec = 0.0f;
	PlayerGameObject->DoExecuteAction(&NewAction);
}

void FT4BaseViewModel::ClientStopAction(const FT4ActionKey& InActionKey)
{
	ClientStopAction(LayerType, InActionKey);
}

IT4GameObject* FT4BaseViewModel::ClientSpawnObjectEx(
	const FT4EntityKey& InEntityKey,
	const FVector& InLocation,
	const FRotator& InRotation,
	const FName InStanceName,
	bool bPlayer
) // #83
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return nullptr;
	}
	ET4EntityType SelectEntityType = InEntityKey.Type;
	if (!IsSpawnable(SelectEntityType))
	{
		return nullptr;
	}
	const UT4EntityAsset* EntityAsset = T4AssetEntityManagerGet()->GetEntity(InEntityKey);
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return nullptr;
	}
	IT4WorldNavigationSystem* NavigationSystem = GameWorld->GetNavigationSystem();
	if (nullptr == NavigationSystem)
	{
		return nullptr;
	}
	FVector SpawnLocation;
	if (!NavigationSystem->ProjectPoint(InLocation, INVALID_NAVEXTENT, SpawnLocation))
	{
		return nullptr;
	}
	FT4SpawnObjectAction NewAction;
	NewAction.ObjectID = GameFrame->GenerateObjectIDForServer();
	NewAction.Name = TEXT("T4EntityObject");
	NewAction.EntityType = SelectEntityType;
	NewAction.EntityAssetPath = FSoftObjectPath(EntityAsset);
	NewAction.StanceName = InStanceName; // #73
	NewAction.GameDataIDName = TEXT("T4RehearsalSendback");
	NewAction.SpawnLocation = SpawnLocation;
	NewAction.SpawnRotation = InRotation;
	NewAction.bPlayer = bPlayer;
	bool bResult = GameWorld->DoExecuteAction(&NewAction);
	if (bResult)
	{
		IT4PlayerController* PlayerController = GameFrame->GetPlayerController();
		if (nullptr != PlayerController)
		{
			if (!PlayerController->HasGameObject())
			{
				if (IsControllable(SelectEntityType)) // #94 : 컨트롤 가능한 Entity
				{
					PlayerController->SetGameObject(NewAction.ObjectID);
				}
				else
				{
					// #94 : 컨트롤이 불가능한 모델이라면 뷰포트 카메라라도 옮겨주자.
					if (nullptr != ViewportClientRef)
					{
						ViewportClientRef->SetViewLocation(SpawnLocation + FVector(0.0f, 0.0f, 200.0f));
					}
					GameFrame->SetInputControlLock(false); // #94 : 자동 스폰이 되지 않음으로 Control Lock 을 풀어준다.
				}
			}
		}
	}
	return GameWorld->GetContainer()->FindGameObject(NewAction.ObjectID);
}

bool FT4BaseViewModel::ClientSpawnObject(
	UT4EntityAsset* InEntityAsset, 
	const FName InStanceName
)
{
	check(nullptr != InEntityAsset);
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return false;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return false;
	}
	IT4WorldNavigationSystem* NavigationSystem = GameWorld->GetNavigationSystem();
	if (nullptr == NavigationSystem)
	{
		return false;
	}
	FVector SpawnLocation = FVector::ZeroVector;
	IT4PlayerController* PlayerController = GameFrame->GetPlayerController();
	if (nullptr != PlayerController && PlayerController->HasGameObject())
	{
		FVector OriginPosition = FVector::ZeroVector;
		OriginPosition = PlayerController->GetGameObject()->GetRootLocation();
		if (!NavigationSystem->GetRandomLocation(OriginPosition, 500.0f, SpawnLocation))
		{
			return false;
		}
	}
	FT4EntityKey SpawnEntityKey(InEntityAsset->GetEntityType(), InEntityAsset->GetEntityKeyPath());
	IT4GameObject* GameObject = ClientSpawnObjectEx(SpawnEntityKey, SpawnLocation, FRotator::ZeroRotator, InStanceName, false);
	return (nullptr != GameObject) ? true : false;
}

void FT4BaseViewModel::ClientDespawnObject(const FT4ObjectID InObjectID) // #67
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return;
	}
	IT4GameObject* EnteredObject = GameWorld->GetContainer()->FindGameObject(InObjectID);
	if (nullptr == EnteredObject)
	{
		return;
	}
	bool bIsMyPC = GameWorld->IsPlayerObject(EnteredObject);
	if (bIsMyPC)
	{
		IT4PlayerController* MyPC = GameFrame->GetPlayerController();
		check(nullptr != MyPC);
		MyPC->ClearGameObject(true);
	}
	FT4DespawnObjectAction NewAction;
	NewAction.ObjectID = InObjectID;
	NewAction.FadeOutTimeSec = 0.0f;
	GameWorld->DoExecuteAction(&NewAction);
}

bool FT4BaseViewModel::ClientPlayAnimSequence(UAnimSequence* InAnimSequence)
{
	// #39
	IT4GameObject* PlayerGameObject = GetPlayerObject();
	if (nullptr == PlayerGameObject)
	{
		return false;
	}
	IT4AnimControl* AnimControl = PlayerGameObject->GetAnimControl();
	check(nullptr != AnimControl);
	bool bResult = AnimControl->EditorPlayAnimation(InAnimSequence, 1.0f, 0.15f, 0.15f);
	return bResult;
}

void FT4BaseViewModel::ClientPlayConti(
	ET4LayerType InLayerType,
	UT4ContiAsset* InContiAsset,
	const FT4ActionKey& InActionKey,
	const FT4ActionParameters* InActionParameters,
	bool bOverride
) // #39, #56
{
	IT4GameObject* PlayerGameObject = GetPlayerObject(InLayerType);
	if (nullptr == PlayerGameObject)
	{
		return;
	}
	FT4ContiAction NewAction;
	NewAction.ActionKey = InActionKey;
	if (bOverride) // #56
	{
		FT4StopAction StopAction;
		StopAction.ActionKey = NewAction.ActionKey;
		StopAction.StartTimeSec = 0.0f;
		PlayerGameObject->DoExecuteAction(&StopAction);
	}
	NewAction.ContiAsset = InContiAsset;
	PlayerGameObject->DoExecuteAction(&NewAction, InActionParameters);
}

void FT4BaseViewModel::ClientPlayConti(
	UT4ContiAsset* InContiAsset,
	const FT4ActionKey& InActionKey,
	const FT4ActionParameters* InActionParameters,
	bool bOverride
) // #39, #56
{
	ClientPlayConti(LayerType, InContiAsset, InActionKey, InActionParameters, true);
}

void FT4BaseViewModel::ClientPlayConti(
	UT4ContiAsset* InContiAsset,
	const FT4ActionParameters* InActionParameters
) // #39, #56
{
	static const FT4ActionKey DoPlayContiPrimaryKey(TEXT("DoPlayContiPrimaryKey"), true);
	ClientPlayConti(LayerType, InContiAsset, DoPlayContiPrimaryKey, InActionParameters, true);
}

void FT4BaseViewModel::ClientEquipWeapon(
	UT4EntityAsset* InWeaponEntity,
	FName InEquipPointName,
	bool bEquip
) // #72
{
	if (ET4EntityType::Weapon != InWeaponEntity->GetEntityType())
	{
		return;
	}
	IT4GameObject* PlayerGameObject = GetPlayerObject();
	if (nullptr == PlayerGameObject)
	{
		return;
	}
	const FString EditorEquipItemActionKeyName = FString::Printf(
		TEXT("Editor%s%sEquipped"),
		*(InWeaponEntity->GetEntityKeyPath().ToString()),
		*InEquipPointName.ToString()
	);
	const FT4ActionKey EditorEquipItemActionKey(EditorEquipItemActionKeyName);
	if (!bEquip)
	{
		FT4UnEquipWeaponAction NewAction;
		NewAction.ActionKey = EditorEquipItemActionKey; // #48
		NewAction.StartTimeSec = 0.0f;
		PlayerGameObject->DoExecuteAction(&NewAction);
	}
	else
	{
		FT4EquipWeaponAction NewAction;
		NewAction.ActionKey = EditorEquipItemActionKey;
		NewAction.WeaponEntityAsset = Cast<UT4WeaponEntityAsset>(InWeaponEntity);
		NewAction.EquipPoint = InEquipPointName;
		NewAction.LifecycleType = ET4LifecycleType::Looping;
		PlayerGameObject->DoExecuteAction(&NewAction);
	}
}

void FT4BaseViewModel::ClientExchangeCostume(
	UT4EntityAsset* InCostumeEntity,
	FName InCompositePartName,
	bool bSet
) // #72
{
	if (nullptr != InCostumeEntity)
	{
		if (ET4EntityType::Costume != InCostumeEntity->GetEntityType())
		{
			return;
		}
	}
	IT4GameObject* PlayerGameObject = GetPlayerObject();
	if (nullptr == PlayerGameObject)
	{
		return;
	}
	const FString EditorExchangeItemActionKeyName = FString::Printf(
		TEXT("Editor%s%sExchanged"),
		(nullptr != InCostumeEntity) ? *(InCostumeEntity->GetEntityKeyPath().ToString()) : TEXT("Default"),
		*InCompositePartName.ToString()
	);
	const FT4ActionKey EditorExchangeItemActionKey(EditorExchangeItemActionKeyName);
	if (!bSet)
	{
		FT4ExchangeCostumeAction NewAction;
		NewAction.ActionKey = EditorExchangeItemActionKey;
		NewAction.TargetPartsName = InCompositePartName;
		NewAction.bClearDefault = true;
		PlayerGameObject->DoExecuteAction(&NewAction);
	}
	else
	{
		FT4ExchangeCostumeAction NewAction;
		NewAction.ActionKey = EditorExchangeItemActionKey;
		NewAction.CostumeEntityAsset = Cast<UT4CostumeEntityAsset>(InCostumeEntity);
		NewAction.TargetPartsName = InCompositePartName;
		NewAction.bClearDefault = false;
		PlayerGameObject->DoExecuteAction(&NewAction);
	}
}

void FT4BaseViewModel::ClientChangeStance(FName InStanceName) // #73
{
	IT4GameObject* PlayerGameObject = GetPlayerObject();
	if (nullptr == PlayerGameObject)
	{
		return;
	}
	FT4ChangeStanceAction NewAction;
	NewAction.StanceName = InStanceName;
	PlayerGameObject->DoExecuteAction(&NewAction);
}

void FT4BaseViewModel::ClientGetPlayerStanceList(TSet<FName>& OutStanceNamelist) // #73
{
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return;
	}
	IT4GameObject* PlayerObject = PlayerController->GetGameObject();
	if (nullptr == PlayerObject)
	{
		return;
	}
	// TODO : refactoring!
	const UT4EntityAsset* EntityAsset = PlayerObject->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return;
	}
	const ET4EntityType SelectEntityType = EntityAsset->GetEntityType();
	if (ET4EntityType::Character != SelectEntityType)
	{
		return;
	}
	const UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(EntityAsset);
	if (nullptr == CharacterEntityAsset)
	{
		return;
	}
	const FT4EntityCharacterStanceSetData& StanceSetData = CharacterEntityAsset->StanceSetData;
	for (TMap<FName, FT4EntityCharacterStanceData>::TConstIterator It = StanceSetData.StanceMap.CreateConstIterator(); It; ++It)
	{
		OutStanceNamelist.Add(It.Key());
	}
}

void FT4BaseViewModel::ClientPlayReaction(
	FName InReactionName,
	ET4EntityReactionType InReactionType,
	const FVector& InShotDirection
) // #76
{
	IT4GameObject* PlayerGameObject = GetPlayerObject();
	if (nullptr == PlayerGameObject)
	{
		return;
	}
	switch (InReactionType)
	{
		case ET4EntityReactionType::Die:
			{
				FT4DieAction NewAction;
				NewAction.ReactionName = InReactionName;
				NewAction.ShotDirection = InShotDirection;
				NewAction.bTransientPlay = true;
				PlayerGameObject->DoExecuteAction(&NewAction);
			}
			break;

		case ET4EntityReactionType::Resurrect:
			{
				FT4ResurrectAction NewAction;
				NewAction.ReactionName = InReactionName;
				NewAction.bTransientPlay = true;
				PlayerGameObject->DoExecuteAction(&NewAction);
			}
			break;

		case ET4EntityReactionType::Hit:
			{
				FT4HitAction NewAction;
				NewAction.ReactionName = InReactionName;
				NewAction.ShotDirection = InShotDirection;
				NewAction.bTransientPlay = true;
				PlayerGameObject->DoExecuteAction(&NewAction);
			}
			break;

		default:
			{
				UE_LOG(
					LogT4RehearsalEditor,
					Error,
					TEXT("FT4BaseViewModel::ClientPlayReaction '%u' failed. no implementation."),
					uint8(InReactionType)
				);
				return;
			}
			break;
	}
}

bool FT4BaseViewModel::ClientTeleport(const FVector& InLocation) // #86
{
	IT4GameObject* PlayerGameObject = GetPlayerObject();
	if (nullptr == PlayerGameObject)
	{
		return false;
	}
	FT4TeleportAction NewAction;
	NewAction.TargetLocation = InLocation;
	PlayerGameObject->DoExecuteAction(&NewAction);
	return true;
}

bool FT4BaseViewModel::ClientTeleport(const FVector2D& InLocation) // #90
{
	IT4GameObject* PlayerGameObject = GetPlayerObject();
	if (nullptr == PlayerGameObject)
	{
		return false;
	}
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return false;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return false;
	}
	FCollisionQueryParams DistanceTraceParams = FCollisionQueryParams(
		FName(TEXT("ClientTeleport")),
		true,
		PlayerGameObject->GetPawn()
	);
	FVector TestLocation = FVector::ZeroVector;
	TestLocation.X = InLocation.X;
	TestLocation.Y = InLocation.Y;
	FVector TestStartLocation = TestLocation;
	FVector TestEndLocation = TestLocation;
	TestStartLocation.Z += 1000000.0f;
	TestEndLocation.Z -= 1000000.0f;
	FT4HitSingleResult HitResult;
	bool bResult = GameWorld->GetCollisionSystem()->QueryLineTraceSingle(
		ET4CollisionChannel::CollisionVisibility,
		TestStartLocation,
		TestEndLocation,
		DistanceTraceParams,
		HitResult
	);
	if (!bResult)
	{
		return false;
	}
	FT4TeleportAction NewAction;
	NewAction.TargetLocation = HitResult.ResultLocation;
	PlayerGameObject->DoExecuteAction(&NewAction);
	return true;
}

void FT4BaseViewModel::ClientPlayLayerTag(
	FName InLayerTagName, 
	ET4LayerTagType InLayerTagType
) // #81
{
	IT4GameObject* PlayerGameObject = GetPlayerObject();
	if (nullptr == PlayerGameObject)
	{
		return;
	}
	FString ActionString = FString::Printf(
		TEXT("EditorLayerSet_%s_u"),
		*(InLayerTagName.ToString()),
		uint8(InLayerTagType)
	);
	FT4EditorAction NewAction;
	NewAction.ActionKey = FT4ActionKey(ActionString);
	NewAction.EditorActionType = ET4EditorAction::LayerSet;
	NewAction.LayerTagName = InLayerTagName;
	NewAction.LayerTagType = InLayerTagType;
	PlayerGameObject->DoExecuteAction(&NewAction);
}

void FT4BaseViewModel::ClientStopLayerTag(
	FName InLayerTagName,
	ET4LayerTagType InLayerTagType
) // #81
{
	IT4GameObject* PlayerGameObject = GetPlayerObject();
	if (nullptr == PlayerGameObject)
	{
		return;
	}
	FString ActionString = FString::Printf(
		TEXT("EditorLayerSet_%s_u"),
		*(InLayerTagName.ToString()),
		uint8(InLayerTagType)
	);
	FT4EditorAction NewAction;
	NewAction.ActionKey = FT4ActionKey(ActionString);
	NewAction.EditorActionType = ET4EditorAction::LayerSetClear;
	NewAction.LayerTagName = InLayerTagName;
	NewAction.LayerTagType = InLayerTagType;
	PlayerGameObject->DoExecuteAction(&NewAction);
}

void FT4BaseViewModel::SavePlayerSettingsInfo() // #87
{
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr != PlayerController && PlayerController->HasGameObject())
	{
		bCachedPlayerSettingsSaved = true;
		PlayerController->GetCameraInfoCached(CachedCameraControlRotation, CachedCameraZoomDistance);
		CachedPlayerRotation = PlayerController->GetGameObject()->GetRotation();
	}
}

void FT4BaseViewModel::RestorePlayerSettingsInfo() // #87
{
	if (!bCachedPlayerSettingsSaved)
	{
		return;
	}
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr != PlayerController)
	{
		PlayerController->SetCameraInfoCached(CachedCameraControlRotation, CachedCameraZoomDistance);
	}
	bCachedPlayerSettingsSaved = false;
}

bool FT4BaseViewModel::ClientWorldTravel(const UT4EntityAsset* InEntityAsset) // #79
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return false;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return false;
	}
	const UT4MapEntityAsset* MapEntityAsset = nullptr;
	if (nullptr != InEntityAsset)
	{
		if (IsSpawnable(InEntityAsset->GetEntityType()))
		{
			return false;
		}
		MapEntityAsset = Cast<UT4MapEntityAsset>(InEntityAsset);
		check(nullptr != MapEntityAsset);
		const FT4EntityMapData& MapData = MapEntityAsset->MapData;
		if (MapData.LevelAsset.IsNull())
		{
			return false;
		}
	}
	SavePlayerSettingsInfo(); // #87
	HandleOnViewTargetChanged(nullptr);
	GameFrame->OnWorldTravel(MapEntityAsset); // #87
	RegisterPlayerViewTargetChanged();
	RestartPlay(); // #94
	return true;
}

bool FT4BaseViewModel::ClientWorldTravel(const FT4EntityKey& InMapEntityKey) // #87
{
	IT4EntityManager* EntityManager = T4AssetEntityManagerGet();
	check(nullptr != EntityManager);
	const UT4EntityAsset* EntityAsset = EntityManager->GetEntity(InMapEntityKey);
	if (nullptr == EntityAsset)
	{
		return false;
	}
	return ClientWorldTravel(EntityAsset);
}

void FT4BaseViewModel::ClientEditorAction(ET4EditorAction InEditorActionType) // #71
{
	IT4GameObject* PlayerGameObject = GetPlayerObject();
	if (nullptr == PlayerGameObject)
	{
		return;
	}
	FT4EditorAction NewAction;
	NewAction.EditorActionType = InEditorActionType;
	PlayerGameObject->DoExecuteAction(&NewAction);
}

bool FT4BaseViewModel::TryValidSpawnObjectLocation(FVector& OutLocation) // #87
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return false;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return false;
	}
	IT4WorldController* WorldController = GameWorld->GetController();
	if (nullptr == WorldController)
	{
		return false;
	}
	FVector SpawnLocation = FVector::ZeroVector;
	if (!WorldController->IsPreviewScene()) // #87 : PreviewScene 은 원점!
	{
		IT4WorldNavigationSystem* NavigationSystem = GameWorld->GetNavigationSystem();
		if (nullptr == NavigationSystem)
		{
			return false;
		}
		if (!NavigationSystem->GetRandomLocation(SpawnLocation))
		{
			return false;
		}
	}
	IT4WorldCollisionSystem* CollisionSystem = GameWorld->GetCollisionSystem();
	if (nullptr == CollisionSystem)
	{
		return false;
	}
	FCollisionQueryParams DistanceTraceParams = FCollisionQueryParams(
		FName(TEXT("TryValidSpawnObjectLocation")),
		true
	);
	DistanceTraceParams.bTraceComplex = false;
	DistanceTraceParams.bReturnPhysicalMaterial = false;
	FVector TestStartLocation = SpawnLocation;
	FVector TestEndLocation = SpawnLocation;
	TestStartLocation.Z += 10000.0f;
	TestEndLocation.Z -= 10000.0f;
	FT4HitSingleResult HitResult;
	bool bResult = CollisionSystem->QueryLineTraceSingle(
		ET4CollisionChannel::CollisionVisibility,
		TestStartLocation,
		TestEndLocation,
		DistanceTraceParams,
		HitResult
	);
	if (!bResult)
	{
		return false; 
	}
	OutLocation = HitResult.ResultLocation;
	return true;
}

UWorld* FT4BaseViewModel::GetWorld() const
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return nullptr;
	}
	return GameFrame->GetWorld();
}

IT4PlayerController* FT4BaseViewModel::GetPlayerController() const
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return nullptr;
	}
	return GameFrame->GetPlayerController();
}

IT4GameObject* FT4BaseViewModel::GetPlayerObject() const
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return nullptr;
	}
	IT4PlayerController* PlayerController = GameFrame->GetPlayerController();
	if (nullptr == PlayerController)
	{
		return nullptr;
	}
	return PlayerController->GetGameObject();
}

IT4GameObject* FT4BaseViewModel::GetPlayerObject(ET4LayerType InLayerType) const
{
	IT4GameFrame* FoundFramework = T4FrameGet(InLayerType);
	if (nullptr == FoundFramework)
	{
		return nullptr;
	}
	IT4GameWorld* GameWorld = FoundFramework->GetGameWorld();
	if (nullptr == GameWorld || !GameWorld->HasPlayerObject())
	{
		return nullptr;
	}
	IT4PlayerController* PlayerController = FoundFramework->GetPlayerController();
	if (nullptr == PlayerController || !PlayerController->HasGameObject())
	{
		return nullptr;
	}
	return PlayerController->GetGameObject();
}

void FT4BaseViewModel::HandleOnDestroyViewportClient()
{
	OnReset();
	ViewportClientRef = nullptr;
}

void FT4BaseViewModel::HandleOnViewTargetChanged(IT4GameObject* InViewTarget)
{
	bool bUseControl = (nullptr != InViewTarget) ? true : false;
	if (nullptr != ViewportClientRef)
	{
		// #68 : 컨트롤 캐릭터가 변경되면 SetUpdateCameraForPlayer 를 제어해준다. 
		//       ViewportClient 가 EnableCameraLock 설정에 따라 Tick 여부가 결정된다.
		ViewportClientRef->SetUpdateCameraForPlayer(bUseControl);
	}
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr != GameFrame)
	{
		IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
		if (nullptr != GameWorld && GameWorld->GetController()->IsPreviewScene())
		{
			// #83 : Player 가 사라지면 HandleOnViewTargetChanged 로 컨트롤을 막기 때문에
			//		 PreviewWorld 는 컨트롤을 막지 않도록 처리한다. World 의 Input 을 사용하기 위함!
			GameFrame->SetInputControlLock(!bUseControl);
		}
		else
		{
			GameFrame->SetInputControlLock(false);
		}
	}
	NotifyViewTargetChanged(InViewTarget); // #87
}

void FT4BaseViewModel::RegisterPlayerViewTargetChanged()
{
	check(nullptr != GameFrameOwner);
	if (ET4ViewModelEditMode::Preview != GetEditMode())
	{
		IT4PlayerController* PlayerController = GameFrameOwner->GetPlayerController();
		if (nullptr != PlayerController)
		{
			PlayerController->GetOnViewTargetChanged().AddRaw(
				this,
				&FT4BaseViewModel::HandleOnViewTargetChanged
			);
		}
	}
	else
	{
		GameFrameOwner->SetInputControlLock(true);
	}
}

#undef LOCTEXT_NAMESPACE