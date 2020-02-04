// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4AssetModule.h"
#include "T4AssetInternal.h"

#define LOCTEXT_NAMESPACE "T4Asset"

/**
  *
 */
class FT4AssetModule : public IT4AssetModule
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};

IMPLEMENT_MODULE(FT4AssetModule, T4Asset)
DEFINE_LOG_CATEGORY(LogT4Asset)

void FT4AssetModule::StartupModule()
{
}

void FT4AssetModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE