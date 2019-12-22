// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ContiViewModel.h"
#include "Products/T4RehearsalEditorUtils.h"

#include "Products/ContiEditor/Sequencer/MovieScene/T4ContiActionTrack.h"
#include "Products/ContiEditor/Sequencer/MovieScene/T4ContiActionSection.h"

#include "Products/ContiEditor/Sequencer/T4ContiActionMovieScene.h" // #54
#include "Products/ContiEditor/Sequencer/T4ContiActionSequence.h"

#include "Products/ContiEditor/T4RehearsalContiEditor.h"

#include "Products/Common/Viewport/T4RehearsalViewportClient.h"
#include "Products/Common/Helper/T4EditorGameplaySettingObject.h" // #60

#include "Products/ContiEditor/Helper/T4CameraWorkSectionKeyObject.h" // #58

#include "Products/Common/Helper/T4EditorActionPlaybackController.h" // #68

#include "T4RehearsalEditorSettings.h"

#include "T4Asset/Public/T4AssetUtils.h" // #39, #62

#include "T4Asset/Classes/Entity/T4EntityAsset.h"
#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h" // #39

#include "T4Asset/Classes/AnimSet/T4AnimSetAsset.h"

#include "T4Asset/Public/Entity/T4Entity.h" // #87
#include "T4Asset/Classes/Conti/T4ContiAsset.h"

#include "T4Engine/Public/T4Engine.h"
#include "T4Engine/Public/Action/T4ActionCodeCommon.h"
#include "T4Engine/Classes/Camera/T4EditorCameraActor.h" // #58

#include "T4Frame/Public/T4Frame.h" // #30

#include "Framework/Commands/GenericCommands.h" // #75

#include "HAL/PlatformApplicationMisc.h" // #75
#include "JsonObjectConverter.h" // #75

#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "ScopedTransaction.h"
#include "MovieScene.h"
#include "MovieSceneFolder.h" // #56
#include "MovieSceneTimeHelpers.h"
#include "ISequencerModule.h"
#include "SequencerSettings.h"
#include "Modules/ModuleManager.h"

#include "T4RehearsalEditorInternal.h"

static const TCHAR* DefaultContiEditorActionPlaybackFolderName = TEXT("Conti");

#define LOCTEXT_NAMESPACE "FT4ContiViewModel"

/**
  * 
 */
static TMap<uint32, FT4ContiViewModel*> GContiViewModelMap; // #65

FT4ContiViewModelOptions::FT4ContiViewModelOptions()
	: InstanceKey(INDEX_NONE) // #65
	, ContiAsset(nullptr)
	, ContiEditor(nullptr)
{
}

FT4ContiViewModel::FT4ContiViewModel(const FT4ContiViewModelOptions& InOptions)
	: InstanceKey(InOptions.InstanceKey) // #65
	, ContiSequence(nullptr)
	, ContiAsset(InOptions.ContiAsset)
	, EditorPlaySettingObject(nullptr) // #60
	, EditorActionPlaybackController(nullptr) // #68
	, ViewTargetSelectorPtr(MakeShareable(new FT4EditorViewTargetSelector)) // #57
	, AnimSetAssetSelectorPtr(MakeShareable(new FT4EditorAnimSetAssetSelector)) // #39
	, ContiEditor(InOptions.ContiEditor)
	, OnGetSequencerAddMenuContent(InOptions.OnGetSequencerAddMenuContent)
	, bSequencerPlayPaused(false) // #54
	, bSequencerPlayRestart(false) // #102
	, bUpdatingContiSelectionFromSequencer(false)
	, bPlayerSpawnd(false) // #87
	, bSimulationEnabled(false) // #102
	, bMirrorToPIEEnabled(false) // #59
	, MirrorLayerType(ET4LayerType::Max) // #59
	, FocusCameraWorkActionKeySelected(INDEX_NONE) // #58 : N개중 포커스가 가있는 편집중인 한개
{
	EditorPlaySettingObject = NewObject<UT4EditorGameplaySettingObject>();
	EditorPlaySettingObject->SetFlags(RF_Transactional); // Undo, Redo

	EditorActionPlaybackController = NewObject<UT4EditorActionPlaybackController>(); // #68
	EditorActionPlaybackController->SetFlags(RF_Transactional); // Undo, Redo

	SetupSequencer();
	GEditor->RegisterForUndo(this);

	if (nullptr != ContiAsset)
	{
		GContiViewModelMap.Add(InstanceKey, this); // #65
	}
}

FT4ContiViewModel::~FT4ContiViewModel()
{
}

FT4ContiViewModel* FT4ContiViewModel::GetContiViewModelByInstanceKey(uint32 InInstanceKey) // #65
{
	if (!GContiViewModelMap.Contains(InInstanceKey))
	{
		return nullptr;
	}
	return GContiViewModelMap[InInstanceKey];
}

void FT4ContiViewModel::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(ContiSequence);
	Collector.AddReferencedObject(ContiAsset);
	Collector.AddReferencedObject(EditorPlaySettingObject); // #60
	Collector.AddReferencedObject(EditorActionPlaybackController); // #68

	for (TMap<uint32, UT4CameraWorkSectionKeyObject*>::TConstIterator It(CameraWorkSectionKeyObjects); It; ++It)
	{
		UT4CameraWorkSectionKeyObject* EditorCameraSectionKeyObject = It.Value();
		Collector.AddReferencedObject(EditorCameraSectionKeyObject);
	}
}

void FT4ContiViewModel::PostUndo(bool bSuccess)
{
	// #77 : Refresh 코드에서는 가능하면 Asset Property 쓰기를 하지 않도록 주의한다!!
	RefreshAll();
}

void FT4ContiViewModel::Tick(float DeltaTime)
{
	if (!bPlayerSpawnd)
	{
		CheckAndSpawnEntity(); // #87 : 레벨 이동으로 재스폰!
	}
	UpdateCameraSectionKeyObjects();
	IT4GameObject* PlayerObject = GetPlayerObject();
	if (nullptr != PlayerObject)
	{
		if (SequencerPtr.IsValid()) // #102
		{
			const float TimeScale = PlayerObject->GetTimeScale();
			SequencerPtr->SetPlaybackSpeed(TimeScale);
		}
	}
}

TStatId FT4ContiViewModel::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FT4ContiViewModel, STATGROUP_Tickables);
}

AActor* FT4ContiViewModel::GetEditWidgetModeTarget() const // #94, #85
{
	if (INDEX_NONE == FocusCameraWorkActionKeySelected ||
		!CameraWorkSectionKeyObjects.Contains(FocusCameraWorkActionKeySelected))
	{
		return nullptr;
	}
	UT4CameraWorkSectionKeyObject* EditorCameraSectionKeyObject = CameraWorkSectionKeyObjects[FocusCameraWorkActionKeySelected];
	check(nullptr != EditorCameraSectionKeyObject);
	return EditorCameraSectionKeyObject->GetFocusCameraActor(GetGameWorld(), false);
}

void FT4ContiViewModel::NotifyActionPlaybackRec() // #104
{
}

void FT4ContiViewModel::NotifyActionPlaybackPlay() // #104
{
	if (!bSimulationEnabled)
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

void FT4ContiViewModel::ClientChangeStance(FName InStanceName) // #73
{
	FT4BaseViewModel::ClientChangeStance(InStanceName);
	UT4AnimSetAsset* AnimSetAssetSelected = GetAnimSetAssetByStance();
	AnimSetAssetSelectorPtr->SetAnimSetAsset(AnimSetAssetSelected, true);
}

void FT4ContiViewModel::Cleanup()
{
	ClearCameraSectionKeyObjects(); // #58

	if (bMirrorToPIEEnabled)
	{
		ToggleMirrorToPIE(); // #59 : 만약 툴을 종료한다면 Mirror 도 종료해준다.
	}

	if (nullptr != ContiAsset)
	{
		GContiViewModelMap.Remove(InstanceKey); // #65
	}

	GEditor->UnregisterForUndo(this);
	ViewTargetSelectorPtr.Reset(); // #57
	AnimSetAssetSelectorPtr.Reset(); // #39
	if (SequencerPtr.IsValid())
	{
		SequencerPtr->OnPlayEvent().RemoveAll(this);
		SequencerPtr->OnStopEvent().RemoveAll(this);

		SequencerPtr->OnMovieSceneDataChanged().RemoveAll(this);
		SequencerPtr->OnGlobalTimeChanged().RemoveAll(this);
		SequencerPtr->GetSelectionChangedTracks().RemoveAll(this);
		SequencerPtr->GetSelectionChangedSections().RemoveAll(this);
		SequencerPtr.Reset();
	}

	ContiAsset = nullptr;
	ContiSequence = nullptr;
	EditorPlaySettingObject = nullptr;
	EditorActionPlaybackController = nullptr;
}

void FT4ContiViewModel::Reset() // #79
{
	ClearCameraSectionKeyObjects(); // 맵이동!
	bPlayerSpawnd = false;
}

void FT4ContiViewModel::StartPlay()
{
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);

	// #60
	if (nullptr != EditorPlaySettingObject)
	{
		EditorPlaySettingObject->SetLayerType(GetLayerType());
		EditorPlaySettingObject->SetUseGameplaySettings(true); // #104 : conti 에서만 true, world 에서는 false

		GetGameFrame()->SetEditorGameplayCustomHandler(EditorPlaySettingObject); // #60 : NPC AI가 EditorPlaySettingObject 의 통제를 받도록 설정

		// #T4_ADD_EDITOR_PLAY_TAG

		EditorPlaySettingObject->ThisContiAsset = GetContiAsset();
		const FT4ContiTestSettings& ContiTestSttings = ContiAsset->TestSettings;
		EditorPlaySettingObject->WeaponContentNameIDSelected = ContiTestSttings.WeaponNameID; // #60
		EditorPlaySettingObject->NPCContentNameIDSelected = ContiTestSttings.SandbackNameID; // #60
		EditorPlaySettingObject->bAIDisabled = ContiTestSttings.bAISystemDisabled; // #60
		EditorPlaySettingObject->SandbackRole = (ContiTestSttings.bSandbackRoleAttacker) ? ET4EditorPlayRole::Attacker : ET4EditorPlayRole::Defender;
		EditorPlaySettingObject->bSandbackOneHitDie = ContiTestSttings.bSandbackOneHitDie; // #76
		EditorPlaySettingObject->bOverrideSkillData = ContiTestSttings.bOverrideSkillData; // #63
		EditorPlaySettingObject->bOverrideEffectData = ContiTestSttings.bOverrideEffectData; // #68
		EditorPlaySettingObject->SkillContentNameIDSelected = ContiTestSttings.SkillDataNameID; // #60
		EditorPlaySettingObject->EffectContentNameIDSelected = ContiTestSttings.EffectDataNameID; // #60
		EditorPlaySettingObject->DieReactionNameIDSelected = ContiTestSttings.DieReactionNameID; // ##67

		UpdateEditorPlayNPCEntityAsset(); // #76
		UpdateEditorPlaySkillDataInfo(); // #60, #63
		UpdateEditorPlayEffectDataInfo(); // #60, #63
	}

	// #68
	if (nullptr != EditorActionPlaybackController)
	{
		EditorActionPlaybackController->Set(
			GetLayerType(),
			ContiAsset->GetName(), 
			DefaultContiEditorActionPlaybackFolderName
		);
	}

	CheckAndSpawnEntity(); // #67
}

void FT4ContiViewModel::RestartPlay() // #87, #94 : 월드 이동후 호출
{
	bPlayerSpawnd = false;

	check(nullptr != ContiAsset);
	FT4ContiTestSettings& ContiTestSttings = ContiAsset->TestSettings;
	if (ContiTestSttings.MapEntitySelected != NAME_None)
	{
		IT4GameWorld* GameWorld = GetGameWorld();
		check(nullptr != GameWorld);
		if (GameWorld->GetController()->IsPreviewScene())
		{
			const FScopedTransaction Transaction(LOCTEXT("ChangeStartMapySettings_Transaction", "Change Start World"));
			ContiAsset->Modify(); //
			ContiTestSttings.MapEntitySelected = NAME_None;
		}
	}

	if (GetOnViewModelChanged().IsBound())
	{
		GetOnViewModelChanged().Broadcast();
	}
}

void FT4ContiViewModel::DrawHUD(
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

	int32 StartHeightOffset = 12;

	{
		IT4GameFrame* GameFrame = GetGameFrame(); // #99
		if (nullptr != GameFrame)
		{
			IT4GameWorld* GameWorld = GameFrame->GetGameWorld();
			if (nullptr != GameWorld)
			{
				FString DrawTimeString = GameWorld->GetController()->GetGameTimeString();
				int32 XL;
				int32 YL;
				StringSize(DrawFont, XL, YL, *DrawTimeString);
				const float DrawX = FMath::FloorToFloat(SrcWidth - XL - 123.0f); // add Toggle Simulation Buttn
				const float DrawY = SrcHeight - YL - StartHeightOffset;
				InCanvas->DrawShadowedString(DrawX, DrawY, *DrawTimeString, DrawFont, FLinearColor::Green);
				StartHeightOffset += YL + 5;
			}
		}
	}

	StartHeightOffset += 5;

	if (bSimulationEnabled) // #102
	{
		const FString DrawString = TEXT("* Gameplay Simulating...");
		int32 XL;
		int32 YL;
		StringSize(DrawFont, XL, YL, *DrawString);
		const float DrawX = FMath::FloorToFloat(SrcWidth - XL - 10.0f);
		const float DrawY = SrcHeight - YL - StartHeightOffset;
		InCanvas->DrawShadowedString(DrawX, DrawY, *DrawString, DrawFont, FLinearColor::Red);
		StartHeightOffset += YL + 5;
	}

	if (bMirrorToPIEEnabled) // #59
	{
		const FString DrawString = TEXT("* PIE Mirroring...");
		int32 XL;
		int32 YL;
		StringSize(DrawFont, XL, YL, *DrawString);
		const float DrawX = FMath::FloorToFloat(SrcWidth - XL - 10.0f);
		const float DrawY = SrcHeight - YL - StartHeightOffset;
		InCanvas->DrawShadowedString(DrawX, DrawY, *DrawString, DrawFont, FLinearColor::Red);
		StartHeightOffset += YL + 5;
	}
}

void FT4ContiViewModel::SetupStartWorld(
	FT4WorldConstructionValues& InWorldConstructionValues
) // #87
{
	if (nullptr == ContiAsset)
	{
		return;
	}
	const FT4ContiTestSettings& ContiTestSttings = ContiAsset->TestSettings;
	if (ContiTestSttings.MapEntitySelected == NAME_None)
	{
		return;
	}
	FT4EntityKey MapEntityKey(ET4EntityType::Map, ContiTestSttings.MapEntitySelected);
	IT4EntityManager* EntityManager = T4AssetEntityManagerGet();
	check(nullptr != EntityManager);
	const UT4EntityAsset* EntityAsset = EntityManager->GetEntity(MapEntityKey);
	if (nullptr == EntityAsset)
	{
		return;
	}
	if (nullptr != EntityAsset && ET4EntityType::Map == EntityAsset->GetEntityType())
	{
		InWorldConstructionValues.MapEntityOrLevelObjectPath = EntityAsset;
	}
}

void FT4ContiViewModel::NotifyViewTargetChanged(IT4GameObject* InViewTarget)
{
	// #99 : Detail 창에서의 정보 전달을 위한 업데이트! Conti 에서만 사용
	check(ViewTargetSelectorPtr.IsValid());
	check(AnimSetAssetSelectorPtr.IsValid());
	if (nullptr != InViewTarget)
	{
		ViewTargetSelectorPtr->SetViewTarget(InViewTarget, true); // #57
		UT4AnimSetAsset* AnimSetAssetSelected = GetAnimSetAssetByStance();
		AnimSetAssetSelectorPtr->SetAnimSetAsset(AnimSetAssetSelected, true);
		return;
	}
	ViewTargetSelectorPtr->SetViewTarget(nullptr, true); // #57
	AnimSetAssetSelectorPtr->SetAnimSetAsset(nullptr, true);
	if (SequencerPtr.IsValid())
	{
		SequencerPtr->SetPlaybackSpeed(1.0f); // #102
	}
}

UObject* FT4ContiViewModel::GetEditObject() const // #103
{
	return Cast<UObject>(ContiAsset);
}

FT4EditorTestAutomation* FT4ContiViewModel::GetTestAutomation() const // #103
{
	if (nullptr == ContiAsset)
	{
		return nullptr;
	}
	return &ContiAsset->TestAutomation;
}

FString FT4ContiViewModel::GetActionPlaybackAssetName() const // #68, #104
{
	return ContiAsset->GetName();
}

FString FT4ContiViewModel::GetActionPlaybackFolderName() const // #68, #104
{
	return TEXT("Conti");
}

void FT4ContiViewModel::RefreshAll()
{
	RefreshAllSequencer();
}

bool FT4ContiViewModel::CanSave() const
{
	return true;
}

bool FT4ContiViewModel::DoSave() // #56
{
	check(nullptr != ContiSequence);
	UMovieScene* MovieScene = ContiSequence->GetMovieScene();
	check(nullptr != MovieScene);
	check(nullptr != ContiAsset);
	bool bIsDirty = T4AssetUtil::HasDirtyAsset(ContiAsset);
	if (!bIsDirty)
	{
		if (CheckActionSortOrderChanged())
		{
			bIsDirty = true;
			ContiAsset->Modify();
		}
	}
	if (!bIsDirty)
	{
		return false;
	}
	const FScopedTransaction Transaction(LOCTEXT("DoSave_Transaction", "Save to ContiAsset"));
	UpdateMovieSceneFolders(); // #56 : 폴더!!
	bool bResult = UpdateMovieSceneSortOrder(true);
	if (!bResult)
	{
		return false;
	}
	{
		// #56 : Playback Range 저장
		ContiAsset->TotalPlayTimeSec = GetMovieScenePlaybackEndTimeSec();
	}
	bResult = T4AssetUtil::SaveAsset(ContiAsset, true); // #39
	return bResult;
}

TSharedPtr<ISequencer> FT4ContiViewModel::GetSequencer()
{
	return SequencerPtr;
}

void FT4ContiViewModel::UpdateMovieScenePlaybackRange() // #56
{
	check(nullptr != ContiSequence);
	UMovieScene* MovieScene = ContiSequence->GetMovieScene();
	check(nullptr != MovieScene);
	check(nullptr != ContiAsset);
	TRange<float> PlaybackRange = TRange<float>(0.0f, ContiAsset->TotalPlayTimeSec); // #56
	FFrameTime StartTime = PlaybackRange.GetLowerBoundValue() * MovieScene->GetTickResolution();
	int32      Duration = (PlaybackRange.Size<float>() * MovieScene->GetTickResolution()).FrameNumber.Value;
	MovieScene->SetPlaybackRange(StartTime.RoundToFrame(), Duration);
}

void FT4ContiViewModel::RefreshMovieSceneActionTrack(uint32 InActionHeaderKey) // #100
{
	UT4ContiActionTrack* OldActionTrack = GetMovieSceneActionTrack(InActionHeaderKey);
	if (nullptr == OldActionTrack)
	{
		return;
	}
	check(nullptr != ContiAsset);
	FT4ActionCompositeData& CompositeData = ContiAsset->CompositeData;
	if (!CompositeData.HeaderInfoMap.Contains(InActionHeaderKey))
	{
		return;
	}

	FT4ContiActionStruct* ActionStruct = ContiAsset->CompositeData.GetActionStruct(InActionHeaderKey);
	check(nullptr != ActionStruct);

	const FT4ActionHeaderInfo& ActionHeaderInfo = CompositeData.HeaderInfoMap[InActionHeaderKey];

	FT4ContiActionInfo RefreshContiActionInfo;
	RefreshContiActionInfo.ActionType = ActionHeaderInfo.ActionType;
	RefreshContiActionInfo.ActionArrayIndex = ActionHeaderInfo.ActionArrayIndex;
	RefreshContiActionInfo.ActionHeaderKey = InActionHeaderKey;
	RefreshContiActionInfo.ActionSortOrder = OldActionTrack->GetActionSortOrder(); // #56 : lower win
	RefreshContiActionInfo.FolderName = ActionHeaderInfo.FolderName; // #56
	RefreshContiActionInfo.DebugColorTint = ActionStruct->DebugColorTint; // #100

	check(nullptr != ContiSequence);
	UMovieScene* MovieScene = ContiSequence->GetMovieScene();
	check(nullptr != MovieScene);
	UMovieSceneFolder* CurrentFolder = GetMovieSceneForder(ActionHeaderInfo.FolderName);
	if (nullptr != CurrentFolder)
	{
		CurrentFolder->RemoveChildMasterTrack(OldActionTrack);
	}
	MovieScene->RemoveMasterTrack(*OldActionTrack);

	AddMovieSceneActionTrack(RefreshContiActionInfo, true, true);
}

void FT4ContiViewModel::SetPreviewViewTarget(UT4EntityAsset* EntityAsset) // #60
{
	if (nullptr == ContiEditor)
	{
		return;
	}
	ContiEditor->SetPreviewViewTarget(EntityAsset);
}

void FT4ContiViewModel::DoSaveEditorPlaySettings() // #60
{
	check(nullptr != ContiAsset);
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("DoSaveEditorPlaySettings_Transaction", "Save to TestSettings"));
	ContiAsset->Modify();

	// #T4_ADD_EDITOR_PLAY_TAG

	FT4ContiTestSettings& ContiTestSttings = ContiAsset->TestSettings;
	ContiTestSttings.WeaponNameID = EditorPlaySettingObject->WeaponContentNameIDSelected;
	ContiTestSttings.SandbackNameID = EditorPlaySettingObject->NPCContentNameIDSelected;
	ContiTestSttings.bAISystemDisabled = EditorPlaySettingObject->bAIDisabled; // #60
	ContiTestSttings.bSandbackRoleAttacker = (ET4EditorPlayRole::Attacker == EditorPlaySettingObject->SandbackRole) ? true : false; // #63
	ContiTestSttings.bSandbackOneHitDie = EditorPlaySettingObject->bSandbackOneHitDie; // #76
	ContiTestSttings.bOverrideSkillData = EditorPlaySettingObject->bOverrideSkillData; // #63
	ContiTestSttings.bOverrideEffectData = EditorPlaySettingObject->bOverrideEffectData; // #68
	ContiTestSttings.SkillDataNameID = EditorPlaySettingObject->SkillContentNameIDSelected;
	ContiTestSttings.EffectDataNameID = EditorPlaySettingObject->EffectContentNameIDSelected;
	ContiTestSttings.DieReactionNameID = EditorPlaySettingObject->DieReactionNameIDSelected; // #76
}

void FT4ContiViewModel::UpdateEditorPlayNPCEntityAsset() // #76
{
	check(nullptr != EditorPlaySettingObject);
	if (EditorPlaySettingObject->NPCContentNameIDSelected != NAME_None)
	{
		IT4EditorGameData* EditorGameData = EditorPlaySettingObject->GetEditorGameData();
		if (nullptr != EditorGameData)
		{
			EditorPlaySettingObject->NPCEntityAsset = EditorGameData->GetEntityAsset(
				ET4EditorGameDataType::EdData_NPC,
				EditorPlaySettingObject->NPCContentNameIDSelected
			);
		}
	}
	else
	{
		EditorPlaySettingObject->NPCEntityAsset.Reset();
	}
}

void FT4ContiViewModel::UpdateEditorPlaySkillDataInfo() // #63
{
	check(nullptr != EditorPlaySettingObject);
	if (EditorPlaySettingObject->SkillContentNameIDSelected != NAME_None)
	{
		IT4EditorGameData* EditorGameData = EditorPlaySettingObject->GetEditorGameData();
		if (nullptr != EditorGameData)
		{
			bool bResult = EditorGameData->GetSkillDataInfo(
				EditorPlaySettingObject->SkillContentNameIDSelected,
				EditorPlaySettingObject->SkillDataInfo
			);
			if (bResult)
			{
				UpdateEditorPlayActionParameters();
			}
		}
	}
	else
	{
		EditorPlaySettingObject->SkillDataInfo.Reset();
	}
}

void FT4ContiViewModel::UpdateEditorPlayEffectDataInfo() // #63
{
	check(nullptr != EditorPlaySettingObject);
	if (EditorPlaySettingObject->EffectContentNameIDSelected != NAME_None)
	{
		IT4EditorGameData* EditorGameData = EditorPlaySettingObject->GetEditorGameData();
		if (nullptr != EditorGameData)
		{
			bool bResult = EditorGameData->GetEffectDataInfo(
				EditorPlaySettingObject->EffectContentNameIDSelected,
				EditorPlaySettingObject->EffectDataInfo
			);
			if (bResult)
			{
				UpdateEditorPlayActionParameters();
			}
		}
	}
	else
	{
		EditorPlaySettingObject->EffectDataInfo.Reset();
	}
}

void FT4ContiViewModel::UpdateEditorPlayActionParameters() // #63
{
	check(nullptr != EditorPlaySettingObject);
	const FT4EditorSkillDataInfo& EditorSkillDataInfo = EditorPlaySettingObject->GetOverrideSkillDataInfo();
	EditorPlaySettingObject->GetContiParameters().SetProjectileSpeed(EditorSkillDataInfo.ProjectileSpeed);
}

void FT4ContiViewModel::ClearCameraSectionKeyObjects() // #58
{
	IT4GameWorld* GameWorld = GetGameWorld();
	if (nullptr != GameWorld)
	{
		for (TMap<uint32, UT4CameraWorkSectionKeyObject*>::TConstIterator It(CameraWorkSectionKeyObjects); It; ++It)
		{
			UT4CameraWorkSectionKeyObject* EditorCameraSectionKeyObject = It.Value();
			if (nullptr != EditorCameraSectionKeyObject)
			{
				EditorCameraSectionKeyObject->ClearCameraActors(GameWorld);
			}
		}
	}
	FocusCameraWorkActionKeySelected = INDEX_NONE;
	CameraWorkSectionKeyObjects.Empty();
}

UT4CameraWorkSectionKeyObject* FT4ContiViewModel::FindOrCreateCameraSectionKeyObject(
	int32 InActionHeaderKey,
	int32 InChannelKey,
	bool bInUpdate
) // #58
{
	UT4CameraWorkSectionKeyObject* EditorCameraSectionKeyObject = nullptr;
	if (CameraWorkSectionKeyObjects.Contains(InActionHeaderKey))
	{
		EditorCameraSectionKeyObject = CameraWorkSectionKeyObjects[InActionHeaderKey];
	}
	else
	{
		EditorCameraSectionKeyObject = NewObject<UT4CameraWorkSectionKeyObject>();
		check(nullptr != EditorCameraSectionKeyObject);
		EditorCameraSectionKeyObject->ViewModelRef = this;
		EditorCameraSectionKeyObject->ActionHeaderKey = InActionHeaderKey;
		CameraWorkSectionKeyObjects.Add(InActionHeaderKey, EditorCameraSectionKeyObject);
	}
	if (bInUpdate)
	{
		EditorCameraSectionKeyObject->CheckAndSpawnCameraActors(ContiAsset, GetPlayerObject());

		FFrameNumber SelectChannelKey = InChannelKey;
		if (InChannelKey == INDEX_NONE && SequencerPtr.IsValid()) // #58
		{
			SelectChannelKey = SequencerPtr->GetGlobalTime().Time.RoundToFrame();
		}
		bool bResult = EditorCameraSectionKeyObject->Select(ContiAsset, SelectChannelKey.Value);
		if (bResult)
		{
			EditorCameraSectionKeyObject->SetFocusCameraActor(GetPlayerObject());
		}
		else
		{
			EditorCameraSectionKeyObject->Reset();
		}
	}
	return EditorCameraSectionKeyObject;
}

void FT4ContiViewModel::UpdateCameraSectionKeyObjects()
{
	IT4GameObject* PlayerObject = GetPlayerObject();
	if (nullptr == PlayerObject)
	{
		return;
	}
	bool bUpdate = false;
	for (TMap<uint32, UT4CameraWorkSectionKeyObject*>::TConstIterator It(CameraWorkSectionKeyObjects); It; ++It)
	{
		UT4CameraWorkSectionKeyObject* EditorCameraSectionKeyObject = It.Value();
		if (nullptr != EditorCameraSectionKeyObject)
		{
			bool bResult = EditorCameraSectionKeyObject->UpdateCameraActor(PlayerObject);
			if (bResult)
			{
				bUpdate |= EditorCameraSectionKeyObject->SaveTo(ContiAsset);
			}
		}
	}
	if (bUpdate)
	{
		if (GetOnViewModelChanged().IsBound())
		{
			GetOnViewModelChanged().Broadcast();
		}
	}
}

void FT4ContiViewModel::JumpToPlayAction() // #99 : Keys::Up
{
	bSequencerPlayRestart = true;
	bSequencerPlayPaused = false;
	if (SequencerPtr.IsValid())
	{
		SequencerPtr->SetGlobalTime(FFrameTime(0));
		SequencerPtr->SetPlaybackSpeed(1.0f);
		SequencerPtr->Pause();
		SequencerPtr->OnPlay(false);
	}
}

void FT4ContiViewModel::JumpToEndAction() // #99 : Keys::Up + CTRL
{
	ClientStopAction(DefaultContiActionPrimaryKey);
	if (SequencerPtr.IsValid())
	{
		UMovieScene* MovieScene = ContiSequence->GetMovieScene();
		check(nullptr != MovieScene);
		const TRange<FFrameNumber> PlaybackRange = MovieScene->GetPlaybackRange();
		SequencerPtr->SetGlobalTime(PlaybackRange.GetUpperBoundValue());
		SequencerPtr->SetPlaybackSpeed(1.0f);
		SequencerPtr->Pause();
	}
	bSequencerPlayPaused = false;
}

void FT4ContiViewModel::TogglePlayAction() // #99 : Keys::Down
{
	UMovieScene* MovieScene = ContiSequence->GetMovieScene();
	check(nullptr != MovieScene);
	const TRange<FFrameNumber> PlaybackRange = MovieScene->GetPlaybackRange();
	const FFrameTime MaxFrameTime = PlaybackRange.GetUpperBoundValue();
	const FFrameTime CurrentFrameTime = SequencerPtr->GetGlobalTime().Time;
	if (0 == CurrentFrameTime || MaxFrameTime.GetFrame().Value <= (CurrentFrameTime.GetFrame().Value + 1))
	{
		bSequencerPlayPaused = false;
		JumpToPlayAction();
		return;
	}
	if (bSequencerPlayPaused)
	{
		if (SequencerPtr.IsValid())
		{
			SequencerPtr->OnPlay();
		}
	}
	else
	{
		if (SequencerPtr.IsValid())
		{
			SequencerPtr->Pause();
		}
	}
}

void FT4ContiViewModel::ToggleSimulation() // #102
{
	check(nullptr != EditorPlaySettingObject);
	bSimulationEnabled = !bSimulationEnabled;
	EditorPlaySettingObject->SetSimulationEnabled(bSimulationEnabled);

	if (HasActionPlaybackController())
	{
		// #104 : Simulation 상태에 따라 Playback 동작을 제어한다.
		UT4EditorActionPlaybackController* PlaybackController = GetActionPlaybackController();
		check(nullptr != PlaybackController);
		if (PlaybackController->IsPlayed())
		{
			if (!bSimulationEnabled)
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

void FT4ContiViewModel::ToggleMirrorToPIE() // #59
{
	if (bMirrorToPIEEnabled)
	{
		if (ET4LayerType::Max != MirrorLayerType && MirrorObjectID.IsValid())
		{
			IT4GameFrame* GameFrameMirrored = T4FrameGet(MirrorLayerType);
			if (nullptr != GameFrameMirrored)
			{
				IT4GameWorld* GameWorldMirrored = GameFrameMirrored->GetGameWorld();
				check(nullptr != GameWorldMirrored);
				IT4GameObject* PlayerObjectMirroed = GameWorldMirrored->GetPlayerObject();
				if (nullptr != PlayerObjectMirroed)
				{
					ClientStopAction(MirrorLayerType, DefaultContiActionPrimaryKey);
				}
			}
			MirrorObjectID.SetNone();
			MirrorLayerType = ET4LayerType::Max;
		}
	}
	bMirrorToPIEEnabled = !bMirrorToPIEEnabled;
	if (bMirrorToPIEEnabled)
	{
		MirrorLayerType = ET4LayerType::Client;
		IT4GameFrame* GameFrameMirrored = T4FrameGet(MirrorLayerType);
		if (nullptr == GameFrameMirrored)
		{
			UE_LOG(
				LogT4RehearsalEditor,
				Error,
				TEXT("ToggleMirrorToPIE failed. Active PIEViewport not found.")
			);
			bMirrorToPIEEnabled = !bMirrorToPIEEnabled;
			MirrorLayerType = ET4LayerType::Max;
			return;
		}
		IT4GameWorld* GameWorldMirrored = GameFrameMirrored->GetGameWorld();
		check(nullptr != GameWorldMirrored);
		IT4GameObject* PlayerObjectMirroed = GameWorldMirrored->GetPlayerObject();
		if (nullptr != PlayerObjectMirroed)
		{
			MirrorObjectID = PlayerObjectMirroed->GetObjectID();
		}
	}
}

static const uint32 AddTestActionPlayKeyValue = 1000000; // #100

void FT4ContiViewModel::PlayActionBy(int32 InActionHeaderKey) // #100
{
	check(nullptr != ContiAsset);
	const FT4ActionCompositeData& CompositeData = ContiAsset->CompositeData;
	if (!CompositeData.HeaderInfoMap.Find(InActionHeaderKey))
	{
		return;
	}
	float OffsetTimeSec = 0.0f;
	if (SequencerPtr.IsValid())
	{
		OffsetTimeSec = SequencerPtr->GetGlobalTime().AsSeconds();
	}
	IT4GameObject* PlayerObject = GetPlayerObject();
	const FT4ActionHeaderInfo& ActionHeaderInfo = CompositeData.HeaderInfoMap[InActionHeaderKey];
	switch (ActionHeaderInfo.ActionType)
	{
#define DEFINE_CONTI_ACTION_MACRO(x)															\
		case ET4ActionType::##x:																\
			{																					\
				check(ActionHeaderInfo.ActionArrayIndex < CompositeData.##x##Actions.Num());	\
				const FT4##x##Action& Action = CompositeData.##x##Actions[ActionHeaderInfo.ActionArrayIndex];	\
				if (nullptr != PlayerObject)													\
				{																				\
					FT4ActionParameters ActionParam;											\
					ActionParam.SetActionKey(FT4ActionKey(InActionHeaderKey + AddTestActionPlayKeyValue));	\
					ActionParam.SetOffsetTimeSec(OffsetTimeSec);								\
					ActionParam.SetOverrideMaxPlayTimSec(ContiAsset->TotalPlayTimeSec);			\
					ActionParam.EditorParams.bDebugPlay = true;								    \
					PlayerObject->DoExecuteAction(&Action, &ActionParam);						\
				}																				\
			}																					\
			break;

#include "Products/ContiEditor/ViewModel/T4ContiDefineMacros.h"

		default:
			{
				UE_LOG(
					LogT4RehearsalEditor,
					Error,
					TEXT("DoActionPlay '%u' failed. no implementation."),
					uint32(ActionHeaderInfo.ActionType)
				);
			}
			break;
	};
}

void FT4ContiViewModel::PlayActionStopBy(int32 InActionHeaderKey) // #100
{
	check(nullptr != ContiAsset);
	const FT4ActionCompositeData& CompositeData = ContiAsset->CompositeData;
	if (!CompositeData.HeaderInfoMap.Find(InActionHeaderKey))
	{
		return;
	}
	IT4GameObject* PlayerObject = GetPlayerObject();
	const FT4ActionHeaderInfo& ActionHeaderInfo = CompositeData.HeaderInfoMap[InActionHeaderKey];
	switch (ActionHeaderInfo.ActionType)
	{
#define DEFINE_CONTI_ACTION_MACRO(x)															\
		case ET4ActionType::##x:																\
			{																					\
				if (nullptr != PlayerObject)													\
				{																				\
					FT4StopAction StopAction;													\
					StopAction.ActionKey = FT4ActionKey(InActionHeaderKey + AddTestActionPlayKeyValue);	\
					StopAction.StartTimeSec = 0.0f;												\
					PlayerObject->DoExecuteAction(&StopAction);									\
				}																				\
			}																					\
			break;

#include "Products/ContiEditor/ViewModel/T4ContiDefineMacros.h"

		default:
			{
				UE_LOG(
					LogT4RehearsalEditor,
					Error,
					TEXT("DoActionPlayStop '%u' failed. no implementation."),
					uint32(ActionHeaderInfo.ActionType)
				);
			}
			break;
	};
}

#define DEFINE_CONTI_ACTION_RESET_MACRO(x)														\
		case ET4ActionType::##x:																\
			{																					\
				check(ActionHeaderInfo.ActionArrayIndex < CompositeData.##x##Actions.Num());	\
				FT4##x##Action& x##Action = CompositeData.##x##Actions[ActionHeaderInfo.ActionArrayIndex]; \
				PlayActionStopBy(InActionHeaderKey);											\
				x##Action.Reset();																\
			}																					\
			break;

void FT4ContiViewModel::PlayActionResetBy(int32 InActionHeaderKey) // #100
{
	check(nullptr != ContiAsset);
	FT4ActionCompositeData& CompositeData = ContiAsset->CompositeData;
	if (!CompositeData.HeaderInfoMap.Find(InActionHeaderKey))
	{
		return;
	}
	const FScopedTransaction Transaction(LOCTEXT("PlayActionResetBy_Transaction", "Reset Action"));
	ContiAsset->Modify();
	const FT4ActionHeaderInfo& ActionHeaderInfo = CompositeData.HeaderInfoMap[InActionHeaderKey];
	switch (ActionHeaderInfo.ActionType)
	{
		// T4_ADD_ACTION_TAG_CONTI
		// Optional

		DEFINE_CONTI_ACTION_RESET_MACRO(CameraWork)
		DEFINE_CONTI_ACTION_RESET_MACRO(CameraShake)
		DEFINE_CONTI_ACTION_RESET_MACRO(PostProcess)

		default:
			{
				UE_LOG(
					LogT4RehearsalEditor,
					Error,
					TEXT("DoActionReset '%u' failed. no implementation."),
					uint32(ActionHeaderInfo.ActionType)
				);
			}
			break;
	};
}

bool FT4ContiViewModel::AddNewMovieSceneActionTrack(
	ET4ActionType InActionType,
	FT4ContiActionStruct* InSourceData, // #75 : nullptr != Copy&Paste
	FName InFolderName // #75
)
{
	check(nullptr != ContiAsset);
	ContiAsset->Modify();

	FT4ActionCompositeData& CompositeData = ContiAsset->CompositeData;
	FT4ContiActionStruct* NewAction = CompositeData.NewAndAddAction(InActionType);
	if (nullptr == NewAction)
	{
		return false;
	}

	if (!CompositeData.HeaderInfoMap.Contains(NewAction->HeaderKey))
	{
		UE_LOG(
			LogT4RehearsalEditor,
			Error,
			TEXT("AddNewMovieSceneActionTrack failed. ActionHeaderKey '%u' not found"),
			NewAction->HeaderKey
		);
		return false;
	}

	UMovieScene* MovieScene = ContiSequence->GetMovieScene();
	check(nullptr != MovieScene);

	if (nullptr != InSourceData) // #75 : nullptr != Copy&Paste
	{
		int32 SwapHeaderKey = NewAction->HeaderKey;
		//uint32 SwapSortOrder = NewAction->SortOrder;
		switch (InActionType)
		{
#define DEFINE_CONTI_ACTION_MACRO(x)															\
			case ET4ActionType::##x:															\
				{																				\
					FT4##x##Action& TargetAction = *(static_cast<FT4##x##Action*>(NewAction));	\
					TargetAction = *(static_cast<FT4##x##Action*>(InSourceData));				\
				}																				\
				break;

#include "Products/ContiEditor/ViewModel/T4ContiDefineMacros.h"
			default:
				{
					UE_LOG(
						LogT4RehearsalEditor,
						Error,
						TEXT("AddNewMovieSceneActionTrack '%u' failed. no implementation."),
						uint32(InActionType)
					);
				}
				break;
		}
		NewAction->HeaderKey = SwapHeaderKey;
		NewAction->SortOrder = InSourceData->SortOrder + 1; //NewAction->SortOrder = SwapSortOrder;
		NewAction->DebugColorTint = InSourceData->DebugColorTint; // #100
	}
	else
	{
		// #58 : 첫 생성이라면 DelayTime 을 Bar 시작 위치에 생성하도록 처리
		if (SequencerPtr.IsValid())
		{
			NewAction->StartTimeSec = SequencerPtr->GetGlobalTime().AsSeconds();
		}
		if (ET4ActionType::CameraWork == InActionType)
		{
			// #58 : CameraWork 는 오직 Key 만 사용! todo : 케이스가 하나 더 생기면 Action Struct 에 쿼리하도록 처리한다.
			FT4CameraWorkAction* CameraWorkStruct = static_cast<FT4CameraWorkAction*>(NewAction);
			check(nullptr != CameraWorkStruct);
			FT4CameraWorkSectionKeyData& NewSectionKey = CameraWorkStruct->SectionData.KeyDatas.AddDefaulted_GetRef();
			{
				// Track Section 의 FFrameNumber 즉, FrameNumber 가 Unique Key 가 됨으로 저장해준다.
				const FFrameRate FrameResolution = MovieScene->GetTickResolution();
				FFrameTime KeySectionStartTime = FrameResolution.AsFrameTime(NewAction->StartTimeSec);
				NewSectionKey.ChannelKey = KeySectionStartTime.RoundToFrame().Value;
			}
			NewSectionKey.StartTimeSec = NewAction->StartTimeSec;
			NewAction->StartTimeSec = 0.0f; // Section Key 가 찍히기 때문에 delayTime 은 0으로 만들어준다.
		}
	}

	const FT4ActionHeaderInfo& ActionHeaderInfo = CompositeData.HeaderInfoMap[NewAction->HeaderKey];

	FT4ContiActionInfo NewContiActionInfo;
	NewContiActionInfo.ActionType = InActionType;
	NewContiActionInfo.ActionArrayIndex = ActionHeaderInfo.ActionArrayIndex;
	NewContiActionInfo.ActionHeaderKey = NewAction->HeaderKey;
	NewContiActionInfo.ActionSortOrder = NewAction->SortOrder; // #56 : lower win
	NewContiActionInfo.FolderName = InFolderName; // #56
	NewContiActionInfo.DebugColorTint = NewAction->DebugColorTint; // #100

	SelectedActionInfos.Empty();
	SelectedActionInfos.Add(NewContiActionInfo);

	AddMovieSceneActionTrack(NewContiActionInfo, true, true);

	UpdateMovieSceneSortOrder(true);

	// #54 : 변경사항을 DetailView 에 알린다.
	if (GetOnViewModelChanged().IsBound())
	{
		GetOnViewModelChanged().Broadcast();
	}
	return true;
}

bool FT4ContiViewModel::AddNewMovieSceneActionSectionKey(
	uint32 InActionHeaderKey,
	int32 InChannelKey
) // #58 : ChannelKey (FrameNumber)
{
	FT4ContiActionStruct* ActionStruct = GetActionStruct(InActionHeaderKey);
	if (nullptr == ActionStruct)
	{
		return false;
	}
	FT4CameraWorkAction* CameraWorkStruct = static_cast<FT4CameraWorkAction*>(ActionStruct);
	check(nullptr != CameraWorkStruct);
	FT4CameraWorkSectionData& SectionData = CameraWorkStruct->SectionData;

	for (const FT4CameraWorkSectionKeyData& KeyData : SectionData.KeyDatas)
	{
		if (KeyData.ChannelKey == InChannelKey)
		{
			return false; // 중복은 허용 안함!
		}
	}
	FT4CameraWorkSectionKeyData& NewKeyData = SectionData.KeyDatas.AddDefaulted_GetRef();
	NewKeyData = SectionData.KeyDatas[SectionData.KeyDatas.Num()-2]; // 하나 앞의 키에서 값 복사!
	NewKeyData.ChannelKey = InChannelKey;

	UMovieScene* MovieScene = ContiSequence->GetMovieScene();
	check(nullptr != MovieScene);
	FFrameRate TickResolution = MovieScene->GetTickResolution();
	NewKeyData.StartTimeSec = FFrameTime(FFrameNumber(InChannelKey)) / TickResolution;

	FindOrCreateCameraSectionKeyObject(InActionHeaderKey, InChannelKey, true); // 추가된 Key 로 자동 선택!

	// #54 : 변경사항을 DetailView 에 알린다.
	if (GetOnViewModelChanged().IsBound())
	{
		GetOnViewModelChanged().Broadcast();
	}
	return true;
}

bool FT4ContiViewModel::RemoveMovieSceneActionSectionKey(
	uint32 InActionHeaderKey,
	int32 InChannelKey
) // #58 : ChannelKey (FrameNumber)
{
	FT4ContiActionStruct* ActionStruct = GetActionStruct(InActionHeaderKey);
	if (nullptr == ActionStruct)
	{
		return false;
	}
	FT4CameraWorkAction* CameraWorkStruct = static_cast<FT4CameraWorkAction*>(ActionStruct);
	check(nullptr != CameraWorkStruct);
	FT4CameraWorkSectionData& SectionData = CameraWorkStruct->SectionData;

	if (1 >= SectionData.KeyDatas.Num())
	{
		return false; // 1개는 삭제하지 못하도록, Track or Section 을 삭제하는 것으로 정리...
	}

	bool bValid = false;
	int32 KeyCount = 0;
	for (const FT4CameraWorkSectionKeyData& KeyData : SectionData.KeyDatas)
	{
		if (KeyData.ChannelKey == InChannelKey)
		{
			SectionData.KeyDatas.RemoveAt(KeyCount);
			bValid = true;
			break;
		}
		KeyCount++;
	}

	if (!bValid)
	{
		return false;
	}

	int32 NextChannelKey = INDEX_NONE;
	if (KeyCount < SectionData.KeyDatas.Num())
	{
		NextChannelKey = SectionData.KeyDatas[KeyCount].ChannelKey; // 다음 Key 자동 선택
	}
	else
	{
		check(0 < KeyCount);
		check(KeyCount - 1 < SectionData.KeyDatas.Num())
		NextChannelKey = SectionData.KeyDatas[KeyCount - 1].ChannelKey; // 다음 Key 자동 선택
	}
	FindOrCreateCameraSectionKeyObject(InActionHeaderKey, NextChannelKey, true);

	// #54 : 변경사항을 DetailView 에 알린다.
	if (GetOnViewModelChanged().IsBound())
	{
		GetOnViewModelChanged().Broadcast();
	}

	return true;
}

bool FT4ContiViewModel::IsActionInvisible(
	const UT4ContiActionTrack* InContiActionTrack
) const 
{ // #56
	check(nullptr != InContiActionTrack);
	check(nullptr != EditorPlaySettingObject);
	const uint32 CurrHeaderKey = InContiActionTrack->GetActionHeaderKey();
	if (!EditorPlaySettingObject->GetContiParameters().EditorParams.InvisibleActionSet.Contains(CurrHeaderKey))
	{
		return false;
	}
	return true;
}

bool FT4ContiViewModel::IsActionIsolate(
	const UT4ContiActionTrack* InContiActionTrack
) const
{ // #56
	check(nullptr != InContiActionTrack);
	check(nullptr != EditorPlaySettingObject);
	const uint32 CurrHeaderKey = InContiActionTrack->GetActionHeaderKey();
	if (!EditorPlaySettingObject->GetContiParameters().EditorParams.IsolationActionSet.Contains(CurrHeaderKey))
	{
		return false;
	}
	return true;
}

void FT4ContiViewModel::SetActionInvisible(
	UT4ContiActionTrack* InContiActionTrack,
	bool bInvisible
) // #56
{
	check(nullptr != EditorPlaySettingObject);
	check(nullptr != InContiActionTrack);
	const uint32 CurrHeaderKey = InContiActionTrack->GetActionHeaderKey();
	FT4EditorParameters& EditorParams = EditorPlaySettingObject->GetContiParameters().EditorParams;
	bool bHasAction = EditorParams.InvisibleActionSet.Contains(CurrHeaderKey);
	if (bInvisible)
	{
		if (!bHasAction)
		{
			EditorParams.InvisibleActionSet.Add(CurrHeaderKey);
		}
	}
	else
	{
		if (bHasAction)
		{
			EditorParams.InvisibleActionSet.Remove(CurrHeaderKey);
		}
	}
	TArray<UMovieSceneSection*> AllSections = InContiActionTrack->GetAllSections();
	for (UMovieSceneSection* Section : AllSections)
	{
		Section->Modify();
		Section->SetIsActive(!bInvisible);
	}
}

void FT4ContiViewModel::SetActionIsolation(
	UT4ContiActionTrack* InContiActionTrack,
	bool bIsolate
) // #56
{
	check(nullptr != EditorPlaySettingObject);
	check(nullptr != InContiActionTrack);
	const uint32 CurrHeaderKey = InContiActionTrack->GetActionHeaderKey();
	FT4EditorParameters& EditorParams = EditorPlaySettingObject->GetContiParameters().EditorParams;
	bool bHasAction = EditorParams.IsolationActionSet.Contains(CurrHeaderKey);
	if (bIsolate)
	{
		if (!bHasAction)
		{
			EditorParams.IsolationActionSet.Add(CurrHeaderKey);
		}
	}
	else
	{
		if (bHasAction)
		{
			EditorParams.IsolationActionSet.Remove(CurrHeaderKey);
		}
	}
}

const FText FT4ContiViewModel::GetActionSectionDisplayName(
	const UT4ContiActionSection* InContiActionSection
) // #54
{
	FString SectionDisplayName = TEXT("Untitled");
	check(nullptr != ContiAsset);
	const uint32 CurrHeaderKey = InContiActionSection->GetActionHeaderKey();
	FT4ContiActionStruct* ActionStruct = ContiAsset->CompositeData.GetActionStruct(CurrHeaderKey);
	if (nullptr != ActionStruct)
	{
		SectionDisplayName = ActionStruct->ToDisplayText();
	}
	return FText::FromString(SectionDisplayName);;
}

void FT4ContiViewModel::CheckAndSpawnEntity() // #67
{
	if (nullptr == ContiAsset)
	{
		return;
	}
	const bool bEntityValid = (!ContiAsset->PreviewEntityAsset.IsNull()) ? true : false;
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
		}
		return;
	}
	if (!bEntityValid)
	{
		return;
	}
	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	if (bTestAutomation) // #100
	{
		SpawnLocation = TestAutomationSpawnLocation; // #100
		SpawnLocation.Z += 100.0f;
		SpawnRotation = TestAutomationSpawnRotation; // #100
		GetGameWorld()->GetController()->SetGameTimeHour(TestAutomationGameTimeHour); // #103
	}
	else
	{
		bool bResult = TryValidSpawnObjectLocation(SpawnLocation); // #87 : Entity 임으로 반드시(최대한) 스폰이 되어야 한다!!
		if (!bResult)
		{
			return;
		}
	}
	FT4ContiTestSettings& ContiTestSttings = ContiAsset->TestSettings;
	FT4EntityKey SpawnEntityKey(
		ContiAsset->PreviewEntityAsset->GetEntityType(),
		ContiAsset->PreviewEntityAsset->GetEntityKeyPath()
	);
	IT4GameObject* GameObject = ClientSpawnObjectEx(
		SpawnEntityKey,
		SpawnLocation,
		SpawnRotation,
		ContiTestSttings.StanceSelected,
		true
	);
	if (nullptr == GameObject)
	{
		return;
	}
	if (ContiTestSttings.WeaponNameID != NAME_None)
	{
		ServerEquipWeapon(ContiTestSttings.WeaponNameID, false); // #60
	}
	bPlayerSpawnd = true;
	bTestAutomation = false; // #100
}

void FT4ContiViewModel::SetupSequencer()
{
	check(nullptr == ContiSequence);
	check(nullptr != ContiAsset);
	ContiSequence = NewObject<UT4ContiActionSequence>(GetTransientPackage());
	UT4ContiActionMovieScene* MovieScene = NewObject<UT4ContiActionMovieScene>(
		ContiSequence,
		FName("ContiActionMovieScene"),
		RF_Transactional
	); // #54
	MovieScene->SetDisplayRate(FFrameRate(240, 1));

	ContiSequence->Initialize(this, MovieScene);

	const float PlaybackRangeMin = 0;
	const float PlaybackRangeMax = ContiAsset->TotalPlayTimeSec; // #56
	TRange<float> PlaybackRange = TRange<float>(PlaybackRangeMin, PlaybackRangeMax);

	FFrameTime StartTime = PlaybackRange.GetLowerBoundValue() * MovieScene->GetTickResolution();
	int32      Duration = (PlaybackRange.Size<float>() * MovieScene->GetTickResolution()).FrameNumber.Value;
	MovieScene->SetPlaybackRange(StartTime.RoundToFrame(), Duration);

	FMovieSceneEditorData& EditorData = MovieScene->GetEditorData();
	const float ViewTimeOffset = 0.1f;
	EditorData.WorkStart = PlaybackRange.GetLowerBoundValue() - ViewTimeOffset;
	EditorData.WorkEnd = PlaybackRange.GetUpperBoundValue() + ViewTimeOffset;
	EditorData.ViewStart = EditorData.WorkStart;
	EditorData.ViewEnd = EditorData.WorkEnd;

	FSequencerViewParams ViewParams(TEXT("T4RhearsalContiSettings"));
	{
		ViewParams.UniqueName = "T4RhearsalContiEditor";
		ViewParams.OnGetAddMenuContent = OnGetSequencerAddMenuContent;
		ViewParams.ScrubberStyle = ESequencerScrubberStyle::FrameBlock;
	}

	FSequencerInitParams SequencerInitParams;
	{
		SequencerInitParams.ViewParams = ViewParams;
		SequencerInitParams.RootSequence = ContiSequence;
		SequencerInitParams.bEditWithinLevelEditor = false;
		SequencerInitParams.ToolkitHost = nullptr;
	}

	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked< ISequencerModule >("Sequencer");
	SequencerPtr = SequencerModule.CreateSequencer(SequencerInitParams);
	check(SequencerPtr.IsValid());

	SequencerPtr->OnPlayEvent().AddRaw(this, &FT4ContiViewModel::HandleOnSequencerPlayEvent);
	SequencerPtr->OnStopEvent().AddRaw(this, &FT4ContiViewModel::HandleOnSequencerStopEvent);

	SequencerPtr->OnMovieSceneDataChanged().AddRaw(this, &FT4ContiViewModel::HandleOnSequencerDataChanged);
	SequencerPtr->OnGlobalTimeChanged().AddRaw(this, &FT4ContiViewModel::HandleOnSequencerTimeChanged);
	SequencerPtr->GetSelectionChangedTracks().AddRaw(this, &FT4ContiViewModel::HandleOnSequencerTrackSelectionChanged);
	SequencerPtr->GetSelectionChangedSections().AddRaw(this, &FT4ContiViewModel::HandleOnSequencerSectionSelectionChanged);
	SequencerPtr->OnGetIsTrackVisible().BindRaw(this, &FT4ContiViewModel::HandleOnSequencerIsTrackVisible);
	SequencerPtr->SetPlaybackStatus(EMovieScenePlayerStatus::Stopped);

	// #75 : Sequencer 의 Key Binding 을 가로채어 Track 제어를 구현한다. #65 에서 임시처리했던 것들을 함께 복구함
	TSharedPtr<FUICommandList> SequencerCommandBindings = SequencerPtr->GetCommandBindings();
	SequencerCommandBindings->MapAction(
		FGenericCommands::Get().Copy,
		FExecuteAction::CreateRaw(this, &FT4ContiViewModel::HandleOnCopySelection)
	);
	SequencerCommandBindings->MapAction(
		FGenericCommands::Get().Paste,
		FExecuteAction::CreateRaw(this, &FT4ContiViewModel::HandleOnPaste)
	);
#if 0
	SequencerCommandBindings->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateRaw(this, &FT4ContiViewModel::HandleOnDeleteSelectedItems)
	);
#endif

	// #75 : 기존 Sequencer 에 바인딩된 Action 을 Conti 에서는 무력화하기 위한 등록...
	{
		SequencerCommandBindings->MapAction(
			FGenericCommands::Get().Cut,
			FExecuteAction::CreateRaw(this, &FT4ContiViewModel::HandleOnNoAction)
		);
		SequencerCommandBindings->MapAction(
			FGenericCommands::Get().Duplicate,
			FExecuteAction::CreateRaw(this, &FT4ContiViewModel::HandleOnNoAction)
		);
	}

	USequencerSettings* SequencerSettings = SequencerPtr->GetSequencerSettings();
	if (nullptr != SequencerSettings)
	{
		SequencerSettings->SetKeepPlayRangeInSectionBounds(false); // #56 : Playback Range 가 자동 조절되지 않도록 처리한다.
	}

	RefreshMovieSceneActionTracks(false);
}

void FT4ContiViewModel::PlayContiInternal(float InPlayOffsetTime) // #99
{
	check(nullptr != EditorPlaySettingObject);
	check(nullptr != ContiAsset);

	const FT4ActionCompositeData& CompositeData = ContiAsset->CompositeData;
	if (0 >= CompositeData.HeaderInfoMap.Num())
	{
		return;
	}

	IT4GameObject* PlayerObject = GetPlayerObject();
	if (nullptr != PlayerObject)
	{
		// #100 : 테스트로 플레이 했던 Action 을 모두 Stop 처리 해준다.
		for (TMap<uint32, FT4ActionHeaderInfo>::TConstIterator It(CompositeData.HeaderInfoMap); It; ++It)
		{
			const FT4ActionKey TestActionKey(It.Key());
			if (PlayerObject->HasPublicAction(TestActionKey))
			{
				FT4StopAction StopAction;
				StopAction.ActionKey = TestActionKey;
				StopAction.StartTimeSec = 0.0f;
				PlayerObject->DoExecuteAction(&StopAction);
			}
		}
	}

	FT4ActionParameters CloneContiParameters = EditorPlaySettingObject->GetContiParameters();
	CloneContiParameters.SetOffsetTimeSec(InPlayOffsetTime); // #56
	CloneContiParameters.SetAnimationNoBlendInTimeWithOffsetPlay(); // #54 : 애니 BlendIn Time 을 없앤다.
	ClientPlayConti(
		ContiAsset,
		DefaultContiActionPrimaryKey,
		&CloneContiParameters,
		true
	);

	if (bMirrorToPIEEnabled) // #59
	{
		if (ET4LayerType::Max != MirrorLayerType && MirrorObjectID.IsValid())
		{
			ClientPlayConti(
				MirrorLayerType,
				ContiAsset,
				DefaultContiActionPrimaryKey,
				&CloneContiParameters,
				true
			);
		}
	}
}

UT4ContiActionTrack* FT4ContiViewModel::GetMovieSceneActionTrack(uint32 InActionHeaderKey)
{
	check(nullptr != ContiSequence);
	UMovieScene* MovieScene = ContiSequence->GetMovieScene();
	check(nullptr != MovieScene);
	const TArray<UMovieSceneTrack*> AllMasterTracks = MovieScene->GetMasterTracks();
	for (UMovieSceneTrack* SceneTrack : AllMasterTracks)
	{
		UT4ContiActionTrack* ActionTrack = Cast<UT4ContiActionTrack>(SceneTrack);
		check(nullptr != ActionTrack);
		const uint32 CurrHeaderKey = ActionTrack->GetActionHeaderKey();
		if (InActionHeaderKey == CurrHeaderKey)
		{
			return ActionTrack;
		}
	}
	return nullptr;
}

bool FT4ContiViewModel::AddMovieSceneActionTrack(
	const FT4ContiActionInfo& InContiActionInfo,
	bool bSelection,
	bool bNotify // #56
)
{
	check(nullptr != ContiSequence);
	check(nullptr != ContiAsset);
	FT4ContiActionStruct* ActionStruct = GetActionStruct(InContiActionInfo.ActionHeaderKey);
	if (nullptr == ActionStruct)
	{
		return false;
	}

	check(ActionStruct->ActionType == InContiActionInfo.ActionType);

	const FName DisplayName = ActionStruct->DisplayName;
	const ET4LifecycleType LifecycleType = ActionStruct->LifecycleType;

	UMovieScene* MovieScene = ContiSequence->GetMovieScene();
	check(nullptr != MovieScene);

	UT4ContiActionTrack* NewActionTrack = MovieScene->AddMasterTrack<UT4ContiActionTrack>();
	if (InContiActionInfo.FolderName != NAME_None)
	{
		UMovieSceneFolder* MovieSceneFolder = FindAndAddMovieSceneForder(
			InContiActionInfo.FolderName, 
			DefaultActionSortOrder
		); // #56, #75
		if (nullptr != MovieSceneFolder)
		{
			MovieSceneFolder->AddChildMasterTrack(NewActionTrack);
		}
	}

	NewActionTrack->Initialize(
		InstanceKey,
		InContiActionInfo.ActionType,
		InContiActionInfo.ActionHeaderKey,
		LifecycleType,
		InContiActionInfo.ActionSortOrder,
		FText::FromName(DisplayName)
	);

	NewActionTrack->SetSortingOrder(InContiActionInfo.ActionSortOrder);
	NewActionTrack->SetColorTint(InContiActionInfo.DebugColorTint);

	UMovieSceneSection* NewSection = NewActionTrack->CreateNewSection();
	check(NewSection);

	UT4ContiActionSection* NewContiSection = Cast<UT4ContiActionSection>(NewSection);
	check(NewContiSection);

	NewContiSection->Initialize(
		InstanceKey,
		NewActionTrack->GetActionType(),
		NewActionTrack->GetActionHeaderKey(),
		NewActionTrack->GetLifecycleType()
	);

	NewActionTrack->AddSection(*NewSection);

	const FFrameRate FrameResolution = MovieScene->GetTickResolution();

	if (ET4ActionType::CameraWork == InContiActionInfo.ActionType)
	{
		// #58 : CameraWork 는 오직 Key 만 사용! todo : 케이스가 하나 더 생기면 Action Struct 에 쿼리하도록 처리한다.
		FT4CameraWorkAction* CameraWorkStruct = static_cast<FT4CameraWorkAction*>(ActionStruct);
		check(nullptr != CameraWorkStruct);
		check(ET4LifecycleType::Duration != LifecycleType);
		const FT4CameraWorkSectionData& SectionData = CameraWorkStruct->SectionData;
		check(0 < SectionData.KeyDatas.Num());
		for (const FT4CameraWorkSectionKeyData& Data : SectionData.KeyDatas)
		{
			NewContiSection->AddKey(FFrameNumber(Data.ChannelKey));
		}
	}
	else
	{
		// #56
		if (ET4LifecycleType::Duration == LifecycleType)
		{
			FFrameTime SectionStartTime = FrameResolution.AsFrameTime(ActionStruct->StartTimeSec);
			FFrameTime SectionDuration = FrameResolution.AsFrameTime(ActionStruct->DurationSec);

			NewContiSection->SetRange(TRange<FFrameNumber>(
				SectionStartTime.RoundToFrame(),
				(SectionStartTime + SectionDuration).RoundToFrame()
			));
		}
		else
		{
			FFrameTime KeySectionStartTime = FrameResolution.AsFrameTime(ActionStruct->StartTimeSec);
			NewContiSection->AddKey(KeySectionStartTime.RoundToFrame());
		}
	}

	if (SequencerPtr.IsValid())
	{
		if (bSelection)
		{
			SequencerPtr->OnAddTrack(NewActionTrack, FGuid());
			if (nullptr != NewSection)
			{
				SequencerPtr->SelectSection(NewSection);
			}
		}
		if (bNotify)
		{
			SequencerPtr->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
		}
	}
	return true;
}

void FT4ContiViewModel::RefreshMovieSceneActionTracks(bool bSelection)
{
	check(nullptr != ContiSequence);
	check(nullptr != ContiAsset);
	{
		UMovieScene* MovieScene = ContiSequence->GetMovieScene();
		check(nullptr != MovieScene);
		const TArray<UMovieSceneTrack*> AllMasterTracks = MovieScene->GetMasterTracks();
		for (UMovieSceneTrack* SceneTrack : AllMasterTracks)
		{
			check(nullptr != SceneTrack);
			MovieScene->RemoveMasterTrack(*SceneTrack);
		}
		if (SequencerPtr.IsValid())
		{
			SequencerPtr->EmptySelection();
		}
	}
	{
		ClearCameraSectionKeyObjects(); // #58
	}
	bool bAdded = false;
	const FT4ActionCompositeData& CompositeData = ContiAsset->CompositeData;
	// #56 : 폴더 만들기
	for (TMap<FName, FT4ActionFolderInfo>::TConstIterator It(CompositeData.FolderInfoMap); It; ++It)
	{
		const FT4ActionFolderInfo& FolderInfo = It.Value();
		FindAndAddMovieSceneForder(It.Key(), FolderInfo.SortOrder);
	}
	for (TMap<uint32, FT4ActionHeaderInfo>::TConstIterator It(CompositeData.HeaderInfoMap); It; ++It)
	{
		const uint32 CurrHierarchyKey = It.Key();
		FT4ContiActionStruct* ActionStruct = ContiAsset->CompositeData.GetActionStruct(CurrHierarchyKey); // #56
		if (nullptr == ActionStruct)
		{
			UE_LOG(
				LogT4RehearsalEditor,
				Error,
				TEXT("UpdateMovieSceneSortOrder failed. ActionStruct not found.")
			);
			continue;
		}
		check(CurrHierarchyKey == ActionStruct->HeaderKey);
		const FT4ActionHeaderInfo& HierarchyInfo = It.Value();

		FT4ContiActionInfo NewContiActionInfo;
		NewContiActionInfo.ActionType = HierarchyInfo.ActionType;
		NewContiActionInfo.ActionArrayIndex = HierarchyInfo.ActionArrayIndex;
		NewContiActionInfo.ActionHeaderKey = ActionStruct->HeaderKey;
		NewContiActionInfo.ActionSortOrder = ActionStruct->SortOrder; // #56 : lower win
		NewContiActionInfo.FolderName = HierarchyInfo.FolderName; // #56
		NewContiActionInfo.DebugColorTint = ActionStruct->DebugColorTint; // #100
		bAdded |= AddMovieSceneActionTrack(NewContiActionInfo, bSelection, false);
	}
	if (bAdded && SequencerPtr.IsValid())
	{
		SequencerPtr->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
	}
}

void FT4ContiViewModel::RefreshAllSequencer()
{
	check(SequencerPtr.IsValid());
	check(nullptr != ContiAsset);
	check(nullptr != ContiSequence);

	// #56 : UI 갱신 및 Selection 까지 모두 Refresh
	TGuardValue<bool> UpdateGuard(bUpdatingContiSelectionFromSequencer, true);
	RefreshMovieSceneActionTracks(false);
	UMovieScene* MovieScene = ContiSequence->GetMovieScene();
	check(nullptr != MovieScene);
	for (UMovieSceneTrack* MasterTrack : MovieScene->GetMasterTracks())
	{
		if (HasActionSelected(MasterTrack))
		{
			SequencerPtr->SelectTrack(MasterTrack);
			const TArray<UMovieSceneSection*>& AllSections = MasterTrack->GetAllSections();
			for (UMovieSceneSection* Section : AllSections)
			{
				SequencerPtr->SelectSection(Section);
			}
		}
	}
	// #54 : 변경사항을 DetailView 에 알린다.
	if (GetOnViewModelChanged().IsBound())
	{
		GetOnViewModelChanged().Broadcast();
	}
}

void FT4ContiViewModel::UpdateMovieSceneFolders() // #56
{
	check(nullptr != ContiSequence);
	check(nullptr != ContiAsset);
	UMovieScene* MovieScene = ContiSequence->GetMovieScene();
	check(nullptr != MovieScene);

	ContiAsset->Modify(); // TODO : dirty check!!

	FT4ActionCompositeData& CompositeData = ContiAsset->CompositeData;

	// 새로운 폴더 추가
	TArray<UMovieSceneFolder*>& RootForder = MovieScene->GetRootFolders();
	for (UMovieSceneFolder* SceneFolder : RootForder)
	{
		const FName FolderName = SceneFolder->GetFolderName();
		if (!CompositeData.FolderInfoMap.Contains(FolderName))
		{
			FT4ActionFolderInfo NewFolderInfo;
			NewFolderInfo.SortOrder = SceneFolder->GetSortingOrder();
			CompositeData.FolderInfoMap.Add(FolderName, NewFolderInfo);
			FindAndAddMovieSceneForder(FolderName, NewFolderInfo.SortOrder);
		}
		else
		{
			FT4ActionFolderInfo& FolderInfo = CompositeData.FolderInfoMap[FolderName];
			FolderInfo.SortOrder = SceneFolder->GetSortingOrder();
		}
	}
	// 삭제된 폴더 정리
	for (TMap<FName, FT4ActionFolderInfo>::TConstIterator It(CompositeData.FolderInfoMap); It; ++It)
	{
		bool bValid = false;
		for (UMovieSceneFolder* SceneFolder : RootForder)
		{
			if (SceneFolder->GetFolderName() == It.Key())
			{
				bValid = true;
				break;
			}
		}
		if (!bValid)
		{
			CompositeData.FolderInfoMap.Remove(It.Key());
		}
	}
	// 전체 Track 업데이트
	const TArray<UMovieSceneTrack*> AllMasterTracks = MovieScene->GetMasterTracks();
	for (UMovieSceneTrack* SceneTrack : AllMasterTracks)
	{
		FName FolderName = NAME_None;
		for (UMovieSceneFolder* SceneFolder : RootForder)
		{
			const TArray<UMovieSceneTrack*>& FolderChildTracks = SceneFolder->GetChildMasterTracks();
			for (UMovieSceneTrack* ChildTrack : FolderChildTracks)
			{
				if (SceneTrack == ChildTrack)
				{
					FolderName = SceneFolder->GetFolderName();
					break;
				}
			}
			if (FolderName != NAME_None)
			{
				break;
			}
		}
		// 있든 없든, FolderName 은 업데이트
		UT4ContiActionTrack* ActionTrack = Cast<UT4ContiActionTrack>(SceneTrack);
		check(nullptr != ActionTrack);
		uint32 CurrHeaderKey = ActionTrack->GetActionHeaderKey();
		if (CompositeData.HeaderInfoMap.Contains(CurrHeaderKey))
		{
			FT4ActionHeaderInfo& HierarchyInfo = CompositeData.HeaderInfoMap[CurrHeaderKey];
			HierarchyInfo.FolderName = FolderName;
		}
	}
}

bool FT4ContiViewModel::UpdateMovieSceneSortOrder(bool bCheckDirty)
{
	check(nullptr != ContiAsset);
	if (!CheckActionSortOrderChanged())
	{
		return true;
	}
	UMovieScene* MovieScene = ContiSequence->GetMovieScene();
	check(nullptr != MovieScene);
	if (bCheckDirty)
	{
		bool bIsDirty = T4AssetUtil::HasDirtyAsset(ContiAsset);
		if (!bIsDirty)
		{
			ContiAsset->Modify();
		}
	}
	// #56 : 저장 또는 Track 위치 변경시 SortOrder 변경
	FT4ActionCompositeData& CompositeData = ContiAsset->CompositeData;
	const TArray<UMovieSceneTrack*> AllMasterTracks = MovieScene->GetMasterTracks();
	for (UMovieSceneTrack* SceneTrack : AllMasterTracks)
	{
		UT4ContiActionTrack* ActionTrack = Cast<UT4ContiActionTrack>(SceneTrack);
		check(nullptr != ActionTrack);
		const uint32 CurrHeaderKey = ActionTrack->GetActionHeaderKey();
		FT4ContiActionStruct* ActionStruct = CompositeData.GetActionStruct(CurrHeaderKey);
		if (nullptr == ActionStruct)
		{
			UE_LOG(
				LogT4RehearsalEditor,
				Error,
				TEXT("UpdateMovieSceneSortOrder failed. ActionStruct not found.")
			);
			return false;
		}
		const int32 SortOrder = ActionTrack->GetSortingOrder();
		ActionStruct->SortOrder = SortOrder;
	}
	return true;
}

void FT4ContiViewModel::UpdateMovieSceneTrackValueChanged()
{
	check(nullptr != ContiAsset);

	TArray<UMovieSceneSection*> UpdateTargetSections;

	UMovieScene* MovieScene = ContiSequence->GetMovieScene();
	check(nullptr != MovieScene);
	for (UMovieSceneTrack* MasterTrack : MovieScene->GetMasterTracks())
	{
		if (HasActionSelected(MasterTrack))
		{
			TArray<UMovieSceneSection*> AllSections = MasterTrack->GetAllSections();
			UpdateTargetSections += AllSections;
		}
	}
	if (0 >= UpdateTargetSections.Num())
	{
		return;
	}

	FFrameRate TickResolution = MovieScene->GetTickResolution(); // #58

	bool bDetailViewUpdate = false;

	const FScopedTransaction Transaction(LOCTEXT("UpdateMovieSceneTrackValueChanged_Transaction", "Update a TimeData"));
	ContiAsset->Modify();

	FT4ActionCompositeData& CompositeData = ContiAsset->CompositeData;
	for (UMovieSceneSection* SceneSection : UpdateTargetSections)
	{
		UT4ContiActionSection* ActionSection = Cast<UT4ContiActionSection>(SceneSection);
		check(nullptr != ActionSection);
		const uint32 CurrHeaderKey = ActionSection->GetActionHeaderKey();
		FT4ContiActionStruct* ActionSelected = CompositeData.GetActionStruct(CurrHeaderKey); // #56
		if (nullptr != ActionSelected)
		{
			// #101 : Track 에서 Rename 이 되었을 수 있어 체크!
			{
				UT4ContiActionTrack* ActionTrack = ActionSection->GetParentTrack();
				check(nullptr != ActionTrack);
				const FName CurrentDisplayName = *ActionTrack->GetDisplayName().ToString();
				if (CurrentDisplayName != ActionSelected->DisplayName)
				{
					ActionSelected->DisplayName = CurrentDisplayName;
					bDetailViewUpdate = true; // 이름이 바뀌어서 Detail View 의 Group Name 도 함께 변경되어야 함으로 Refresh
				}
				const FColor DebugColorTint = ActionTrack->GetColorTint();
				if (DebugColorTint != ActionSelected->DebugColorTint)
				{
					ActionSelected->DebugColorTint = DebugColorTint;
				}
			}
			if (ET4ActionType::CameraWork == ActionSelected->ActionType)
			{
				// #58 : CameraWork 는 오직 Key 만 사용! todo : 케이스가 하나 더 생기면 Action Struct 에 쿼리하도록 처리한다.
				FT4CameraWorkAction* CameraWorkStruct = static_cast<FT4CameraWorkAction*>(ActionSelected);
				check(nullptr != CameraWorkStruct);
				FT4CameraWorkSectionData& SectionData = CameraWorkStruct->SectionData;

				TMovieSceneChannelData<FT4ContiActionSectionKey> ChannelData = ActionSection->ActionChannel.GetData();
				const TArrayView<FFrameNumber>& ChannelKeys = ChannelData.GetTimes();
				const TArrayView<FT4ContiActionSectionKey>& ChannelValues = ChannelData.GetValues();
				check(ChannelValues.Num() == ChannelKeys.Num());
				for (int32 i = 0; i < ChannelValues.Num(); ++i)
				{
					FT4ContiActionSectionKey& CurSectionKey = ChannelValues[i];
					int32 OldChannelKey = CurSectionKey.ChannelKey;
					const int32 CurChannelKey = ChannelKeys[i].Value;
					if (OldChannelKey != CurChannelKey) // #58 : Time 이 변경되었음으로 업데이트 해준다.
					{
						for (FT4CameraWorkSectionKeyData& KeyData : SectionData.KeyDatas)
						{
							if (OldChannelKey == KeyData.ChannelKey)
							{
								KeyData.ChannelKey = CurChannelKey;
								KeyData.StartTimeSec = FFrameTime(FFrameNumber(CurChannelKey)) / TickResolution;
								break;
							}
						}
						CurSectionKey.ChannelKey = CurChannelKey;
						bDetailViewUpdate = true;
					}
				}
			}
			else
			{
				// Track Timeline 에서 Section 이동을 했을 수 있어서 Action 업데이트!
				// 참고로, Section 의 Range <=> Key 는 Track 을 다시 생성함으로 Detail View 전체가 재업데이트 된다.
				if (ET4LifecycleType::Duration == ActionSelected->LifecycleType) // #56
				{
					TRange<FFrameNumber> SectionRange = ActionSection->GetRange();
					const float SectionStart = FFrameTime(MovieScene::DiscreteInclusiveLower(SectionRange).Value) / TickResolution;
					const float SectionDuration = FFrameTime(MovieScene::DiscreteSize(SectionRange)) / TickResolution;

					// #56 : DelayTime 과 Duration 이 변경되면 Action 을 업데이트 해준다.
					if (ActionSelected->StartTimeSec != SectionStart || ActionSelected->DurationSec != SectionDuration)
					{
						ActionSelected->StartTimeSec = SectionStart;
						ActionSelected->DurationSec = SectionDuration;
					}
				}
				else
				{
					// #56 :: SectionKey 사용
					FFrameNumber SectionKeyTime = ActionSection->GetKeyTime();
					const float SectionStart = FFrameTime(SectionKeyTime) / TickResolution;

					// #56 : DelayTime 이 변경되면 Action 을 업데이트 해준다.
					if (ActionSelected->StartTimeSec != SectionStart)
					{
						ActionSelected->StartTimeSec = SectionStart;
					}
				}
			}
		}
	}
	if (!bDetailViewUpdate)
	{
		return;
	}
	// #54 : 변경사항을 DetailView 에 알린다.
	if (GetOnViewModelChanged().IsBound())
	{
		GetOnViewModelChanged().Broadcast();
	}
}

bool FT4ContiViewModel::UpdateTrackSelectionChanged(
	const TArray<UMovieSceneTrack*>& InSelectedTracks,
	bool bForceUpdate
) // #56
{
	check(nullptr != ContiAsset);
	SelectedActionInfos.Empty();
	if (0 >= InSelectedTracks.Num())
	{
		ClearCameraSectionKeyObjects(); // #58
		return true;
	}
	bool bValidCameraActions = false;
	FFrameNumber GlobalFrameNumber;
	if (SequencerPtr.IsValid()) // #58
	{
		GlobalFrameNumber = SequencerPtr->GetGlobalTime().Time.RoundToFrame();
	}
	FT4ActionCompositeData& CompositeData = ContiAsset->CompositeData;
	for (UMovieSceneTrack* SceneTrack : InSelectedTracks)
	{
		UT4ContiActionTrack* ActionTrack = Cast<UT4ContiActionTrack>(SceneTrack);
		check(nullptr != ActionTrack);

		const uint32 CurrHeaderKey = ActionTrack->GetActionHeaderKey();
		FT4ContiActionStruct* ActionStruct = GetActionStruct(CurrHeaderKey);
		if (nullptr != ActionStruct)
		{
			const FT4ActionHeaderInfo& HierarchyInfo = CompositeData.HeaderInfoMap[CurrHeaderKey];
			FT4ContiActionInfo NewSelectContiActionInfo;
			NewSelectContiActionInfo.ActionType = HierarchyInfo.ActionType;
			NewSelectContiActionInfo.ActionArrayIndex = HierarchyInfo.ActionArrayIndex;
			NewSelectContiActionInfo.ActionHeaderKey = CurrHeaderKey;
			NewSelectContiActionInfo.FolderName = HierarchyInfo.FolderName; // #56
			NewSelectContiActionInfo.ActionSortOrder = ActionStruct->SortOrder; // #56 : lower win
			NewSelectContiActionInfo.DebugColorTint = ActionStruct->DebugColorTint; // #100
			SelectedActionInfos.Add(NewSelectContiActionInfo);

			// #58 : CameraWork 는 오직 Key 만 사용! todo : 케이스가 하나 더 생기면 Action Struct 에 쿼리하도록 처리한다.
			if (ET4ActionType::CameraWork == ActionStruct->ActionType)
			{
				// #58 : 만약, 이전에 편집중이던 CameraWork 가 있으면 Clear 해준다.
				if (FocusCameraWorkActionKeySelected != CurrHeaderKey &&
					CameraWorkSectionKeyObjects.Contains(FocusCameraWorkActionKeySelected))
				{
					UT4CameraWorkSectionKeyObject* EditorCameraSectionKeyObject = CameraWorkSectionKeyObjects[FocusCameraWorkActionKeySelected];
					check(nullptr != EditorCameraSectionKeyObject);
					EditorCameraSectionKeyObject->ClearCameraActors(GetGameWorld());
					FocusCameraWorkActionKeySelected = INDEX_NONE;
				}
				bool bResult = false;
				const FT4CameraWorkAction* CameraWorkStruct = static_cast<FT4CameraWorkAction*>(ActionStruct);
				check(nullptr != CameraWorkStruct);
				const FT4CameraWorkSectionData& SectionData = CameraWorkStruct->SectionData;
				for (const FT4CameraWorkSectionKeyData& KeyData : SectionData.KeyDatas)
				{
					if (GlobalFrameNumber.Value == KeyData.ChannelKey)
					{
						// #58 : 이 Section Key 가 선택되었다. DetailView 의 List 변경!!
						bResult = FindOrCreateCameraSectionKeyObject(CurrHeaderKey, KeyData.ChannelKey, true);
						if (bResult)
						{
							FocusCameraWorkActionKeySelected = CurrHeaderKey;
						}
						break;
					}
				}
				if (!bResult)
				{
					// #58 : 현재 FrmaeNumber 에 해당하는 ChannelKey 가 없음으로 첫번째로 할당!
					bResult = FindOrCreateCameraSectionKeyObject(CurrHeaderKey, 0, true);
					if (bResult)
					{
						FocusCameraWorkActionKeySelected = CurrHeaderKey;
					}
				}
				bValidCameraActions = true;
			}
		}
	}
	if (!bValidCameraActions)
	{
		ClearCameraSectionKeyObjects(); // #58
	}
	return true;
}

void FT4ContiViewModel::UpdateMovieSceneActionSection(
	UT4ContiActionTrack* InContiActionTrack, 
	ET4LifecycleType InLifecycleType
) // #56 : Track ContextMenu 로 변경 지원
{
	check(nullptr != InContiActionTrack);
	const uint32 CurrHeaderKey = InContiActionTrack->GetActionHeaderKey();
	FT4ContiActionStruct* ActionStruct = ContiAsset->CompositeData.GetActionStruct(CurrHeaderKey);
	if (nullptr == ActionStruct)
	{
		return;
	}
	const FScopedTransaction Transaction(LOCTEXT("T4ContiViewModelChangeLifecycleType_Transaction", "Update a Action LifecycleType"));
	ContiAsset->Modify();
	{
		ActionStruct->LifecycleType = InLifecycleType;
	}
	{
		// #100 : #99 에서 Section 만 재생성했는데, Track Outliner 에서 Key Menu 가 사라지지 않는 문제가 있어
		//        Track 을 재생성하도록 처리함
		FT4ActionCompositeData& CompositeData = ContiAsset->CompositeData;
		const FT4ActionHeaderInfo& ActionHeaderInfo = CompositeData.HeaderInfoMap[CurrHeaderKey];

		FT4ContiActionInfo NewContiActionInfo;
		NewContiActionInfo.ActionType = ActionHeaderInfo.ActionType;
		NewContiActionInfo.ActionArrayIndex = ActionHeaderInfo.ActionArrayIndex;
		NewContiActionInfo.ActionHeaderKey = CurrHeaderKey;
		NewContiActionInfo.ActionSortOrder = ActionStruct->SortOrder; // #56 : lower win
		NewContiActionInfo.FolderName = ActionHeaderInfo.FolderName; // #56
		NewContiActionInfo.DebugColorTint = ActionStruct->DebugColorTint; // #100
		
		check(nullptr != ContiSequence);
		UMovieScene* MovieScene = ContiSequence->GetMovieScene();
		check(nullptr != MovieScene);
		UMovieSceneFolder* CurrentFolder = GetMovieSceneForder(NewContiActionInfo.FolderName);
		if (nullptr != CurrentFolder)
		{
			CurrentFolder->RemoveChildMasterTrack(InContiActionTrack);
		}
		MovieScene->RemoveMasterTrack(*InContiActionTrack);

		SelectedActionInfos.Empty();
		SelectedActionInfos.Add(NewContiActionInfo);

		AddMovieSceneActionTrack(NewContiActionInfo, true, true);
	}
}

float FT4ContiViewModel::GetMovieScenePlaybackEndTimeSec() // #56
{
	check(nullptr != ContiSequence);
	UMovieScene* MovieScene = ContiSequence->GetMovieScene();
	check(nullptr != MovieScene);
	const FFrameRate TickResolution = MovieScene->GetTickResolution();
	const TRange<FFrameNumber> PlaybackRange = MovieScene->GetPlaybackRange();
	return static_cast<float>(
		FFrameTime(MovieScene::DiscreteExclusiveUpper(PlaybackRange).Value) / TickResolution
	);
}

bool FT4ContiViewModel::HasActionSelected(UMovieSceneTrack* InSceneTrack)
{
	check(nullptr != InSceneTrack);
	check(nullptr != ContiAsset);
	const FT4ActionCompositeData& CompositeData = ContiAsset->CompositeData;

	UT4ContiActionTrack* ActionTrack = Cast<UT4ContiActionTrack>(InSceneTrack);
	check(nullptr != ActionTrack);
	const ET4ActionType CurrActionType = ActionTrack->GetActionType();
	const uint32 CurrHeaderKey = ActionTrack->GetActionHeaderKey();

	int32 ActionArrayIndex = INDEX_NONE;
	if (CompositeData.HeaderInfoMap.Contains(CurrHeaderKey))
	{
		const FT4ActionHeaderInfo& HierarchyInfo = CompositeData.HeaderInfoMap[CurrHeaderKey];
		ActionArrayIndex = HierarchyInfo.ActionArrayIndex;
	}

	for (const FT4ContiActionInfo& SelectedActionInfo : SelectedActionInfos)
	{
		if (SelectedActionInfo.ActionType == CurrActionType &&
			SelectedActionInfo.ActionHeaderKey == CurrHeaderKey &&
			SelectedActionInfo.ActionArrayIndex == ActionArrayIndex)
		{
			return true;
		}
	}
	return false;
}

bool FT4ContiViewModel::CheckActionSortOrderChanged() // #56
{
	UMovieScene* MovieScene = ContiSequence->GetMovieScene();
	check(nullptr != MovieScene);
	const TArray<UMovieSceneTrack*> AllMasterTracks = MovieScene->GetMasterTracks();
	for (UMovieSceneTrack* SceneTrack : AllMasterTracks)
	{
		UT4ContiActionTrack* ActionTrack = Cast<UT4ContiActionTrack>(SceneTrack);
		check(nullptr != ActionTrack);
		const uint32 CurrHeaderKey = ActionTrack->GetActionHeaderKey();
		FT4ContiActionStruct* ActionStruct = GetActionStruct(CurrHeaderKey);
		check(nullptr != ActionStruct);
		int32 CurrSortOrder = 0;
		if (ActionTrack->GetSortingOrder() != ActionStruct->SortOrder)
		{
			return true;
		}
	}
	return false;
}

void FT4ContiViewModel::SetPlayingActionPaused(bool bInPause) // #56, #59
{
	ClientSetPauseObject(bInPause); // #54

	if (bMirrorToPIEEnabled) // #59
	{
		if (ET4LayerType::Max != MirrorLayerType && MirrorObjectID.IsValid())
		{
			ClientSetPauseObject(MirrorLayerType, bInPause);
		}
	}
}

FT4ContiActionStruct* FT4ContiViewModel::GetActionStruct(const uint32 InActionHeaderKey)
{
	check(nullptr != ContiAsset);
	FT4ContiActionStruct* ActionStruct = ContiAsset->CompositeData.GetActionStruct(InActionHeaderKey);
	if (nullptr == ActionStruct)
	{
		return nullptr;
	}
	return ActionStruct;
}

UT4AnimSetAsset* FT4ContiViewModel::GetAnimSetAssetByStance() // #73
{
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return nullptr;
	}
	IT4GameObject* PlayerObject = PlayerController->GetGameObject();
	if (nullptr == PlayerObject)
	{
		return nullptr;
	}
	// TODO : refactoring!
	const UT4EntityAsset* EntityAsset = PlayerObject->GetEntityAsset();
	if (nullptr == EntityAsset)
	{
		return nullptr;
	}
	const ET4EntityType SelectEntityType = EntityAsset->GetEntityType();
	if (ET4EntityType::Character != SelectEntityType)
	{
		return nullptr;
	}
	const UT4CharacterEntityAsset* CharacterEntityAsset = Cast<UT4CharacterEntityAsset>(EntityAsset);
	if (nullptr == CharacterEntityAsset)
	{
		return nullptr;
	}
	const FName CurrentStanceName = PlayerObject->GetStanceName();
	const FT4EntityCharacterStanceSetData& StanceSetData = CharacterEntityAsset->StanceSetData;
	if (!StanceSetData.StanceMap.Contains(CurrentStanceName))
	{
		return nullptr;
	}
	const FT4EntityCharacterStanceData& StanceData = StanceSetData.StanceMap[CurrentStanceName];
	return StanceData.AnimSetAsset.LoadSynchronous();
}

UMovieSceneFolder* FT4ContiViewModel::GetMovieSceneForder(const FName& InFolderName) // #56
{
	if (InFolderName == NAME_None)
	{
		return nullptr;
	}
	UMovieScene* MovieScene = ContiSequence->GetMovieScene();
	check(nullptr != MovieScene);
	TArray<UMovieSceneFolder*>& RootForder = MovieScene->GetRootFolders();
	for (UMovieSceneFolder* CurrFolder : RootForder)
	{
		if (CurrFolder->GetFolderName() == InFolderName)
		{
			return CurrFolder;
		}
	}
	return nullptr;
}

UMovieSceneFolder* FT4ContiViewModel::FindAndAddMovieSceneForder(
	const FName& InFolderName,
	int32 InSortOrder
) // #56
{
	if (InFolderName == NAME_None)
	{
		return nullptr;
	}
	UMovieSceneFolder* ReturnFolder = GetMovieSceneForder(InFolderName);
	if (nullptr != ReturnFolder)
	{
		return ReturnFolder;
	}
	UMovieScene* MovieScene = ContiSequence->GetMovieScene();
	check(nullptr != MovieScene);
	TArray<UMovieSceneFolder*>& RootForder = MovieScene->GetRootFolders();
	ReturnFolder = NewObject<UMovieSceneFolder>(MovieScene, NAME_None, RF_Transactional);
	ReturnFolder->SetFolderName(InFolderName);
	ReturnFolder->SetSortingOrder(InSortOrder);
	RootForder.Add(ReturnFolder);
	return ReturnFolder;
}

#undef LOCTEXT_NAMESPACE