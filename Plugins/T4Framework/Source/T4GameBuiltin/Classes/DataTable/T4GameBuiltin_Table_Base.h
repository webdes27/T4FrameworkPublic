// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4GameBuiltin_GameDataTypes.h" // #48

#include "Classes/Engine/DataTable.h"

#include "T4GameBuiltin_Table_Base.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
USTRUCT()
struct FT4GameBuiltin_TableBase : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category= Editor)
	FString Description;
#endif

public:
	FT4GameBuiltin_TableBase()
	{
	}
};
