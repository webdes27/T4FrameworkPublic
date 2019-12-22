// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4EntityViewModel.h"
#include "Products/T4RehearsalEditorUtils.h"

#include "Products/Common/Viewport/T4RehearsalViewportClient.h"
#include "Products/EntityEditor/T4RehearsalEntityEditor.h"

#include "Products/Common/DetailView/T4EnvironmentDetailObject.h" // #90, #94

#include "Products/EntityEditor/Utility/T4AssetAnimSetUtils.h"
#include "Products/EntityEditor/Utility/T4AssetEntityUtils.h"

#include "T4Asset/Public/T4AssetUtils.h" // #39, #62
#include "T4Asset/Classes/Entity/T4EntityAsset.h"
#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h" // #39
#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h"
#include "T4Engine/Public/T4Engine.h"
#include "T4Frame/Public/T4Frame.h" // #30
#include "T4Frame/Classes/Controller/Player/T4PlayerController.h" // #86

#include "Animation/AnimSequence.h"
#include "ScopedTransaction.h" // #77

#include "T4RehearsalEditorInternal.h"

#define LOCTEXT_NAMESPACE "FT4EntityViewModel"

/**
  * 
 */
FT4EntityViewModelOptions::FT4EntityViewModelOptions()
	: EntityAsset(nullptr)
	, EntityEditor(nullptr)
{
}

FT4EntityViewModel::FT4EntityViewModel(const FT4EntityViewModelOptions& InOptions)
	: EntityAsset(InOptions.EntityAsset)
	, EnvironmentDetailObjectOwner(nullptr) // #94
	, ViewTargetSelectorPtr(MakeShareable(new FT4EditorViewTargetSelector)) // #74
	, AnimSetAssetSelectorPtr(MakeShareable(new FT4EditorAnimSetAssetSelector))
	, EntityEditor(InOptions.EntityEditor)
	, bPlayerSpawnd(false) // #71
	, bUpdatingStartPlay(false)
{
	const ET4EntityType EntityType = EntityAsset->GetEntityType();
	if (ET4EntityType::Zone == EntityType)
	{
		EnvironmentDetailObjectOwner = NewObject<UT4EnvironmentDetailObject>(); // #94 : Zone 만 사용
	}

	EntityAsset->ResetEditorTransientData(); // #73

	AnimSetAssetSelectorPtr->SetAnimSetAsset(GetAnimSetAssetByStance(), true); // #39
	SetPropertiesChangedDelegate(true);
	RefreshAll();
	GEditor->RegisterForUndo(this);
}

FT4EntityViewModel::~FT4EntityViewModel()
{
}

void FT4EntityViewModel::Cleanup()
{
	SetPropertiesChangedDelegate(false);
	GEditor->UnregisterForUndo(this);
	ViewTargetSelectorPtr.Reset(); // #74
	AnimSetAssetSelectorPtr.Reset();
	EnvironmentDetailObjectOwner = nullptr; // #94
	EntityAsset = nullptr;
}

void FT4EntityViewModel::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(EntityAsset);
	Collector.AddReferencedObject(EnvironmentDetailObjectOwner); // #94
}

void FT4EntityViewModel::PostUndo(bool bSuccess)
{
	// #77 : Refresh 코드에서는 가능하면 Asset Property 쓰기를 하지 않도록 주의한다!!
	RefreshAll();
}

void FT4EntityViewModel::Tick(float DeltaTime)
{
	if (!bPlayerSpawnd)
	{
		CheckAndSpawnEntity(); // #87 : 레벨 이동으로 재스폰!
	}
}

TStatId FT4EntityViewModel::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FT4EntityViewModel, STATGROUP_Tickables);
}

bool FT4EntityViewModel::IsEditWidgetMode() const // #94
{
	check(nullptr != EntityAsset);
	const ET4EntityType EntityType = EntityAsset->GetEntityType();
	return (ET4EntityType::Zone == EntityType) ? true : false;
}

const FString FT4EntityViewModel::GetAssetPath() // #79
{
	if (nullptr == EntityAsset)
	{
		return FString();
	}
	return FSoftObjectPath(EntityAsset).ToString();
}

AActor* FT4EntityViewModel::GetEditWidgetModeTarget() const // #94
{
	const ET4EntityType EntityType = EntityAsset->GetEntityType();
	if (ET4EntityType::Zone != EntityType) // #94
	{
		return nullptr;
	}
	if (!ZoneEntityObjectID.IsValid())
	{
		return nullptr;
	}
	IT4GameFrame* GameFrame = GetGameFrame();
	if (nullptr == GameFrame)
	{
		return nullptr;
	}
	IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
	if (nullptr == GameWorld)
	{
		return nullptr;
	}
	IT4GameObject* GameObject = GameWorld->GetContainer()->FindGameObject(ZoneEntityObjectID);
	if (nullptr == GameObject)
	{
		return nullptr;
	}
	return GameObject->GetPawn();
}

void FT4EntityViewModel::ChangeWorldEnvironment(FName InTimeTagName) // #94
{
	UWorld* UnrealWorld = GetWorld();
	if (nullptr != UnrealWorld)
	{
		FString ErrorMessage;
		bool bResult = EnvironmentDetailObjectOwner->ApplyTo(
			UnrealWorld,
			InTimeTagName,
			ErrorMessage
		);
		if (!bResult)
		{
			UE_LOG(
				LogT4RehearsalEditor,
				Warning,
				TEXT("ChangeWorldEnvironment : ApplyTo 'World' failed. '%s'"),
				*ErrorMessage
			);
		}
	}
}

void FT4EntityViewModel::Reset() // #79
{
	bPlayerSpawnd = false;
	ZoneEntityObjectID.SetNone(); // #94
}

void FT4EntityViewModel::SetupInternal()
{
	check(nullptr != EntityAsset);
	IT4GameFrame* GameFrame = GetGameFrame();
	check(nullptr != GameFrame);
	const ET4EntityType EntityType = EntityAsset->GetEntityType();
	if (IsControllable(EntityType)) // #79, #94 : Map, Zone 이외에는 캐릭터 변경 금지!
	{
		GameFrame->SetPlayerChangeDisable(true); // Entity Editor 에서는 캐릭터 변경을 막는다.
	}
	else
	{
		GameFrame->SetPlayerChangeDisable(false);
	}
	if (ET4EntityType::Zone == EntityType) // #94
	{
		AT4PlayerController* EditorPlayerController = GameFrame->GetEditorPlayerController();
		check(nullptr != EditorPlayerController);
		EditorPlayerController->SetCameraZoomMaxScale(3.0f); // 플레이 테스트를 위해 카메라 거리를 늘린다.
	}
	bPlayerSpawnd = false; // #87 : 레벨이 변경되었다.
	ZoneEntityObjectID.SetNone(); // #94
}

void FT4EntityViewModel::StartPlay()
{
	if (bUpdatingStartPlay)
	{
		return;
	}
	TGuardValue<bool> UpdateGuard(bUpdatingStartPlay, true);
	SetupInternal(); // #94
	CheckAndSpawnEntity();
}

void FT4EntityViewModel::RestartPlay() // #94 : 월드 이동후 호출
{
	SetupInternal();
}

void FT4EntityViewModel::DrawHUD(
	FViewport* InViewport,
	FCanvas* InCanvas,
	FT4HUDDrawInfo* InOutDrawInfo
) // #94
{
	check(nullptr != InViewport);
	check(nullptr != InCanvas);

	UFont* DrawFont = GEngine->GetLargeFont();
	check(nullptr != DrawFont);

	const int32 SrcWidth = InViewport->GetSizeXY().X;
	const int32 SrcHeight = InViewport->GetSizeXY().Y;

	int32 StartHeightOffset = 5;

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
				const float DrawX = FMath::FloorToFloat(SrcWidth - XL - 7.0f);
				const float DrawY = SrcHeight - YL - StartHeightOffset;
				InCanvas->DrawShadowedString(DrawX, DrawY, *DrawTimeString, DrawFont, FLinearColor::Green);
				StartHeightOffset += YL + 5;
			}
		}
	}
}

void FT4EntityViewModel::SetupStartWorld(
	FT4WorldConstructionValues& InWorldConstructionValues
) // #87
{ 
	if (nullptr != EntityAsset && ET4EntityType::Map == EntityAsset->GetEntityType())
	{
		InWorldConstructionValues.MapEntityOrLevelObjectPath = EntityAsset;
		bPlayerSpawnd = true; // #87 : StartPlay 에서 로드되지 않도록 체크!
	}
}

void FT4EntityViewModel::NotifyViewTargetChanged(IT4GameObject* InViewTarget) // #87
{
	if (nullptr != InViewTarget)
	{
		return;
	}
	const ET4EntityType EntityType = EntityAsset->GetEntityType();
	if (!IsControllable(EntityType))
	{
		return; // #87, #94 : Map & Zone 은 제외
	}
	bPlayerSpawnd = false; // #87 : 레벨이 변경되었다.
	ZoneEntityObjectID.SetNone(); // #94
}

UObject* FT4EntityViewModel::GetEditObject() const // #103
{
	return Cast<UObject>(EntityAsset);
}

FT4EditorTestAutomation* FT4EntityViewModel::GetTestAutomation() const // #103
{
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	return &EntityAsset->TestAutomation;
}

FString FT4EntityViewModel::GetActionPlaybackAssetName() const // #68, #104
{ 
	return EntityAsset->GetName();
}

FString FT4EntityViewModel::GetActionPlaybackFolderName() const // #68, #104
{ 
	return TEXT("Entity");
} 

void FT4EntityViewModel::RefreshAll()
{
	if (nullptr == EntityAsset)
	{
		return;
	}

	check(AnimSetAssetSelectorPtr.IsValid());
	AnimSetAssetSelectorPtr->SetAnimSetAsset(GetAnimSetAssetByStance(), true); // #39

	// #77 : 변경사항을 DetailView 에 알린다.
	if (GetOnViewModelChanged().IsBound())
	{
		GetOnViewModelChanged().Broadcast();
	}
	// #77 : 변경사항을 AnimSet DetailView 에 알린다.
	if (GetOnViewModelDetailPropertyChanged().IsBound())
	{
		GetOnViewModelDetailPropertyChanged().Broadcast();
	}
}

UT4AnimSetAsset* FT4EntityViewModel::GetAnimSetAssetByStance() const
{
	// #39
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	ET4EntityType EntityType = EntityAsset->GetEntityType();
	if (ET4EntityType::Character != EntityType)
	{
		return nullptr;
	}
	UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(EntityAsset);
	if (nullptr == CharacterEntityAsset)
	{
		return nullptr;
	}
	bool bFallback = false;
	FName EditorStanceSelected = CharacterEntityAsset->EditorTransientCharacterData.TransientStanceName;
	if (EditorStanceSelected == NAME_None)
	{
		EditorStanceSelected = T4StanceDefaultStanceName; // #73 : 초기설정은 Default Stance 로
		bFallback = true;
	}
	const FT4EntityCharacterStanceSetData& StanceSetData = CharacterEntityAsset->StanceSetData;
	if (!StanceSetData.StanceMap.Contains(EditorStanceSelected))
	{
		return nullptr;
	}
	const FT4EntityCharacterStanceData& StanceData = StanceSetData.StanceMap[EditorStanceSelected];
	if (StanceData.AnimSetAsset.IsNull())
	{
		return nullptr;
	}
	if (bFallback) // #73 : 초기 설정 업데이트
	{
		T4AssetUtil::EntityCharacterSelectStanceDataByName(CharacterEntityAsset, EditorStanceSelected); // #73, #74
	}
	UT4AnimSetAsset* AnimSetAsset = StanceData.AnimSetAsset.LoadSynchronous();
	return AnimSetAsset;
}

bool FT4EntityViewModel::DoSave(FString& OutErrorMessage)
{
	// #39
	if (nullptr == EntityAsset)
	{
		OutErrorMessage = TEXT("No Set EntityAsset!");
		return false;
	}
	{
		DoAnimSetSaveAll(OutErrorMessage);
	}
	const FScopedTransaction Transaction(LOCTEXT("DoSave_Transaction", "Save to EntityAsset"));
	UPackage* EntityAssetPackage = EntityAsset->GetOutermost();
	check(nullptr != EntityAssetPackage);
	bool bWasEntityPackageDirty = EntityAssetPackage->IsDirty();
	if (bWasEntityPackageDirty)
	{
		T4AssetUtil::SaveAsset(EntityAsset, true); // #39
	}
	return true;
}

bool FT4EntityViewModel::DoAnimSetSaveAll(FString& OutErrorMessage)
{
	// #39
	UT4AnimSetAsset* AnimSetAsset = GetAnimSetAssetByStance();
	if (nullptr == AnimSetAsset)
	{
		OutErrorMessage = TEXT("No Set AnimSetAsset");
		return false;
	}
	const FScopedTransaction Transaction(LOCTEXT("DoAnimSetSaveAll_Transaction", "Save to AnimSetAsset"));
	bool bResult = T4AssetUtil::AnimSetSaveAll(AnimSetAsset, OutErrorMessage);
	return bResult;
}

bool FT4EntityViewModel::DoAnimSetUpdateAll(FString& OutErrorMessage)
{
	// #39
	UT4AnimSetAsset* AnimSetAsset = GetAnimSetAssetByStance();
	if (nullptr == AnimSetAsset)
	{
		OutErrorMessage = TEXT("No Set AnimSetAsset");
		return false;
	}
	const FScopedTransaction Transaction(LOCTEXT("DoAnimSetUpdateAll_Transaction", "Update a AnimSetAsset"));
	bool bResult = T4AssetUtil::AnimSetUpdateAll(AnimSetAsset, OutErrorMessage);
	if (bResult)
	{
		ClientEditorAction(ET4EditorAction::ReloadObject);
	}
	return bResult;
}

bool FT4EntityViewModel::DoUpdateThumbnailAnimSetAsset(FString& OutErrorMessage)
{
	if (nullptr == EntityEditor)
	{
		OutErrorMessage = TEXT("Not found RehearsalEntityEditor!");
		return false;
	}
	UT4AnimSetAsset* AnimSetAsset = GetAnimSetAssetByStance();
	if (nullptr == AnimSetAsset)
	{
		OutErrorMessage = TEXT("No Set AnimSetAsset");
		return false;
	}
	EntityEditor->DoUpdateThumbnail(AnimSetAsset);
	return true;
}

void FT4EntityViewModel::ReloadPlayerSpawn() // #87
{
	IT4GameObject* PlayerObject = GetPlayerObject();
	if (nullptr != PlayerObject)
	{
		if (EntityAsset == PlayerObject->GetEntityAsset())
		{
			ClientEditorAction(ET4EditorAction::ReloadObject); // #38
			return;
		}
	}
	bPlayerSpawnd = false; // respawn
	ZoneEntityObjectID.SetNone(); // #94
}

void FT4EntityViewModel::CheckAndSpawnEntity() // #79
{
	const bool bEntityValid = (nullptr != EntityAsset) ? true : false;
	if (bPlayerSpawnd)
	{
		if (!bEntityValid)
		{
			IT4PlayerController* PlayerController = GetPlayerController();
			if (nullptr != PlayerController && PlayerController->HasGameObject())
			{
				// 기존 Player 가 있는데, Entity 가 사라졌다면 despawn 해준다.
				ClientDespawnObject(PlayerController->GetGameObjectID());
			}
			bPlayerSpawnd = false;
			ZoneEntityObjectID.SetNone(); // #94
		}
		return;
	}
	if (!bEntityValid)
	{
		return;
	}
	bPlayerSpawnd = DoEntitySpawn(EntityAsset);
}

bool FT4EntityViewModel::DoEntitySpawn(UT4EntityAsset* InEntityAsset) // #79
{
	bool bResult = false;
	const ET4EntityType EntityType = InEntityAsset->GetEntityType();
	if (!IsSpawnable(EntityType))
	{
		bResult = ClientWorldTravel(InEntityAsset);
	}
	else
	{
		FVector SpawnLocation = FVector::ZeroVector;
		FRotator SpawnRotation = CachedPlayerRotation;
		if (bTestAutomation) // #103 : POI 기능 추가
		{
			SpawnLocation = TestAutomationSpawnLocation; // #100
			SpawnLocation.Z += 100.0f;
			SpawnRotation = TestAutomationSpawnRotation; // #100
			GetGameWorld()->GetController()->SetGameTimeHour(TestAutomationGameTimeHour); // #103
		}
		else
		{
			bResult = TryValidSpawnObjectLocation(SpawnLocation); // #87 : Entity 임으로 반드시(최대한) 스폰이 되어야 한다!!
			if (!bResult)
			{
				return false;
			}
		}
		FT4EntityKey SpawnEntityKey(EntityType, InEntityAsset->GetEntityKeyPath());
		IT4GameObject* GameObject = ClientSpawnObjectEx(
			SpawnEntityKey, 
			SpawnLocation,
			SpawnRotation, 
			NAME_None, 
			true
		);
		bResult = (nullptr != GameObject) ? true : false;
		if (bResult)
		{
			RestorePlayerSettingsInfo();
			if (ET4EntityType::Zone == EntityType)
			{
				ZoneEntityObjectID = GameObject->GetObjectID(); // #94 : Zone Entity 는 ID 를 기록해서 EditMode 에서 편집할 수 있도록 처리
			}
		}
		bTestAutomation = false;
	}
	return bResult;
}

void FT4EntityViewModel::ClientChangeStance(FName InStanceName) // #73
{
	FT4BaseViewModel::ClientChangeStance(InStanceName);
	check(AnimSetAssetSelectorPtr.IsValid());
	AnimSetAssetSelectorPtr->SetAnimSetAsset(GetAnimSetAssetByStance(), true); // #39
}

bool FT4EntityViewModel::ClientSpawnObject(UT4EntityAsset* InEntityAsset, const FName InStanceName)
{
	bool bResult = FT4BaseViewModel::ClientSpawnObject(InEntityAsset, NAME_None);
	if (bResult)
	{
		IT4PlayerController* PlayerController = GetPlayerController();
		if (nullptr != PlayerController && nullptr != PlayerController->GetGameObject())
		{
			ViewTargetSelectorPtr->SetViewTarget(PlayerController->GetGameObject(), true); // #57
		}
		else
		{
			ViewTargetSelectorPtr->SetViewTarget(nullptr, true); // #57
		}
	}
	return bResult;
}

void FT4EntityViewModel::ClientEditorAction(ET4EditorAction InEditorActionType) // #71
{
	CheckAndSpawnEntity();
	if (!bPlayerSpawnd)
	{
		return;
	}
	FT4BaseViewModel::ClientEditorAction(InEditorActionType);
}

void FT4EntityViewModel::HandleOnDetailsPropertiesChanged(const FName& InPropertyName)
{
	if (nullptr == EntityAsset)
	{
		return;
	}
	check(AnimSetAssetSelectorPtr.IsValid());
	AnimSetAssetSelectorPtr->SetAnimSetAsset(GetAnimSetAssetByStance(), false); // #39
	if (GetOnViewModelDetailPropertyChanged().IsBound())
	{
		GetOnViewModelDetailPropertyChanged().Broadcast();
	}
}

void FT4EntityViewModel::HandleOnAnimSetDetailsPropertiesChanged(const FName& InPropertyName)
{
	// #39
	if (nullptr == EntityAsset)
	{
		return;
	}
	check(AnimSetAssetSelectorPtr.IsValid());
	AnimSetAssetSelectorPtr->SetAnimSetAsset(GetAnimSetAssetByStance(), true); // #39
}

void FT4EntityViewModel::HandleOnEntityPropertiesChanged()
{
	if (nullptr == EntityAsset)
	{
		return;
	}
	// #37
	ET4EntityType SelectEntityType = EntityAsset->GetEntityType();
	if (ET4EntityType::Map == SelectEntityType)
	{
		// TODO : World
		return;
	}
	ClientEditorAction(ET4EditorAction::ReloadAttributes);
}

void FT4EntityViewModel::SetPropertiesChangedDelegate(bool bRegister)
{
	if (nullptr == EntityAsset)
	{
		return;
	}
	if (bRegister)
	{
		EntityAsset->OnPropertiesChanged().AddRaw(
			this,
			&FT4EntityViewModel::HandleOnEntityPropertiesChanged
		);
	}
	else
	{
		EntityAsset->OnPropertiesChanged().RemoveAll(this);
	}
}

#undef LOCTEXT_NAMESPACE