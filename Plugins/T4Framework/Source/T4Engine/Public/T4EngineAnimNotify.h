// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4EngineTypes.h"

/**
  * #111
 */
struct FT4AnimNotifyMessage
{
	ET4AnimNotifyType AnimNotifyType;
#if WITH_EDITOR
	FString DebugSting;
#endif
};

struct FT4AnimNotifyEquipment : public FT4AnimNotifyMessage
{
	FT4AnimNotifyEquipment()
	{
		AnimNotifyType = ET4AnimNotifyType::Equipment;
		SameStanceName = NAME_None;
	}
	ET4EquipmentType EquipmentType;
	FName SameStanceName;
};

struct FT4AnimNotifyFootstep : public FT4AnimNotifyMessage
{
	FT4AnimNotifyFootstep()
	{
		AnimNotifyType = ET4AnimNotifyType::Footstep;
	}
	ET4FootstepType FootstepType;
};
