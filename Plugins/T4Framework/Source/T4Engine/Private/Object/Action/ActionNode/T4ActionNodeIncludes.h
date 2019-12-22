// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// #T4_ADD_ACTION_TAG
// #T4_ADD_ACTION_TAG_CONTI

#include "Default/T4ActionAnimationNode.h"
#include "Default/T4ActionReactionNode.h" // #76
#include "Default/T4ActionTimeScaleNode.h" // #102
#include "Default/T4ActionLayerSetNode.h" // #81
#include "Default/T4ActionSpecialMoveNode.h" // #54
#include "Default/T4ActionTurnNode.h"
#include "Default/T4ActionDummyNode.h" // #56 : Conti Editor 에서 Invisible or Isolate 로 출력을 제어할 때 더미용으로 사용 (delay, duration 동작 보장)

#include "Camera/T4ActionCameraWorkNode.h" // #54
#include "Camera/T4ActionCameraShakeNode.h" // #101
#include "Camera/T4ActionPostProcessNode.h" // #100

#include "Attached/T4ActionEquipWeaponNode.h"
#include "Attached/T4ActionParticleNode.h"
#include "Attached/T4ActionDecalNode.h" // #54
#include "Attached/T4ActionEnvironmentNode.h" // #99

#include "Composite/T4ActionProjectileNode.h" // #63
#include "Composite/T4ActionBranchNode.h" // #54
#include "Composite/T4ActionContiNode.h"
