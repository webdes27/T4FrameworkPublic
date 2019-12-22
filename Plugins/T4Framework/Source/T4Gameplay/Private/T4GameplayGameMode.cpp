// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/T4GameplayGameMode.h" // #61
#include "Classes/Controller/Player/T4GameplayPlayerController.h" // #42
#include "Classes/Controller/Player/T4GameplayDefaultPawn.h" // #42

#include "T4Frame/Public/T4Frame.h"

#include "Kismet/GameplayStatics.h"

#include "T4GameplayInternal.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Framework/GameMode/
 */
AT4GameplayGameMode::AT4GameplayGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerControllerClass = AT4GameplayPlayerController::StaticClass(); // #42
	DefaultPawnClass = AT4GameplayDefaultPawn::StaticClass(); // #42
}
