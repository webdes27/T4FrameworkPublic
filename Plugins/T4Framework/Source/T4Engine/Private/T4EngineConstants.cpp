// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "Public/T4EngineConstants.h"

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
	ET4ConstantValueType::ConstantValue_Int, // MapZone
	ET4ConstantValueType::ConstantValue_Float, // TimeTag

#if WITH_EDITOR
	ET4ConstantValueType::ConstantValue_Name, // BlendSpace
	ET4ConstantValueType::ConstantValue_Name, // DefaultSection
	ET4ConstantValueType::ConstantValue_Name, // AdditiveSection
	ET4ConstantValueType::ConstantValue_Name, // SkillSection

	ET4ConstantValueType::ConstantValue_Name, // ActionPoint

	ET4ConstantValueType::ConstantValue_Name, // EquipPoint
	ET4ConstantValueType::ConstantValue_Name, // CompositePart

	ET4ConstantValueType::ConstantValue_Name, // LayerTag

	ET4ConstantValueType::ConstantValue_Name, // Stance
	ET4ConstantValueType::ConstantValue_Name, // Reaction
#endif
};

static_assert(UE_ARRAY_COUNT(GConstantValueTypes) == (uint8)(ET4EngineConstantType::Nums), "Constants type doesn't match!");

FT4EngineConstants::FT4EngineConstants()
{
}

FT4EngineConstants::~FT4EngineConstants()
{
}

void FT4EngineConstants::Reset()
{
	for (uint32 idx = 0; idx < ET4EngineConstantType::Nums; ++idx)
	{
		if (DataTable[idx].IsValid())
		{
			DataTable[idx]->RemoveFromRoot();
			DataTable[idx].Reset();
		}
		ConstantDatas[idx].Empty();
	}
}

bool FT4EngineConstants::LoadEngineConstants(
	ET4EngineConstantType InConstantType,
	const FSoftObjectPath& InConstantTablePath
)
{
	check(ET4EngineConstantType::Engine_Start <= InConstantType && ET4EngineConstantType::Engine_End > InConstantType);
	bool bResult = LoadTable(InConstantType, InConstantTablePath);
	return bResult;
}

#if WITH_EDITOR
bool FT4EngineConstants::LoadEditorConstants(
	ET4EngineConstantType InConstantType,
	const FSoftObjectPath& InConstantTablePath
)
{
	check(ET4EngineConstantType::Editor_Start <= InConstantType && ET4EngineConstantType::Editor_End > InConstantType);
	bool bResult = LoadTable(InConstantType, InConstantTablePath);
	return bResult;
}
#endif

bool FT4EngineConstants::LoadTable(
	ET4EngineConstantType InConstantType,
	const FSoftObjectPath& InConstantTablePath
)
{
	check(ET4EngineConstantType::Engine_Start <= InConstantType && ET4EngineConstantType::Nums > InConstantType);
	FSoftObjectPath ConstantTablePath(InConstantTablePath);
	UObject* LoadTableObject = ConstantTablePath.TryLoad();
	if (nullptr == LoadTableObject)
	{
		UE_LOG(
			LogT4Engine,
			Warning,
			TEXT("FT4EngineConstants : Failed to load ConstantDataTable. (%s)"),
			*InConstantTablePath.ToString()
		);
		return false;
	}
	UDataTable* LoadDataTable = Cast<UDataTable>(LoadTableObject);
	if (nullptr == LoadDataTable)
	{
		UE_LOG(
			LogT4Engine,
			Warning,
			TEXT("FT4EngineConstants : Failed to convert UDataTable. (%s)"),
			*InConstantTablePath.ToString()
		);
		return false;
	}
	DataTable[InConstantType] = LoadDataTable;
	DataTable[InConstantType]->AddToRoot();
	LoadFromDataTable(InConstantType);
#if WITH_EDITOR
	if (DataTable[InConstantType].IsValid())
	{
		DataTable[InConstantType]->OnDataTableChanged().AddRaw(
			this,
			&FT4EngineConstants::OnTableChanged
		);
	}
#endif
	return true;
}

TArray<FT4ConstantDataRow>& FT4EngineConstants::GetConstantDatas(ET4EngineConstantType InConstantType)
{
	check(ET4EngineConstantType::Nums > InConstantType);
	return ConstantDatas[InConstantType];
}

const FT4ConstantDataRow& FT4EngineConstants::GetConstantData(
	ET4EngineConstantType InConstantType,
	FName InName
) // #71, #92
{
	TArray<FT4ConstantDataRow>& ConstantDataSelected = GetConstantDatas(InConstantType);
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

void FT4EngineConstants::LoadFromDataTable(ET4EngineConstantType InConstantType)
{
	check(DataTable[InConstantType].IsValid());

	int32 ItemCount = 0;
	ET4ConstantValueType ValueType = GConstantValueTypes[InConstantType];
	switch (ValueType)
	{
		case ET4ConstantValueType::ConstantValue_Name:
			{
				TArray<FT4ConstantNameTableRow*> AllRowDatas;
				DataTable[InConstantType]->GetAllRows<FT4ConstantNameTableRow>(TEXT("T4ConstantNameTableRow"), AllRowDatas);
				if (0 == AllRowDatas.Num())
				{
					UE_LOG(
						LogT4Engine,
						Warning,
						TEXT("FT4EngineConstants : Failed to empty UDataTable.")
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
					ConstantDatas[InConstantType].Add(NewItemInfo);
				}
			}
			break;

		case ET4ConstantValueType::ConstantValue_Float:
			{
				TArray<FT4ConstantFloatValueTableRow*> AllRowDatas;
				DataTable[InConstantType]->GetAllRows<FT4ConstantFloatValueTableRow>(TEXT("T4ConstantFloatValueTableRow"), AllRowDatas);
				if (0 == AllRowDatas.Num())
				{
					UE_LOG(
						LogT4Engine,
						Warning,
						TEXT("FT4EngineConstants : Failed to empty UDataTable.")
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
					ConstantDatas[InConstantType].Add(NewItemInfo);
				}
			}
			break;

		case ET4ConstantValueType::ConstantValue_Int:
			{
				TArray<FT4ConstantIntValueTableRow*> AllRowDatas;
				DataTable[InConstantType]->GetAllRows<FT4ConstantIntValueTableRow>(TEXT("T4ConstantIntValueTableRow"), AllRowDatas);
				if (0 == AllRowDatas.Num())
				{
					UE_LOG(
						LogT4Engine,
						Warning,
						TEXT("FT4EngineConstants : Failed to empty UDataTable.")
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
					ConstantDatas[InConstantType].Add(NewItemInfo);
				}
			}
			break;

		default:
			{
				UE_LOG(
					LogT4Engine,
					Error,
					TEXT("FT4EngineConstants : Unknown Constants Value type '%u'"),
					uint8(ValueType)
				);
			}
			break;
	};
}

#if WITH_EDITOR
void FT4EngineConstants::OnTableChanged()
{
	for (uint32 idx = 0; idx < ET4EngineConstantType::Nums; ++idx)
	{
		if (!DataTable[idx].IsValid())
		{
			return;
		}
		ConstantDatas[idx].Empty();
		LoadFromDataTable((ET4EngineConstantType)idx);
	}
}
#endif

static FT4EngineConstants GT4EngineDataTable;
FT4EngineConstants* T4EngineConstantsGet()
{
	return &GT4EngineDataTable;
}
