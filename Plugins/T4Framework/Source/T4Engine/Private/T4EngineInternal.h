// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
  *
 */
DECLARE_LOG_CATEGORY_EXTERN(LogT4Engine, Log, All)

class IT4GameObject;

namespace T4EnginetUtil
{
	IT4GameObject* TryCastGameObject(AActor* InActor);
}