// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "Public/T4EngineConstants.h"
#include "Public/T4EngineSettings.h" // #107

#include "Classes/DataTable/T4ConstantDataTable.h" // #39

#include "T4EngineInternal.h"

/**
  * #39
 */

enum ET4ConstantValueType // #92
{
	ConstantValue_Name,
	ConstantValue_Float,
	ConstantValue_Int,
};

static const ET4ConstantValueType GConstantValueTypes[]
{
	ET4ConstantValueType::ConstantValue_Int, // WorldZone
	ET4ConstantValueType::ConstantValue_Float, // TimeTag

#if WITH_EDITOR
	ET4ConstantValueType::ConstantValue_Name, // BlendSpace
	ET4ConstantValueType::ConstantValue_Name, // DefaultSection
	ET4ConstantValueType::ConstantValue_Name, // OverlaySection
	ET4ConstantValueType::ConstantValue_Name, // SkillSection
	ET4ConstantValueType::ConstantValue_Name, // #107 : WeaponSection

	ET4ConstantValueType::ConstantValue_Name, // ActionPoint

	ET4ConstantValueType::ConstantValue_Name, // EquipPoint
	ET4ConstantValueType::ConstantValue_Name, // CompositePart

	ET4ConstantValueType::ConstantValue_Name, // PlayTagMaterial
	ET4ConstantValueType::ConstantValue_Name, // PlayTagAttachment
	ET4ConstantValueType::ConstantValue_Name, // PlayTagConti

	ET4ConstantValueType::ConstantValue_Name, // Stance
	ET4ConstantValueType::ConstantValue_Name, // SubStance
	ET4ConstantValueType::ConstantValue_Name, // Reaction
#endif
};

static_assert(UE_ARRAY_COUNT(GConstantValueTypes) == (uint8)(ET4EngineConstantTable::Nums), "Constants type doesn't match!");

class FT4ConstantTableManager : public IT4ConstantTableManager
{
public:
	FT4ConstantTableManager();
	~FT4ConstantTableManager();

	void Reset() override;

	bool LoadEngineTable(ET4EngineConstantTable InTable, const FSoftObjectPath& InTablePath) override;  // #90

#if WITH_EDITOR
	bool LoadEditorTable(ET4EngineConstantTable InTable, const FSoftObjectPath& InTablePath) override;
#endif

	TArray<FT4ConstantDataRow>& GetConstantDatas(ET4EngineConstantTable InTable) override;
	const FT4ConstantDataRow& GetConstantData(ET4EngineConstantTable InTable, FName InName) override; // #71

private:
	bool LoadTableInternal(ET4EngineConstantTable InTable, const FSoftObjectPath& InTablePath); // #90

	void LoadFromDataTable(ET4EngineConstantTable InTable);

#if WITH_EDITOR
	void OnTableChanged();
#endif

private:
	TArray<FT4ConstantDataRow> ConstantDatas[ET4EngineConstantTable::Nums];
	TWeakObjectPtr<UDataTable> DataTable[ET4EngineConstantTable::Nums];
};

FT4ConstantTableManager::FT4ConstantTableManager()
{
}

FT4ConstantTableManager::~FT4ConstantTableManager()
{
}

void FT4ConstantTableManager::Reset()
{
	for (uint32 idx = 0; idx < ET4EngineConstantTable::Nums; ++idx)
	{
		if (DataTable[idx].IsValid())
		{
			DataTable[idx]->RemoveFromRoot();
			DataTable[idx].Reset();
		}
		ConstantDatas[idx].Empty();
	}
}

bool FT4ConstantTableManager::LoadEngineTable(
	ET4EngineConstantTable InTable,
	const FSoftObjectPath& InTablePath
)
{
	check(ET4EngineConstantTable::Engine_Start <= InTable && ET4EngineConstantTable::Engine_End > InTable);
	bool bResult = LoadTableInternal(InTable, InTablePath);
	return bResult;
}

#if WITH_EDITOR
bool FT4ConstantTableManager::LoadEditorTable(
	ET4EngineConstantTable InTable,
	const FSoftObjectPath& InTablePath
)
{
	check(ET4EngineConstantTable::Editor_Start <= InTable && ET4EngineConstantTable::Editor_End > InTable);
	bool bResult = LoadTableInternal(InTable, InTablePath);
	return bResult;
}
#endif

bool FT4ConstantTableManager::LoadTableInternal(
	ET4EngineConstantTable InTable,
	const FSoftObjectPath& InTablePath
)
{
	check(ET4EngineConstantTable::Engine_Start <= InTable && ET4EngineConstantTable::Nums > InTable);
	FSoftObjectPath ConstantTablePath(InTablePath);
	UObject* LoadTableObject = ConstantTablePath.TryLoad();
	if (nullptr == LoadTableObject)
	{
		T4_LOG(
			Warning,
			TEXT("Failed to load ConstantDataTable. (%s)"),
			*InTablePath.ToString()
		);
		return false;
	}
	UDataTable* LoadDataTable = Cast<UDataTable>(LoadTableObject);
	if (nullptr == LoadDataTable)
	{
		T4_LOG(
			Warning,
			TEXT("Failed to convert UDataTable. (%s)"),
			*InTablePath.ToString()
		);
		return false;
	}
	DataTable[InTable] = LoadDataTable;
	DataTable[InTable]->AddToRoot();
	LoadFromDataTable(InTable);
#if WITH_EDITOR
	if (DataTable[InTable].IsValid())
	{
		DataTable[InTable]->OnDataTableChanged().AddRaw(
			this,
			&FT4ConstantTableManager::OnTableChanged
		);
	}
#endif
	return true;
}

TArray<FT4ConstantDataRow>& FT4ConstantTableManager::GetConstantDatas(ET4EngineConstantTable InTable)
{
	check(ET4EngineConstantTable::Nums > InTable);
	return ConstantDatas[InTable];
}

const FT4ConstantDataRow& FT4ConstantTableManager::GetConstantData(
	ET4EngineConstantTable InTable,
	FName InName
) // #71, #92
{
	TArray<FT4ConstantDataRow>& ConstantDataSelected = GetConstantDatas(InTable);
	for (const FT4ConstantDataRow& ConstantData : ConstantDataSelected)
	{
		if (InName == ConstantData.Name)
		{
			return ConstantData;
		}
	}
	static FT4ConstantDataRow GEmptyConstantDataRow;
	return GEmptyConstantDataRow;
}

void FT4ConstantTableManager::LoadFromDataTable(ET4EngineConstantTable InTable)
{
	check(DataTable[InTable].IsValid());

	int32 ItemCount = 0;
	ET4ConstantValueType ValueType = GConstantValueTypes[InTable];
	switch (ValueType)
	{
		case ET4ConstantValueType::ConstantValue_Name:
			{
				TArray<FT4ConstantNameTableRow*> AllRowDatas;
				DataTable[InTable]->GetAllRows<FT4ConstantNameTableRow>(TEXT("T4ConstantNameTableRow"), AllRowDatas);
				if (0 == AllRowDatas.Num())
				{
					T4_LOG(
						Warning,
						TEXT("Failed to empty UDataTable")
					);
					return;
				}
				for (const FT4ConstantNameTableRow* InfoRow : AllRowDatas)
				{
					FT4ConstantDataRow NewItemInfo;
					NewItemInfo.Name = InfoRow->Name;
#if WITH_EDITOR
					NewItemInfo.Description = InfoRow->Description;
					NewItemInfo.SortOrder = ItemCount++;
#endif
					ConstantDatas[InTable].Add(NewItemInfo);
				}
			}
			break;

		case ET4ConstantValueType::ConstantValue_Float:
			{
				TArray<FT4ConstantFloatValueTableRow*> AllRowDatas;
				DataTable[InTable]->GetAllRows<FT4ConstantFloatValueTableRow>(TEXT("T4ConstantFloatValueTableRow"), AllRowDatas);
				if (0 == AllRowDatas.Num())
				{
					T4_LOG(
						Warning,
						TEXT("Failed to empty UDataTable")
					);
					return;
				}
				for (const FT4ConstantFloatValueTableRow* InfoRow : AllRowDatas)
				{
					FT4ConstantDataRow NewItemInfo;
					NewItemInfo.Name = InfoRow->Name;
					NewItemInfo.FloatValue = InfoRow->Value;
#if WITH_EDITOR
					NewItemInfo.Description = InfoRow->Description;
					NewItemInfo.SortOrder = ItemCount++;
#endif
					ConstantDatas[InTable].Add(NewItemInfo);
				}
			}
			break;

		case ET4ConstantValueType::ConstantValue_Int:
			{
				TArray<FT4ConstantIntValueTableRow*> AllRowDatas;
				DataTable[InTable]->GetAllRows<FT4ConstantIntValueTableRow>(TEXT("T4ConstantIntValueTableRow"), AllRowDatas);
				if (0 == AllRowDatas.Num())
				{
					T4_LOG(
						Warning,
						TEXT("Failed to empty UDataTable")
					);
					return;
				}
				for (const FT4ConstantIntValueTableRow* InfoRow : AllRowDatas)
				{
					FT4ConstantDataRow NewItemInfo;
					NewItemInfo.Name = InfoRow->Name;
					NewItemInfo.IntValue = InfoRow->Value;
#if WITH_EDITOR
					NewItemInfo.Description = InfoRow->Description;
					NewItemInfo.SortOrder = ItemCount++;
#endif
					ConstantDatas[InTable].Add(NewItemInfo);
				}
			}
			break;

		default:
			{
				T4_LOG(
					Error,
					TEXT("Unknown Constants Value type '%u'"),
					uint8(ValueType)
				);
			}
			break;
	};
}

#if WITH_EDITOR
void FT4ConstantTableManager::OnTableChanged()
{
	for (uint32 idx = 0; idx < ET4EngineConstantTable::Nums; ++idx)
	{
		if (!DataTable[idx].IsValid())
		{
			return;
		}
		ConstantDatas[idx].Empty();
		LoadFromDataTable((ET4EngineConstantTable)idx);
	}
}
#endif

namespace T4EngineConstant
{
	static FName MPCGlobalPathKeyName = TEXT("MPCGlobalPath"); // #115
	static FName MaterialPostProcessOutlinerPathKeyName = TEXT("MaterialPostProcessOutlinerPath");
	static TMap<FName, UObject*> GlobalConstantObjectMap; // #115
	bool Initailize() // #115 : TODO Preloading
	{
		UT4EngineSettings* EngineSettings = GetMutableDefault<UT4EngineSettings>();
		check(nullptr != EngineSettings);
		if (!EngineSettings->MPCGlobalPath.IsNull())
		{
			UObject* LoadedObject = EngineSettings->MPCGlobalPath.TryLoad(); // TODO : 초기화시 로드!
			if (nullptr != LoadedObject)
			{
				LoadedObject->AddToRoot();
				GlobalConstantObjectMap.Add(MPCGlobalPathKeyName, LoadedObject);
			}
		}
		if (!EngineSettings->MaterialPostProcessOutlinerPath.IsNull())
		{
			UObject* LoadedObject = EngineSettings->MaterialPostProcessOutlinerPath.TryLoad(); // TODO : 초기화시 로드!
			if (nullptr != LoadedObject)
			{
				LoadedObject->AddToRoot();
				GlobalConstantObjectMap.Add(MaterialPostProcessOutlinerPathKeyName, LoadedObject);
			}
		}
		return true;
	}

	void Finalize() // #115 : TODO Preloading
	{
		for (TMap<FName, UObject*>::TConstIterator It(GlobalConstantObjectMap); It; ++It)
		{
			It.Value()->RemoveFromRoot();
		}
		GlobalConstantObjectMap.Empty();
	}

	UObject* GetMPCGlobalObject() // #115
	{
		if (!GlobalConstantObjectMap.Contains(MPCGlobalPathKeyName))
		{
			return nullptr;
		}
		return GlobalConstantObjectMap[MPCGlobalPathKeyName];
	}

	UObject* GetMaterialPostProcessOutlinerObject() // #115
	{
		if (!GlobalConstantObjectMap.Contains(MaterialPostProcessOutlinerPathKeyName))
		{
			return nullptr;
		}
		return GlobalConstantObjectMap[MaterialPostProcessOutlinerPathKeyName];
	}

	FName GetMPCGlobalOutlineColorName() // #115
	{
		UT4EngineSettings* EngineSettings = GetMutableDefault<UT4EngineSettings>();
		check(nullptr != EngineSettings);
		return EngineSettings->MPCGlobalOutlineColorName;
	}

	FName GetMaterialParameterOpacityName() // #78, #108
	{
		UT4EngineSettings* EngineSettings = GetMutableDefault<UT4EngineSettings>();
		check(nullptr != EngineSettings);
		return EngineSettings->MaterialParameterOpacityName;
	}

	float GetGameTimeHoursPerDay()
	{
		UT4EngineSettings* EngineSettings = GetMutableDefault<UT4EngineSettings>();
		check(nullptr != EngineSettings);
		return EngineSettings->GameTimeHoursPerDay;
	}

	IT4ConstantTableManager* GetTableManager()
	{
		static FT4ConstantTableManager GT4ConstantTableManager;
		return &GT4ConstantTableManager;
	}
};
