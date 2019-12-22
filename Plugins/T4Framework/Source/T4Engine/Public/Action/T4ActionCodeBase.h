// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Asset/Public/Action/T4ActionStruct.h"
#include "T4ActionKey.h"
#include "Public/T4EngineTypes.h"
#include "T4ActionCodeBase.generated.h"

/**
  *
 */
USTRUCT()
struct T4ENGINE_API FT4CodeActionStruct : public FT4ActionStruct
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FT4ActionKey ActionKey;

public:
	FT4CodeActionStruct()
		: FT4ActionStruct()
	{
	}

	FT4CodeActionStruct(ET4ActionType InObjectAction)
		: FT4ActionStruct(InObjectAction)
	{
	}

	virtual ~FT4CodeActionStruct() {}

	ET4ActionStructType GetActionStructType() const override { return ET4ActionStructType::Code; } // #52

	virtual bool Validate(FString& OutMsg)
	{
		return true;
	}

	virtual FString ToString() const
	{
		return FString(TEXT("CodeActionStruct"));
	}

	virtual FString ToDisplayText()
	{
		return FString(TEXT("Untitled")); // #54
	}
};
