// Copyright 2019-2020 SoonBo Noh.s All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
  *
 */

// Common

static const FName T4Const_DefaultName = TEXT("Default");

static const FName T4Const_DefaultActionPointName = TEXT("Root"); // #63

// #74 : Entity

static const FName T4Const_DefaultFullBodySkinName = T4Const_DefaultName; // #130
static const FName T4Const_DefaultPlayTagName = T4Const_DefaultName;


// #73, #106 : Stance

static const FName T4Const_DefaultStanceName = T4Const_DefaultName;

static const FName T4Const_DefaultPostureName = T4Const_DefaultName;
static const FName T4Const_CombatPostureName = TEXT("Combat"); // #108
static const FName T4Const_CrouchPostureName = TEXT("Crouch"); // #109


// #78

static const float T4Const_ObjectWorldEnterTimeSec = 1.0f;
static const float T4Const_ObjectWorldLeaveTimeSec = 1.0f;


// #39 : AnimSet

static const float T4Const_DefaultAnimBlendTimeSec = 0.1f;

static const FName T4Const_ItemBlendSpaceName = TEXT("Item_Default"); // #107

static const FName T4Const_DefaultAnimMontageName = TEXT("Default");
static const FName T4Const_OverlayAnimMontageName = TEXT("Overlay");
static const FName T4Const_SkillAnimMontageName = TEXT("Skill");

static const FName T4Const_DefaultAnimMontageSlotName = TEXT("DefaultSlot"); // #39, #107
static const FName T4Const_OverlayAnimMontageSlotName = TEXT("OverlaySlot"); // #39, #107
static const FName T4Const_SkillAnimMontageSlotName = TEXT("SkillSlot"); // #39, #107
