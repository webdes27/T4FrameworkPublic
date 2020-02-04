// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/**
 * The public interface to this module
 */
class IT4FrameModule : public IModuleInterface
{
public:
	static inline IT4FrameModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IT4FrameModule>("T4Frame");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("T4Frame");
	}
};
