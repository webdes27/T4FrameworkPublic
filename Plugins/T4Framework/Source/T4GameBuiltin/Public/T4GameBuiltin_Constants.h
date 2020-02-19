// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
  * #114
 */
enum ET4GameBuiltin_ConstantTable
{
	Races,

	EGB_Const_Nums,
};

struct T4GAMEBUILTIN_API FT4GameBuiltin_ConstantDataRow
{
	FT4GameBuiltin_ConstantDataRow()
		: Name(NAME_None)
	{
	}
	FName Name;
	union 
	{
		float FloatValue;
		float IntValue;
	};
#if WITH_EDITOR
	FString Description;
	int32 SortOrder; // UI
#endif
};

class UDataTable;
class T4GAMEBUILTIN_API IT4GameBuiltin_ConstantTableManager
{
public:
	virtual ~IT4GameBuiltin_ConstantTableManager() {}

	virtual void Reset() = 0;

	virtual bool LoadEngineTable(ET4GameBuiltin_ConstantTable InTable, const FSoftObjectPath& InTablePath) = 0; // #90

	virtual TArray<FT4GameBuiltin_ConstantDataRow>& GetConstantDatas(ET4GameBuiltin_ConstantTable InTable) = 0;
	virtual const FT4GameBuiltin_ConstantDataRow& GetConstantData(ET4GameBuiltin_ConstantTable InTable, FName InName) = 0; // #71
};

class UObject;
namespace T4GameBuiltinConstant
{
	T4GAMEBUILTIN_API bool Initailize();
	T4GAMEBUILTIN_API void Finalize();

	T4GAMEBUILTIN_API IT4GameBuiltin_ConstantTableManager* GetTableManager();
}