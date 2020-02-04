// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
  *
 */
DECLARE_LOG_CATEGORY_EXTERN(LogT4Engine, Log, All)

// #105
#define T4_FILENAME_STR FString(FileStr.Right(FileStr.Len() - FileStr.Find("\\", ESearchCase::Type::IgnoreCase, ESearchDir::Type::FromEnd, FileStr.Len()) - 1))
#define T4_FUNCTION_STR FString(FunctionStr.Right(FunctionStr.Len() - FunctionStr.Find(TEXT("::")) - 2 ))
#define T4_LOG(Verbosity, Format, ...)																							\
	{																															\
		const FString FileStr = FString(__FILE__);																				\
		const FString FunctionStr = FString(__FUNCTION__);																		\
		const FString OutputString = FString::Printf(Format, ##__VA_ARGS__);													\
		UE_LOG(LogT4Engine, Verbosity, TEXT("%s (%i) => %s / %s"), *T4_FILENAME_STR, __LINE__, *T4_FUNCTION_STR, *OutputString);	\
	}

class IT4GameObject;

namespace T4EnginetInternal
{
	IT4GameObject* TryCastGameObject(AActor* InActor);
}