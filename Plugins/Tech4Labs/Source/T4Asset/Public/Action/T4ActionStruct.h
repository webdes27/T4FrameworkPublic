// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionTypes.h"
#include "T4ActionStruct.generated.h"

/**
  *
 */
USTRUCT()
struct T4ASSET_API FT4ActionStruct
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ContiDetailCustomization::CustomizeCommonActionDetails

	UPROPERTY(VisibleAnywhere)
	ET4ActionType ActionType;

	UPROPERTY(EditAnywhere)
	ET4LifecycleType LifecycleType;

	UPROPERTY()
	ET4LifecycleType LifecyclePolicy_DEPRECATED;

	UPROPERTY(EditAnywhere)
	float StartTimeSec;

	UPROPERTY()
	float DelayTimeSec_DEPRECATED;

	UPROPERTY(EditAnywhere)
	float DurationSec; // #20 : TotalTimeSec = StartTimeSec + DurationSec;

	UPROPERTY(VisibleAnywhere)
	int32 SortOrder; // #56 : lower win

public:
	FT4ActionStruct()
		: ActionType(ET4ActionType::None)
		, LifecycleType(ET4LifecycleType::Default)
		, StartTimeSec(0.0f)
		, DurationSec(0.0f)
		, SortOrder(TNumericLimits<int32>::Max()) // #56 : lower win
	{
	}

	FT4ActionStruct(ET4ActionType InObjectAction)
		: ActionType(InObjectAction)
		, LifecycleType(ET4LifecycleType::Default)
		, StartTimeSec(0.0f)
		, DurationSec(0.0f)
		, SortOrder(TNumericLimits<int32>::Max()) // #56 : lower win
	{
	}

	virtual ~FT4ActionStruct() {}

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
