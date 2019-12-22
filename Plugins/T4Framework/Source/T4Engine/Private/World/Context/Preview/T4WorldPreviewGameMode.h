// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "T4WorldPreviewGameMode.generated.h"

/**
  * #79
 */

UCLASS()
class AT4WorldPreviewGameMode : public AGameModeBase
{
	GENERATED_UCLASS_BODY()

public:
	void StartPlay() override;
};
