// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4FrameworkExampleGameMode.h"

#include "T4Frame/Public/T4Frame.h"

#include "T4Frame/Classes/Controller/Player/T4PlayerController.h" // #42
#include "T4Frame/Classes/Controller/Player/T4PlayerDefaultPawn.h" // #42

#include "Kismet/GameplayStatics.h"

#include "T4FrameworkExample.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Framework/GameMode/
 */
AT4FrameworkExampleGameMode::AT4FrameworkExampleGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerControllerClass = AT4PlayerController::StaticClass(); // #42
	DefaultPawnClass = AT4PlayerDefaultPawn::StaticClass(); // #42
}
