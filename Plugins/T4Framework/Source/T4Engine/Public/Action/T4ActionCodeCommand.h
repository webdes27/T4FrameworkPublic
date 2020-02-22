// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Asset/Public/Action/T4ActionCommand.h"
#include "T4ActionKey.h"
#include "Public/T4EngineTypes.h"
#include "T4ActionCodeCommand.generated.h"

/**
  *
 */
USTRUCT()
struct T4ENGINE_API FT4ActionCodeCommand : public FT4ActionCommand
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FT4ActionKey ActionKey;

public:
	FT4ActionCodeCommand()
		: FT4ActionCommand()
	{
	}

	FT4ActionCodeCommand(ET4ActionType InObjectAction)
		: FT4ActionCommand(InObjectAction)
	{
	}

	virtual ~FT4ActionCodeCommand() {}

	ET4ActionStructType GetActionStructType() const override { return ET4ActionStructType::Code; } // #52

	virtual bool Validate(FString& OutMsg)
	{
		return true;
	}

	virtual FString ToString() const
	{
		return FString(TEXT("CodeAction"));
	}

	virtual FString ToDisplayText()
	{
		return FString(TEXT("Untitled")); // #54
	}
};
