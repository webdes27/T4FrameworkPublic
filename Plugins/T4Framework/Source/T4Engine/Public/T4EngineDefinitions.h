// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4Asset/Public/T4AssetDefinitions.h" // #62

/**
  *
 */

// Common

static const float T4Const_DefaultLineTraceMaxDistance = 100000.0f;

#if (WITH_EDITOR || WITH_SERVER_CODE)
// DefaultEngine.ini [/Script/Engine.CollisionProfile]
#define T4COLLISION_WorldActor		ECC_GameTraceChannel1
#define T4COLLISION_WEAPON			ECC_GameTraceChannel2
#endif

#define T4_INVALID_NAVEXTENT		(FVector::ZeroVector)

// #93 : World Zone & Environment & Time of Day
static const FName T4Const_GlobalWorldZoneName = TEXT("Global"); // #93 : WorldZoneConstantTable 의 GlobalZone 과 같아야 함!!

// #97 : TimeTag 를 추가할 경우 반드시 FT4WorldTimeControl::Initialize(float InStartHour) 에도 추가해줄 것!
// #97 : 만약, 20 ~ 7 시 같은 시간대를 추가한다면 void FT4WorldTimeControl::AddConstantGameTimeData 처리를 체크해볼 것!
static const FName T4Const_WorldTimeTagDayName = TEXT("Day");			// Hour : 9 ~ 18 or 7 ~ 18 or 24
static const FName T4Const_WorldTimeTagSunsetName = TEXT("Sunset");		// Hour : 18 ~ 20
static const FName T4Const_WorldTimeTagNightName = TEXT("Night");		// Hour : 20 ~ 7 or 18 ~ 7
static const FName T4Const_WorldTimeTagSunriseName = TEXT("Sunrise");	// Hour : 7 ~ 9

static const FName T4Const_WorldTimeTagFallbackName = T4Const_WorldTimeTagDayName; // #123 : Day 를 Fallback 으로 사용한다. 별도로 세팅하지 않음

// #39 : AnimSet

static const FName T4Const_AnimNotifyFootStepName = TEXT("Footstep_C"); // TODO : T4AnimNotify_Footstep 으로 변경
static const FName T4Const_AnimNotifyFootStepLeftName = TEXT("Footstep_Left"); // TODO : T4AnimNotify_Footstep 으로 변경
static const FName T4Const_AnimNotifyFootStepRightName = TEXT("Footstep_Right"); // TODO : T4AnimNotify_Footstep 으로 변경

static const FName T4Const_DefaultSectionJumpStartName = TEXT("Jump_Start");
static const FName T4Const_DefaultSectionJumpEndName = TEXT("Jump_End");

static const FName T4Const_DefaultSectionRollFrontName = TEXT("Roll_F"); // #46
static const FName T4Const_DefaultSectionRollBackName = TEXT("Roll_B"); // #46
static const FName T4Const_DefaultSectionRollLeftName = TEXT("Roll_L"); // #46
static const FName T4Const_DefaultSectionRollRightName = TEXT("Roll_R"); // #46
static const FName T4Const_DefaultSectionRollFrontLeftName = TEXT("Roll_FL"); // #46
static const FName T4Const_DefaultSectionRollFrontRightName = TEXT("Roll_FR"); // #46
static const FName T4Const_DefaultSectionRollBackLeftName = TEXT("Roll_BL"); // #46
static const FName T4Const_DefaultSectionRollBackRightName = TEXT("Roll_BR"); // #46

static const FName T4Const_DefaultSectionStanceTransitionName = TEXT("Transition_Idle"); // #111

static const FName T4Const_OverlaySectionSubStanceDefaultToCombatName = TEXT("DefaultToCombat"); // #111
static const FName T4Const_OverlaySectionSubStanceCombatToDefaultName = TEXT("CombatToDefault"); // #111

static const FName T4Const_OverlaySectionEquipWeaponName = TEXT("EquipWeapon"); // #111
static const FName T4Const_OverlaySectionUnequipWeaponName = TEXT("UnequipWeapon"); // #111

static const FName T4Const_OverlaySectionDefaultTurnLeft90Name = TEXT("DefaultTurnLeft90"); // #113
static const FName T4Const_OverlaySectionDefaultTurnRight90Name = TEXT("DefaultTurnRight90"); // #113
static const FName T4Const_OverlaySectionCombatTurnLeft90Name = TEXT("CombatTurnLeft90"); // #113
static const FName T4Const_OverlaySectionCombatTurnRight90Name = TEXT("CombatTurnRight90"); // #113

// #47 : AnimState

static const FName T4Const_EmptyAnimStateName = TEXT("T4EmptyState");
static const FName T4Const_ErrorAnimStateName = TEXT("T4ErrorState");

static const FName T4Const_JumpAnimStateName = TEXT("T4JumpingState");
static const FName T4Const_RollAnimStateName = TEXT("T4RollingState"); // #46
static const FName T4Const_RunAnimStateName = TEXT("T4RunningState"); // #48

static const FName T4Const_VoidAnimStateName = TEXT("T4VoidState"); // #76

static const FName T4Const_CombatStanceAnimStateName = TEXT("T4CombatStanceState"); // #48
static const FName T4Const_UnarmedStanceAnimStateName = TEXT("T4UnarmedStanceState"); // #48
