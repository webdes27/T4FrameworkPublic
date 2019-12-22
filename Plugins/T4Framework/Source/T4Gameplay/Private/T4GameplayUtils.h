// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/GameTable/T4GameTableDataTypes.h"

/**
  *
 */
class FT4ServerEventManager;
namespace T4GameplayUtil
{
	bool DoPlayerSpawn(
		ET4LayerType InLayerType, 
		const FT4GameDataID& InPlayerDataID
	); // #43
	bool DoNPCSpawn(
		ET4LayerType InLayerType, 
		const FT4GameDataID& InNPCDataID
	); // #43
	bool DoNPCSpawn(
		ET4LayerType InLayerType,
		const FT4GameDataID& InNPCDataID,
		const FVector& InSpawnLocation
	); // #60
	bool DoDespawnAll(
		ET4LayerType InLayerType, 
		bool bClearPlayerObject
	); // #68
	bool DoEquipWeapon(
		ET4LayerType InLayerType,
		const FT4GameDataID& InWeaponDataID,
		bool bUnEquip,
		bool bInMainWeapon
	); // #48

	FT4ServerEventManager* GetServerEventManager(const ET4LayerType InLayerType); // #63
}
