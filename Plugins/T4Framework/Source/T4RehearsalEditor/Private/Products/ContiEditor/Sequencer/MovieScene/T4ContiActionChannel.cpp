// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4ContiActionChannel.h"
#include "T4ContiActionTrack.h"

#include "Products/ContiEditor/Sequencer/MovieScene/T4ContiActionSection.h"
#include "Products/ContiEditor/ViewModel/T4ContiViewModel.h"

#include "UObject/PropertyPortFlags.h"

/**
  *
 */
FT4ContiActionChannel::FT4ContiActionChannel()
	: bInitialized(false) // #65
	, ParentSection(nullptr) // #56
{
}

bool FT4ContiActionChannel::Evaluate(FFrameTime InTime, FT4ContiActionSectionKey& OutValue) const
{
	if (KeyTimes.Num())
	{
		const int32 Index = FMath::Max(0, Algo::UpperBound(KeyTimes, InTime.FrameNumber) - 1);
		OutValue = KeyValues[Index];
		return true;
	}

	return false;
}

void FT4ContiActionChannel::Initialize(
	UT4ContiActionSection* InParentSection
) // #56
{
	ParentSection = InParentSection; // #56
	bInitialized = true; // #65
}

bool FT4ContiActionChannel::IsUseMultiKey() const // #58 : CameraWork 는 오직 Key 만 사용! todo : 케이스가 하나 더 생기면 Action Struct 에 쿼리하도록 처리한다.
{
	check(nullptr != ParentSection);
	return ParentSection->IsUseMultiKey();
}

bool FT4ContiActionChannel::TryAddKey(FFrameNumber InTime) // #58 : ViewModel 을 통해 추가 여부를 결정한다.
{
	check(nullptr != ParentSection);
	check(IsUseMultiKey()); // #58: MultiKey 를 허용하는 Action 만!
	check(ET4LifecycleType::Duration != ParentSection->GetLifecycleType());
	// #58 : AddKey 에 대한 특별한 Notify 가 없어서 별도로 구현함
	FT4ContiViewModel* ContiViewModel = ParentSection->GetContiViewModel();
	if (nullptr == ContiViewModel)
	{
		return false;
	}
	bool bResult = ContiViewModel->AddNewMovieSceneActionSectionKey(
		ParentSection->GetActionHeaderKey(), 
		InTime.Value
	);
	if (!bResult)
	{
		return false;
	}
	return true;
}

void FT4ContiActionChannel::GetKeys(
	const TRange<FFrameNumber>& WithinRange, 
	TArray<FFrameNumber>* OutKeyTimes, 
	TArray<FKeyHandle>* OutKeyHandles
)
{
	GetData().GetKeys(WithinRange, OutKeyTimes, OutKeyHandles);
}

void FT4ContiActionChannel::GetKeyTimes(
	TArrayView<const FKeyHandle> InHandles, 
	TArrayView<FFrameNumber> OutKeyTimes
)
{
	GetData().GetKeyTimes(InHandles, OutKeyTimes);
	// #56 : SectionKey 선택시 호출됨. 특별한 delegate 가 없어 별도 처리함
	FT4ContiViewModel* ContiViewModel = ParentSection->GetContiViewModel(); // #65
	check(nullptr != ContiViewModel);
	if (ContiViewModel->GetSequencer().IsValid())
	{
		ContiViewModel->GetSequencer()->SelectTrack(
			Cast<UMovieSceneTrack>(ParentSection->GetParentTrack())
		);
	}
}

void FT4ContiActionChannel::SetKeyTimes(
	TArrayView<const FKeyHandle> InHandles, 
	TArrayView<const FFrameNumber> InKeyTimes
)
{
	GetData().SetKeyTimes(InHandles, InKeyTimes);
}

void FT4ContiActionChannel::DuplicateKeys(
	TArrayView<const FKeyHandle> InHandles, 
	TArrayView<FKeyHandle> OutNewHandles
)
{
	GetData().DuplicateKeys(InHandles, OutNewHandles);
}

void FT4ContiActionChannel::DeleteKeys(TArrayView<const FKeyHandle> InHandles)
{
	// #58 : SectionKey 삭제
	bool bResult = false;
	{
		const uint32 ActionHeaderKey = ParentSection->GetActionHeaderKey();
		FT4ContiViewModel* ContiViewModel = ParentSection->GetContiViewModel();; // #65
		check(nullptr != ContiViewModel);
		TArray<FFrameNumber> KeyTimeDatas;
		KeyTimeDatas.SetNum(InHandles.Num());
		GetData().GetKeyTimes(InHandles, KeyTimeDatas);
		for (const FFrameNumber FrameNumber : KeyTimeDatas)
		{
			bResult |= ContiViewModel->RemoveMovieSceneActionSectionKey(ActionHeaderKey, FrameNumber.Value);
		}
	}
	if (!bResult)
	{
		return;
	}
	GetData().DeleteKeys(InHandles);
}

void FT4ContiActionChannel::ChangeFrameResolution(
	FFrameRate SourceRate, 
	FFrameRate DestinationRate
)
{
	GetData().ChangeFrameResolution(SourceRate, DestinationRate);
}

TRange<FFrameNumber> FT4ContiActionChannel::ComputeEffectiveRange() const
{
	return GetData().GetTotalRange();
}

int32 FT4ContiActionChannel::GetNumKeys() const
{
	return KeyTimes.Num();
}

void FT4ContiActionChannel::Reset()
{
	KeyTimes.Reset();
	KeyValues.Reset();
	KeyHandles.Reset();
}

void FT4ContiActionChannel::Offset(FFrameNumber DeltaPosition)
{
	GetData().Offset(DeltaPosition);
}

TSharedPtr<FStructOnScope> GetKeyStruct(
	TMovieSceneChannelHandle<FT4ContiActionChannel> Channel, 
	FKeyHandle InHandle
)
{
	int32 KeyValueIndex = Channel.Get()->GetData().GetIndex(InHandle);
	if (KeyValueIndex != INDEX_NONE)
	{
		//FNiagaraTypeDefinition KeyType = Channel.Get()->GetData().GetValues()[KeyValueIndex].Value.GetType();
		//uint8* KeyData = Channel.Get()->GetData().GetValues()[KeyValueIndex].Value.GetData();
		//return MakeShared<FStructOnScope>(KeyType.GetStruct(), KeyData);
	}
	return TSharedPtr<FStructOnScope>();
}

bool EvaluateChannel(
	const FT4ContiActionChannel* InChannel,
	FFrameTime InTime,
	FT4ContiActionSectionKey& OutValue
) // #56
{
	bool bResult = InChannel->Evaluate(InTime, OutValue);
	return bResult;
}

FKeyHandle AddKeyToChannel(
	FT4ContiActionChannel* InChannel,
	FFrameNumber InTime,
	FT4ContiActionSectionKey& InValue,
	EMovieSceneKeyInterpolation InInterpolation
) // #56
{
	check(nullptr != InChannel);

	TMovieSceneChannelData<FT4ContiActionSectionKey> ChannelInterface = InChannel->GetData();
	if (!InChannel->IsUseMultiKey()) // #58
	{
		if (0 < ChannelInterface.GetValues().Num())
		{
			return FKeyHandle::Invalid(); // #56 : 한개만 허용한다.
		}
	}

	bool bAdded = InChannel->TryAddKey(InTime);
	if (!bAdded)
	{
		return FKeyHandle::Invalid(); // #58 : 등록 실패!
	}

	FT4ContiActionSectionKey NewSectionKey;
	NewSectionKey.ChannelKey = InTime.Value;

	int32 ExistingIndex = ChannelInterface.FindKey(InTime);
	check(INDEX_NONE == ExistingIndex);
	ExistingIndex = ChannelInterface.AddKey(InTime, NewSectionKey);
	FKeyHandle Handle = ChannelInterface.GetHandle(ExistingIndex);

#if 0
	if (ExistingIndex != INDEX_NONE)
	{
		Handle = ChannelInterface.GetHandle(ExistingIndex);
		MovieScene::AssignValue(InChannel, Handle, Forward<FT4ContiActionSectionKey>(InValue));
	}
	else
	{
		ExistingIndex = ChannelInterface.AddKey(InTime, Forward<FT4ContiActionSectionKey>(InValue));
		Handle = ChannelInterface.GetHandle(ExistingIndex);
	}
#endif

	return Handle;
}
