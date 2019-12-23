// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4FrameworkExample.h"
#include "Modules/ModuleManager.h"

/**
  * http://api.unrealengine.com/KOR/Programming/Modules/Gameplay/
 */
class FT4FrameworkExampleModuleImpl : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FT4FrameworkExampleModuleImpl, T4FrameworkExample, "T4FrameworkExample");
DEFINE_LOG_CATEGORY(LogT4FrameworkExample);