// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "T4Frame/Public/T4Frame.h"

/**
  * #60
 */
#if WITH_EDITOR

class FT4GameplayEditorGameData : public IT4EditorGameData
{
public:
	explicit FT4GameplayEditorGameData();
	~FT4GameplayEditorGameData();

	// IT4EditorGameData
	void GetNameIDList(
		ET4EditorGameDataType InEditorGameDataType, 
		TArray<FName>& OutDataNameIDs
	) override;

	class UT4EntityAsset* GetEntityAsset(
		ET4EditorGameDataType InEditorGameDataType,
		const FName& InDataNameID
	) override;

	bool GetSkillDataInfo(const FName& InSkillDataNameID, FT4EditorSkillDataInfo& OutSkillData) override;
	bool GetEffectDataInfo(const FName& InEffectDataNameID, FT4EditorEffectDataInfo& OutEffectData) override;

	bool DoNPCSpawn(const FName& InNPCDataNameID, const FVector& InSpawnLocation) override; // #60 : to player
	bool DoDespawnAll(bool bClearPlayerObject) override; // #68

	bool DoEquipWeaponItem(const FName& InWeaponDataNameID, bool bInUnEquip) override; // #60 : to player
	bool DoExchangeCostumeItem(const FName& InCostumeDataNameID) override; // #60 : to player

public:
	bool Initialize(ET4LayerType InLayerType);
	void Finalize();

	void Reset();

private:
	void GetNameIDList(ET4GameDataType InGameDataType, TArray<FName>& OutDataNameIDs);

private:
	ET4LayerType LayerType;
};

#endif