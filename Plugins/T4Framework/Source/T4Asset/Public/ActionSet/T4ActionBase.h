// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionSetTypes.h"
#include "T4ActionBase.generated.h"

/**
  *
 */
USTRUCT()
struct T4ASSET_API FT4ActionBase
{
	GENERATED_USTRUCT_BODY()

public:
	// #39 : FT4ActionDetails::CustomizeCommonActionDetails

	UPROPERTY(EditAnywhere)
	ET4LifecycleType LifecycleType;

	UPROPERTY(EditAnywhere)
	float StartTimeSec;

	UPROPERTY(EditAnywhere)
	float DurationSec; // #20 : TotalTimeSec = StartTimeSec + DurationSec;

public:
	FT4ActionBase()
		: LifecycleType(ET4LifecycleType::Default)
		, StartTimeSec(0.0f)
		, DurationSec(0.0f)
	{
	}

	virtual ~FT4ActionBase() {}

	virtual ET4ActionBaseType GetActionBaseType() const // #62
	{
		return ET4ActionBaseType::None;
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
