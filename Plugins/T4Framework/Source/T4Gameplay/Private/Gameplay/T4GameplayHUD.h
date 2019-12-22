// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4Engine/Public/T4EngineTypes.h"

/**
  * // #68
 */
class FViewport;
class FCanvas;
class IT4PlayerController;
class IT4GameFrame;
class FT4GameplayInstance;
class FT4GameplayHUD
{
public:
	explicit FT4GameplayHUD(ET4LayerType InLayerType);
	~FT4GameplayHUD();

	bool Initialize();
	void Finalize();

	void Draw(FViewport* InViewport, FCanvas* InCanvas, FT4HUDDrawInfo& InOutDrawInfo); // #68

protected:
	void DrawText(
		FViewport* InViewport,
		FCanvas* InCanvas,
		const FString& InMessages,
		const FLinearColor& InColor,
		FT4HUDDrawInfo& InOutDrawInfo
	);

private:
	IT4PlayerController* GetPlayerController() const;
	IT4GameFrame* GetGameFrame() const;
	FT4GameplayInstance* GetGameplayInstance() const;

private:
	ET4LayerType LayerType;
};
