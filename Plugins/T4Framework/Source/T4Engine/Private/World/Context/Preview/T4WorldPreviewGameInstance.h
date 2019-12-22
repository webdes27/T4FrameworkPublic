// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "T4WorldPreviewGameInstance.generated.h"

/**
  * #79
 */
struct FWorldContext;
class AGameModeBase;
UCLASS()
class UT4WorldPreviewGameInstance : public UGameInstance
{
	GENERATED_UCLASS_BODY()

public:
	AGameModeBase* CreateGameModeForURL(FURL InURL) override;

	TSubclassOf<AGameModeBase> OverrideGameModeClass(
		TSubclassOf<AGameModeBase> GameModeClass, 
		const FString& MapName, 
		const FString& Options, 
		const FString& Portal
	) const override;

public:
	void SetWorldContext(FWorldContext* InWorldContext);

	void SetLocalPlayer(ULocalPlayer* InLocalPlayer);// #386
	void SetLevelStreaming(bool bEnable); // #86

private:
	// #86 : Rehearsal GameInstance 는 기본적으로 LocalPlayer 를 제공하지 않으나,
	//       SimulationMode 일 경우에만 노출함. 즉, 노출이 되어야 UpdateStreamingState 가 업데이트 되기 때문
	bool bLevelStreamingEnabled; // #86
	TArray<ULocalPlayer*> BackupLocalPlayer; // #86
};
