// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ContiActionTrack.h"
#include "T4ContiActionSection.h"
#include "T4ContiActionTrackTemplate.h"

#include "MovieScene.h"
#include "Evaluation/MovieSceneEvalTemplate.h"
#include "Evaluation/MovieSceneEvaluationTrack.h"
#include "Compilation/IMovieSceneTemplateGenerator.h"

#include "T4RehearsalEditorInternal.h"

/**
  *
 */
// #56
UT4ContiActionTrack::UT4ContiActionTrack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ContiViewModelInstanceKey(INDEX_NONE)
	, ActionType(ET4ActionType::None)
	, ActionHeaderKey(INDEX_NONE)
	, ActionLifecycleType(ET4LifecycleType::Default)
	, ActionSortOrder(INDEX_NONE)
	, bInitialized(false) // #65
{
	bSupportsDefaultSections = false; // #56 : 자동 선택되지 않는다.
}

bool UT4ContiActionTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UT4ContiActionSection::StaticClass();
}

UMovieSceneSection* UT4ContiActionTrack::CreateNewSection()
{
	return NewObject<UT4ContiActionSection>(this, NAME_None, RF_Transactional);
}

FMovieSceneEvalTemplatePtr UT4ContiActionTrack::CreateTemplateForSection(
	const UMovieSceneSection& InSection
) const
{
	return FT4ContiActionTrackTemplate();
}

void UT4ContiActionTrack::RemoveSection(UMovieSceneSection& Section) // #58
{
	Super::RemoveSection(Section);
}

void UT4ContiActionTrack::RemoveSectionAt(int32 SectionIndex) // #58
{
	Super::RemoveSectionAt(SectionIndex);
}

void UT4ContiActionTrack::Initialize(
	uint32 InContiViewModelInstanceKey, // #56, #65
	ET4ActionType InActionType,
	uint32 InActionHeaderKey,
	ET4LifecycleType InLifecycleType,
	uint32 InActionSortOrder, // #56
	const FText& InDisplayName
) // #54
{
	ContiViewModelInstanceKey = InContiViewModelInstanceKey;
	ActionType = InActionType;
	ActionHeaderKey = InActionHeaderKey;
	ActionLifecycleType = InLifecycleType;
	ActionSortOrder = InActionSortOrder;
	SetDisplayName(InDisplayName);
	bInitialized = true; // #65
}