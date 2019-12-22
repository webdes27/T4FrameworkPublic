// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Products/Common/ViewModel/T4BaseViewModel.h"

#include "Products/Common/Helper/T4EditorViewTargetSelector.h" // #57
#include "Products/Common/Helper/T4EditorAnimSetAssetSelector.h" // #39

#include "T4Asset/Public/Action/T4ActionTypes.h"
#include "T4Engine/Public/T4EngineTypes.h"

#include "UObject/GCObject.h"
#include "TickableEditorObject.h"
#include "EditorUndoClient.h"

#include "ISequencerModule.h"

/**
  *
 */
static const FT4ActionKey DefaultContiActionPrimaryKey(TEXT("RehearsalConti"), true);
static const int32 DefaultActionSortOrder = TNumericLimits<int32>::Max(); // #56 : lower win

struct FT4ContiActionInfo
{
public:
	ET4ActionType ActionType;
	uint32 ActionArrayIndex;
	uint32 ActionHeaderKey;
	int32 ActionSortOrder; // #56
	FName FolderName; // #56
	FColor DebugColorTint; // #100

public:
	FT4ContiActionInfo()
		: ActionType(ET4ActionType::None)
		, ActionArrayIndex(INDEX_NONE)
		, ActionHeaderKey(INDEX_NONE)
		, ActionSortOrder(DefaultActionSortOrder) // #56 : lower win
		, FolderName(NAME_None) // #56
		, DebugColorTint(FColor::Black) // #100
	{
	}

	FORCEINLINE bool operator==(const FT4ContiActionInfo& InRhs) const
	{
		return (ActionType == InRhs.ActionType &&
				ActionArrayIndex == InRhs.ActionArrayIndex &&
				ActionHeaderKey == InRhs.ActionHeaderKey &&
				ActionSortOrder == InRhs.ActionSortOrder &&
				FolderName == InRhs.FolderName &&
				DebugColorTint == InRhs.DebugColorTint) ? true : false;
	}

	FORCEINLINE bool operator!=(const FT4ContiActionInfo& InRhs) const
	{
		return (ActionType != InRhs.ActionType ||
			    ActionArrayIndex != InRhs.ActionArrayIndex ||
			    ActionHeaderKey != InRhs.ActionHeaderKey ||
			    ActionSortOrder != InRhs.ActionSortOrder ||
			    FolderName != InRhs.FolderName ||
				DebugColorTint != InRhs.DebugColorTint) ? true : false;
	}
};

class UT4ContiAsset;
class FT4RehearsalContiEditor;
struct FT4ContiViewModelOptions
{
	FT4ContiViewModelOptions();

	uint32 InstanceKey; // #65

	UT4ContiAsset* ContiAsset;
	FT4RehearsalContiEditor* ContiEditor;

	FOnGetAddMenuContent OnGetSequencerAddMenuContent;
};

// #30
class AT4EditorCameraActor; // #85
class IT4GameObject;
class ISequencer;
class IT4PlayerController;
struct FT4ContiActionStruct;
struct FT4ActionParameters;
class UT4EntityAsset;
class UT4ContiActionSequence;
class UT4ContiActionSection;
class UT4ContiActionTrack;
class UT4EditorGameplaySettingObject; // #60
class UT4CameraWorkSectionKeyObject; // #58
class UT4EditorActionPlaybackController; // #68
class FT4ContiViewModel
	: public TSharedFromThis<FT4ContiViewModel>
	, public FGCObject
	, public FEditorUndoClient
	, public FTickableEditorObject
	, public FT4BaseViewModel
{
public:
	FT4ContiViewModel(const FT4ContiViewModelOptions& InOptions);
	~FT4ContiViewModel();

	static FT4ContiViewModel* GetContiViewModelByInstanceKey(uint32 InInstanceKey); // #65

	//~ FGCObject interface
	void AddReferencedObjects(FReferenceCollector& Collector) override;

	//~ FEditorUndoClient interface
	void PostUndo(bool bSuccess) override;
	void PostRedo(bool bSuccess) override { PostUndo(bSuccess); }

	// ~ FTickableEditorObject
	void Tick(float DeltaTime) override;
	bool IsTickable() const override { return true; }
	TStatId GetStatId() const override;

	// FT4BaseViewModel
	ET4ViewModelEditMode GetEditMode() const override { return ET4ViewModelEditMode::Conti; }

	bool IsEditWidgetMode() const override { return true; } // #94, #85

	AActor* GetEditWidgetModeTarget() const override; // #94, #85

	void NotifyActionPlaybackRec() override; // #104
	void NotifyActionPlaybackPlay() override; // #104

public:
	void ClientChangeStance(FName InStanceName) override; // #73

public:
	TSharedPtr<ISequencer> GetSequencer();
	UT4ContiAsset* GetContiAsset() const { return ContiAsset; }

	TSharedRef<FT4EditorViewTargetSelector> GetViewTargetSelector() { return ViewTargetSelectorPtr->AsShared(); } // #57
	TSharedRef<FT4EditorAnimSetAssetSelector> GetAnimSetAssetSelector() { return AnimSetAssetSelectorPtr->AsShared(); } // #49
	UT4EditorGameplaySettingObject* GetEditorPlaySettings() const { return EditorPlaySettingObject; } // #60

	const TArray<FT4ContiActionInfo>& GetSelectedActionInfos() const { return SelectedActionInfos; }

	void RefreshAll();

	bool CanSave() const;

	bool DoSave();
	void DoSaveEditorPlaySettings(); // #60

	void JumpToPlayAction(); // #99
	void JumpToEndAction(); // #99
	void TogglePlayAction(); // #99

	void PlayActionBy(int32 InActionHeaderKey); // #100
	void PlayActionStopBy(int32 InActionHeaderKey); // #100
	void PlayActionResetBy(int32 InActionHeaderKey); // #100

	void ToggleSimulation(); // #102
	void ToggleMirrorToPIE(); // #59

	void ReloadPlayerSpawn() { bPlayerSpawnd = false; } // #87

	void UpdateMovieScenePlaybackRange(); // #56, #100
	void RefreshMovieSceneActionTrack(uint32 InActionHeaderKey); // #100

	void SetPreviewViewTarget(UT4EntityAsset* EntityAsset); // #60

	void UpdateEditorPlayNPCEntityAsset(); // #76
	void UpdateEditorPlaySkillDataInfo(); // #63
	void UpdateEditorPlayEffectDataInfo(); // #63
	void UpdateEditorPlayActionParameters(); // #63

	void ClearCameraSectionKeyObjects(); // #58 : 삭제되어도 다시 재생성 된다.
	UT4CameraWorkSectionKeyObject* FindOrCreateCameraSectionKeyObject(int32 InActionHeaderKey, int32 InChannelKey, bool bInUpdate); // #58
	void UpdateCameraSectionKeyObjects(); // #58

	bool AddNewMovieSceneActionTrack(ET4ActionType InActionType, FT4ContiActionStruct* InSourceData, FName InFolderName); // #75

	bool AddNewMovieSceneActionSectionKey(uint32 InActionHeaderKey, int32 InChannelKey); // #58 : ChannelKey (FrameNumber)
	bool RemoveMovieSceneActionSectionKey(uint32 InActionHeaderKey, int32 InChannelKey); // #58 : ChannelKey (FrameNumber)

	bool IsActionInvisible(const UT4ContiActionTrack* InContiActionTrack) const; // #56
	bool IsActionIsolate(const UT4ContiActionTrack* InContiActionTrack) const; // #56

	void SetActionInvisible(UT4ContiActionTrack* InContiActionTrack, bool bInvisible); // #56
	void SetActionIsolation(UT4ContiActionTrack* InContiActionTrack, bool bIsolate); // #56

	const FText GetActionSectionDisplayName(const UT4ContiActionSection* InContiActionSection); // #54

	void HandleOnToggleActionIsolation(UT4ContiActionTrack* InContiActionTrack); // #56
	void HandleOnIsolateAllSelectedActions(); // #56

	bool HandleOnSequencerIsTrackVisible(const UMovieSceneTrack* InMovieSceneTrack);

	void HandleOnChangeActionSectionKey(UT4ContiActionTrack* InContiActionTrack); // #56 : Track ContextMenu 로 변경 지원
	void HandleOnChangeActionSectionRange(UT4ContiActionTrack* InContiActionTrack); // #56 : Track ContextMenu 로 변경 지원

	void HandleOnSequencerPlayEvent();
	void HandleOnSequencerStopEvent();
	void HandleOnSequencerDataChanged(EMovieSceneDataChangeType InDataChangeType); // #54 : Data 변경시 호출
	void HandleOnSequencerTimeChanged(); // #54 : Timeline 의 시간 변경시 호출
	void HandleOnSequencerTrackSelectionChanged(const TArray<UMovieSceneTrack*> InSelectedTracks); // #54 : Track 선택시 호출
	void HandleOnSequencerSectionSelectionChanged(const TArray<UMovieSceneSection*> InSelectedSections); // #54 : Section 선택시 호출

protected:
	// FT4BaseViewModel
	void Cleanup() override; // #85
	void Reset() override; // #79
	void StartPlay() override; // #76, #86
	void RestartPlay() override; // #87, #94 : 월드 이동후 호출

	void DrawHUD(FViewport* InViewport, FCanvas* InCanvas, FT4HUDDrawInfo* InOutDrawInfo) override; // #59, #83

	// #87 : ViewModel 시작시 특정 레벨을 열고 싶다면, MapEntityAssetPath 를 채울 것!
	void SetupStartWorld(FT4WorldConstructionValues& InWorldConstructionValues) override;

	// #39 : 뷰포트에서 컨트롤 캐릭터가 변경될 경우 호출
	// #99 : SubClass 별로 처리해야 할 기능이 있다면...구현
	void NotifyViewTargetChanged(IT4GameObject* InViewTarget) override;

	UObject* GetEditObject() const override; // #103
	FT4EditorTestAutomation* GetTestAutomation() const override; // #103

	FString GetActionPlaybackAssetName() const override; // #68, #104
	FString GetActionPlaybackFolderName() const override; // #68, #104

	void HandleOnCopySelection(); // #75
	void HandleOnPaste();

	void HandleOnNoAction(); // #75

private:
	void CheckAndSpawnEntity(); // #67

	void SetupSequencer();

	void PlayContiInternal(float InPlayOffsetTime); // #99

	UT4ContiActionTrack* GetMovieSceneActionTrack(uint32 InActionHeaderKey); // #100
	bool AddMovieSceneActionTrack(const FT4ContiActionInfo& InContiActionInfo, bool bSelection, bool bNotify); // #54

	void RefreshMovieSceneActionTracks(bool bSelection); // #56 : 모든 UI Track 을 삭제하고, 추가
	void RefreshAllSequencer(); // #56 : UI 갱신 및 Selection 까지 모두 Refresh

	void UpdateMovieSceneFolders(); // #56
	bool UpdateMovieSceneSortOrder(bool bCheckDirty); // #56 : 저장 또는 Track 위치 변경시 SortOrder 변경

	void UpdateMovieSceneTracks(); // #58

	void UpdateMovieSceneTrackValueChanged(); // #101 : Section Time 수정 또는 Track Rename 시 호출됨
	bool UpdateTrackSelectionChanged(const TArray<UMovieSceneTrack*>& InSelectedTracks, bool bForceUpdate); // #56

	void UpdateMovieSceneActionSection(
		UT4ContiActionTrack* InContiActionTrack, 
		ET4LifecycleType InLifecycleType
	); /// #56 : Track ContextMenu 로 변경 지원

	float GetMovieScenePlaybackEndTimeSec(); // #56

	bool HasActionSelected(UMovieSceneTrack* InSceneTrack);
	bool CheckActionSortOrderChanged(); // #56

	void SetPlayingActionPaused(bool bPause); // #56, #59

	FT4ContiActionStruct* GetActionStruct(const uint32 InActionHeaderKey); // #58
	UT4AnimSetAsset* GetAnimSetAssetByStance(); // #73

	UMovieSceneFolder* GetMovieSceneForder(const FName& InFolderName); // #56
	UMovieSceneFolder* FindAndAddMovieSceneForder(const FName& InFolderName, int32 InSortOrder); // #56

private:
	uint32 InstanceKey; // #65

	UT4ContiActionSequence* ContiSequence;
	UT4ContiAsset* ContiAsset;
	UT4EditorGameplaySettingObject* EditorPlaySettingObject; // #60
	UT4EditorActionPlaybackController* EditorActionPlaybackController; // #68

	TSharedPtr<FT4EditorViewTargetSelector> ViewTargetSelectorPtr; // #57
	TSharedPtr<FT4EditorAnimSetAssetSelector> AnimSetAssetSelectorPtr; // #39

	FT4RehearsalContiEditor* ContiEditor;

	TSharedPtr<ISequencer> SequencerPtr;
	
	TArray<FT4ContiActionInfo> SelectedActionInfos; // #30 : only Editor

	TMap<uint32, UT4CameraWorkSectionKeyObject*> CameraWorkSectionKeyObjects; // #58

	FOnGetAddMenuContent OnGetSequencerAddMenuContent;

	bool bSequencerPlayPaused; // #54
	bool bSequencerPlayRestart; // #102
	bool bUpdatingContiSelectionFromSequencer; // #54 : 이벤트에 의한 루핑 방지

	bool bPlayerSpawnd; // #87

	bool bSimulationEnabled; // #102

	// #59
	bool bMirrorToPIEEnabled; 
	ET4LayerType MirrorLayerType;
	FT4ObjectID MirrorObjectID; 
	// ~#59

	uint32 FocusCameraWorkActionKeySelected; // #58 : N개중 포커스가 가있는 편집중인 한개
};
