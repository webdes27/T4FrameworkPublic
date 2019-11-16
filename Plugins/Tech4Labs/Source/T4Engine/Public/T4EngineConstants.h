// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
  * #39, #90
 */
enum ET4EngineConstantType
{
    // Engine
    Engine_Start,

    // {

		MapZone = Engine_Start, // #92 : Int Value
        TimeTag, // #90 : Float Value

    // }

	Engine_End,

#if WITH_EDITOR

    // Editor only
    Editor_Start = Engine_End,

    // {

        BlendSpace = Editor_Start,
        DefaultSection, // #38
        AdditiveSection,
        SkillSection,

        ActionPoint, // #47

        EquipPoint, // #72
        CompositePart, // #71

        LayerTag, // #74

        Stance, // #73

        Reaction, // #76

    // }

    Editor_End,

	Nums = Editor_End

#else

	Nums = Engine_End

#endif
};

struct T4ENGINE_API FT4ConstantDataRow
{
	FT4ConstantDataRow()
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
class T4ENGINE_API FT4EngineConstants
{
public:
	FT4EngineConstants();
	~FT4EngineConstants();

	void Reset();

	bool LoadEngineConstants(ET4EngineConstantType InConstantType, const FSoftObjectPath& InConstantTablePath);  // #90

#if WITH_EDITOR
	bool LoadEditorConstants(ET4EngineConstantType InConstantType, const FSoftObjectPath& InConstantTablePath);
#endif

	TArray<FT4ConstantDataRow>& GetConstantDatas(ET4EngineConstantType InConstantType);
	const FT4ConstantDataRow& GetConstantData(ET4EngineConstantType InConstantType, FName InName); // #71

private:
	bool LoadTable(ET4EngineConstantType InConstantType, const FSoftObjectPath& InConstantTablePath); // #90

	void LoadFromDataTable(ET4EngineConstantType InConstantType);

#if WITH_EDITOR
	void OnTableChanged();
#endif

private:
	TArray<FT4ConstantDataRow> ConstantDatas[ET4EngineConstantType::Nums];
	TWeakObjectPtr<UDataTable> DataTable[ET4EngineConstantType::Nums];
};

T4ENGINE_API FT4EngineConstants* T4EngineConstantsGet();