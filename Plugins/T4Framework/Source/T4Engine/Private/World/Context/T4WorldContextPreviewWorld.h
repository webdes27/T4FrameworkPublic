// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "World/Controller/T4WorldController.h"

#if WITH_EDITOR

/**
  * #87
 */
class FT4WorldController;
class UT4WorldPreviewGameInstance; // #79
class FT4WorldContextPreviewWorld : public IT4WorldContext
{
public:
	explicit FT4WorldContextPreviewWorld(FT4WorldController* InWorldController);
	virtual ~FT4WorldContextPreviewWorld();

	void Reset() override;

	void ProcessPre(float InDeltaTime) override; // #34 : OnWorldPreActorTick
	void ProcessPost(float InDeltaTime) override; // #34 : OnWorldPostActorTick

	bool IsPreviewScene() const override { return false; }

	FWorldContext* GetOwnerWorldContext() const override { return WorldContext; } // #87 : WorldContext 소유권이 있다.
	bool WorldTravel(const FSoftObjectPath& InAssetPath, const FVector& InStartLocation) override;

	void SetPlayerController(APlayerController* InPlayerController) override; // #86, #87 : Only PreviewWorld
	void SetLevelStreamingFrozen(bool bInFrozen) override; // #86 : Only PreviewWorld
	bool IsLevelStreamingFrozen() const override; // #86, #104

private:
	void Reinitialize();

	void FlushAndCleanUpCurrentWorld(); // #83
	
	void CheckAndNewSourceWorldContext(const TCHAR* InTravelURL); // #79

	UWorld* NewEmptyWorld();

private:
	FT4WorldController* WorldControllerRef;

	FWorldContext* WorldContext;
	UT4WorldPreviewGameInstance* GameInstance; // #79 : only PreviewWorld
};

#endif