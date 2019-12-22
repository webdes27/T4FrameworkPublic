// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameDB.h"
#include "Classes/GameTable/T4GameTable_Master.h"

#include "T4GameplayInternal.h"

/**
  * 
 */
void FT4GameDataInfo::Reset()
{
	NameMap.Empty();
	GuidMap.Empty();
	for (FT4GameDataBase* GameData : GameDatas)
	{
		check(nullptr != GameData);
		delete GameData;
	}
	GameDatas.Empty();
#if WITH_EDITOR
	if (DataTable.IsValid())
	{
		DataTable->RemoveFromRoot();
		DataTable.Reset();
	}
#endif
}

template <class T> 
const T* FT4GameDataInfo::GetRowByName(const FName& InName)
{
	if (!NameMap.Contains(InName))
	{
		return nullptr;
	}
	return static_cast<T*>(NameMap[InName]);
}

template <class T> 
const T* FT4GameDataInfo::GetRowByGuid(const FGuid& InGuid)
{
	if (!GuidMap.Contains(InGuid))
	{
		return nullptr;
	}
	return static_cast<T*>(GuidMap[InGuid]);
}

FT4GameDB::FT4GameDB()
	: bInitialized(false)
	, InitCount(0)
{
}

FT4GameDB::~FT4GameDB()
{
}

bool FT4GameDB::Initialize(const FSoftObjectPath& InGameMasterTablePath)
{
	InitCount++;

	if (bInitialized)
	{
		return true;
	}

	FSoftObjectPath MasterTablePath = InGameMasterTablePath;
	UObject* MasterTableObject = MasterTablePath.TryLoad();
	if (nullptr == MasterTableObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("FT4GameDB : Failed to load Game MasterTable. (%s)"),
			*(InGameMasterTablePath.ToString())
		);
		return false;
	}
	UDataTable* MasterDataTable = Cast<UDataTable>(MasterTableObject);
	if (nullptr == MasterDataTable)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("FT4GameDB : Failed to convert UDataTable. (%s)"),
			*(InGameMasterTablePath.ToString())
		);
		return false;
	}
	TArray<FT4GameMasterTableRow*> AllRowDatas;
	MasterDataTable->GetAllRows<FT4GameMasterTableRow>(TEXT("GameMasterTables"), AllRowDatas);
	if (0 == AllRowDatas.Num())
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("FT4GameDB : Failed to empty UDataTable. (%s)"),
			*(InGameMasterTablePath.ToString())
		);
		return false;
	}

	for (const FT4GameMasterTableRow* KeyTableRow : AllRowDatas)
	{
		ET4GameDataType GameTableType = KeyTableRow->Type;
		FString TableName = KeyTableRow->Name.ToString();
		FString GameTablePath = KeyTableRow->Table.ToString();
		switch (GameTableType)
		{
			case ET4GameDataType::World: // #27
				{
					if (!LoadTableInternal<FT4GameWorldTableRow, FT4GameWorldData>(
						*TableName,
						*GameTablePath,
						GameTableType,
						GameDataInfos[uint8(GameTableType)] // #66
					))
					{
						continue;
					}
				}
				break;

			case ET4GameDataType::Player: // #27
				{
					if (!LoadTableInternal<FT4GamePlayerTableRow, FT4GamePlayerData>(
						*TableName,
						*GameTablePath,
						GameTableType,
						GameDataInfos[uint8(GameTableType)] // #66
					))
					{
						continue;
					}
				}
				break;

			case ET4GameDataType::NPC: // #31
				{
					if (!LoadTableInternal<FT4GameNPCTableRow, FT4GameNPCData>(
						*TableName,
						*GameTablePath,
						GameTableType,
						GameDataInfos[uint8(GameTableType)] // #66
					))
					{
						continue;
					}
				}
				break;

			case ET4GameDataType::FO: // #27
				{
					if (!LoadTableInternal<FT4GameFOTableRow, FT4GameFOData>(
						*TableName,
						*GameTablePath,
						GameTableType,
						GameDataInfos[uint8(GameTableType)] // #66
					))
					{
						continue;
					}
				}
				break;

			case ET4GameDataType::Item_Weapon: // #27, #48
				{
					if (!LoadTableInternal<FT4GameItemWeaponTableRow, FT4GameItemWeaponData>(
						*TableName,
						*GameTablePath,
						GameTableType,
						GameDataInfos[uint8(GameTableType)] // #66
					))
					{
						continue;
					}
				}
				break;

			case ET4GameDataType::Item_Costume: // #27, #48
				{
					if (!LoadTableInternal<FT4GameItemCostumeTableRow, FT4GameItemCostumeData>(
						*TableName,
						*GameTablePath,
						GameTableType,
						GameDataInfos[uint8(GameTableType)] // #66
					))
					{
						continue;
					}
				}
				break;

			case ET4GameDataType::Skill: // #25
				{
					if (!LoadTableInternal<FT4GameSkillTableRow, FT4GameSkillData>(
						*TableName,
						*GameTablePath,
						GameTableType,
						GameDataInfos[uint8(GameTableType)] // #66
					))
					{
						continue;
					}
				}
				break;

			case ET4GameDataType::SkillSet: // #50
				{
					if (!LoadTableInternal<FT4GameSkillSetTableRow, FT4GameSkillSetData>(
						*TableName,
						*GameTablePath,
						GameTableType,
						GameDataInfos[uint8(GameTableType)] // #66
					))
					{
						continue;
					}
				}
				break;

			case ET4GameDataType::Effect: // #25
				{
					if (!LoadTableInternal<FT4GameEffectTableRow, FT4GameEffectData>(
						*TableName,
						*GameTablePath,
						GameTableType,
						GameDataInfos[uint8(GameTableType)] // #66
					))
					{
						continue;
					}
				}
				break;

			default:
				{
					UE_LOG(
						LogT4Gameplay,
						Error,
						TEXT("FT4GameDB : Unknown game '%s' table. (%s)"),
						*TableName,
						*GameTablePath
					);
				}
				break;
		}
	}

	bInitialized = true;
	return true;
}

void FT4GameDB::Finalize()
{
	if (!bInitialized)
	{
		return;
	}
	InitCount--;
	if (0 < InitCount)
	{
		return;
	}
	Reset();
	bInitialized = false;
}

void FT4GameDB::Reload()
{
	// TODO
}

void FT4GameDB::Reset()
{
	GameDataIDs.Empty(); // #48
	GameDataGuids.Empty(); // #48
	for (uint8 idx = 0; idx < (uint8)ET4GameDataType::Nums; ++idx)
	{
		GameDataInfos[idx].Reset(); // #66
	}
}

template <class T>
const T* FT4GameDB::GetGameData(const FT4GameDataID& InDataID) // #48
{
	if (ET4GameDataType::Nums <= InDataID.Type)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("FT4GameDB::GetGameData : Unknown GameData '%s'."),
			InDataID.ToTypeString()
		);
		return nullptr;
	}
	return GameDataInfos[uint8(InDataID.Type)].GetRowByName<T>(InDataID.RowName);
}

template <class T>
const T* FT4GameDB::GetGameData(const FGuid& InDataGuid) // #48
{
	if (!GameDataGuids.Contants(InDataGuid))
	{
		return nullptr;
	}
	const FT4GameDataID& GameDataID = GameDataGuids[InDataGuid];
	return GetGameData<T>(GameDataID);
}

bool FT4GameDB::HasGameData(const FT4GameDataID& InDataID) const // #48
{
	return GameDataIDs.Contains(InDataID);
}

#if WITH_EDITOR
const FT4GameDataInfo& FT4GameDB::GetGameDataInfo(ET4GameDataType InGameDataType) // #60
{
	if (ET4GameDataType::Nums <= InGameDataType)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("FT4GameDB::GetGameData : Unknown GameData '%u'."),
			uint8(InGameDataType)
		);
		return EmptyDatas;
	}
	return GameDataInfos[uint8(InGameDataType)];
}
#endif

template <class T, class U>
bool FT4GameDB::LoadTableInternal(
	const TCHAR* InTableName,
	const TCHAR* InTablePath,
	ET4GameDataType InGameTableType, // #48
	FT4GameDataInfo& OutTableInfo
)
{
	FSoftObjectPath TablePath = FString(InTablePath);
	UObject* TableObject = TablePath.TryLoad();
	if (nullptr == TableObject)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("LoadTableInternal : Failed to load content '%s' table. (%s)"),
			InTableName,
			InTablePath
		);
		return false;
	}
	UDataTable* DataTable = Cast<UDataTable>(TableObject);
	if (nullptr == DataTable)
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("LoadTableInternal : Failed to convert '%s' UDataTable. (%s)"),
			InTableName,
			InTablePath
		);
		return false;
	}

#if WITH_EDITOR
	OutTableInfo.TableName = InTableName; // #66
#endif

	TArray<T*> AllRowDatas;
	DataTable->GetAllRows<T>(InTableName, AllRowDatas);
	for (const T* TableData : AllRowDatas)
	{
		FT4GameDataID NewGameDataID(InGameTableType, TableData->Name);
		if (GameDataIDs.Contains(NewGameDataID))
		{
			UE_LOG(
				LogT4Gameplay,
				Error,
				TEXT("LoadTableInternal : Failed to Add GameData. '%s' already exists. skipped."),
				*(NewGameDataID.ToString())
			);
			continue;
		}
		if (GameDataGuids.Contains(TableData->Guid))
		{
			UE_LOG(
				LogT4Gameplay,
				Error,
				TEXT("LoadTableInternal : Failed to Add GameData. GameData '%s' Guid '%s' already exists. skipped."),
				*(NewGameDataID.ToString()),
				*(TableData->Guid.ToString())
			);
			continue;
		}

		U* NewGameData = new U(TableData->Name);
		NewGameData->RawData = *TableData;
		OutTableInfo.GameDatas.Add(NewGameData);
		OutTableInfo.NameMap.Add(TableData->Name, NewGameData);
		OutTableInfo.GuidMap.Add(TableData->Guid, NewGameData);

		GameDataIDs.Add(NewGameDataID); // #48
		GameDataGuids.Add(TableData->Guid, NewGameDataID); // #48
	}

#if WITH_EDITOR
	OutTableInfo.DataTable = DataTable;
	OutTableInfo.DataTable->AddToRoot();
	DataTable->OnDataTableChanged().AddRaw(this, &FT4GameDB::HandleOnGameTableChanged, InGameTableType);
#endif
	return true;
}

#if WITH_EDITOR

void FT4GameDB::HandleOnGameTableChanged(ET4GameDataType InGameTableType)
{
	switch (InGameTableType)
	{
		case ET4GameDataType::World:
			{
				CopyDataFromRawInternal<FT4GameWorldTableRow, FT4GameWorldData>(
					InGameTableType,
					GameDataInfos[uint8(InGameTableType)]
				);
			}
			break;

		case ET4GameDataType::Player:
			{
				CopyDataFromRawInternal<FT4GamePlayerTableRow, FT4GamePlayerData>(
					InGameTableType,
					GameDataInfos[uint8(InGameTableType)]
				);
			}
			break;

		case ET4GameDataType::NPC:
			{
				CopyDataFromRawInternal<FT4GameNPCTableRow, FT4GameNPCData>(
					InGameTableType,
					GameDataInfos[uint8(InGameTableType)]
				);
			}
			break;

		case ET4GameDataType::FO:
			{
				CopyDataFromRawInternal<FT4GameFOTableRow, FT4GameFOData>(
					InGameTableType,
					GameDataInfos[uint8(InGameTableType)]
				);
			}
			break;

		case ET4GameDataType::Item_Weapon:
			{
				CopyDataFromRawInternal<FT4GameItemWeaponTableRow, FT4GameItemWeaponData>(
					InGameTableType,
					GameDataInfos[uint8(InGameTableType)]
				);
			}
			break;

		case ET4GameDataType::Item_Costume:
			{
				CopyDataFromRawInternal<FT4GameItemCostumeTableRow, FT4GameItemCostumeData>(
					InGameTableType,
					GameDataInfos[uint8(InGameTableType)]
				);
			}
			break;

		case ET4GameDataType::Skill:
			{
				CopyDataFromRawInternal<FT4GameSkillTableRow, FT4GameSkillData>(
					InGameTableType,
					GameDataInfos[uint8(InGameTableType)]
				);
			}
			break;

		case ET4GameDataType::SkillSet: // #50
			{
				CopyDataFromRawInternal<FT4GameSkillSetTableRow, FT4GameSkillSetData>(
					InGameTableType,
					GameDataInfos[uint8(InGameTableType)]
				);
			}
			break;

		case ET4GameDataType::Effect:
			{
				CopyDataFromRawInternal<FT4GameEffectTableRow, FT4GameEffectData>(
					InGameTableType,
					GameDataInfos[uint8(InGameTableType)]
				);
			}
			break; 

		default:
			{
				UE_LOG(
					LogT4Gameplay,
					Error,
					TEXT("FT4GameDB : Unknown GameDataType '%u'"),
					(uint8)(InGameTableType)
				);
			}
			break;
	}
}

template <class T, class U>
bool FT4GameDB::CopyDataFromRawInternal(
	ET4GameDataType InGameTableType,
	FT4GameDataInfo& InOutTableInfo
)
{
	if (!InOutTableInfo.DataTable.IsValid())
	{
		return false;
	}
	const TCHAR* ContextString = *(InOutTableInfo.TableName.ToString());
	TArray<T*> AllRowDatas;
	InOutTableInfo.DataTable->GetAllRows<T>(ContextString, AllRowDatas);
	for (const T* TableData : AllRowDatas)
	{
		if (InOutTableInfo.NameMap.Contains(TableData->Name))
		{
			U* ConvertData = static_cast<U*>(InOutTableInfo.NameMap[TableData->Name]);
			check(nullptr != ConvertData);
			ConvertData->RawData = *TableData;
		}
		else
		{
			// #66
			FT4GameDataID NewGameDataID(InGameTableType, TableData->Name);
			if (GameDataIDs.Contains(NewGameDataID))
			{
				UE_LOG(
					LogT4Gameplay,
					Error,
					TEXT("CopyDataFromRawInternal : Failed to Add GameData. '%s' already exists. skipped."),
					*(NewGameDataID.ToString())
				);
				continue;
			}
			if (GameDataGuids.Contains(TableData->Guid))
			{
				UE_LOG(
					LogT4Gameplay,
					Error,
					TEXT("CopyDataFromRawInternal : Failed to Add GameData. GameData '%s' Guid '%s' already exists. skipped."),
					*(NewGameDataID.ToString()),
					*(TableData->Guid.ToString())
				);
				continue;
			}

			U* NewGameData = new U(TableData->Name);
			NewGameData->RawData = *TableData;
			InOutTableInfo.GameDatas.Add(NewGameData);
			InOutTableInfo.NameMap.Add(TableData->Name, NewGameData);
			InOutTableInfo.GuidMap.Add(TableData->Guid, NewGameData);

			GameDataIDs.Add(NewGameDataID); // #48
			GameDataGuids.Add(TableData->Guid, NewGameDataID); // #48
		}
	}
	return true;
}
#endif

static FT4GameDB GT4GameDB;
FT4GameDB& GetGameDB()
{
	return GT4GameDB;
}
