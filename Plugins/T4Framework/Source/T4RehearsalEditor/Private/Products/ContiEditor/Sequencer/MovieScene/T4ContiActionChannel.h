// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Asset/Public/Action/T4ActionTypes.h"

#include "MovieSceneClipboard.h"
#include "Channels/MovieSceneChannel.h"
#include "Channels/MovieSceneChannelData.h"
#include "Channels/MovieSceneChannelHandle.h"
#include "Channels/MovieSceneChannelTraits.h"
#include "T4ContiActionChannel.generated.h"

/**
  *
 */
USTRUCT()
struct FT4ContiActionSectionKey
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ChannelKey; // #58 : Key 이동, 삭제 대응을 위한 ChannelKey 백업
};

namespace MovieSceneClipboard
{
	template<> inline FName GetKeyTypeName<FT4ContiActionSectionKey>()
	{
		return "FT4ContiActionSectionKey";
	}
}

class FT4ContiViewModel;
class UT4ContiActionSection;

USTRUCT()
struct FT4ContiActionChannel : public FMovieSceneChannel
{
	GENERATED_BODY()

	FT4ContiActionChannel();

	FORCEINLINE TMovieSceneChannelData<FT4ContiActionSectionKey> GetData()
	{
		return TMovieSceneChannelData<FT4ContiActionSectionKey>(&KeyTimes, &KeyValues, &KeyHandles);
	}

	FORCEINLINE TMovieSceneChannelData<const FT4ContiActionSectionKey> GetData() const
	{
		return TMovieSceneChannelData<const FT4ContiActionSectionKey>(&KeyTimes, &KeyValues);
	}

	bool Evaluate(FFrameTime InTime, FT4ContiActionSectionKey& OutValue) const;

public:
	bool IsInitialized() const { return bInitialized; } // #65

	void Initialize(UT4ContiActionSection* InParentSection); // #56, #65

	bool IsUseMultiKey() const; // #58 : CameraWork 는 오직 Key 만 사용! todo : 케이스가 하나 더 생기면 Action Struct 에 쿼리하도록 처리한다.
	bool TryAddKey(FFrameNumber InTime); // #58 : ViewModel 을 통해 추가 여부를 결정한다.

public:
	// ~ FMovieSceneChannel Interface
	virtual void GetKeys(const TRange<FFrameNumber>& WithinRange, TArray<FFrameNumber>* OutKeyTimes, TArray<FKeyHandle>* OutKeyHandles) override;
	virtual void GetKeyTimes(TArrayView<const FKeyHandle> InHandles, TArrayView<FFrameNumber> OutKeyTimes) override;
	virtual void SetKeyTimes(TArrayView<const FKeyHandle> InHandles, TArrayView<const FFrameNumber> InKeyTimes) override;
	virtual void DuplicateKeys(TArrayView<const FKeyHandle> InHandles, TArrayView<FKeyHandle> OutNewHandles) override;
	virtual void DeleteKeys(TArrayView<const FKeyHandle> InHandles) override;
	virtual void ChangeFrameResolution(FFrameRate SourceRate, FFrameRate DestinationRate) override;
	virtual TRange<FFrameNumber> ComputeEffectiveRange() const override;
	virtual int32 GetNumKeys() const override;
	virtual void Reset() override;
	virtual void Offset(FFrameNumber DeltaPosition) override;

protected:

	/** Array of times for each key */
	UPROPERTY(meta=(KeyTimes))
	TArray<FFrameNumber> KeyTimes;

	/** Array of values that correspond to each key time */
	UPROPERTY(meta=(KeyValues))
	TArray<FT4ContiActionSectionKey> KeyValues;

	FMovieSceneKeyHandleMap KeyHandles;

protected:
	bool bInitialized; // #65
	UT4ContiActionSection* ParentSection; // #56
};

template<>
struct TMovieSceneChannelTraits<FT4ContiActionChannel> : TMovieSceneChannelTraitsBase<FT4ContiActionChannel>
{
	enum { SupportsDefaults = false };
};

// #56 : 아래 두 템플릿 함수를 Override 해서 처리한다. 
//       특히 AddKeyToCannel 은 Section 당 1개의 Key 만 가지도록 처리하는 중요 역할을 담당...
//		 #58 에서 IsUseMultiKey 로 1개 이상을 사용할 수 있도록 수정됨
T4REHEARSALEDITOR_API bool EvaluateChannel(
	const FT4ContiActionChannel* InChannel,
	FFrameTime InTime,
	FT4ContiActionSectionKey& OutValue
);

/**
 * Overload for adding a new key to a ContiAction channel at a given time. See MovieScene::AddKeyToChannel for default implementation.
 */
T4REHEARSALEDITOR_API FKeyHandle AddKeyToChannel(
	FT4ContiActionChannel* InChannel,
	FFrameNumber InTime,
	FT4ContiActionSectionKey& InValue,
	EMovieSceneKeyInterpolation InInterpolation
); // #56

TSharedPtr<FStructOnScope> GetKeyStruct(TMovieSceneChannelHandle<FT4ContiActionChannel> Channel, FKeyHandle InHandle);