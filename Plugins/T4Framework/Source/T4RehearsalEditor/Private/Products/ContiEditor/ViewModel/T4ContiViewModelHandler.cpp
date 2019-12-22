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
#include "T4Asset/Public/Entity/T4Entity.h" // #87
#include "T4Asset/Classes/Conti/T4ContiAsset.h"

#include "T4Engine/Public/T4Engine.h"
#include "T4Engine/Public/Action/T4ActionCodeCommon.h"

#include "T4Frame/Public/T4Frame.h" // #30

#include "Framework/Commands/GenericCommands.h" // #75

#include "SequencerKeyCollection.h" // #58

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

#define LOCTEXT_NAMESPACE "FT4ContiViewModel"

/**
  * 
 */
void FT4ContiViewModel::HandleOnToggleActionIsolation(
	UT4ContiActionTrack* InContiActionTrack
) // #56
{
	check(nullptr != EditorPlaySettingObject);
	check(nullptr != InContiActionTrack);
	const uint32 CurrHeaderKey = InContiActionTrack->GetActionHeaderKey();
	FT4EditorParameters& EditorParams = EditorPlaySettingObject->GetContiParameters().EditorParams;
	bool bHasAction = EditorParams.IsolationActionSet.Contains(CurrHeaderKey);
	bool bIsolateChanged = false;
	if (!bHasAction)
	{
		bIsolateChanged = true;
	}
	else
	{
		bIsolateChanged = !bHasAction;
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnToggleActionIsolation_Transaction", "Update a Editor Parameter"));
	EditorParams.IsolationActionSet.Empty(); // #56 : Isolate 는 켜진 것 이외에는 모두 끄는 것임으로 일단 모두 삭제해준다.
	if (bIsolateChanged)
	{
		SetActionIsolation(InContiActionTrack, bIsolateChanged);
	}
}

void FT4ContiViewModel::HandleOnIsolateAllSelectedActions() // #56
{
	check(nullptr != EditorPlaySettingObject);
	if (!SequencerPtr.IsValid())
	{
		return;
	}
	TArray<UMovieSceneTrack*> SelectedTracks;
	SequencerPtr->GetSelectedTracks(SelectedTracks);
	if (0 >= SelectedTracks.Num())
	{
		return;
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnIsolateAllSelectedActions_Transaction", "Update a Editor Parameter"));
	FT4EditorParameters& EditorParams = EditorPlaySettingObject->GetContiParameters().EditorParams;
	EditorParams.IsolationActionSet.Empty(); // #56 : Isolate 는 켜진 것 이외에는 모두 끄는 것임으로 일단 모두 삭제해준다.
	for (UMovieSceneTrack* MovieSceneTrack : SelectedTracks)
	{
		SetActionIsolation(Cast<UT4ContiActionTrack>(MovieSceneTrack), true);
	}
}

bool FT4ContiViewModel::HandleOnSequencerIsTrackVisible(
	const UMovieSceneTrack* InMovieSceneTrack
)
{
	return IsActionInvisible(Cast<const UT4ContiActionTrack>(InMovieSceneTrack)); // #56
}

void FT4ContiViewModel::HandleOnChangeActionSectionKey(
	UT4ContiActionTrack* InContiActionTrack
) // #56 : Track ContextMenu 로 변경 지원
{
	check(nullptr != InContiActionTrack);
	UpdateMovieSceneActionSection(InContiActionTrack, ET4LifecycleType::Default);
}

void FT4ContiViewModel::HandleOnChangeActionSectionRange(
	UT4ContiActionTrack* InContiActionTrack
) // #56 : Track ContextMenu 로 변경 지원
{
	check(nullptr != InContiActionTrack);
	UpdateMovieSceneActionSection(InContiActionTrack, ET4LifecycleType::Duration);
}

void FT4ContiViewModel::HandleOnSequencerPlayEvent()
{
	check(nullptr != ContiSequence);
	check(nullptr != ContiAsset);
	if (!SequencerPtr.IsValid())
	{
		return;
	}
	SetPlayingActionPaused(false); // #56
	if (bSequencerPlayPaused)
	{
		bSequencerPlayPaused = false;
		IT4GameObject* PlayerObject = GetPlayerObject();
		if (nullptr != PlayerObject)
		{
			// #104 : Stop 후 재생인데, 현재 플레이중인 Conti 가 없다면 다시 재생해주도록 한다.
			if (PlayerObject->HasPublicAction(DefaultContiActionPrimaryKey))
			{
				return;
			}
		}
	}
	float PlayOffsetTime = 0.0f;
	if (!bSequencerPlayRestart)
	{
		float CurrentMaxPlayTimeSec = GetMovieScenePlaybackEndTimeSec();
		PlayOffsetTime = SequencerPtr->GetGlobalTime().AsSeconds();
		if (KINDA_SMALL_NUMBER >= FMath::Abs(CurrentMaxPlayTimeSec - PlayOffsetTime))
		{
			PlayOffsetTime = 0.0f; // #54 : Playback Range 를 넘어갈 경우 처음부터 재생!
		}
	}
	PlayContiInternal(PlayOffsetTime);
	bSequencerPlayRestart = false;
}

void FT4ContiViewModel::HandleOnSequencerStopEvent()
{
	if (bSequencerPlayPaused || bSequencerPlayRestart)
	{
		bSequencerPlayPaused = false; // Play pos 를 옮겼으니 재시작을 하도록 조치
		return;
	}
	IT4GameObject* PlayerObject = GetPlayerObject();
	if (nullptr != PlayerObject)
	{
		// #102 : Pasued 를 계속하다보면 실 플레이 시간과 Bar 가 어긋나는 문제가 있어 해제시 bar 시간을 맞춰준다.
		UMovieScene* MovieScene = ContiSequence->GetMovieScene();
		check(nullptr != MovieScene);
		const FFrameRate FrameResolution = MovieScene->GetTickResolution();
		float PlayOffsetTime = PlayerObject->GetActionControl()->GetElapsedTimeSec(DefaultContiActionPrimaryKey);
		FFrameTime SectionStartTime = FrameResolution.AsFrameTime(PlayOffsetTime);
		SequencerPtr->SetGlobalTime(SectionStartTime);
	}
	SetPlayingActionPaused(true); // #56
	bSequencerPlayPaused = true; // #54
}

void FT4ContiViewModel::HandleOnSequencerDataChanged(
	EMovieSceneDataChangeType InDataChangeType
)
{
	check(nullptr != ContiAsset);
	if (EMovieSceneDataChangeType::TrackValueChanged == InDataChangeType)
	{
		UpdateMovieSceneTrackValueChanged();
	}
	else if (EMovieSceneDataChangeType::MovieSceneStructureItemsChanged == InDataChangeType)
	{
		UpdateMovieSceneFolders(); // #56 : Folder 변경
		UpdateMovieSceneSortOrder(true); // #56 : SortOder 변경
	}
	else if (EMovieSceneDataChangeType::MovieSceneStructureItemRemoved == InDataChangeType)
	{
		// #58 : Track 및 Section Instance 에서 Delete Key 를 사용한 삭제는 제외. Conti 에서는 Track 과 1:1 이다.
		//       MovieSceneStructureItemRemoved 를 통해 Track 과 Section 이 함께 삭제되도록 처리되어 있다.
		UpdateMovieSceneTracks();
	}
}

void FT4ContiViewModel::HandleOnSequencerTimeChanged()
{
	// #56
	check(nullptr != ContiSequence);
	check(nullptr != ContiAsset);
	float UpdateMaxPlayTimeSec = GetMovieScenePlaybackEndTimeSec();
	if (UpdateMaxPlayTimeSec != ContiAsset->TotalPlayTimeSec)
	{
		const FScopedTransaction Transaction(LOCTEXT("HandleOnSequencerTimeChanged_Transaction", "Update TotalPlayTime"));
		ContiAsset->Modify();
		ContiAsset->TotalPlayTimeSec = UpdateMaxPlayTimeSec;
		bSequencerPlayPaused = false; // #54 : Pause 상태에서 GlobalTime 을 이동했다면 리플레이가 되도록 처리해준다.
	}

	// #102 : 루핑 상태에 대한 Notify 를 받을 수 없어서 아래와 같이 처리함
	if (EMovieScenePlayerStatus::Playing == SequencerPtr->GetPlaybackStatus())
	{
		IT4GameObject* PlayerObject = GetPlayerObject();
		if (nullptr != PlayerObject)
		{
			if (!PlayerObject->HasPublicAction(DefaultContiActionPrimaryKey))
			{
				float CurrentMaxPlayTimeSec = GetMovieScenePlaybackEndTimeSec();
				float PlayOffsetTime = SequencerPtr->GetGlobalTime().AsSeconds();
				if (KINDA_SMALL_NUMBER >= FMath::Abs(CurrentMaxPlayTimeSec - PlayOffsetTime))
				{
					PlayOffsetTime = 0.0f; // #54 : Playback Range 를 넘어갈 경우 처음부터 재생!
				}
				PlayContiInternal(PlayOffsetTime);
			}
		}
	}
}

void FT4ContiViewModel::HandleOnSequencerTrackSelectionChanged(
	const TArray<UMovieSceneTrack*> InSelectedTracks
)
{
	if (bUpdatingContiSelectionFromSequencer)
	{
		return;
	}
	TGuardValue<bool> UpdateGuard(bUpdatingContiSelectionFromSequencer, true);
	bool bResult = UpdateTrackSelectionChanged(InSelectedTracks, false); // #56
	if (!bResult)
	{
		return;
	}
	if (GetOnViewModelChanged().IsBound())
	{
		GetOnViewModelChanged().Broadcast(); // #54 : 변경사항을 DetailView 에 알린다.
	}
}

void FT4ContiViewModel::HandleOnSequencerSectionSelectionChanged(
	const TArray<UMovieSceneSection*> InSelectedSections
)
{
	if (bUpdatingContiSelectionFromSequencer)
	{
		return;
	}
	if (0 >= InSelectedSections.Num())
	{
#if 0 // #58 : 현재 키를 Drag 로 선택했을 경우 Eky 와 Section, Track 을 찾을 수 있는 방법이 없다. (TODO)
		TUniquePtr<FSequencerKeyCollection> KeyCollection;
		SequencerPtr->GetKeysFromSelection(KeyCollection, 0.1f);
		if (KeyCollection.IsValid())
		{

		}
#endif
		return;
	}
	TArray<UMovieSceneTrack*> UpdateSelectedTracks;
	for (UMovieSceneSection* SceneSection : InSelectedSections)
	{
		UT4ContiActionSection* ActionSection = Cast<UT4ContiActionSection>(SceneSection);
		check(nullptr != ActionSection);
		UpdateSelectedTracks.Add(ActionSection->GetParentTrack());
	}
	if (0 >= UpdateSelectedTracks.Num())
	{
		return;
	}
	TGuardValue<bool> UpdateGuard(bUpdatingContiSelectionFromSequencer, true);
	bool bResult = UpdateTrackSelectionChanged(UpdateSelectedTracks, false); // #56
	if (!bResult)
	{
		return;
	}
	if (SequencerPtr.IsValid())
	{
		// #54 : Section 을 선택하면 Track 이 선택되도록 처리. Section Selection UI 는 Conti Editor 에는 없다.
		for (UMovieSceneTrack* SceneTrack : UpdateSelectedTracks)
		{
			SequencerPtr->SelectTrack(SceneTrack);
		}
		for (UMovieSceneSection* SceneSection : InSelectedSections)
		{
			// #99 : 4.22 => 4.23 업데이트 후 SelectTrack 이 콜되면 SelectSection 을 empty 로 만들어서 강제로 다시 켜준다.
			//       현재 Conti 는 Track과 Section 은 1:1 기조를 유지하고 있기 때문!
			SequencerPtr->SelectSection(SceneSection);
		}
	}
	if (GetOnViewModelChanged().IsBound())
	{
		GetOnViewModelChanged().Broadcast(); // #54 : 변경사항을 DetailView 에 알린다.
	}
}

struct FT4JsonObjectData
{
	ET4ActionType ActionType;
	FName FolderName;
	FString JsonObjectString;
};

static TArray<FT4JsonObjectData> ActionJsonObjectDatas; // #75

template <class T>
void ExportActionStructToJsonObject(
	int32 InHeaderKey,
	TArray<T>& InOutTargetActions,
	TMap<uint32, FT4ActionHeaderInfo>& InOutHeaderInfoMap,
	FString& OutExportedText
) // #75
{
	check(InOutHeaderInfoMap.Contains(InHeaderKey));
	const FT4ActionHeaderInfo& HierarchyInfo = InOutHeaderInfoMap[InHeaderKey];
	check(InOutTargetActions.Num() > HierarchyInfo.ActionArrayIndex);
	const T& ActionStruct = InOutTargetActions[HierarchyInfo.ActionArrayIndex];
	FT4JsonObjectData& NewJsonObjectData = ActionJsonObjectDatas.AddDefaulted_GetRef();
	NewJsonObjectData.ActionType = HierarchyInfo.ActionType;
	NewJsonObjectData.FolderName = HierarchyInfo.FolderName;
	FJsonObjectConverter::UStructToJsonObjectString(ActionStruct, NewJsonObjectData.JsonObjectString);
	OutExportedText += NewJsonObjectData.JsonObjectString;
}

template <class T>
void ImportActionStructFromJsonObject(
	FT4ContiViewModel* InViewModel,
	const FT4JsonObjectData& InJsonObjectData
) // #75
{
	T NewActionStruct;
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(InJsonObjectData.JsonObjectString, &NewActionStruct, 0, 0))
	{
		return;
	}
	NewActionStruct.DisplayName.SetNumber(NewActionStruct.DisplayName.GetNumber() + 1);
	InViewModel->AddNewMovieSceneActionTrack(InJsonObjectData.ActionType, &NewActionStruct, InJsonObjectData.FolderName);
}

void FT4ContiViewModel::HandleOnCopySelection() // #75
{
	check(nullptr != ContiAsset);

	if (0 >= SelectedActionInfos.Num())
	{
		return;
	}
	
	ActionJsonObjectDatas.Empty();

	FString ExportedText;
	FT4ActionCompositeData& CompositeData = ContiAsset->CompositeData;
	for (const FT4ContiActionInfo& SelectedActionInfo : SelectedActionInfos)
	{
		if (!CompositeData.HeaderInfoMap.Contains(SelectedActionInfo.ActionHeaderKey))
		{
			continue; // #54 : Queue 에 ActionInfo 가 중복 저장되었을 수 있음으로 체크한다.
		}
		switch (SelectedActionInfo.ActionType)
		{
#define DEFINE_CONTI_ACTION_MACRO(x)															\
			case ET4ActionType::##x:															\
				ExportActionStructToJsonObject<FT4##x##Action>(SelectedActionInfo.ActionHeaderKey, CompositeData.##x##Actions, CompositeData.HeaderInfoMap, ExportedText); \
				break;

#include "Products/ContiEditor/ViewModel/T4ContiDefineMacros.h"
			default:
				{
					UE_LOG(
						LogT4RehearsalEditor,
						Error,
						TEXT("HandleOnCopySelection '%u' failed. no implementation."),
						uint32(SelectedActionInfo.ActionType)
					);
				}
				break;
		}
	}

	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
}

void FT4ContiViewModel::HandleOnPaste() // #75
{
	if (0 >= ActionJsonObjectDatas.Num())
	{
		return;
	}
	const FScopedTransaction Transaction(LOCTEXT("HandleOnPaste_Transaction", "Add to Clipboard Action"));
	for (const FT4JsonObjectData& JsonObjectData : ActionJsonObjectDatas)
	{
		switch (JsonObjectData.ActionType)
		{
#define DEFINE_CONTI_ACTION_MACRO(x)													  \
			case ET4ActionType::##x:													  \
				ImportActionStructFromJsonObject<FT4##x##Action>(this, JsonObjectData);	  \
				break;

#include "Products/ContiEditor/ViewModel/T4ContiDefineMacros.h"
			default:
				{
					UE_LOG(
						LogT4RehearsalEditor,
						Error,
						TEXT("HandleOnPaste '%u' failed. no implementation."),
						uint32(JsonObjectData.ActionType)
					);
				}
				break;
		}
	}
	// Grab the text to paste from the clipboard
	//FString TextToImport;
	//FPlatformApplicationMisc::ClipboardPaste(TextToImport);
}

template <class T>
void RemoveActionHeaderInfo(
	int32 InHeaderKey,
	TArray<T>& InOutTargetActions,
	TMap<uint32, FT4ActionHeaderInfo>& InOutHeaderInfoMap
)
{
	// #54 : 액션을 삭제하고, 액션 Array Index 를 재설정 해준다.
	check(InOutHeaderInfoMap.Contains(InHeaderKey));
	const FT4ActionHeaderInfo& RemoveHierarchyInfo = InOutHeaderInfoMap[InHeaderKey];
	check(InOutTargetActions.Num() > RemoveHierarchyInfo.ActionArrayIndex);
	InOutTargetActions.RemoveAt(RemoveHierarchyInfo.ActionArrayIndex);
	for (T& CurrentAction : InOutTargetActions)
	{
		check(INDEX_NONE != CurrentAction.HeaderKey);
		check(InOutHeaderInfoMap.Contains(CurrentAction.HeaderKey))
		FT4ActionHeaderInfo& CurrentHierarchyInfo = InOutHeaderInfoMap[CurrentAction.HeaderKey];
		check(RemoveHierarchyInfo.ActionType == CurrentHierarchyInfo.ActionType);
		if (RemoveHierarchyInfo.ActionArrayIndex < CurrentHierarchyInfo.ActionArrayIndex)
		{
			check(0 < CurrentHierarchyInfo.ActionArrayIndex);
			CurrentHierarchyInfo.ActionArrayIndex = CurrentHierarchyInfo.ActionArrayIndex - 1;
		}
	}
	InOutHeaderInfoMap.Remove(InHeaderKey);
}

void FT4ContiViewModel::UpdateMovieSceneTracks() // #54, #75
{
	// #58 : Track or Section 에서 Delete Key 삭제할 경우 MovieSceneStructureItemRemoved Notify 를 받아서 Track 을 정리함
	//	     #58 이전에는 Delete Key 를 후킹해 Sequencer 보다 먼저 삭제를 처리했으나 Section Key 삭제 처리를 위해
	//       UI 에서 먼저 삭제하고, Notify 에서 사라진 Track 을 정리하는 방향으로 처리함.
	TArray<uint32> ActiveHeaderKeys;
	{
		check(nullptr != ContiSequence);
		UMovieScene* MovieScene = ContiSequence->GetMovieScene();
		check(nullptr != MovieScene);
		TArray<UMovieSceneTrack*> RemoveTracks;
		for (UMovieSceneTrack* MasterTrack : MovieScene->GetMasterTracks())
		{
			UT4ContiActionTrack* ActionTrack = Cast<UT4ContiActionTrack>(MasterTrack);
			check(nullptr != ActionTrack);
			if (ActionTrack->IsEmpty())
			{
				RemoveTracks.Add(ActionTrack);
				continue;
			}
			ActiveHeaderKeys.Add(ActionTrack->GetActionHeaderKey());
		}
		for (UMovieSceneTrack* RemoveTrack : RemoveTracks)
		{
			MovieScene->RemoveMasterTrack(*RemoveTrack); // Conti 에서는 하위 Section 이 없다면 삭제되었다고 본다. 쌍이다!
		}
		if (0 >= ActiveHeaderKeys.Num())
		{
			if (0 < RemoveTracks.Num())
			{
				RefreshAllSequencer();
			}
			return;
		}
	}
	{
		const FScopedTransaction Transaction(LOCTEXT("UpdateMovieSceneTracks_Transaction", "Remove a Action"));
		ContiAsset->Modify();
		FT4ActionCompositeData& CompositeData = ContiAsset->CompositeData;
		for (TMap<uint32, FT4ActionHeaderInfo>::TConstIterator It(CompositeData.HeaderInfoMap); It; ++It)
		{
			uint32 ActionHeaderKey = It.Key();
			const FT4ActionHeaderInfo& HeaderInfo = It.Value();
			bool bHasTrack = false;
			for (uint32 HeaderKey : ActiveHeaderKeys)
			{
				if (HeaderKey == ActionHeaderKey)
				{
					bHasTrack = true;
					break;
				}
			}
			if (bHasTrack)
			{
				continue;
			}
			switch (HeaderInfo.ActionType)
			{
#define DEFINE_CONTI_ACTION_MACRO(x)																\
				case ET4ActionType::##x:															\
					RemoveActionHeaderInfo<FT4##x##Action>(ActionHeaderKey, CompositeData.##x##Actions, CompositeData.HeaderInfoMap); \
					break;

#include "Products/ContiEditor/ViewModel/T4ContiDefineMacros.h"
				default:
				{
					UE_LOG(
						LogT4RehearsalEditor,
						Error,
						TEXT("UpdateMovieSceneTracks ItemRemoved '%u' failed. no implementation."),
						uint32(HeaderInfo.ActionType)
					);
				}
				break;
			};
		}
	}
	UpdateMovieSceneFolders();
	UpdateMovieSceneSortOrder(true);
	RefreshAllSequencer();
}

void FT4ContiViewModel::HandleOnNoAction() // #75
{
}

#undef LOCTEXT_NAMESPACE