// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4FrameworkBuiltin.h"
#include "Modules/ModuleManager.h"

/**
  * http://api.unrealengine.com/KOR/Programming/Modules/Gameplay/
 */
class FT4FrameworkBuiltinModuleImpl : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FT4FrameworkBuiltinModuleImpl, T4FrameworkBuiltin, "T4FrameworkBuiltin");
DEFINE_LOG_CATEGORY(LogT4FrameworkBuiltin);