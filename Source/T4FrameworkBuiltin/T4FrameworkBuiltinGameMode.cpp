// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4FrameworkBuiltinGameMode.h"

#include "T4Framework/Public/T4Framework.h"

#include "T4Framework/Classes/Controller/Player/T4PlayerControllerBase.h" // #42
#include "T4Framework/Classes/Controller/Player/T4PlayerDefaultPawnBase.h" // #42

#include "Kismet/GameplayStatics.h"

#include "T4FrameworkBuiltin.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Framework/GameMode/
 */
AT4FrameworkBuiltinGameMode::AT4FrameworkBuiltinGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerControllerClass = AT4PlayerControllerBase::StaticClass(); // #42
	DefaultPawnClass = AT4PlayerDefaultPawnBase::StaticClass(); // #42
}
