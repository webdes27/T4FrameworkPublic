// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4GameBuiltin_GameDataTypes.h" // #48
#include "Classes/DataTable/T4GameBuiltin_Table_Master.h" // #118
#include "Classes/DataTable/T4GameBuiltin_Table_World.h" // #27
#include "Classes/DataTable/T4GameBuiltin_Table_Player.h" // #27
#include "Classes/DataTable/T4GameBuiltin_Table_NPC.h" // #31
#include "Classes/DataTable/T4GameBuiltin_Table_Weapon.h" // #27, #48
#include "Classes/DataTable/T4GameBuiltin_Table_Costume.h" // #27, #48
#include "Classes/DataTable/T4GameBuiltin_Table_Skill.h" // #25
#include "Classes/DataTable/T4GameBuiltin_Table_SkillSet.h" // #50
#include "Classes/DataTable/T4GameBuiltin_Table_Effect.h" // #25
#include "Classes/DataTable/T4GameBuiltin_Table_Stat.h" // #114
#include "Classes/DataTable/T4GameBuiltin_Table_Experience.h" // #114

#if WITH_EDITOR
#include "UObject/StructOnScope.h" // #118 : Visual Studio Code Link
#endif

/**
  * #118
 */
struct T4GAMEBUILTIN_API FT4GameBuiltin_GameDataBase
{
	FT4GameBuiltin_GameDataBase(const FName& InRowName)
		: RowName(InRowName)
#if WITH_EDITOR
		, bDirtyed(false)
#endif
	{
	}
	virtual ~FT4GameBuiltin_GameDataBase() {}

	virtual ET4GameBuiltin_GameDataType GetType() const = 0; // #48

#if WITH_EDITOR
	// #118
	virtual TSharedPtr<FStructOnScope> GetRawDataStruct() = 0;
	virtual bool CheckValidationAll() = 0;
	virtual bool CheckValidationBy(FName InPropertyName) = 0;
	virtual const FString& GetDescription() const = 0;
	virtual bool HasError() const = 0;
	// ~#118
#endif
	
	FName RowName;
	FGuid RowGuid;

#if WITH_EDITOR
	bool bDirtyed; // #118 : Content Editor 에서 편집이 있을 경우 true
#endif
};

struct T4GAMEBUILTIN_API FT4GameBuiltin_GameMasterData : public FT4GameBuiltin_GameDataBase
{
	FT4GameBuiltin_GameMasterData(const FName& InRowName)
		: FT4GameBuiltin_GameDataBase(InRowName)
	{
	}

	ET4GameBuiltin_GameDataType GetType() const override { return ET4GameBuiltin_GameDataType::Master; } // #118

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override { return true; }
	bool CheckValidationBy(FName InPropertyName) override { return true; }
	const FString& GetDescription() const override { return RawData.Description; }
	bool HasError() const override { return false; }
#endif

	FT4GameBuiltin_MasterTableRow RawData; // #27
};

struct T4GAMEBUILTIN_API FT4GameBuiltin_GameWorldData : public FT4GameBuiltin_GameDataBase
{
	FT4GameBuiltin_GameWorldData(const FName& InRowName)
		: FT4GameBuiltin_GameDataBase(InRowName)
	{
	}
	ET4GameBuiltin_GameDataType GetType() const override { return ET4GameBuiltin_GameDataType::World; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override { return true; }
	bool CheckValidationBy(FName InPropertyName) override { return true; }
	const FString& GetDescription() const override { return RawData.Description; }
	bool HasError() const override { return false; }
#endif

	FT4GameBuiltin_WorldTableRow RawData; // #27
};

struct T4GAMEBUILTIN_API FT4GameBuiltin_GamePlayerData : public FT4GameBuiltin_GameDataBase
{
	FT4GameBuiltin_GamePlayerData(const FName& InRowName)
		: FT4GameBuiltin_GameDataBase(InRowName)
	{
	}
	ET4GameBuiltin_GameDataType GetType() const override { return ET4GameBuiltin_GameDataType::Player; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override;
	bool CheckValidationBy(FName InPropertyName) override;
	const FString& GetDescription() const override { return RawData.Description; }
	bool HasError() const override;
#endif

	FT4GameBuiltin_PlayerTableRow RawData; // #27
};

struct T4GAMEBUILTIN_API FT4GameBuiltin_GameNPCData : public FT4GameBuiltin_GameDataBase
{
	FT4GameBuiltin_GameNPCData(const FName& InRowName)
		: FT4GameBuiltin_GameDataBase(InRowName)
	{
	}
	ET4GameBuiltin_GameDataType GetType() const override { return ET4GameBuiltin_GameDataType::NPC; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override;
	bool CheckValidationBy(FName InPropertyName) override;
	const FString& GetDescription() const override { return RawData.Description; }
	bool HasError() const override;
#endif

	FT4GameBuiltin_NPCTableRow RawData; // #31
};

struct T4GAMEBUILTIN_API FT4GameBuiltin_GameWeaponData : public FT4GameBuiltin_GameDataBase
{
	FT4GameBuiltin_GameWeaponData(const FName& InRowName)
		: FT4GameBuiltin_GameDataBase(InRowName)
	{
	}
	ET4GameBuiltin_GameDataType GetType() const override { return ET4GameBuiltin_GameDataType::Weapon; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override;
	bool CheckValidationBy(FName InPropertyName) override;
	const FString& GetDescription() const override { return RawData.Description; }
	bool HasError() const override;
#endif

	FT4GameBuiltin_WeaponTableRow RawData; // #27, #48
};

struct T4GAMEBUILTIN_API FT4GameBuiltin_GameCostumeData : public FT4GameBuiltin_GameDataBase
{
	FT4GameBuiltin_GameCostumeData(const FName& InRowName)
		: FT4GameBuiltin_GameDataBase(InRowName)
	{
	}
	ET4GameBuiltin_GameDataType GetType() const override { return ET4GameBuiltin_GameDataType::Costume; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override { return true; }
	bool CheckValidationBy(FName InPropertyName) override { return true; }
	const FString& GetDescription() const override { return RawData.Description; }
	bool HasError() const override { return false; }
#endif

	FT4GameBuiltin_CostumeTableRow RawData; // #27, #48
};

struct T4GAMEBUILTIN_API FT4GameBuiltin_GameSkillSetData : public FT4GameBuiltin_GameDataBase
{
	FT4GameBuiltin_GameSkillSetData(const FName& InRowName)
		: FT4GameBuiltin_GameDataBase(InRowName)
	{
	}
	ET4GameBuiltin_GameDataType GetType() const override { return ET4GameBuiltin_GameDataType::SkillSet; } // #50

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override;
	bool CheckValidationBy(FName InPropertyName) override;
	const FString& GetDescription() const override { return RawData.Description; }
	bool HasError() const override;
#endif

	FT4GameBuiltin_SkillSetTableRow RawData; // #27
};

struct T4GAMEBUILTIN_API FT4GameBuiltin_GameSkillData : public FT4GameBuiltin_GameDataBase
{
	FT4GameBuiltin_GameSkillData(const FName& InRowName)
		: FT4GameBuiltin_GameDataBase(InRowName)
	{
	}
	ET4GameBuiltin_GameDataType GetType() const override { return ET4GameBuiltin_GameDataType::Skill; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override;
	bool CheckValidationBy(FName InPropertyName) override;
	const FString& GetDescription() const override { return RawData.Description; }
	bool HasError() const override;
#endif

	FT4GameBuiltin_SkillTableRow RawData; // #27
};

struct T4GAMEBUILTIN_API FT4GameBuiltin_GameEffectData : public FT4GameBuiltin_GameDataBase
{
	FT4GameBuiltin_GameEffectData(const FName& InRowName)
		: FT4GameBuiltin_GameDataBase(InRowName)
	{
	}
	ET4GameBuiltin_GameDataType GetType() const override { return ET4GameBuiltin_GameDataType::Effect; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override;
	bool CheckValidationBy(FName InPropertyName) override;
	const FString& GetDescription() const override { return RawData.Description; }
	bool HasError() const override;
#endif

	FT4GameBuiltin_EffectTableRow RawData; // #27
};

#if (WITH_EDITOR || WITH_SERVER_CODE)
struct T4GAMEBUILTIN_API FT4GameBuiltin_GameStatData : public FT4GameBuiltin_GameDataBase // #114
{
	FT4GameBuiltin_GameStatData(const FName& InRowName)
		: FT4GameBuiltin_GameDataBase(InRowName)
	{
	}
	ET4GameBuiltin_GameDataType GetType() const override { return ET4GameBuiltin_GameDataType::Stat; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override { return true; }
	bool CheckValidationBy(FName InPropertyName) override { return true; }
	const FString& GetDescription() const override { return RawData.Description; }
	bool HasError() const override { return false; }
#endif

	FT4GameBuiltin_StatTableRow RawData; // #27
};

struct T4GAMEBUILTIN_API FT4GameBuiltin_GameExperienceData : public FT4GameBuiltin_GameDataBase // #114
{
	FT4GameBuiltin_GameExperienceData(const FName& InRowName)
		: FT4GameBuiltin_GameDataBase(InRowName)
	{
	}
	ET4GameBuiltin_GameDataType GetType() const override { return ET4GameBuiltin_GameDataType::Experience; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override { return true; }
	bool CheckValidationBy(FName InPropertyName) override { return true; }
	const FString& GetDescription() const override { return RawData.Description; }
	bool HasError() const override { return false; }
#endif

	FT4GameBuiltin_ExperienceTableRow RawData; // #27
};
#endif

class UDataTable;
class T4GAMEBUILTIN_API IT4GameBuiltin_GameData
{
public:
	virtual ~IT4GameBuiltin_GameData() {}

	virtual bool HasGameData(const FT4GameBuiltin_GameDataID& InGameDataID) const = 0;
	virtual bool HasGameData(const FGuid& InGuid) const = 0;

	virtual const FT4GameBuiltin_GameDataBase* GetGameDataBase(const FT4GameBuiltin_GameDataID& InGameDataID) const = 0;
	virtual const FT4GameBuiltin_GameDataBase* GetGameDataBase(const FGuid& InDataGuid) const = 0;

	virtual const FT4GameBuiltin_GameWorldData* GetGameWorldData(const FT4GameBuiltin_GameDataID& InGameDataID) const = 0;
	virtual const FT4GameBuiltin_GamePlayerData* GetGamePlayerData(const FT4GameBuiltin_GameDataID& InGameDataID) const = 0;
	virtual const FT4GameBuiltin_GameNPCData* GetGameNPCData(const FT4GameBuiltin_GameDataID& InGameDataID) const = 0;
	virtual const FT4GameBuiltin_GameWeaponData* GetGameWeaponData(const FT4GameBuiltin_GameDataID& InGameDataID) const = 0;
	virtual const FT4GameBuiltin_GameCostumeData* GetGameCostumeData(const FT4GameBuiltin_GameDataID& InGameDataID) const = 0;
	virtual const FT4GameBuiltin_GameSkillSetData* GetGameSkillSetData(const FT4GameBuiltin_GameDataID& InGameDataID) const = 0;
	virtual const FT4GameBuiltin_GameSkillData* GetGameSkillData(const FT4GameBuiltin_GameDataID& InGameDataID) const = 0;
	virtual const FT4GameBuiltin_GameEffectData* GetGameEffectData(const FT4GameBuiltin_GameDataID& InGameDataID) const = 0;
#if (WITH_EDITOR || WITH_SERVER_CODE)
	virtual const FT4GameBuiltin_GameStatData* GetGameStatData(const FT4GameBuiltin_GameDataID& InGameDataID) const = 0;
	virtual const FT4GameBuiltin_GameExperienceData* GetGameExperienceData(const FT4GameBuiltin_GameDataID& InGameDataID) const = 0;
#endif

#if WITH_EDITOR
	// #118
	virtual bool SaveDataTable(ET4GameBuiltin_GameDataType InGameDataType, FString& OutErrorMessage) = 0;
	virtual UDataTable* GetDataTable(ET4GameBuiltin_GameDataType InGameDataType) const = 0; 

	virtual const TArray<FT4GameBuiltin_GameDataBase*>& GetGameRawDatas(
		ET4GameBuiltin_GameDataType InGameDataType,
		bool bInCheckValidation
	) = 0;
	virtual FT4GameBuiltin_GameDataBase* GetGameRawData(const FT4GameBuiltin_GameDataID& InGameDataID) = 0;

	virtual void AddRawData(ET4GameBuiltin_GameDataType InGameDataType, FName InRowName) = 0;
	virtual void RemoveRawData(ET4GameBuiltin_GameDataType InGameDataType, FName InRowName) = 0;
	virtual void RenameRawData(ET4GameBuiltin_GameDataType InGameDataType, FName InOldRowName, FName InNewRowName) = 0;
	virtual void DuplicateRawData(ET4GameBuiltin_GameDataType InGameDataType, FName InSourceRowName, FName InNewRowName) = 0;
	virtual void MoveUpRawData(ET4GameBuiltin_GameDataType InGameDataType, FName InRowName) = 0;
	virtual void MoveDownRawData(ET4GameBuiltin_GameDataType InGameDataType, FName InRowName) = 0;
	virtual void UpdateRawData(const FT4GameBuiltin_GameDataID& InGameDataID) = 0; // GameData to RawData
	// ~#118
#endif
};

T4GAMEBUILTIN_API IT4GameBuiltin_GameData* GetGameBuiltinData();
