// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ContiActionSection.h"
#include "T4ContiActionTrack.h" // #100
#include "Products/ContiEditor/ViewModel/T4ContiViewModel.h" // #65
#include "Channels/MovieSceneChannelProxy.h"
#include "T4RehearsalEditorInternal.h"

/**
  * #54
 */
// #56
UT4ContiActionSection::UT4ContiActionSection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ViewModelInstanceKey(INDEX_NONE) // #56, #65 : ViewModel 이 Instance 로 실행되기 때문에 어디의 Section 인지 확인 필요
	, ActionType(ET4ActionType::None)
	, ActionHeaderKey(INDEX_NONE)
	, ActionLifecycleType(ET4LifecycleType::Default)
	, bInitialized(false) // #65
{
	bSupportsInfiniteRange = false; // #56 : 무한대는 허용하지 않는다.
	EvalOptions.CompletionMode = EMovieSceneCompletionMode::ProjectDefault;
}

void UT4ContiActionSection::Initialize(
	uint32 InViewModelInstanceKey, // #56, #65
	ET4ActionType InActionType,
	uint32 InActionHeaderKey,
	ET4LifecycleType InLifecycleType
) // #56
{
	ViewModelInstanceKey = InViewModelInstanceKey; // #56, #65
	ActionType = InActionType;
	ActionHeaderKey = InActionHeaderKey;
	ActionLifecycleType = InLifecycleType;

	if (ET4LifecycleType::Duration != ActionLifecycleType)
	{
		bSupportsInfiniteRange = true;
		SetRange(TRange<FFrameNumber>::All());

		ActionChannel.Initialize(this); // #56, #65

		ChannelProxy = MakeShared<FMovieSceneChannelProxy>(ActionChannel, FMovieSceneChannelMetaData());
	}

	SetIsLocked(false); // 4.22 => 4.23
	bInitialized = true; // #65
}

UT4ContiActionTrack* UT4ContiActionSection::GetParentTrack() 
{ 
	return Cast<UT4ContiActionTrack>(GetOuter());
}

bool UT4ContiActionSection::IsUseMultiKey() const // #58 : CameraWork 는 오직 Key 만 사용! todo : 케이스가 하나 더 생기면 Action Struct 에 쿼리하도록 처리한다.
{
	return (ET4ActionType::CameraWork == GetActionType()) ? true : false;
}

void UT4ContiActionSection::AddKey(FFrameNumber InTime) // #56
{
	if (ET4LifecycleType::Duration == ActionLifecycleType)
	{
		return;
	}
	if (!IsUseMultiKey()) // #58
	{
		TArrayView<FFrameNumber> FrameTimes = ActionChannel.GetData().GetTimes();
		if (0 < FrameTimes.Num())
		{
			return;
		}
	}
	FT4ContiActionSectionKey NewSectionKey;
	NewSectionKey.ChannelKey = InTime.Value;
	ActionChannel.GetData().AddKey(InTime, NewSectionKey);
}

FFrameNumber UT4ContiActionSection::GetKeyTime() // #56
{
	if (ET4LifecycleType::Duration == ActionLifecycleType)
	{
		return FFrameNumber();
	}
	TArrayView<FFrameNumber> FrameTimes = ActionChannel.GetData().GetTimes();
	if (0 >= FrameTimes.Num())
	{
		return FFrameNumber();
	}
	return FrameTimes[0];
}

FT4ContiViewModel* UT4ContiActionSection::GetContiViewModel() const // #56, #65
{
	return FT4ContiViewModel::GetContiViewModelByInstanceKey(ViewModelInstanceKey);
}

FLinearColor UT4ContiActionSection::GetBackgroundColor() // #100
{
	UT4ContiActionTrack* ParentActionTrack = GetParentTrack();
	if (nullptr == ParentActionTrack)
	{
		return FLinearColor::Transparent;
	}
	FColor TrackColorTint = ParentActionTrack->GetColorTint();
	return TrackColorTint.ReinterpretAsLinear();
}
