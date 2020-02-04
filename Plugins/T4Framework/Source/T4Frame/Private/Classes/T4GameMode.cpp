// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Classes/T4GameMode.h"
#include "Classes/Controller/Player/T4PlayerController.h" // #42
#include "Classes/Controller/Player/T4PlayerDefaultPawn.h" // #42

#include "Public/T4Frame.h"

#include "Kismet/GameplayStatics.h"

#include "T4FrameInternal.h"

/**
  * http://api.unrealengine.com/KOR/Gameplay/Framework/GameMode/
 */
AT4GameMode::AT4GameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerControllerClass = AT4PlayerController::StaticClass(); // #42
	DefaultPawnClass = AT4PlayerDefaultPawn::StaticClass(); // #42
}

void AT4GameMode::StartPlay()
{
	Super::StartPlay();

#if !WITH_EDITOR
	ET4LayerType LayerType = T4EngineLayer::Get(GetWorld());
	check(ET4LayerType::Max != LayerType);

	// only game used. edit call to UT4GameInstance::StartPlayInEditorGameInstance.
	IT4GameFrame* Framework = T4FrameGet(LayerType);
	check(nullptr != Framework);
	Framework->OnStartPlay();
#endif
}
