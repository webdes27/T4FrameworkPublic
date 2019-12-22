// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameplayInternal.h"

#include "Gameplay/T4GameplayInstance.h"

#include "T4Frame/Public/T4Frame.h"

#include "T4GameplayInternal.h"

/**
  *
 */
IT4PacketHandlerSC* GetPacketHandlerSC(ET4LayerType InLayerType) // #49
{
	check(ET4LayerType::Max > InLayerType);
	IT4GameFrame* GameFrame = T4FrameGet(InLayerType);
	check(nullptr != GameFrame);
	FT4GameplayInstance* GameplayInstance = FT4GameplayInstance::CastFrom(
		GameFrame->GetGameplayInstance()
	);
	if (nullptr == GameplayInstance)
	{
		return nullptr;
	}
	return GameplayInstance->GetPacketHandlerSC();
}