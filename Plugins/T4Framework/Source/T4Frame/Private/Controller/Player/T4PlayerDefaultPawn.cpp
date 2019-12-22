// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Classes/Controller/Player/T4PlayerDefaultPawn.h"

#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerInput.h"

/**
  *
 */
AT4PlayerDefaultPawn::AT4PlayerDefaultPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// WARN : T4 에서는 스폰 메시를 별도로 스폰함으로 DefaultPawn 은 무력화 해야한다. (네트웍도 마찬가지...)
	
	bReplicates = false; // #15
	bNetLoadOnClient = false; // #15

	GetCollisionComponent()->SetVisibility(false);
	GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetMeshComponent()->SetVisibility(false);
	GetMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void AT4PlayerDefaultPawn::Tick(float InDeltaTime)
{
	Super::Tick(InDeltaTime);
}

// #49
void InitializeT4DefaultPawnInputBindings()
{
	static bool bT4BindingsAdded = false;
	if (!bT4BindingsAdded)
	{
		bT4BindingsAdded = true;

		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveForward", EKeys::W, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveForward", EKeys::S, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveForward", EKeys::Up, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveForward", EKeys::Down, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveForward", EKeys::Gamepad_LeftY, 1.f));

		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveRight", EKeys::A, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveRight", EKeys::D, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveRight", EKeys::Gamepad_LeftX, 1.f));

		// HACK: Android controller bindings in ini files seem to not work
		//  Direct overrides here some to work
#if !PLATFORM_ANDROID
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveUp", EKeys::Gamepad_LeftThumbstick, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveUp", EKeys::Gamepad_RightThumbstick, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveUp", EKeys::Gamepad_FaceButton_Bottom, 1.f));
		// #49 : Player 를 스폰하지 않은 경우 Control 키가 반응하지 않도록 처리 (일부 키는 T4Framework 에서 예약해 사용중)
		//UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveUp", EKeys::LeftControl, -1.f));
		//UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveUp", EKeys::SpaceBar, 1.f));
		//UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveUp", EKeys::C, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveUp", EKeys::E, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveUp", EKeys::Q, -1.f));
#else
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveUp", EKeys::Gamepad_LeftTriggerAxis, -0.5f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_MoveUp", EKeys::Gamepad_RightTriggerAxis, 0.5f));
#endif

		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_TurnRate", EKeys::Gamepad_RightX, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_TurnRate", EKeys::Left, -1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_TurnRate", EKeys::Right, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_Turn", EKeys::MouseX, 1.f));

		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_LookUpRate", EKeys::Gamepad_RightY, 1.f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("T4DefaultPawn_LookUp", EKeys::MouseY, -1.f));
	}
}

void AT4PlayerDefaultPawn::SetupPlayerInputComponent(UInputComponent* InInputComponent)
{
	// #49 : Player 를 스폰하지 않은 경우 Control 키가 반응하지 않도록 처리 (일부 키는 T4Framework 에서 예약해 사용중)
#if 1
	check(InInputComponent);

	if (bAddDefaultMovementBindings)
	{
		InitializeT4DefaultPawnInputBindings();

		InInputComponent->BindAxis("T4DefaultPawn_MoveForward", this, &ADefaultPawn::MoveForward);
		InInputComponent->BindAxis("T4DefaultPawn_MoveRight", this, &ADefaultPawn::MoveRight);
		InInputComponent->BindAxis("T4DefaultPawn_MoveUp", this, &ADefaultPawn::MoveUp_World);
		InInputComponent->BindAxis("T4DefaultPawn_Turn", this, &ADefaultPawn::AddControllerYawInput);
		InInputComponent->BindAxis("T4DefaultPawn_TurnRate", this, &ADefaultPawn::TurnAtRate);
		InInputComponent->BindAxis("T4DefaultPawn_LookUp", this, &ADefaultPawn::AddControllerPitchInput);
		InInputComponent->BindAxis("T4DefaultPawn_LookUpRate", this, &ADefaultPawn::LookUpAtRate);
	}
#else
	Super::SetupPlayerInputComponent(InInputComponent);
#endif
}
