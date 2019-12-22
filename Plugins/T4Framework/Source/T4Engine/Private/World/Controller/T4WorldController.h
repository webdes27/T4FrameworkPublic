// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4WorldTimeControl.h" // #93
#include "T4WorldEnvironmentControl.h" // #92
#include "Public/T4Engine.h"
#include "Public/Action/T4ActionCodeWorld.h"

/**
  * #87
 */
struct FWorldContext;
class UT4MapEntityAsset;
class APlayerController;
class IT4WorldContext
{
public:
	virtual ~IT4WorldContext() {}

	virtual void Reset() = 0;

	virtual void ProcessPre(float InDeltaTime) = 0;
	virtual void ProcessPost(float InDeltaTime) = 0;

	virtual bool IsPreviewScene() const = 0;

	virtual FWorldContext* GetOwnerWorldContext() const = 0; // #87 : Preview 만 유효!!

	virtual bool WorldTravel(const FSoftObjectPath& InAssetPath, const FVector& InStartLocation) = 0;
	
	virtual void SetPlayerController(APlayerController* InPlayerController) = 0; // #86, #87 : Only PreviewWorld
	virtual void SetLevelStreamingFrozen(bool bInFrozen) = 0; // #86 : Only PreviewWorld
	virtual bool IsLevelStreamingFrozen() const = 0; // #86, #104
};

class UWorld;
class FT4GameWorld;
class AT4MapZoneVolume; // #94
class AT4MovableZoneObject; // #94
class FT4WorldController : public IT4WorldController
{
public:
	explicit FT4WorldController(FT4GameWorld* InGameWorld);
	virtual ~FT4WorldController();

	ET4GameWorldType GetGameWorldType() const override { return GameWorldType; }

	bool CheckLevelLoadComplated() override; // #87

	UWorld* GetWorld() const override;

	// #93
	FName GetGameTimeTagName() const override { return WorldTimeControl.GetTimeTagName(); }
	FString GetGameTimeString() override { return WorldTimeControl.GetDisplayString(); }

	void SetGameTimeHour(float InHour) override { WorldTimeControl.SetTimeHour(InHour); }
	float GetGameTimeHour() const override { return WorldTimeControl.GetTimeHour(); }

	void SetGameTimeScale(float InScale) override { WorldTimeControl.SetTimeScale(InScale); }
	float GetGameTimeScale() const override { return WorldTimeControl.GetTimeScale(); }
	// ~#93

#if WITH_EDITOR
	bool IsPreviewScene() const override; // #87
#endif

public:
	void Initialize(const FT4WorldConstructionValues& InWorldConstructionValues); // #87
	void Finalize();

	void Reset();

	void ProcessPre(float InDeltaTime); // #34 : OnWorldPreActorTick
	void ProcessPost(float InDeltaTime); // #34 : OnWorldPostActorTick

	FWorldContext* GetWorldContext() const { return WorldContextRef; }
	
	IT4GameWorld* GetGameWorld() const; // #93
	FT4GameWorld* GetGameWorldImpl() const { return GameWorldRef; } // #93

	FT4WorldTimeControl* GetWorldTimeControl() { return &WorldTimeControl; } // #93

	bool ProcessWorldTravelAction(const FT4WorldTravelAction& InAction);

	void SetPlayerController(APlayerController* InPlayerController); // #86

	bool IsDisabledLevelStreaming() const; // #86, #104
	void SetDisableLevelStreaming(bool bInDisable); // #86 : World 의 UpdateStreamingState 를 제어하기 위한 옵션 처리
	void SetDisableEnvironmentUpdating(bool bInDisable); // #92 : Map Environemnt Update 제어 옵션 처리
	void SetDisableTimelapse(bool bInDisable); // #93 : 시간 경과 옵션 처리
	bool GetTimelapseDisabled() const; // #94

#if WITH_EDITOR
	FName GetWorldContextName() const { return SaveWorldContextName; } // #87 : ContextHandle 로 LayerType 을 만들기 때문에 보존
#endif

private:
	void CreateGameWorldContext(const FT4WorldConstructionValues& InWorldConstructionValues); // #87

	FSoftObjectPath SelectWorldTravelObjectPath(const FSoftObjectPath& InEntityOrLevelObjectPath); // #87

private:
	ET4GameWorldType GameWorldType; // #87
	FT4GameWorld* GameWorldRef;
	FWorldContext* WorldContextRef;
	IT4WorldContext* WorldContextImplement;
	FT4WorldTimeControl WorldTimeControl; // #93
	FT4WorldEnvironmentControl WorldEnvironmentControl; // #92

#if WITH_EDITOR
	FName SaveWorldContextName; // #87 : ContextHandle 로 LayerType 을 만들기 때문에 보존
#endif
};
