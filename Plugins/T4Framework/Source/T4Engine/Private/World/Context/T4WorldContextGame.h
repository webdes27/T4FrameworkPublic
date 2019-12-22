// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "World/Controller/T4WorldController.h"

/**
  * #87
 */
class FT4WorldController;
class FT4WorldContextGame : public IT4WorldContext
{
public:
	explicit FT4WorldContextGame(FT4WorldController* InWorldController);
	virtual ~FT4WorldContextGame();

	void Reset() override;

	void ProcessPre(float InDeltaTime) override; // #34 : OnWorldPreActorTick
	void ProcessPost(float InDeltaTime) override; // #34 : OnWorldPostActorTick

	bool IsPreviewScene() const override { return false; }

	FWorldContext* GetOwnerWorldContext() const override { return nullptr; } // #87 : WorldContext 소유권이 있다.
	bool WorldTravel(const FSoftObjectPath& InAssetPath, const FVector& InStartLocation) override;

	void SetPlayerController(APlayerController* InPlayerController) override {} // #86, #87 : Only PreviewWorld
	void SetLevelStreamingFrozen(bool bInFrozen) override {} // #86 : Only PreviewWorld
	bool IsLevelStreamingFrozen() const override { return false; } // #86, #104

private:
	FT4WorldController* WorldControllerRef;
};
