// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/**
 * The public interface to this module
 */
class IT4EngineModule : public IModuleInterface
{
public:
	static inline IT4EngineModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IT4EngineModule>("T4Engine");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("T4Engine");
	}
};

