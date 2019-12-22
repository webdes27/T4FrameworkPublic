// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameplayModeBase.h" // #40
#include "T4Engine/Public/T4EngineTypes.h"
#include "InputCoreTypes.h"
#include "T4GameplayModeHandler.generated.h"

/**
  * https://www.unrealengine.com/ko/blog/input-action-and-axis-mappings-in-ue4
 */
class IT4PlayerController;
class IT4PacketHandlerCS;
class IT4GameObject;
UCLASS()
class UT4GameplayModeHandler : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	void Initialize(ET4LayerType InLayerType);
	void Finalize(); // #40

	void Process(float InDeltaTime);

	ET4LayerType GetLayerType() const { return LayerType; }

	void HandleOnNextPlayModePressed(); // #40

	// #43
	void HandleOnPlayerQuickSpawn1Pressed(); 
	void HandleOnPlayerQuickSpawn2Pressed();

	void HandleOnNPCQuickSpawnPressed(const FKey& InPressedKey);
	void HandleOnNPCQuickSpawn1Pressed();
	void HandleOnNPCQuickSpawn2Pressed();
	void HandleOnNPCQuickSpawn3Pressed();
	void HandleOnNPCQuickSpawn4Pressed();
	// ~#43

	// #48
	void HandleOnEquipWeapon1Pressed(); // #22
	void HandleOnEquipWeapon2Pressed(); // #22
	// ~#48

	void HandleOnMoveForward(float InAxisValue);
	void HandleOnMoveRight(float InAxisValue);

	void HandleOnJumpPressed();
	void HandleOnRollPressed(); // #46
	void HandleOnTeleportPressed();

	void HandleOnAttackPressed();

	void HandleOnLockOnPressed();
	void HandleOnLockOnReleased();

	void HandleOnChangePlayer();
	void HandleOnChangeObserver(); // #52

	void HandleOnLeaveObject();

	void HandleOnCameraRotatePressed();
	void HandleOnCameraRotateReleased();

	void HandleOnCameraZoom(float InAxisValue);
	void HandleOnCameraPitch(float InAxisValue);
	void HandleOnCameraYaw(float InAxisValue);

	void SetInputControlLock(bool bLock) { bInputControlLocked = bLock; } // #30
	void SetPlayerChangeDisable(bool bDisable) { bPlayerChangeDisabled = bDisable; }  // #72

private:
	void SetupInput();
	void SetupMouse();

	void SwitchModeStrategy(ET4GameplayGameModeType InChangeModeType); // #40

	IT4PlayerController* GetPlayerController() const;

private:
	ET4LayerType LayerType;

	ET4GameplayGameModeType ActiveModeType; // #40
	FT4GameplayModeBase* ActiveModeStrategy; // #40
	
	bool bInputControlLocked; // #30
	bool bPlayerChangeDisabled; // #72
};
