// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionTypes.h"
#include "T4ActionCommand.generated.h"

/**
  *
 */
USTRUCT()
struct T4ASSET_API FT4ActionCommand
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ActionDetails::CustomizeCommonActionDetails

	UPROPERTY(VisibleAnywhere)
	ET4ActionType ActionType;

	UPROPERTY(EditAnywhere)
	ET4LifecycleType LifecycleType;

	UPROPERTY(EditAnywhere)
	float StartTimeSec;

	UPROPERTY(EditAnywhere)
	float DurationSec; // #20 : TotalTimeSec = StartTimeSec + DurationSec;

	UPROPERTY(VisibleAnywhere)
	int32 SortOrder; // #56 : lower win

	UPROPERTY(Transient)
	bool bTransient; // #110 : true 일 경우 Action Replay 녹화에 포함되지 않는다.

public:
	FT4ActionCommand()
		: ActionType(ET4ActionType::None)
		, LifecycleType(ET4LifecycleType::Default)
		, StartTimeSec(0.0f)
		, DurationSec(0.0f)
		, SortOrder(TNumericLimits<int32>::Max()) // #56 : lower win
		, bTransient(false) // #110
	{
	}

	FT4ActionCommand(ET4ActionType InObjectAction)
		: ActionType(InObjectAction)
		, LifecycleType(ET4LifecycleType::Default)
		, StartTimeSec(0.0f)
		, DurationSec(0.0f)
		, SortOrder(TNumericLimits<int32>::Max()) // #56 : lower win
		, bTransient(false) // #110
	{
	}

	virtual ~FT4ActionCommand() {}

	virtual ET4ActionStructType GetActionStructType() const // #62
	{
		return ET4ActionStructType::None;
	}

	virtual bool Validate(FString& OutMsg)
	{
		return true;
	}

	virtual FString ToString() const
	{
		return FString(TEXT("ActionStruct"));
	}

	virtual FString ToDisplayText()
	{
		return FString(TEXT("Untitled")); // #54
	}
};
