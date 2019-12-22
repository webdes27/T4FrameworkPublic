// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/T4EngineTypes.h"
#include "T4EditorActionPlaybackController.generated.h"

/**
  * #68
 */
class UT4ActionPlaybackAsset;
UCLASS()
class UT4EditorActionPlaybackController : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	// UObject
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

public:
	void Set(
		ET4LayerType InLayerType,
		const FString& InAssetName,
		const FString& InFolderName
	);

	bool IsPlayed(); // #104

	bool CanRec();
	bool DoRec();

	bool CanPlay();
	bool DoPlay();

	bool CanPause();
	bool DoPause();

	bool CanStop();
	bool DoStop();

	void TooglePlayRepeat();
	void TooglePlayerPossessed();

public:
	UPROPERTY(EditAnywhere, Transient, Category = ActionPlayback)
	TSoftObjectPtr<UT4ActionPlaybackAsset> SelectActionPlaybackAsset;

	UPROPERTY(EditAnywhere, Transient, Category = ActionPlayback)
	bool bPlayRepeat;

	UPROPERTY(EditAnywhere, Transient, Category = ActionPlayback)
	bool bPlayerPossessed;

private:
	ET4LayerType LayerType;
	FString AssetName;
	FString FolderName;
};
