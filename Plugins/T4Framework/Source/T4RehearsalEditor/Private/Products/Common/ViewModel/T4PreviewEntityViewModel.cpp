// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4PreviewEntityViewModel.h"
#include "Products/T4RehearsalEditorUtils.h"

#include "Products/EntityEditor/Utility/T4AssetEntityUtils.h"
#include "Products/Common/Viewport/T4RehearsalViewportClient.h"

#include "T4Asset/Public/T4AssetUtils.h" // #39, #62

#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h"
#include "T4Asset/Classes/Entity/T4PropEntityAsset.h"
#include "T4Asset/Classes/Entity/T4WeaponEntityAsset.h"
#include "T4Asset/Classes/Conti/T4ContiAsset.h"

#include "T4Engine/Public/Action/T4ActionCodeWorld.h"
#include "T4Engine/Public/T4Engine.h"

#include "T4Frame/Public/T4Frame.h" // #30

#include "T4RehearsalEditorInternal.h"

static const FT4ActionKey ThumbnailContiActionPrimaryKey(TEXT("ThumbnailConti"), true);

#define LOCTEXT_NAMESPACE "FT4PreviewEntityViewModel"

/**
  * 
 */
FT4PreviewEntityViewModelOptions::FT4PreviewEntityViewModelOptions()
	: EntityAsset(nullptr)
{
}

FT4PreviewEntityViewModel::FT4PreviewEntityViewModel(const FT4PreviewEntityViewModelOptions& InOptions)
	: EntityAsset(InOptions.EntityAsset)

{
	SetPropertiesChangedDelegate(true);
	RefreshAll();
	GEditor->RegisterForUndo(this);
}

FT4PreviewEntityViewModel::~FT4PreviewEntityViewModel()
{
}

void FT4PreviewEntityViewModel::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(EntityAsset);
}

void FT4PreviewEntityViewModel::PostUndo(bool bSuccess)
{
	RefreshAll();
}

void FT4PreviewEntityViewModel::Tick(float DeltaTime)
{
}

TStatId FT4PreviewEntityViewModel::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FT4PreviewEntityViewModel, STATGROUP_Tickables);
}

void FT4PreviewEntityViewModel::Reset() // #79
{

}

void FT4PreviewEntityViewModel::StartPlay()
{
	GetGameFrame()->SetPlayerChangeDisable(true); // Preview 에서는 캐릭터 변경을 막는다.
	if (nullptr != EntityAsset)
	{
		OnChangeViewTarget(EntityAsset, 1.0f);
	}
}

void FT4PreviewEntityViewModel::Cleanup()
{
	SetPropertiesChangedDelegate(false);
	GEditor->UnregisterForUndo(this);
	EntityAsset = nullptr;
}

void FT4PreviewEntityViewModel::RefreshAll()
{
}

void FT4PreviewEntityViewModel::OnChangeViewTarget(
	UObject* InEntityAsset,
	float InRuntimeDurationSec
)
{
	// #36
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
	IT4PlayerController* PlayerController = GameFrame->GetPlayerController();
	if (nullptr != PlayerController && PlayerController->HasGameObject())
	{
		// 이전 스폰 제거!
		FT4DespawnObjectAction LeaveAction;
		LeaveAction.ObjectID = PlayerController->GetGameObjectID();
		LeaveAction.FadeOutTimeSec = 0.0f; // 즉시 제거!
		PlayerController->ClearGameObject(false);
		bool bResult = GameWorld->DoExecuteAction(&LeaveAction);
	}
	SetPropertiesChangedDelegate(false);
	EntityAsset = Cast<UT4EntityAsset>(InEntityAsset);
	if (nullptr == EntityAsset)
	{
		return;
	}
	SetPropertiesChangedDelegate(true);
	ET4EntityType SelectEntityType = EntityAsset->GetEntityType();
	if (!IsPreviewSpawnable(SelectEntityType))
	{
		return;
	}
	FT4SpawnObjectAction NewAction;
	NewAction.ObjectID = GameFrame->GenerateObjectIDForServer();
	NewAction.Name = TEXT("T4PreviewObject");
	NewAction.EntityType = SelectEntityType;
	NewAction.EntityAssetPath = FSoftObjectPath(EntityAsset);
	NewAction.GameDataIDName = TEXT("T4RehearsalEditorThumbnail");
	NewAction.bPlayer = true;
	bool bResult = GameWorld->DoExecuteAction(&NewAction);
	FT4RehearsalViewportClient* ViewportClient = GetViewportClient();
	if (nullptr != ViewportClient)
	{
		const FT4EntityEditorThumbnailAttribute& ThumbnailCameraInfo = EntityAsset->ThumbnailCameraInfo;
		ViewportClient->SetViewRotation(ThumbnailCameraInfo.Rotation);
		ViewportClient->SetViewLocation(ThumbnailCameraInfo.Location);
		ViewportClient->SetUpdateViewport(InRuntimeDurationSec); // #59 : 섬네일 뷰포트는 Realtime 이 꺼져 있어 강제로 업데이트 해준다.
	}
	if (bResult)
	{
		PlayerController->SetGameObject(NewAction.ObjectID);
	}
}

void FT4PreviewEntityViewModel::ClientPlayConti(
	UT4ContiAsset* InContiAsset,
	const FT4ActionParameters* InActionParameters
)
{
	if (nullptr == InContiAsset)
	{
		return;
	}
	if (InContiAsset->TestSettings.StanceSelected != NAME_None) // #73
	{
		ClientChangeStance(InContiAsset->TestSettings.StanceSelected);
	}
	FT4BaseViewModel::ClientPlayConti(InContiAsset, InActionParameters);
}

void FT4PreviewEntityViewModel::HandleOnEntityPropertiesChanged()
{
	if (nullptr == EntityAsset)
	{
		return;
	}
	FT4RehearsalViewportClient* ViewportClient = GetViewportClient();
	if (nullptr != ViewportClient)
	{
		const FT4EntityEditorThumbnailAttribute& ThumbnailCameraInfo = EntityAsset->ThumbnailCameraInfo;
		ViewportClient->SetViewRotation(ThumbnailCameraInfo.Rotation);
		ViewportClient->SetViewLocation(ThumbnailCameraInfo.Location);
	}
	// #37
	ET4EntityType SelectEntityType = EntityAsset->GetEntityType();
	if (ET4EntityType::Map == SelectEntityType)
	{
		return; // TODO : World
	}
	ClientEditorAction(ET4EditorAction::ReloadAttributes);
}

void FT4PreviewEntityViewModel::SaveThumbnailCameraInfo()
{
	if (nullptr == EntityAsset)
	{
		return;
	}
	FT4RehearsalViewportClient* ViewportClient = GetViewportClient();
	if (nullptr != ViewportClient)
	{
		T4AssetUtil::EntitySaveThumbnailCameraInfo(
			EntityAsset,
			ViewportClient->GetViewRotation(),
			ViewportClient->GetViewLocation()
		);
	}
}

void FT4PreviewEntityViewModel::SetPropertiesChangedDelegate(bool bRegister)
{
	if (nullptr == EntityAsset)
	{
		return;
	}
	if (bRegister)
	{
		EntityAsset->OnPropertiesChanged().AddRaw(
			this,
			&FT4PreviewEntityViewModel::HandleOnEntityPropertiesChanged
		);
	}
	else
	{
		EntityAsset->OnPropertiesChanged().RemoveAll(this);
	}
}

#undef LOCTEXT_NAMESPACE