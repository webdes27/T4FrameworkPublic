// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Public/T4GameDataTypes.h" // #48
#include "Classes/DataTable/T4GameTableMaster.h" // #118
#include "Classes/DataTable/T4GameTableWorld.h" // #27
#include "Classes/DataTable/T4GameTablePlayer.h" // #27
#include "Classes/DataTable/T4GameTableNPC.h" // #31
#include "Classes/DataTable/T4GameTableWeapon.h" // #27, #48
#include "Classes/DataTable/T4GameTableCostume.h" // #27, #48
#include "Classes/DataTable/T4GameTableSkill.h" // #25
#include "Classes/DataTable/T4GameTableSkillSet.h" // #50
#include "Classes/DataTable/T4GameTableEffect.h" // #25
#include "Classes/DataTable/T4GameTableStat.h" // #114
#include "Classes/DataTable/T4GameTableExperience.h" // #114

#if WITH_EDITOR
#include "UObject/StructOnScope.h" // #118 : Visual Studio Code Link
#endif

/**
  * #118
 */
class FStructOnScope;
struct T4GAMEPLAY_API FT4GameDataBase
{
	FT4GameDataBase(const FName& InRowName)
		: RowName(InRowName)
#if WITH_EDITOR
		, bDirtyed(false)
		, SortOrder(-1)
#endif
	{
	}
	virtual ~FT4GameDataBase() {}

	virtual ET4GameDataType GetType() const = 0; // #48

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

struct T4GAMEPLAY_API FT4GameMasterData : public FT4GameDataBase
{
	FT4GameMasterData(const FName& InRowName)
		: FT4GameDataBase(InRowName)
	{
	}

	ET4GameDataType GetType() const override { return ET4GameDataType::Master; } // #118

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override { return true; }
	bool CheckValidationBy(FName InPropertyName) override { return true; }
	bool HasError() const override { return false; }

	DEFINE_GAME_DATA_COMMON_METHOD()
#endif

	FT4GameMasterTableRow RawData; // #27
};

struct T4GAMEPLAY_API FT4GameWorldData : public FT4GameDataBase
{
	FT4GameWorldData(const FName& InRowName)
		: FT4GameDataBase(InRowName)
	{
	}
	ET4GameDataType GetType() const override { return ET4GameDataType::World; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override { return true; }
	bool CheckValidationBy(FName InPropertyName) override { return true; }
	bool HasError() const override { return false; }

	DEFINE_GAME_DATA_COMMON_METHOD()
#endif

	FT4GameWorldTableRow RawData; // #27
};

struct T4GAMEPLAY_API FT4GamePlayerData : public FT4GameDataBase
{
	FT4GamePlayerData(const FName& InRowName)
		: FT4GameDataBase(InRowName)
	{
	}
	ET4GameDataType GetType() const override { return ET4GameDataType::Player; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override;
	bool CheckValidationBy(FName InPropertyName) override;
	bool HasError() const override;

	DEFINE_GAME_DATA_COMMON_METHOD()
#endif

	FT4GamePlayerTableRow RawData; // #27
};

struct T4GAMEPLAY_API FT4GameNPCData : public FT4GameDataBase
{
	FT4GameNPCData(const FName& InRowName)
		: FT4GameDataBase(InRowName)
	{
	}
	ET4GameDataType GetType() const override { return ET4GameDataType::NPC; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override;
	bool CheckValidationBy(FName InPropertyName) override;
	bool HasError() const override;

	DEFINE_GAME_DATA_COMMON_METHOD()
#endif

	FT4GameNPCTableRow RawData; // #31
};

struct T4GAMEPLAY_API FT4GameWeaponData : public FT4GameDataBase
{
	FT4GameWeaponData(const FName& InRowName)
		: FT4GameDataBase(InRowName)
	{
	}
	ET4GameDataType GetType() const override { return ET4GameDataType::Weapon; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override;
	bool CheckValidationBy(FName InPropertyName) override;
	bool HasError() const override;

	DEFINE_GAME_DATA_COMMON_METHOD()
#endif

	FT4GameWeaponTableRow RawData; // #27, #48
};

struct T4GAMEPLAY_API FT4GameCostumeData : public FT4GameDataBase
{
	FT4GameCostumeData(const FName& InRowName)
		: FT4GameDataBase(InRowName)
	{
	}
	ET4GameDataType GetType() const override { return ET4GameDataType::Costume; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override { return true; }
	bool CheckValidationBy(FName InPropertyName) override { return true; }
	bool HasError() const override { return false; }

	DEFINE_GAME_DATA_COMMON_METHOD()
#endif

	FT4GameCostumeTableRow RawData; // #27, #48
};

struct T4GAMEPLAY_API FT4GameSkillSetData : public FT4GameDataBase
{
	FT4GameSkillSetData(const FName& InRowName)
		: FT4GameDataBase(InRowName)
	{
	}
	ET4GameDataType GetType() const override { return ET4GameDataType::SkillSet; } // #50

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override;
	bool CheckValidationBy(FName InPropertyName) override;
	bool HasError() const override;

	DEFINE_GAME_DATA_COMMON_METHOD()
#endif

	FT4GameSkillSetTableRow RawData; // #27
};

struct T4GAMEPLAY_API FT4GameSkillData : public FT4GameDataBase
{
	FT4GameSkillData(const FName& InRowName)
		: FT4GameDataBase(InRowName)
	{
	}
	ET4GameDataType GetType() const override { return ET4GameDataType::Skill; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override;
	bool CheckValidationBy(FName InPropertyName) override;
	bool HasError() const override;

	DEFINE_GAME_DATA_COMMON_METHOD()
#endif

	FT4GameSkillTableRow RawData; // #27
};

struct T4GAMEPLAY_API FT4GameEffectData : public FT4GameDataBase
{
	FT4GameEffectData(const FName& InRowName)
		: FT4GameDataBase(InRowName)
	{
	}
	ET4GameDataType GetType() const override { return ET4GameDataType::Effect; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override;
	bool CheckValidationBy(FName InPropertyName) override;
	bool HasError() const override;

	DEFINE_GAME_DATA_COMMON_METHOD()
#endif

	FT4GameEffectTableRow RawData; // #27
};

#if (WITH_EDITOR || WITH_SERVER_CODE)
struct T4GAMEPLAY_API FT4GameStatData : public FT4GameDataBase // #114
{
	FT4GameStatData(const FName& InRowName)
		: FT4GameDataBase(InRowName)
	{
	}
	ET4GameDataType GetType() const override { return ET4GameDataType::Stat; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override { return true; }
	bool CheckValidationBy(FName InPropertyName) override { return true; }
	bool HasError() const override { return false; }

	DEFINE_GAME_DATA_COMMON_METHOD()
#endif

	FT4GameStatTableRow RawData; // #27
};

struct T4GAMEPLAY_API FT4GameExperienceData : public FT4GameDataBase // #114
{
	FT4GameExperienceData(const FName& InRowName)
		: FT4GameDataBase(InRowName)
	{
	}
	ET4GameDataType GetType() const override { return ET4GameDataType::Experience; } // #48

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> GetRawDataStruct() override;
	bool CheckValidationAll() override { return true; }
	bool CheckValidationBy(FName InPropertyName) override { return true; }
	bool HasError() const override { return false; }

	DEFINE_GAME_DATA_COMMON_METHOD()
#endif

	FT4GameExperienceTableRow RawData; // #27
};
#endif

class UDataTable;
class T4GAMEPLAY_API IT4GameDataLibrary
{
public:
	virtual ~IT4GameDataLibrary() {}

	virtual bool HasGameData(const FT4GameDataID& InGameDataID) const = 0;
	virtual bool HasGameData(const FGuid& InGuid) const = 0;

	virtual const FT4GameDataBase* GetDataConst(const FT4GameDataID& InGameDataID) const = 0;
	virtual const FT4GameDataBase* GetDataConst(const FGuid& InDataGuid) const = 0;

	virtual const FT4GameWorldData* GetWorldData(const FT4GameDataID& InGameDataID) const = 0;
	virtual const FT4GamePlayerData* GetPlayerData(const FT4GameDataID& InGameDataID) const = 0;
	virtual const FT4GameNPCData* GetNPCData(const FT4GameDataID& InGameDataID) const = 0;
	virtual const FT4GameWeaponData* GetWeaponData(const FT4GameDataID& InGameDataID) const = 0;
	virtual const FT4GameCostumeData* GetCostumeData(const FT4GameDataID& InGameDataID) const = 0;
	virtual const FT4GameSkillSetData* GetSkillSetData(const FT4GameDataID& InGameDataID) const = 0;
	virtual const FT4GameSkillData* GetSkillData(const FT4GameDataID& InGameDataID) const = 0;
	virtual const FT4GameEffectData* GetEffectData(const FT4GameDataID& InGameDataID) const = 0;
#if (WITH_EDITOR || WITH_SERVER_CODE)
	virtual const FT4GameStatData* GetStatData(const FT4GameDataID& InGameDataID) const = 0;
	virtual const FT4GameExperienceData* GetExperienceData(const FT4GameDataID& InGameDataID) const = 0;
#endif

#if WITH_EDITOR
	// #118
	virtual bool SaveDataTable(ET4GameDataType InGameDataType, FString& OutErrorMessage) = 0;
	virtual UDataTable* GetDataTable(ET4GameDataType InGameDataType) const = 0; 

	virtual const TArray<FT4GameDataBase*>& GetDataBases(
		ET4GameDataType InGameDataType,
		bool bInCheckValidation
	) = 0;
	virtual FT4GameDataBase* GetDataBase(const FT4GameDataID& InGameDataID) = 0;

	virtual void DataTableAddRow(ET4GameDataType InGameDataType, const FName& InNewRowName, const FName& InRowName, bool bInFolder) = 0;
	virtual void DataTableRemoveRow(ET4GameDataType InGameDataType, const FName& InRowName) = 0;
	virtual void DataTableRenameRow(ET4GameDataType InGameDataType, const FName& InOldRowName, const FName& InNewRowName) = 0;
	virtual void DataTableDuplicateRow(ET4GameDataType InGameDataType, const FName& InSourceRowName, const FName& InNewRowName) = 0;
	virtual void DataTableMoveRow(ET4GameDataType InGameDataType, const FName& InTargetRowName, const FName& InSourceRowName) = 0;
	virtual void DataTableWriteRowFromGameData(const FT4GameDataID& InGameDataID) = 0; // GameData to RawData

	virtual void DataTableSetTreeExpansion(ET4GameDataType InGameDataType, const FName& InRowName, bool bInExpand) = 0; // #122
	virtual bool DataTableIsTreeExpanded(ET4GameDataType InGameDataType, const FName& InRowName) const = 0; // #122
	// ~#118
#endif
};

T4GAMEPLAY_API IT4GameDataLibrary* GetGameDataLibrary();
