// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/Datatable/T4GameBuiltin_Table_Base.h"

#include "T4GameBuiltin_Table_ItemBase.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/DataDriven/
 */
class UBehaviorTree;
USTRUCT()
struct FT4GameBuiltin_ItemTableRowBase : public FT4GameBuiltin_TableBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, Category= ServerOnly)
	TSoftObjectPtr<UBehaviorTree> BehaviorTreePath; // DropItem

public:
	FT4GameBuiltin_ItemTableRowBase()
	{
	}
};
