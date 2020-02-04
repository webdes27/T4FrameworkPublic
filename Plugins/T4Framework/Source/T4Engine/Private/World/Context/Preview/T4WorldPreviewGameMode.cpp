// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4WorldPreviewGameMode.h"

#include "T4EngineInternal.h"

/**
  * #79
 */
AT4WorldPreviewGameMode::AT4WorldPreviewGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AT4WorldPreviewGameMode::StartPlay()
{
	// pass; call FT4ClientGameFrame::StartPlayPreview()
	// Super::StartPlay
}