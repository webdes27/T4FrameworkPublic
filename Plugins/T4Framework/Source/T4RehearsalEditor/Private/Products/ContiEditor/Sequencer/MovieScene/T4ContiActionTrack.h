// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "MovieSceneNameableTrack.h"
#include "Tracks/MovieScenePropertyTrack.h"
#include "T4Asset/Public/Action/T4ActionTypes.h"
#include "T4ContiActionTrack.generated.h"

/**
  * #54, #56
 */
class UMovieScene;
UCLASS(MinimalAPI)
class UT4ContiActionTrack : public UMovieScenePropertyTrack
{
	GENERATED_UCLASS_BODY()

public:
	bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override;
	UMovieSceneSection* CreateNewSection() override;
	FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override;

	void RemoveSection(UMovieSceneSection& Section) override; // #58
	void RemoveSectionAt(int32 SectionIndex) override; // #58

#if WITH_EDITORONLY_DATA
	// #101 : Track 에서 Rename 을 할 수 있도록 처리. EMovieSceneDataChangeType::TrackValueChanged 로 수정 이벤트가 간다!
	virtual bool CanRename() const override { return true; }
#endif

public:
	bool IsInitialized() const { return bInitialized; } // #65

	void Initialize(
		uint32 InContiViewModelInstanceKey, // #56, #65
		ET4ActionType InActionType,
		uint32 InActionHeaderKey,
		ET4LifecycleType InLifecycleType,
		uint32 InActionSortOrder, // #56
		const FText& InDisplayName
	); // #54

	ET4ActionType GetActionType() const { return ActionType; }
	uint32 GetActionHeaderKey() const { return ActionHeaderKey; }
	uint32 GetActionSortOrder() const { return ActionSortOrder; }

	void SetLifecycleType(ET4LifecycleType InLifecycleType) { ActionLifecycleType = InLifecycleType; }
	ET4LifecycleType GetLifecycleType() const { return ActionLifecycleType; }

public:
	UPROPERTY()
	uint32 ContiViewModelInstanceKey; // #56, #65

	UPROPERTY()
	ET4ActionType ActionType;

	UPROPERTY()
	uint32 ActionHeaderKey;

	UPROPERTY()
	ET4LifecycleType ActionLifecycleType;

	UPROPERTY()
	uint32 ActionSortOrder; // #56

protected:
	bool bInitialized; // #65
};