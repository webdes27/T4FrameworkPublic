// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameplayEditorGameData.h"
#include "T4GameplayUtils.h"

#include "GameDB/T4GameDB.h"

#include "T4Asset/Classes/Entity/T4EntityAsset.h"
#include "T4Asset/Classes/Entity/T4CostumeEntityAsset.h"
#include "T4Asset/Classes/Entity/T4WeaponEntityAsset.h"
#include "T4Asset/Classes/Entity/T4CharacterEntityAsset.h"

#include "T4GameplayInternal.h"

/**
  * #60
 */
#if WITH_EDITOR

FT4GameplayEditorGameData::FT4GameplayEditorGameData()
	: LayerType(ET4LayerType::Max)
{
}

FT4GameplayEditorGameData::~FT4GameplayEditorGameData()
{
}

void FT4GameplayEditorGameData::GetNameIDList(
	ET4GameDataType InGameDataType,
	TArray<FName>& OutDataNameIDs
)
{
	FT4GameDB& GameDB = GetGameDB();
	const FT4GameDataInfo& GameDatas = GameDB.GetGameDataInfo(InGameDataType);
	if (0 < GameDatas.GameDatas.Num())
	{
		uint32 Idx = 0;
		OutDataNameIDs.SetNum(GameDatas.GameDatas.Num());
		for (FT4GameDataBase* GameData : GameDatas.GameDatas)
		{
			OutDataNameIDs[Idx++] = GameData->RawName;
		}
	}
}

void FT4GameplayEditorGameData::GetNameIDList(
	ET4EditorGameDataType InEditorGameDataType,
	TArray<FName>& OutDataNameIDs
)
{
	FT4GameDB& GameDB = GetGameDB();
	switch (InEditorGameDataType)
	{
		case ET4EditorGameDataType::EdData_PC:
			GetNameIDList(ET4GameDataType::Player, OutDataNameIDs);
			break;
		case ET4EditorGameDataType::EdData_NPC:
			GetNameIDList(ET4GameDataType::NPC, OutDataNameIDs);
			break;
		case ET4EditorGameDataType::EdData_Weapon:
			GetNameIDList(ET4GameDataType::Item_Weapon, OutDataNameIDs);
			break;
		case ET4EditorGameDataType::EdData_Costume:
			GetNameIDList(ET4GameDataType::Item_Costume, OutDataNameIDs);
			break;
		case ET4EditorGameDataType::EdData_Skill:
			GetNameIDList(ET4GameDataType::Skill, OutDataNameIDs);
			break;
		case ET4EditorGameDataType::EdData_Effect:
			GetNameIDList(ET4GameDataType::Effect, OutDataNameIDs);
			break;
	}
}

UT4EntityAsset* FT4GameplayEditorGameData::GetEntityAsset(
	ET4EditorGameDataType InEditorGameDataType,
	const FName& InDataNameID
)
{
	FT4GameDB& GameDB = GetGameDB();
	switch (InEditorGameDataType)
	{
		case ET4EditorGameDataType::EdData_NPC:
			{
				FT4GameDataID SearchDataID(ET4GameDataType::NPC, InDataNameID);
				const FT4GameNPCData* FoundData = GameDB.GetGameData<FT4GameNPCData>(SearchDataID); // #48
				if (nullptr == FoundData)
				{
					return nullptr;
				}
				return Cast<UT4EntityAsset>(FoundData->RawData.EntityAsset.Get());
			}
			break;
		case ET4EditorGameDataType::EdData_Weapon:
			{
				FT4GameDataID SearchDataID(ET4GameDataType::Item_Weapon, InDataNameID);
				const FT4GameItemWeaponData* FoundData = GameDB.GetGameData<FT4GameItemWeaponData>(SearchDataID); // #48
				if (nullptr == FoundData)
				{
					return nullptr;
				}
				return Cast<UT4EntityAsset>(FoundData->RawData.EntityAsset.Get());
			}
			break;
		case ET4EditorGameDataType::EdData_Costume:
			{
				FT4GameDataID SearchDataID(ET4GameDataType::Item_Costume, InDataNameID);
				const FT4GameItemCostumeData* FoundData = GameDB.GetGameData<FT4GameItemCostumeData>(SearchDataID); // #48
				if (nullptr == FoundData)
				{
					return nullptr;
				}
				return Cast<UT4EntityAsset>(FoundData->RawData.EntityAsset.Get());
			}
			break;
	}
	return nullptr;
}

bool FT4GameplayEditorGameData::GetSkillDataInfo(
	const FName& InSkillDataNameID,
	FT4EditorSkillDataInfo& OutSkillData
) // #60
{
	FT4GameDB& GameDB = GetGameDB();
	FT4GameSkillDataID SkillDataID(InSkillDataNameID);
	const FT4GameSkillData* SkillData = GameDB.GetGameData<FT4GameSkillData>(SkillDataID); // #48
	if (nullptr == SkillData)
	{
		return false;
	}
	// #T4_ADD_SKILL_CONTENT_TAG 
	OutSkillData.Name = SkillData->RawData.Name;
	OutSkillData.AttackType = SkillData->RawData.AttackType; // #63
	OutSkillData.HitDelayTimeSec = SkillData->RawData.HitDelayTimeSec;
	OutSkillData.DurationSec = SkillData->RawData.DurationSec;
	OutSkillData.ProjectileSpeed = SkillData->RawData.ProjectileSpeed; // #63
	OutSkillData.bMoveable = SkillData->RawData.bMoveable;
	OutSkillData.ResultEffectDataID = SkillData->RawData.ResultEffectDataID.RowName; // #68
	OutSkillData.ContiAsset = SkillData->RawData.ContiAsset;
	return true;
}

bool FT4GameplayEditorGameData::GetEffectDataInfo(
	const FName& InEffectDataNameID,
	FT4EditorEffectDataInfo& OutEffectData
) // #60
{
	FT4GameDB& GameDB = GetGameDB();
	FT4GameEffectDataID EffectDataID(InEffectDataNameID);
	const FT4GameEffectData* EffectData = GameDB.GetGameData<FT4GameEffectData>(EffectDataID); // #48
	if (nullptr == EffectData)
	{
		return false;
	}
	// #T4_ADD_EFFECT_CONTENT_TAG
	OutEffectData.Name = EffectData->RawData.Name;
	OutEffectData.EffectType = EffectData->RawData.EffectType; // #68
	OutEffectData.HitDelayTimeSec = EffectData->RawData.HitDelayTimeSec; // #68
	OutEffectData.AreaRange = EffectData->RawData.AreaRange; // #68
	OutEffectData.DamageEffectDataID = EffectData->RawData.DamageEffectDataID.RowName; // #68
	OutEffectData.ContiAsset = EffectData->RawData.ContiAsset;
	return true;
}

bool FT4GameplayEditorGameData::DoNPCSpawn(
	const FName& InNPCDataNameID,
	const FVector& InSpawnLocation
) // #60 : to player
{
	FT4GameDataID NPCGameDataID(ET4GameDataType::NPC, InNPCDataNameID);
	bool bResult = T4GameplayUtil::DoNPCSpawn(LayerType, NPCGameDataID, InSpawnLocation);
	return bResult;
}

bool FT4GameplayEditorGameData::DoDespawnAll(bool bClearPlayerObject) // #68
{
	bool bResult = T4GameplayUtil::DoDespawnAll(LayerType, bClearPlayerObject);
	return bResult;
}

bool FT4GameplayEditorGameData::DoEquipWeaponItem(
	const FName& InWeaponDataNameID,
	bool bInUnEquip
) // #60 : to player
{
	FT4GameDataID WeaponGameDataID(ET4GameDataType::Item_Weapon, InWeaponDataNameID);
	bool bResult = T4GameplayUtil::DoEquipWeapon(LayerType, WeaponGameDataID, bInUnEquip, true);
	return bResult;
}

bool FT4GameplayEditorGameData::DoExchangeCostumeItem(
	const FName& InCostumeDataNameID
) // #60 : to player
{

	return true;
}

bool FT4GameplayEditorGameData::Initialize(ET4LayerType InLayerType)
{
	check(ET4LayerType::Max == LayerType);
	LayerType = InLayerType;
	check(ET4LayerType::Max != LayerType);
	return true;
}

void FT4GameplayEditorGameData::Finalize()
{

}

void FT4GameplayEditorGameData::Reset()
{

}

#endif