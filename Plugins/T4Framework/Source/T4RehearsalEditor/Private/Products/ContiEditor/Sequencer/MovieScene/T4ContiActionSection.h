// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ContiActionChannel.h"
#include "MovieSceneSection.h"
#include "T4ContiActionSection.generated.h"

/**
  * #54, #56
 */
class UT4ContiActionTrack;
class FT4ContiViewModel;
UCLASS(MinimalAPI)
class UT4ContiActionSection : public UMovieSceneSection
{
	GENERATED_UCLASS_BODY()

public:
	bool IsInitialized() const { return bInitialized; } // #65

	void Initialize(
		uint32 InContiViewModelInstanceKey, // #56, #65
		ET4ActionType InActionType,
		uint32 InActionHeaderKey,
		ET4LifecycleType InLifecycleType
	); // #56

	void CleanUp(); // #100

	UT4ContiActionTrack* GetParentTrack();
	ET4ActionType GetActionType() const { return ActionType; }
	uint32 GetActionHeaderKey() const { return ActionHeaderKey; }

	ET4LifecycleType GetLifecycleType() const { return ActionLifecycleType; }
	bool IsUseMultiKey() const; // #58 : CameraWork 는 오직 Key 만 사용! todo : 케이스가 하나 더 생기면 Action Struct 에 쿼리하도록 처리한다.

	void AddKey(FFrameNumber InTime); // #56 : !ET4LifecycleType::Duration, #58 : KeyIndex
	FFrameNumber GetKeyTime(); // #56 : !ET4LifecycleType::Duration

	FT4ContiViewModel* GetContiViewModel() const; // #56, #65

	FLinearColor GetBackgroundColor(); // #100

public:
	UPROPERTY()
	uint32 ViewModelInstanceKey; // #56, #65 : ViewModel 이 Instance 로 실행되기 때문에 어디의 Section 인지 확인 필요

	UPROPERTY()
	FT4ContiActionChannel ActionChannel;

	UPROPERTY()
	ET4ActionType ActionType;

	UPROPERTY()
	uint32 ActionHeaderKey;

	UPROPERTY()
	ET4LifecycleType ActionLifecycleType;

protected:
	bool bInitialized; // #65
};
