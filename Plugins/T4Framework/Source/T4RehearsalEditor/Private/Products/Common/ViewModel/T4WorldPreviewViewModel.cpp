// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4WorldPreviewViewModel.h"

#include "Products/T4RehearsalEditorUtils.h"

#include "Products/Common/Viewport/T4RehearsalViewportClient.h"

#include "Products/Common/Helper/T4EditorGameplaySettingObject.h" // #60, #104
#include "Products/Common/Helper/T4EditorActionPlaybackController.h" // #68

#include "T4Asset/Public/T4AssetUtils.h" // #39, #62
#include "T4Asset/Classes/Entity/T4MapEntityAsset.h" // #104

#include "T4Engine/Public/Action/T4ActionCodeWorld.h"
#include "T4Engine/Public/T4Engine.h"

#include "T4Frame/Public/T4Frame.h" // #30
#include "T4Frame/Classes/Controller/Player/T4PlayerController.h" // #86

#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "EditorStyleSet.h"
#include "Engine/LevelStreaming.h"
#include "Engine/WorldComposition.h"

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "FT4WorldPreviewViewModel"

/**
  * #83
 */
FT4WorldPreviewViewModelOptions::FT4WorldPreviewViewModelOptions()
	: MapEntityAsset(nullptr)
{
}

FT4WorldPreviewViewModel::FT4WorldPreviewViewModel(const FT4WorldPreviewViewModelOptions& InOptions)
	: MapEntityAsset(InOptions.MapEntityAsset)
	, EditorPlaySettingObject(nullptr) // #60, #104
	, bSimulating(false) // #86
	, bUpdatingReload(false)
	, bDisplayEditorWorldModified(false)
	, LastValidSubLevelPackageName(NAME_None) // #85
	, bPendingRefreshWorld(false) // #86
	, bPendingPlayerRespawn(false) // #83
	, PlayerStanceName(NAME_None) // #83
	, PlayerLocationCached(FVector::ZeroVector) // #83
	, PlayerRotationCached(FRotator::ZeroRotator) // #83
	, ActionPlaybackAssetName(InOptions.ActionPlaybackAssetName) // #104
	, ActionPlaybackFolderName(InOptions.ActionPlaybackFolderName) // #104
	, CachedTimeHour(12.0f) // #104 : RefreshWorld 시 Time 복구
	, CachedTimeScale(1.0f) // #104 : RefreshWorld 시 Time 복구
{
	EditorPlaySettingObject = NewObject<UT4EditorGameplaySettingObject>();
	EditorPlaySettingObject->SetFlags(RF_Transactional); // Undo, Redo

	GEditor->RegisterForUndo(this);
}

FT4WorldPreviewViewModel::~FT4WorldPreviewViewModel()
{
}

void FT4WorldPreviewViewModel::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(MapEntityAsset);
	Collector.AddReferencedObject(EditorPlaySettingObject); // #60
}

void FT4WorldPreviewViewModel::PostUndo(bool bSuccess)
{
	OnRefreshWorld();
}

void FT4WorldPreviewViewModel::Tick(float InDeltaTime)
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return;
	}
	UWorld* PersistentWorld = GameFrame->GetWorld();
	if (nullptr == PersistentWorld)
	{
		return;
	}
	if (!bSimulating)
	{
		UpdateManualLevelStreaming(PersistentWorld, InDeltaTime);
	}
	else
	{
		// #86 : 시뮬레이션 사용은 FT4RehearsalViewportClient::Draw 에서 처리된다. UGameViewportClient
	}
	if (bPendingPlayerRespawn) // #83
	{
		// #83 : 내가 서있는 곳이 유효한지를 따져본다.
		// #86 : Async 로딩이 동작할 수 있기 때문에 계속 retry 한다.
		IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
		if (nullptr != GameWorld)
		{
			// #86 : Mesh Loading 부터 체크하자.
			FCollisionQueryParams DistanceTraceParams = FCollisionQueryParams(
				FName(TEXT("PendingPlayerRespawn")),
				true
			);
			DistanceTraceParams.bTraceComplex = true;
			DistanceTraceParams.bReturnPhysicalMaterial = false;

			FVector TestStartLocation = PlayerLocationCached;
			FVector TestEndLocation = PlayerLocationCached;
			TestStartLocation.Z += 100.0f;
			TestEndLocation.Z -= 100.0f;
			FT4HitSingleResult HitResult;
			bool bResult = GameWorld->GetCollisionSystem()->QueryLineTraceSingle(
				ET4CollisionChannel::CollisionVisibility,
				TestStartLocation,
				TestEndLocation,
				DistanceTraceParams,
				HitResult
			);
			if (bResult)
			{
				FVector NavLocation;
				if (GameWorld->GetNavigationSystem()->ProjectPoint(PlayerLocationCached, INVALID_NAVEXTENT, NavLocation))
				{
					IT4GameObject* PlayerObject = GetPlayerObject();
					if (nullptr == PlayerObject) // 플레이어가 없다면 Refresh World 로 가정 스폰 해준다.
					{
						ClientSpawnObjectEx(PlayerEntityKey, NavLocation, PlayerRotationCached, PlayerStanceName, true);
					}
					bPendingPlayerRespawn = false;
					RestorePlayerSettingsInfo(); // #87
				}
			}
		}
	}
	else
	{
		if (LastValidSubLevelPackageName == NAME_None)
		{
			return; // 활성화된 서브레벨이 없으면 Retry
		}
		IT4GameObject* PlayerObject = GetPlayerObject();
		if (nullptr != PlayerObject && PlayerObject->IsFalling())
		{
			IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
			if (nullptr != GameWorld)
			{
				// #86 : Mesh Loading 부터 체크하자.
				FCollisionQueryParams DistanceTraceParams = FCollisionQueryParams(
					FName(TEXT("SubLevelPlayerTeleport")),
					true,
					PlayerObject->GetPawn()
				);
				DistanceTraceParams.bTraceComplex = false;
				DistanceTraceParams.bReturnPhysicalMaterial = false;

				FVector TestStartLocation = PlayerObject->GetRootLocation();
				FVector TestEndLocation = TestStartLocation;
				TestStartLocation.Z += 1000.0f;
				TestEndLocation.Z -= 1000.0f;
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
					// 충돌이 없다면 안전한 SubLevel 로 이동해준다.
					FWorldTileInfo WorldTileInfo = PersistentWorld->WorldComposition->GetTileInfo(LastValidSubLevelPackageName);
					FVector TestLocation = FVector(WorldTileInfo.Position);
					TestLocation.Z = WorldTileInfo.Bounds.GetCenter().Z;

					TestStartLocation = TestLocation;
					TestEndLocation = TestLocation;
					TestStartLocation.Z += 1000000.0f;
					TestEndLocation.Z -= 1000000.0f;
					bResult = GameWorld->GetCollisionSystem()->QueryLineTraceSingle(
						ET4CollisionChannel::CollisionVisibility,
						TestStartLocation,
						TestEndLocation,
						DistanceTraceParams,
						HitResult
					);
					if (bResult)
					{
						ClientTeleport(HitResult.ResultLocation);
					}
				}
			}
		}
	}
}

void FT4WorldPreviewViewModel::UpdateManualLevelStreaming(
	UWorld* InPersistentWorld,
	float InDeltaTime
) // #86
{
	check(nullptr != InPersistentWorld);
	const TArray<ULevelStreaming*>& StreamingLevels = InPersistentWorld->GetStreamingLevels();
	if (0 >= StreamingLevels.Num())
	{
		return;
	}
	LastValidSubLevelPackageName = NAME_None;
	bool bFlushLevelStreaming = false;
	for (ULevelStreaming* StreamingLevel : StreamingLevels)
	{
		bool bApplySubLevel = false;
		auto Predicate = [&](FName InPackageName)
		{
			return (StreamingLevel->PackageNameToLoad == InPackageName);
		};
		if (FName*const FoundStreamingLevel = VisibleSubLevelPackageNames.FindByPredicate(Predicate))
		{
			bApplySubLevel = true;
		}
		if (StreamingLevel->ShouldBeLoaded() != bApplySubLevel)
		{
			StreamingLevel->SetShouldBeLoaded(bApplySubLevel);
			bFlushLevelStreaming = true;
		}
		if (StreamingLevel->ShouldBeVisible() != bApplySubLevel)
		{
			StreamingLevel->SetShouldBeVisible(bApplySubLevel);
			bFlushLevelStreaming = true;
		}
		if (bApplySubLevel)
		{
			LastValidSubLevelPackageName = StreamingLevel->GetWorldAssetPackageFName();
		}
	}
	if (!bFlushLevelStreaming)
	{
		return;
	}
#if 0
	// #83 : World (SubLevel) 의 경우 DuplicateWorldForPIE 를 하지 않으면 중복된 World 를 띄우면 Guid 가 겹치는 Actor 로 인해
	//       Warning 이 발생하고, Transient 대상이 아니어서 저장이 되는 문제가 발생함.
	//       이 문제를 우회하기 위해, PreviewWorld 에서 Level 로드시 EditorWorld 가 있는지 확인하고, 없다면
	//       원본 (EditorWorld) 패키지를 로드한 후 복제하도록 처리한다. (Persistent World 생성시 EditorWorld 를 먼저 생성해주는 처리와 사실상 같음:FT4RehearsalPreviewWorld::WorldTravel(const TCHAR* InTravelURL))
	for (ULevelStreaming* StreamingLevel : StreamingLevels)
	{
		EPackageFlags PackageFlags = PKG_ContainsMap;
		int32 PIEInstanceID = INDEX_NONE;
		if (StreamingLevel->ShouldBeLoaded() && StreamingLevel->ShouldBeVisible())
		{
			const FName DesiredPackageName = *StreamingLevel->GetWorldAssetPackageName();
			const FString NonPrefixedLevelName = UWorld::StripPIEPrefixFromPackageName(DesiredPackageName.ToString(), InPersistentWorld->StreamingLevelsPrefix);
			UPackage* EditorLevelPackage = FindObjectFast<UPackage>(nullptr, FName(*NonPrefixedLevelName));
			if (nullptr == EditorLevelPackage)
			{
				if (FPackageName::DoesPackageExist(NonPrefixedLevelName))
				{
					// #83 : PerviewWorld SubLevel 로드시 Editor World 의 SubLevel 을 로드한 후 DuplicateWorldForPIE 로 Preview SubLevel  을 로드하는데, 
					//       Editor World 의 SubLevel 로드시 Package 만 로드하고 Editor Persistent World 의 SubLevel 로의 등록을 하지 않은 상태로 캐시에 담겨져 
					//       DuplicateWorldForPIE 되는 PreviewWorld 의 SubLevel 의 좌표도 함께 오류 유발. 이 상태로 저장하면 Editor World 정보까지 문제가 생기는 심각한 버그.
					FWorldContext& EditorContext = GEditor->GetEditorWorldContext();
					UWorld* EditorPersistentWorld = EditorContext.World();
					check(nullptr != EditorPersistentWorld);
					ULevel::StreamedLevelsOwningWorld.Add(*NonPrefixedLevelName, EditorPersistentWorld);
					UWorld::WorldTypePreLoadMap.FindOrAdd(*NonPrefixedLevelName) = EditorPersistentWorld->WorldType;
					{
						LoadPackageAsync(
							NonPrefixedLevelName,
							nullptr,
							*NonPrefixedLevelName,
							nullptr, // FLoadPackageAsyncDelegate::CreateUObject(this, &ULevelStreaming::AsyncLevelLoadComplete), 
							PackageFlags,
							PIEInstanceID
						);
						FlushAsyncLoading();

						EditorLevelPackage = FindObjectFast<UPackage>(nullptr, FName(*NonPrefixedLevelName));
						check(nullptr != EditorLevelPackage);
					}
					UWorld* EditorSubLevelWorld = UWorld::FindWorldInPackage(EditorLevelPackage);
					check(nullptr != EditorSubLevelWorld);
					check(EWorldType::Editor == EditorSubLevelWorld->WorldType);
					UWorld::WorldTypePreLoadMap.Remove(*NonPrefixedLevelName);
					ULevel::StreamedLevelsOwningWorld.Remove(*NonPrefixedLevelName);
				}
			}
		}
	}
#endif

	// #83 : SubLevel 의 DuplicateWorldForPIE 는 PersietentLevel 의 WorldType == PIE 일 경우만 동작함으로 임시로 변경해준다.
	//EWorldType::Type WorldTypeCahched = InPersistentWorld->WorldType;
	//InPersistentWorld->WorldType = EWorldType::PIE;
	{
		InPersistentWorld->bIsLevelStreamingFrozen = false; // 자동 레벨 스트리밍 flag 를 켠다.
		{
			// #104 : 4.24 업데이트 후 Level 의 첫 로딩에서 ULevelStreaming 의 AsyncLevelLoadComplete 시 PIE Instance ID 가 있을 경우
			//        FixupForPIE 를 시도하는데, 이때 GPlayInEditorID != -1 가 아닌 조건으로 ensure 에 걸린다. (PreviewWorld PIE로직을 타지 않기 때문)
			//	      해당 문제를 우회하기 위해 GPlayInEditorID 를 임시로 조작해 FixupForPIE 가 정상동작하도록 처리해준다.
			int32 BackupPlayInEditorID = GPlayInEditorID;
			GPlayInEditorID = InPersistentWorld->GetOutermost()->PIEInstanceID;
			{
				InPersistentWorld->FlushLevelStreaming();
			}
			GPlayInEditorID = BackupPlayInEditorID;
		}
		InPersistentWorld->bIsLevelStreamingFrozen = true; // 자동 레벨 스트리밍 flag 를 끈다.
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	}
	//InPersistentWorld->WorldType = WorldTypeCahched;

#if 0
	// #83 : 서브레벨 로드 처리인 RequestLevel 에서 SubLevel 로드시 PIE 가 아닌 PreviewWorld 의 SubLevel 은 RF_Transient Flag 를 켜지 않기 때문에 
	//       저장이 되며 문제 발생.참고로 PersistentLevel 은 DuplicateWorldForPIE 를 하기 때문에 해당 문제가 없었음.PreviewWorld 에서 SubLevel 로드 후 강제로 RF_Transient Flag 를 켜서 우회함.
	for (ULevelStreaming* StreamingLevel : StreamingLevels)
	{
		ULevel* LoadedLevel = StreamingLevel->GetLoadedLevel();
		if (nullptr != LoadedLevel)
		{
			const FName DesiredPackageName = *StreamingLevel->GetWorldAssetPackageName();
			UPackage* LevelPackage = (UPackage*)StaticFindObjectFast(
				UPackage::StaticClass(), 
				nullptr, DesiredPackageName, 
				0, 
				0, 
				RF_NoFlags,
				EInternalObjectFlags::PendingKill
			);
			if (nullptr != LevelPackage && !LevelPackage->HasAnyFlags(RF_Transient))
			{
				LevelPackage->SetFlags(RF_Transient); // #83 : 혹시라도 저장되지 않도록 방지한다.
			}
		}
	}
#endif

	GetOnViewModelChanged().Broadcast(); // #85
}

TStatId FT4WorldPreviewViewModel::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FT4WorldPreviewViewModel, STATGROUP_Tickables);
}

void FT4WorldPreviewViewModel::Reset() // #79
{
	if (LevelAddedToWorldHandle.IsValid()) // #86
	{
		FWorldDelegates::LevelAddedToWorld.Remove(LevelAddedToWorldHandle);
		LevelAddedToWorldHandle.Reset();
	}
	if (LevelRemovedFromWorldHandle.IsValid()) // #86
	{
		FWorldDelegates::LevelRemovedFromWorld.Remove(LevelRemovedFromWorldHandle);
		LevelRemovedFromWorldHandle.Reset();
	}
}

void FT4WorldPreviewViewModel::StartPlay()
{
	if (bUpdatingReload)
	{
		return;
	}

	// #60, #104
	if (nullptr != EditorPlaySettingObject)
	{
		EditorPlaySettingObject->SetLayerType(GetLayerType());
		EditorPlaySettingObject->SetUseGameplaySettings(false); // #104 : conti 에서만 true, world 에서는 false
		GetGameFrame()->SetEditorGameplayCustomHandler(EditorPlaySettingObject); // #104 : Simulation 과 AI 연동
	}

	SetSimulationMode(bSimulating);

	// #86
	if (!LevelAddedToWorldHandle.IsValid())
	{
		LevelAddedToWorldHandle = FWorldDelegates::LevelAddedToWorld.AddRaw(this, &FT4WorldPreviewViewModel::HandleOnUpdateSubLevel);
	}
	if (!LevelRemovedFromWorldHandle.IsValid())
	{
		LevelRemovedFromWorldHandle = FWorldDelegates::LevelRemovedFromWorld.AddRaw(this, &FT4WorldPreviewViewModel::HandleOnUpdateSubLevel);
	}

	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr != GameFrame)
	{
		AT4PlayerController* EditorPlayerController = GameFrame->GetEditorPlayerController();
		check(nullptr != EditorPlayerController);
		EditorPlayerController->SetCameraZoomMaxScale(3.0f); // #86

		if (bPendingRefreshWorld) // #83
		{
			EditorPlayerController->SetInitialLocationAndRotation(PlayerLocationCached, PlayerRotationCached);

			IT4EditorViewportClient* EditorViewportClient = GameFrame->GetEditorViewportClient();
			check(nullptr != EditorPlayerController);
			EditorViewportClient->SetInitialLocationAndRotation(PlayerLocationCached, PlayerRotationCached);

			if (bSimulating)
			{
				UWorld* PersistentWorld = GameFrame->GetWorld();
				if (nullptr != PersistentWorld && nullptr != PersistentWorld->WorldComposition)
				{
					PersistentWorld->WorldComposition->UpdateStreamingState();
					PersistentWorld->FlushLevelStreaming();
				}
			}
		}
	}

	GetOnSubLevelUpdate().Broadcast(); // #91 : Persistent
}

void FT4WorldPreviewViewModel::RestartPlay() // #94, #104 : 월드 이동후 호출
{
	IT4WorldController* WorldController = GetGameWorld()->GetController();
	check(nullptr != WorldController);
	WorldController->SetGameTimeHour(CachedTimeHour); // #104 : RefreshWorld 시 Time 복구
	WorldController->SetGameTimeScale(CachedTimeScale); // #104 : RefreshWorld 시 Time 복구
}

void FT4WorldPreviewViewModel::DrawHUD(
	FViewport* InViewport,
	FCanvas* InCanvas,
	FT4HUDDrawInfo* InOutDrawInfo
) // #59, #83
{
	check(nullptr != InViewport);
	check(nullptr != InCanvas);

	UFont* DrawFont = GEngine->GetLargeFont();
	check(nullptr != DrawFont);

	const int32 SrcWidth = InViewport->GetSizeXY().X;
	const int32 SrcHeight = InViewport->GetSizeXY().Y;

	int32 StartHeightOffset = 35; // RefreshWorld Button

	if (bSimulating)
	{
		IT4GameFrame* GameFrame = GetGameFrame();
		if (nullptr != GameFrame)
		{
			IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
			if (nullptr != GameWorld)
			{
				FString DrawTimeString = GameWorld->GetController()->GetGameTimeString();
				int32 XL;
				int32 YL;
				StringSize(DrawFont, XL, YL, *DrawTimeString);
				const float DrawX = FMath::FloorToFloat(SrcWidth - XL - 10.0f);
				const float DrawY = SrcHeight - YL - StartHeightOffset;
				InCanvas->DrawShadowedString(DrawX, DrawY, *DrawTimeString, DrawFont, FLinearColor::Green);
				StartHeightOffset += YL + 5;
			}
		}
	}

	if (bDisplayEditorWorldModified) // #83
	{
		const FString DrawString = TEXT("* Modified Editor World");
		int32 XL;
		int32 YL;
		StringSize(DrawFont, XL, YL, *DrawString);
		const float DrawX = FMath::FloorToFloat(SrcWidth - XL - 10.0f);
		const float DrawY = SrcHeight - YL - StartHeightOffset;
		InCanvas->DrawShadowedString(DrawX, DrawY, *DrawString, DrawFont, FLinearColor::Red);
		StartHeightOffset += YL + 5;
	}
}

void FT4WorldPreviewViewModel::Cleanup()
{
	GEditor->UnregisterForUndo(this);
	MapEntityAsset = nullptr;
	EditorPlaySettingObject = nullptr;
}

void FT4WorldPreviewViewModel::NotifyActionPlaybackRec() // #104
{
}

void FT4WorldPreviewViewModel::NotifyActionPlaybackPlay() // #104
{
	if (!bSimulating)
	{
		if (HasActionPlaybackController())
		{
			// #104 : Simulation 을 멈추면 Playback 동작이 되지 않도록 처리
			UT4EditorActionPlaybackController* PlaybackController = GetActionPlaybackController();
			check(nullptr != PlaybackController);
			if (PlaybackController->CanPause())
			{
				PlaybackController->DoPause();
			}
		}
	}
}

void FT4WorldPreviewViewModel::OnRefreshWorld() // #83
{
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr != PlayerController)
	{
		// #86 : ViewTarget 이 없을 경우 카메라 동작 및 Level Streaming 이 동작하지 않음으로 Default Pawn 을 스폰처리해준다.
		//       함께, AT4PlayerController 의 CachedDefaultPawn 를 Client 만 사용하다 모두 사용하도록 함께 정리함
		APawn* DefaultPawn = PlayerController->GetDefaultPawn();
		if (nullptr != DefaultPawn)
		{
			bPendingRefreshWorld = true;
			PlayerLocationCached = DefaultPawn->GetActorLocation();
			PlayerRotationCached = DefaultPawn->GetActorRotation();
		}
		else
		{
			IT4GameObject* PlayerObject = PlayerController->GetGameObject();
			if (nullptr != PlayerObject)
			{
				bPendingRefreshWorld = true;
				bPendingPlayerRespawn = true; // #83
				PlayerEntityKey = PlayerObject->GetEntityKey();
				PlayerStanceName = PlayerObject->GetStanceName();
				PlayerLocationCached = PlayerObject->GetRootLocation(); // #83
				PlayerRotationCached = PlayerObject->GetRotation(); // #83
				SavePlayerSettingsInfo(); // #87
			}
		}
	}

	if (nullptr == MapEntityAsset)
	{
		return;
	}
	TGuardValue<bool> UpdateGuard(bUpdatingReload, true);
	{
		if (!MapEntityAsset->MapData.LevelAsset.IsNull())
		{
			IT4WorldController* WorldController = GetGameWorld()->GetController();
			check(nullptr != WorldController);
			CachedTimeHour = WorldController->GetGameTimeHour(); // #104 : RefreshWorld 시 Time 복구
			CachedTimeScale = WorldController->GetGameTimeScale(); // #104 : RefreshWorld 시 Time 복구
			ClientWorldTravel(MapEntityAsset); // #87
		}
	}
	bDisplayEditorWorldModified = false;
}

void FT4WorldPreviewViewModel::OnDisplayEditorWorldModified() // #83
{
	bDisplayEditorWorldModified = true;
}

void FT4WorldPreviewViewModel::OnSubLevelSelection(
	const TArray<FName>& InSubLevelPackageNames,
	bool bFlushLevelStreaming // #86
) // #83
{
	VisibleSubLevelPackageNames.Empty();
	VisibleSubLevelPackageNames = InSubLevelPackageNames;

	if (bFlushLevelStreaming) // #86
	{
		Tick(0.0f);
	}
}

void FT4WorldPreviewViewModel::OnToggleSimulation() // #86
{
	bSimulating = !bSimulating;
	SetSimulationMode(bSimulating);
}

void FT4WorldPreviewViewModel::SetCameraLookAt(
	const FVector& InLocation,
	const FBox& InBoundingBox
) // #85
{
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return;
	}
	if (PlayerController->HasGameObject())
	{
		ClientTeleport(InLocation);
	}
	else
	{
		FT4RehearsalViewportClient* ViewportClient = GetViewportClient();
		if (nullptr != ViewportClient)
		{
			const FVector CenterLocation = InBoundingBox.GetCenter();
			FVector CameraDirection = ViewportClient->GetViewLocation() - CenterLocation;
			CameraDirection.Normalize();
			ViewportClient->SetViewLocation(CenterLocation + (CameraDirection * InBoundingBox.GetSize()));
			ViewportClient->SetLookAtLocation(CenterLocation, true);
		}
	}
}

void FT4WorldPreviewViewModel::SetCameraLocation(const FVector2D& InLocation) // #90
{
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return;
	}
	if (PlayerController->HasGameObject())
	{
		ClientTeleport(InLocation);
	}
	else
	{
		FT4RehearsalViewportClient* ViewportClient = GetViewportClient();
		if (nullptr != ViewportClient)
		{
			const FVector CameraViewLocation = ViewportClient->GetViewLocation();
			const FVector CameraLookAtLocation = ViewportClient->GetLookAtLocation();

			FVector CameraEyeLocation = CameraViewLocation;
			CameraEyeLocation.X = InLocation.X;
			CameraEyeLocation.Y = InLocation.Y;

			ViewportClient->SetViewLocation(CameraEyeLocation);
		}
	}
}

void FT4WorldPreviewViewModel::SetCameraLocationAndRotation(
	const FVector& InLocation, 
	const FRotator& InRotation
) // #90, #103
{
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return;
	}
	if (PlayerController->HasGameObject())
	{
		ClientTeleport(InLocation);
	}
	else
	{
		FT4RehearsalViewportClient* ViewportClient = GetViewportClient();
		if (nullptr != ViewportClient)
		{
			ViewportClient->SetViewLocation(InLocation);
			ViewportClient->SetViewRotation(InRotation);
		}
	}
}

bool FT4WorldPreviewViewModel::GetPlayerViewPoint(
	FVector& OutCameraLocation,
	FRotator& OutCameraRotation,
	FVector& OutPlayerLocation
) // #86
{
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return false;
	}
	if (PlayerController->HasGameObject())
	{
		OutCameraLocation = PlayerController->GetCameraLocation();
		OutCameraRotation = PlayerController->GetCameraRotation();
		OutPlayerLocation = PlayerController->GetCameraLookAtLocation();
		return true;
	}
	else
	{
		APawn* DefaultPawn = PlayerController->GetDefaultPawn();
		if (nullptr != DefaultPawn)
		{
			OutCameraLocation = DefaultPawn->GetActorLocation();
			OutCameraRotation = DefaultPawn->GetActorRotation();
			OutPlayerLocation = FVector::ZeroVector;
			return true;
		}
	}
	return false;
}

bool FT4WorldPreviewViewModel::GetGameObjectLocations(
	TArray<FVector2D>& OutGameObjectLocations
) // #104
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
	TArray<IT4GameObject*> GameObjects;
	if (GameWorld->GetContainer()->GetGameObjects(ET4SpawnMode::All, GameObjects))
	{
		for (IT4GameObject* GameObject : GameObjects)
		{
			check(nullptr != GameObject);
			if (!GameObject->IsPlayer())
			{
				const FVector CurrentLocation = GameObject->GetRootLocation();
				FVector2D& NewLocation = OutGameObjectLocations.AddDefaulted_GetRef();
				NewLocation.X = CurrentLocation.X;
				NewLocation.Y = CurrentLocation.Y;
			}
		}
	}
	return (0 < OutGameObjectLocations.Num()) ? true : false;
}

void FT4WorldPreviewViewModel::SetupStartWorld(
	FT4WorldConstructionValues& InWorldConstructionValues
) // #87
{
	if (nullptr != MapEntityAsset)
	{
		InWorldConstructionValues.MapEntityOrLevelObjectPath = MapEntityAsset->GetEntityKeyPath().ToString();
	}
}

void FT4WorldPreviewViewModel::SetSimulationMode(bool bInSimulating) // #86
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr != GameFrame)
	{
		IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
		if (nullptr != GameWorld)
		{
			const bool bDisalbe = !bInSimulating;
			GameWorld->SetDisableLevelStreaming(bDisalbe); // #86 : World 의 UpdateStreamingState 를 제어하기 위한 옵션 처리
			GameWorld->SetDisableEnvironmentUpdating(bDisalbe); // #92 : World 의 EnvironmentUpdate 제어
			GameWorld->SetDisableTimelapse(bDisalbe); // #93 : 시간 경과 옵션 처리
		}
	}
	if (nullptr != EditorPlaySettingObject) // #104 : Simul 을 AI 와 연동
	{
		EditorPlaySettingObject->SetSimulationEnabled(bInSimulating);
	}
	if (HasActionPlaybackController())
	{
		// #104 : Simulation 상태에 따라 Playback 동작을 제어한다.
		UT4EditorActionPlaybackController* PlaybackController = GetActionPlaybackController();
		check(nullptr != PlaybackController);
		if (PlaybackController->IsPlayed())
		{
			if (!bSimulating)
			{
				if (PlaybackController->CanPause())
				{
					PlaybackController->DoPause();
				}
			}
			else
			{
				if (PlaybackController->CanPlay())
				{
					PlaybackController->DoPlay();
				}
			}
		}
	}
}

void FT4WorldPreviewViewModel::HandleOnUpdateSubLevel(ULevel* InSubLevel, UWorld* InWorld) // #86
{
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return;
	}
	UWorld* PersistentWorld = GameFrame->GetWorld();
	if (nullptr == PersistentWorld)
	{
		return;
	}
	if (PersistentWorld != InWorld)
	{
		return;
	}
	GetOnSubLevelUpdate().Broadcast();
}

#undef LOCTEXT_NAMESPACE