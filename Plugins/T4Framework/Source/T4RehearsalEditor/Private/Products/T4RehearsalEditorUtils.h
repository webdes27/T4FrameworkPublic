// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4Engine/Public/T4EngineTypes.h"

/**
  *
 */
class ULevel; // #91
class UWorld;
class UT4EntityAsset;
class UT4ContiAsset;
class UAnimSequence;
struct FT4ActionParameters;

namespace T4EditorUtil
{
	UWorld* GetWorld(ET4LayerType InLayerType);

	// Client only

	void GoPreviewScene(ET4LayerType InLayerType);

	// Server => Client

	void ServerDespawnAll(ET4LayerType InLayerType, bool bClearPlayerObject); // #68

	// Utility
	FBox CalculateLevelBounds(ULevel* InLevel); // #91 : World Single
}