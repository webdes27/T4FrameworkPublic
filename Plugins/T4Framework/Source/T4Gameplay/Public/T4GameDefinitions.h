// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4Engine/Public/Action/T4ActionKey.h"

/**
  * #114
 */
static const FName T4Const_DefaultPlayerRaceName = TEXT("Human"); // #114
static const FName T4Const_DefaultNPCRaceName = TEXT("Orc"); // #114

static const FT4ActionKey T4Const_ActionJumpPKey(TEXT("T4Jump"), true); // #20
static const FT4ActionKey T4Const_ActionRollPKey(TEXT("T4Roll"), true); // #46
static const FT4ActionKey T4Const_ActionTurnPKey(TEXT("T4Turn"), true); // #40
static const FT4ActionKey T4Const_ActionLockOnPKey(TEXT("T4LockOn"), true); // #20
