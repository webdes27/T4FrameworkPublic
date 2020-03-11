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
class FStructOnScope;
struct T4GAMEBUILTIN_API FT4GameBuiltin_GameDataBase
{
	FT4GameBuiltin_GameDataBase(const FName& InRowName)
		: RowName(InRowName)
#if WITH_EDITOR
		, bDirtyed(false)
		, SortOrder(-1)
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
	virtual bool HasError() const = 0;
	virtual bool HasParent() const = 0; // #122
	virtual bool IsFolder() const = 0; // #122
	virtual FName GetParentRowName() const = 0; // #122
	virtual void SetParentRowName(FName InParentRowName) = 0; // #122
	virtual FName GetFolderName() const = 0; // #122
	virtual void SetFolderName(FName InFolderName) = 0; // #122
	virtual const FString& GetDescription() const = 0;
	// ~#118
#endif
	
	FName RowName;
	FGuid RowGuid;

#if WITH_EDITOR
	bool bDirtyed; // #118 : Content Editor 에서 편집이 있을 경우 true
	int32 SortOrder;
#endif
};

#if WITH_EDITOR
#define DEFINE_GAME_DATA_COMMON_METHOD()															\
	bool HasParent() const override { return (RawData.ParentRowName != NAME_None) ? true : false; } \
	bool IsFolder() const override { return (RawData.FolderName != NAME_None) ? true : false; }		\
	FName GetParentRowName() const { return RawData.ParentRowName; }								\
	void SetParentRowName(FName InParentRowName) { RawData.ParentRowName = InParentRowName; }		\
	FName GetFolderName() const { return RawData.FolderName; }										\
	void SetFolderName(FName InFolderName) { RawData.FolderName = InFolderName; }					\
	const FString& GetDescription() const override { return RawData.Description; }
#endif

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
	bool HasError() const override { return false; }

	DEFINE_GAME_DATA_COMMON_METHOD()
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
	bool HasError() const override { return false; }

	DEFINE_GAME_DATA_COMMON_METHOD()
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
	bool HasError() const override;

	DEFINE_GAME_DATA_COMMON_METHOD()
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
	bool HasError() const override;

	DEFINE_GAME_DATA_COMMON_METHOD()
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
	bool HasError() const override;

	DEFINE_GAME_DATA_COMMON_METHOD()
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
	bool HasError() const override { return false; }

	DEFINE_GAME_DATA_COMMON_METHOD()
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
	bool HasError() const override;

	DEFINE_GAME_DATA_COMMON_METHOD()
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
	bool HasError() const override;

	DEFINE_GAME_DATA_COMMON_METHOD()
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
	bool HasError() const override;

	DEFINE_GAME_DATA_COMMON_METHOD()
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
	bool HasError() const override { return false; }

	DEFINE_GAME_DATA_COMMON_METHOD()
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
	bool HasError() const override { return false; }

	DEFINE_GAME_DATA_COMMON_METHOD()
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

	virtual const FT4GameBuiltin_GameDataBase* GetGameDataConst(const FT4GameBuiltin_GameDataID& InGameDataID) const = 0;
	virtual const FT4GameBuiltin_GameDataBase* GetGameDataConst(const FGuid& InDataGuid) const = 0;

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

	virtual const TArray<FT4GameBuiltin_GameDataBase*>& GetGameDataBases(
		ET4GameBuiltin_GameDataType InGameDataType,
		bool bInCheckValidation
	) = 0;
	virtual FT4GameBuiltin_GameDataBase* GetGameDataBase(const FT4GameBuiltin_GameDataID& InGameDataID) = 0;

	virtual void DataTableAddRow(ET4GameBuiltin_GameDataType InGameDataType, const FName& InNewRowName, const FName& InRowName, bool bInFolder) = 0;
	virtual void DataTableRemoveRow(ET4GameBuiltin_GameDataType InGameDataType, const FName& InRowName) = 0;
	virtual void DataTableRenameRow(ET4GameBuiltin_GameDataType InGameDataType, const FName& InOldRowName, const FName& InNewRowName) = 0;
	virtual void DataTableDuplicateRow(ET4GameBuiltin_GameDataType InGameDataType, const FName& InSourceRowName, const FName& InNewRowName) = 0;
	virtual void DataTableMoveRow(ET4GameBuiltin_GameDataType InGameDataType, const FName& InTargetRowName, const FName& InSourceRowName) = 0;
	virtual void DataTableWriteRowFromGameData(const FT4GameBuiltin_GameDataID& InGameDataID) = 0; // GameData to RawData

	virtual void DataTableSetTreeExpansion(ET4GameBuiltin_GameDataType InGameDataType, const FName& InRowName, bool bInExpand) = 0; // #122
	virtual bool DataTableIsTreeExpanded(ET4GameBuiltin_GameDataType InGameDataType, const FName& InRowName) const = 0; // #122
	// ~#118
#endif
};

T4GAMEBUILTIN_API IT4GameBuiltin_GameData* GetGameBuiltinData();
