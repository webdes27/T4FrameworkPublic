// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "T4GameModeBase.generated.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Framework/GameMode/
 */

UCLASS()
class T4FRAMEWORK_API AT4GameModeBase : public AGameModeBase
{
	GENERATED_UCLASS_BODY()
	
public:
	void StartPlay() override;
};
