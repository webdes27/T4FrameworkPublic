// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameplayLauncher.h"

#include "Classes/Controller/Player/T4GameplayPlayerController.h"
#include "Gameplay/T4GameplayInstance.h"

#include "Public/Protocol/T4PacketSC_Move.h"

#include "T4Frame/Public/T4Frame.h"

#if WITH_EDITOR
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#endif

#include "T4GameplayInternal.h"

/**
  *
 */
bool FT4GameplayLauncher::Initialize()
{
	// #42
	FT4FrameDelegates::OnRegisterGameplayLayerInstancce.BindRaw(
		this, 
		&FT4GameplayLauncher::HandleOnRegisterGameplayLayerInstancce
	);

#if WITH_EDITOR
	FT4FrameDelegates::OnCreateEditorPlayerController.BindRaw(
		this,
		&FT4GameplayLauncher::HandleOnCreateEditorPlayerController
	);
#endif
	// ~#42

	return true;
}

void FT4GameplayLauncher::Finalize()
{
}

void FT4GameplayLauncher::HandleOnRegisterGameplayLayerInstancce(IT4GameFrame* InGameFramework)
{
	// #42
	FT4GameplayInstance* NewGameplayInstance = new FT4GameplayInstance;
	check(nullptr != NewGameplayInstance);
	InGameFramework->RegisterGameplayInstance(NewGameplayInstance); // #42 : GameFramework 소멸 시 인스턴스가 소멸된다!!
}

#if WITH_EDITOR
void FT4GameplayLauncher::HandleOnCreateEditorPlayerController(IT4GameFrame* InGameFramework)
{
	// #42
	check(nullptr != InGameFramework);
	UWorld* World = InGameFramework->GetWorld();
	check(nullptr != World);

	// #30 : Editor/Preview 의 경우 GameMode(PIE)가 없음으로 PC Spawn 을 별도로 처리해준다.
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.ObjectFlags |= RF_Transient;	// We never want to save player controllers into a map
	SpawnInfo.bDeferConstruction = true;
	AT4PlayerController* NewEditorPC = World->SpawnActor<AT4GameplayPlayerController>(
		AT4GameplayPlayerController::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnInfo
	);
	if (nullptr == NewEditorPC)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("FT4GameplayLauncher : Failed to spawn AT4GameplayPlayerController. LayerType '%u'"),
			uint32(InGameFramework->GetLayerType())
		);
		return;
	}

	InGameFramework->SetEditorPlayerController(NewEditorPC);

	const FTransform SpawnTransform(FRotator::ZeroRotator, FVector::ZeroVector);
	UGameplayStatics::FinishSpawningActor(NewEditorPC, SpawnTransform);
}
#endif

static FT4GameplayLauncher GT4GameplayLauncher;
FT4GameplayLauncher& GetGameplayLauncher()
{
	return GT4GameplayLauncher;
}