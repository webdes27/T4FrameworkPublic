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

	UPROPERTY(VisibleAnywhere, Category = Hide)
	FName ParentRowName; // #122

	UPROPERTY(VisibleAnywhere, Category = Hide)
	FName FolderName; // #122
#endif

public:
	FT4GameBuiltin_TableBase()
#if WITH_EDITORONLY_DATA
		: ParentRowName(NAME_None) // #122
		, FolderName(NAME_None) // #122
#endif
	{
	}
};
