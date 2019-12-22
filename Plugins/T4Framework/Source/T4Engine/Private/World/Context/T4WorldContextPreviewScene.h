// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "World/Controller/T4WorldController.h"

#if WITH_EDITOR

/**
  * #87
 */
 // #T4_ADD_TOD_TAG
class ADirectionalLight; // #94
class ASkyLight; // #94
class AAtmosphericFog; // #94
class AExponentialHeightFog; // #94
#if WITH_EDITOR
class AT4MapZoneVolume; // #94
#endif
class FT4WorldController;
class ANavMeshBoundsVolume;
class AT4WorldPreviewFloorMeshActor;
class FT4WorldContextPreviewScene : public IT4WorldContext
{
public:
	explicit FT4WorldContextPreviewScene(FT4WorldController* InWorldController, bool bInThumbnailMode);
	virtual ~FT4WorldContextPreviewScene();

	void Reset() override;

	void ProcessPre(float InDeltaTime) override; // #34 : OnWorldPreActorTick
	void ProcessPost(float InDeltaTime) override; // #34 : OnWorldPostActorTick

	bool IsPreviewScene() const override { return true; }

	FWorldContext* GetOwnerWorldContext() const override; // #87 : WorldContext 소유권이 있다.

	bool WorldTravel(const FSoftObjectPath& InAssetPath, const FVector& InStartLocation) override;

	void SetPlayerController(APlayerController* InPlayerController) override {} // #86, #87 : Only PreviewWorld
	void SetLevelStreamingFrozen(bool bInFrozen) override {} // #86 : Only PreviewWorld
	bool IsLevelStreamingFrozen() const override { return false; } // #86, #104

private:
	void CreateEnvironment(); // #94
	void CreateNavigationSystem();

	void SetThumbnailMode(bool bInShowWindowMode);

private:
	class UWorld* PreviewWorld;
	FT4WorldController* WorldControllerRef;

	bool bThumbnailMode;
	bool bRealWorldUsed;

	// #T4_ADD_TOD_TAG
	TWeakObjectPtr<ADirectionalLight> DirectionalLightActorPtr; // #94
	TWeakObjectPtr<AActor> BPSkySphereActorPtr; // #97
	TWeakObjectPtr<ASkyLight> SkyLightActorPtr; // #94
	TWeakObjectPtr<AAtmosphericFog> AtmosphericFogActorPtr; // #94
	TWeakObjectPtr<AExponentialHeightFog> ExponentialHeightFogActorPtr; // #94
	TWeakObjectPtr<AT4MapZoneVolume> GlobalMapZoneVolumePtr; // #94, #98

	TWeakObjectPtr<ANavMeshBoundsVolume> NavMeshBoundsVolumePtr;
	TWeakObjectPtr<AT4WorldPreviewFloorMeshActor> FloorMeshActorPtr;
};

#endif