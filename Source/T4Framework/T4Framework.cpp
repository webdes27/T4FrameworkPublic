// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4Framework.h"
#include "Modules/ModuleManager.h"

/**
  * http://api.unrealengine.com/KOR/Programming/Modules/Gameplay/
 */
class FT4FrameworkModuleImpl : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FT4FrameworkModuleImpl, T4Framework, "T4Framework");
DEFINE_LOG_CATEGORY(LogT4Framework);