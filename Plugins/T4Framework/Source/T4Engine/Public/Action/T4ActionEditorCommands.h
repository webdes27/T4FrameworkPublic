// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionCodeCommand.h"
#include "T4ActionEditorCommands.generated.h"

/**
  *
 */
 // #T4_ADD_ACTION_TAG_CODE

// ET4ActionType::Editor

USTRUCT()
struct T4ENGINE_API FT4EditorAction : public FT4ActionCodeCommand
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	ET4EditorAction EditorActionType;

	UPROPERTY(EditAnywhere)
	FName PlayTagName; // #81 : PlayTagSet, PlayTagClear

	UPROPERTY(EditAnywhere)
	ET4PlayTagType PlayTagType; // #81 : PlayTagSet, PlayTagClear

public:
	FT4EditorAction()
		: FT4ActionCodeCommand(StaticActionType())
		, EditorActionType(ET4EditorAction::None)
		, PlayTagName(NAME_None) // #81 : PlayTagSet, PlayTagClear
		, PlayTagType(ET4PlayTagType::All) // #81 : PlayTagSet, PlayTagClear
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::Editor; }

	bool Validate(FString& OutMsg) override
	{
		if (ET4EditorAction::None == EditorActionType)
		{
			OutMsg = TEXT("Not set EditorActionType");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("EditorAction"));
	}
};
