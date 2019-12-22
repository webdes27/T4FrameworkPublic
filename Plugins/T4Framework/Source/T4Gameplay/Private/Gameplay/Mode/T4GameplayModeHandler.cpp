// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameplayModeHandler.h"
#include "T4GameplayModeTPS.h" // #40
#include "T4GameplayModeShoulderView.h" // #40

#include "T4GameplaySettings.h"
#include "T4GameplayUtils.h"

#include "T4Engine/Public/T4Engine.h"
#include "T4Frame/Public/T4Frame.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "Components/InputComponent.h"

#include "T4GameplayInternal.h"

/**
  * https://www.unrealengine.com/ko/blog/input-action-and-axis-mappings-in-ue4
 */
UT4GameplayModeHandler::UT4GameplayModeHandler(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, LayerType(ET4LayerType::Max)
	, ActiveModeType(ET4GameplayGameModeType::None)
	, ActiveModeStrategy(nullptr) // #40
	, bInputControlLocked(false)
	, bPlayerChangeDisabled(false) // #72
{
}

void UT4GameplayModeHandler::Initialize(ET4LayerType InLayerType)
{
	check(ET4LayerType::Max == LayerType);
	LayerType = InLayerType;
	check(ET4LayerType::Max != LayerType);
	ET4GameplayGameModeType DefaultModeType = GetDefault<UT4GameplaySettings>()->GameplayMode; // #40
	SwitchModeStrategy(DefaultModeType); // #40
	SetupInput();
	SetupMouse();
}

void UT4GameplayModeHandler::Finalize()
{
	if (nullptr != ActiveModeStrategy)
	{
		delete ActiveModeStrategy; // #40
		ActiveModeStrategy = nullptr;
	}
}

void UT4GameplayModeHandler::SetupInput()
{
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);

	{
		// #104 : DefaultInput.ini 에 있던 키를 DefaultT4Framework.ini 로 옮김
		//        DefaultInput 은 프로젝트에서 사용. 플러그인인 T4Framework 는 툴용 ini 에서 값을 취함
		APlayerController* PCActor = Cast<APlayerController>(PlayerController->GetAController());
		check(nullptr != PCActor);
		UPlayerInput* PlayerInput = PCActor->PlayerInput;
		check(nullptr != PlayerInput);
		const TArray<FInputActionKeyMapping>& ActionMappings = GetDefault<UT4GameplaySettings>()->ActionMappings;
		for (const FInputActionKeyMapping& ActionMapping : ActionMappings)
		{
			PlayerInput->AddActionMapping(ActionMapping);
		}
		const TArray<FInputAxisKeyMapping>& AxisMappings = GetDefault<UT4GameplaySettings>()->AxisMappings;
		for (const FInputAxisKeyMapping& AxisMapping : AxisMappings)
		{
			PlayerInput->AddAxisMapping(AxisMapping);
		}
	}
	
	UInputComponent* NewInputComponent = PlayerController->NewInputComponent();
	if (nullptr != NewInputComponent)
	{
		// #40
		NewInputComponent->BindAction("T4Framework_NextPlayMode", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnNextPlayModePressed);

		// #43
		NewInputComponent->BindAction("T4Framework_PlayerQuickSpawn_1", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnPlayerQuickSpawn1Pressed);
		NewInputComponent->BindAction("T4Framework_PlayerQuickSpawn_2", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnPlayerQuickSpawn2Pressed);
		NewInputComponent->BindAction("T4Framework_NPCQuickSpawn_1", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnNPCQuickSpawn1Pressed);
		NewInputComponent->BindAction("T4Framework_NPCQuickSpawn_2", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnNPCQuickSpawn2Pressed);
		NewInputComponent->BindAction("T4Framework_NPCQuickSpawn_3", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnNPCQuickSpawn3Pressed);
		NewInputComponent->BindAction("T4Framework_NPCQuickSpawn_4", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnNPCQuickSpawn4Pressed);
		// ~#43
	
		// #48
		NewInputComponent->BindAction("T4Framework_EquipWeapon_1", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnEquipWeapon1Pressed);
		NewInputComponent->BindAction("T4Framework_EquipWeapon_2", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnEquipWeapon2Pressed);
		// ~#48

		NewInputComponent->BindAxis("T4Framework_MoveForward", this, &UT4GameplayModeHandler::HandleOnMoveForward);
		NewInputComponent->BindAxis("T4Framework_MoveRight", this, &UT4GameplayModeHandler::HandleOnMoveRight);

		NewInputComponent->BindAction("T4Framework_Jump", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnJumpPressed);
		NewInputComponent->BindAction("T4Framework_Roll", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnRollPressed); // #46
		NewInputComponent->BindAction("T4Framework_Teleport", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnTeleportPressed);

		NewInputComponent->BindAction("T4Framework_Attack", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnAttackPressed);

		NewInputComponent->BindAction("T4Framework_LockOn", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnLockOnPressed);
		NewInputComponent->BindAction("T4Framework_LockOn", IE_Released, this, &UT4GameplayModeHandler::HandleOnLockOnReleased);

		NewInputComponent->BindAction("T4Framework_ChangePlayer", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnChangePlayer);
		NewInputComponent->BindAction("T4Framework_ChangeObserver", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnChangeObserver); // #52

		NewInputComponent->BindAction("T4Framework_LeaveObject", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnLeaveObject);

		NewInputComponent->BindAction("T4Framework_CameraRotate", IE_Pressed, this, &UT4GameplayModeHandler::HandleOnCameraRotatePressed);
		NewInputComponent->BindAction("T4Framework_CameraRotate", IE_Released, this, &UT4GameplayModeHandler::HandleOnCameraRotateReleased);

		NewInputComponent->BindAxis("T4Framework_CameraZoom", this, &UT4GameplayModeHandler::HandleOnCameraZoom);
		NewInputComponent->BindAxis("T4Framework_CameraPitch", this, &UT4GameplayModeHandler::HandleOnCameraPitch);
		NewInputComponent->BindAxis("T4Framework_CameraYaw", this, &UT4GameplayModeHandler::HandleOnCameraYaw);
	
		PlayerController->SetInputComponent(NewInputComponent);
	}
	PlayerController->OnSetInputMode(ET4InputMode::GameAndUI);
}

void UT4GameplayModeHandler::SetupMouse()
{
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);

	PlayerController->ShowMouseCursor(true);

	//PlayerController->DefaultMouseCursor;
	//PlayerController->CurrentMouseCursor;

	//PlayerController->bEnableClickEvents = true;
	//PlayerController->bEnableMouseOverEvents = true;
}

void UT4GameplayModeHandler::Process(float InDeltaTime)
{
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->OnProcess(InDeltaTime); // #40
	}
}

void UT4GameplayModeHandler::HandleOnNextPlayModePressed()
{
	if (bInputControlLocked) // #30
	{
		return;
	}
	// #40
	if (ET4GameplayGameModeType::TPS == ActiveModeType)
	{
		SwitchModeStrategy(ET4GameplayGameModeType::ShoulderView);
	}
	else if (ET4GameplayGameModeType::ShoulderView == ActiveModeType)
	{
		SwitchModeStrategy(ET4GameplayGameModeType::TPS);
	}
}

void UT4GameplayModeHandler::HandleOnPlayerQuickSpawn1Pressed()
{
	if (bInputControlLocked) // #30
	{
		return;
	}
	FName SpawnContentNameID = GetDefault<UT4GameplaySettings>()->PlayerDataRowName_Key1; // #43
	if (SpawnContentNameID != NAME_None)
	{
		const FT4GameDataID SpawnDataID = FT4GameDataID(ET4GameDataType::Player, SpawnContentNameID);
		T4GameplayUtil::DoPlayerSpawn(LayerType, SpawnDataID);
	}
}

void UT4GameplayModeHandler::HandleOnPlayerQuickSpawn2Pressed()
{
	if (bInputControlLocked) // #30
	{
		return;
	}
	FName SpawnContentNameID = GetDefault<UT4GameplaySettings>()->PlayerDataRowName_Key2; // #43
	if (SpawnContentNameID != NAME_None)
	{
		const FT4GameDataID SpawnDataID = FT4GameDataID(ET4GameDataType::Player, SpawnContentNameID);
		T4GameplayUtil::DoPlayerSpawn(LayerType, SpawnDataID);
	}
}

void UT4GameplayModeHandler::HandleOnNPCQuickSpawnPressed(const FKey& InPressedKey)// #43
{
	if (bInputControlLocked) // #30
	{
		return;
	}
	FName SpawnContentNameID = NAME_None;
	if (EKeys::One == InPressedKey)
	{
		SpawnContentNameID = GetDefault<UT4GameplaySettings>()->NPCDataRowName_Key1; 
	}
	else if (EKeys::Two == InPressedKey)
	{
		SpawnContentNameID = GetDefault<UT4GameplaySettings>()->NPCDataRowName_Key2;
	}
	else if (EKeys::Three == InPressedKey)
	{
		SpawnContentNameID = GetDefault<UT4GameplaySettings>()->NPCDataRowName_Key3;
	}
	else if (EKeys::Four == InPressedKey)
	{
		SpawnContentNameID = GetDefault<UT4GameplaySettings>()->NPCDataRowName_Key4;
	}
	if (SpawnContentNameID != NAME_None)
	{
		const FT4GameDataID SpawnDataID = FT4GameDataID(ET4GameDataType::NPC, SpawnContentNameID);
		T4GameplayUtil::DoNPCSpawn(LayerType, SpawnDataID);
	}
}

void UT4GameplayModeHandler::HandleOnNPCQuickSpawn1Pressed()// #43
{
	HandleOnNPCQuickSpawnPressed(EKeys::One);
}

void UT4GameplayModeHandler::HandleOnNPCQuickSpawn2Pressed()// #43
{
	HandleOnNPCQuickSpawnPressed(EKeys::Two);
}

void UT4GameplayModeHandler::HandleOnNPCQuickSpawn3Pressed()// #104
{
	HandleOnNPCQuickSpawnPressed(EKeys::Three);
}

void UT4GameplayModeHandler::HandleOnNPCQuickSpawn4Pressed()// #104
{
	HandleOnNPCQuickSpawnPressed(EKeys::Four);
}

void UT4GameplayModeHandler::HandleOnEquipWeapon1Pressed()
{
	if (bInputControlLocked) // #30
	{
		return;
	}
	FName WeaponDataRowName = GetDefault<UT4GameplaySettings>()->WeaponDataRowName_Key1; // #48
	if (WeaponDataRowName != NAME_None)
	{
		IT4PlayerController* PlayerController = GetPlayerController();
		if (nullptr != PlayerController)
		{
			const FT4GameDataID SpawnDataID = FT4GameDataID(ET4GameDataType::Item_Weapon, WeaponDataRowName);
			bool bUnEquip = PlayerController->HasPublicAction(SpawnDataID.ToPrimaryActionKey());
			T4GameplayUtil::DoEquipWeapon(LayerType, SpawnDataID, bUnEquip, true);
		}
	}
}

void UT4GameplayModeHandler::HandleOnEquipWeapon2Pressed()
{
	if (bInputControlLocked) // #30
	{
		return;
	}
	FName WeaponDataRowName = GetDefault<UT4GameplaySettings>()->WeaponDataRowName_Key2; // #48
	if (WeaponDataRowName != NAME_None)
	{
		IT4PlayerController* PlayerController = GetPlayerController();
		if (nullptr != PlayerController)
		{
			const FT4GameDataID SpawnDataID = FT4GameDataID(ET4GameDataType::Item_Weapon, WeaponDataRowName);
			bool bUnEquip = PlayerController->HasPublicAction(SpawnDataID.ToPrimaryActionKey());
			T4GameplayUtil::DoEquipWeapon(LayerType, SpawnDataID, bUnEquip, true);
		}
	}
}

void UT4GameplayModeHandler::HandleOnMoveForward(float InAxisValue) 
{
	if (bInputControlLocked) // #30
	{
		return;
	}
	if (0.0f == InAxisValue)
	{
		return;
	}
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->DoMoveForward(InAxisValue); // #40
	}
}

void UT4GameplayModeHandler::HandleOnMoveRight(float InAxisValue) 
{
	if (bInputControlLocked) // #30
	{
		return;
	}
	if (0.0f == InAxisValue)
	{
		return;
	}
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->DoMoveRight(InAxisValue); // #40
	}
}

void UT4GameplayModeHandler::HandleOnJumpPressed()
{
	if (bInputControlLocked) // #30
	{
		return;
	}
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->DoJumpPressed(); // #40
	}
}

void UT4GameplayModeHandler::HandleOnRollPressed()
{
	if (bInputControlLocked) // #30
	{
		return;
	}
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->DoRollPressed(); // #46
	}
}

void UT4GameplayModeHandler::HandleOnTeleportPressed()
{
	if (bInputControlLocked) // #30
	{
		return;
	}
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->DoTeleportPressed(); // #40
	}
}

void UT4GameplayModeHandler::HandleOnAttackPressed()
{
	if (bInputControlLocked) // #30
	{
		return;
	}
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->DoAttackPressed(); // #40
	}
}

void UT4GameplayModeHandler::HandleOnLockOnPressed()
{
	if (bInputControlLocked) // #30
	{
		return;
	}
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->DoLockOnPressed(); // #40
	}
}

void UT4GameplayModeHandler::HandleOnLockOnReleased()
{
	if (bInputControlLocked) // #30
	{
		return;
	}
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->DoLockOnReleased(); // #40
	}
}

void UT4GameplayModeHandler::HandleOnChangePlayer()
{
	if (bInputControlLocked || bPlayerChangeDisabled) // #30, #72
	{
		return;
	}
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->DoChangePlayer(); // #40
	}
}

void UT4GameplayModeHandler::HandleOnChangeObserver() // #52
{
	if (bInputControlLocked || bPlayerChangeDisabled) // #30, #72
	{
		return;
	}
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->DoChangeObserver(); // #40
	}
}

void UT4GameplayModeHandler::HandleOnLeaveObject()
{
	if (bInputControlLocked) // #30
	{
		return;
	}
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->DoLeaveObject(); // #40
	}
}

void UT4GameplayModeHandler::HandleOnCameraRotatePressed() 
{
	if (bInputControlLocked) // #30
	{
		return;
	}
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->DoCameraRotatePressed(); // #40
	}
}

void UT4GameplayModeHandler::HandleOnCameraRotateReleased() 
{
	if (bInputControlLocked) // #30
	{
		return;
	}
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->DoCameraRotateReleased(); // #40
	}
}

void UT4GameplayModeHandler::HandleOnCameraZoom(float InAxisValue)
{
	if (0.0f == InAxisValue)
	{
		return;
	}
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->DoCameraZoom(InAxisValue); // #40
	}
}

void UT4GameplayModeHandler::HandleOnCameraPitch(float InAxisValue)
{
	if (0.0f == InAxisValue)
	{
		return;
	}
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->DoCameraPitch(InAxisValue); // #40
	}
}

void UT4GameplayModeHandler::HandleOnCameraYaw(float InAxisValue)
{
	if (0.0f == InAxisValue)
	{
		return;
	}
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->DoCameraYaw(InAxisValue); // #40
	}
}

void UT4GameplayModeHandler::SwitchModeStrategy(ET4GameplayGameModeType InChangeModeType)
{
	// #40
	if (nullptr != ActiveModeStrategy)
	{
		ActiveModeStrategy->OnLeave();
		delete ActiveModeStrategy;
		ActiveModeStrategy = nullptr;
	}
	switch (InChangeModeType)
	{
		case ET4GameplayGameModeType::TPS:
			ActiveModeStrategy = new FT4GameplayModeTPS(LayerType);
			break;

		case ET4GameplayGameModeType::ShoulderView:
			ActiveModeStrategy = new FT4GameplayModeShoulderView(LayerType);
			break;

		default:
			{
				UE_LOG(
					LogT4Gameplay,
					Error,
					TEXT("[SL:%u] SwitchModeStrategy '%u' failed. no implementation."),
					uint32(LayerType),
					uint32(InChangeModeType)
				);
			}
			break;
	}
	check(nullptr != ActiveModeStrategy);
	ActiveModeStrategy->OnEnter();
	ActiveModeType = InChangeModeType;
}

IT4PlayerController* UT4GameplayModeHandler::GetPlayerController() const
{
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	IT4PlayerController* PlayerController = GameFrame->GetPlayerController();
	check(nullptr != PlayerController);
	return PlayerController;
}
