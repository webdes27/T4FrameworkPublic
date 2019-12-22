// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameplayTypes.h"
#include "GameFramework/PlayerInput.h" // #104
#include "T4GameplaySettings.generated.h"

/**
  * #43 : Plugins/T4Framework/Config/DefaultT4Framework.ini
 */
class UDataTable;
UCLASS(config = T4Framework, defaultconfig)
class UT4GameplaySettings : public UObject
{
	GENERATED_BODY()

public:
	UT4GameplaySettings();

public:
	// Plugins/T4Framework/Config/DefaultT4Framework.ini

	UPROPERTY(EditAnywhere, config, Category = "Gameplay Mode")
	ET4GameplayGameModeType GameplayMode;

	UPROPERTY(EditAnywhere, config, Category = "Gameplay Data Table")
	TSoftObjectPtr<UDataTable> ClientGameMasterTablePath; // #48

	UPROPERTY(EditAnywhere, config, Category = "Gameplay Data Table")
	TSoftObjectPtr<UDataTable> ServerGameMasterTablePath; // #48

	UPROPERTY(EditAnywhere, config, Category = "Player QuickSpawn (ALT + 1 ~ 9)")
	FName PlayerDataRowName_Key1;
	
	UPROPERTY(EditAnywhere, config, Category = "Player QuickSpawn (ALT + 1 ~ 9)")
	FName PlayerDataRowName_Key2;

	UPROPERTY(EditAnywhere, config, Category = "NPC QuickSpawn (CTRL + 1 ~ 9)")
	FName NPCDataRowName_Key1; // #50
	
	UPROPERTY(EditAnywhere, config, Category = "NPC QuickSpawn (CTRL + 1 ~ 9)")
	FName NPCDataRowName_Key2;

	UPROPERTY(EditAnywhere, config, Category = "NPC QuickSpawn (CTRL + 1 ~ 9)")
	FName NPCDataRowName_Key3; // #104
	
	UPROPERTY(EditAnywhere, config, Category = "NPC QuickSpawn (CTRL + 1 ~ 9)")
	FName NPCDataRowName_Key4; // #104

	UPROPERTY(EditAnywhere, config, Category = "Equip Weapon (Player Spawned + 1 ~ 9)")
	FName WeaponDataRowName_Key1; // #45

	UPROPERTY(EditAnywhere, config, Category = "Equip Weapon (Player Spawned + 1 ~ 9)")
	FName WeaponDataRowName_Key2; // #48

	UPROPERTY(EditAnywhere, config, Category = "Gameplay Network")
	float GameplayDefaultNetworkLatencySec; // #52

	UPROPERTY(config, EditAnywhere, Category = "Bindings")
	TArray<FInputActionKeyMapping> ActionMappings;

	UPROPERTY(config, EditAnywhere, Category = "Bindings")
	TArray<FInputAxisKeyMapping> AxisMappings;

private:
#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent);
#endif
};