// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/Action/T4ActionKey.h" // #48
#include "T4Framework/Public/T4FrameworkGameplay.h" // #114
#include "T4GameDataTypes.generated.h"

/**
  * #48
 */
// #T4_ADD_GAME_DATATABLE
UENUM()
enum class ET4GameDataType : uint8
{
	Master,

	World,
	Player,
	NPC,

	Weapon, // #48
	Costume, // #48

	Skill,
	SkillSet, // #50
	Effect,

	Stat, // #114
	Experience, // #114

	Nums,
};

UENUM()
enum class ET4GameStatCategory : uint8 // #114
{
	Player,
	NPC,
	Item,

	Skill,
	Effect,

	None UMETA(Hidden),
};

UENUM()
enum class ET4GameStatLevel : uint8 // #114
{
	Level_1,
	Level_2,
	Level_3,
	Level_4,
	Level_5,
	Level_6,
	Level_7,
	Level_8,
	Level_9,
	Level_10,

	Max UMETA(Hidden),
};

UENUM()
enum class ET4GameSkillSetUseType : uint8 // #116
{
	Primary,
	Sequential,
	HotKey,

	None UMETA(Hidden),
};

UENUM()
enum class ET4GameEnemyType : uint8 // #104
{
	None,

	Player,
	Hostile_Tribe,

	All,

	NoEnemy,
};

UENUM()
enum class ET4GameDataValidation : uint8
{
	Checkit,
	NotSet,
	Pass,
	Fail,

	Nums UMETA(Hidden),
};

USTRUCT()
struct FT4GameDataID
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, meta = (DisplayName = "Data Type"))
	ET4GameDataType Type;

	UPROPERTY(EditAnywhere)
	FName RowName;

	UPROPERTY(Transient)
	ET4GameDataValidation ValidationResult; // #118

public:
	FT4GameDataID()
		: Type(ET4GameDataType::Nums)
		, RowName(NAME_None)
		, ValidationResult(ET4GameDataValidation::Checkit)
	{
	}

	FT4GameDataID(const ET4GameDataType InType)
		: Type(InType)
		, RowName(NAME_None)
		, ValidationResult(ET4GameDataValidation::Checkit)
	{
	}

	FT4GameDataID(const ET4GameDataType InType, const TCHAR* InRowName)
		: Type(InType)
		, RowName(InRowName)
		, ValidationResult(ET4GameDataValidation::Checkit)
	{
	}

	FT4GameDataID(const ET4GameDataType InType, const FName& InRowName)
		: Type(InType)
		, RowName(InRowName)
		, ValidationResult(ET4GameDataValidation::Checkit)
	{
	}

	FT4GameDataID(const ET4GameDataType InType, const FString& InRowName)
		: Type(InType)
		, RowName(*InRowName)
		, ValidationResult(ET4GameDataValidation::Checkit)
	{
	}

	FT4GameDataID(const FT4GameDataID& InGameDataID)
		: Type(InGameDataID.Type)
		, RowName(InGameDataID.RowName)
		, ValidationResult(ET4GameDataValidation::Checkit)
	{
	}

	FORCEINLINE FT4GameDataID operator=(const FT4GameDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameDataID operator=(const FString& InRhs)
	{
		RowName = *(InRhs);
		return *this;
	}

	FORCEINLINE bool operator==(const FT4GameDataID& InRhs) const
	{
		return (RowName == InRhs.RowName && Type == InRhs.Type) ? true : false;
	}

	FORCEINLINE bool operator!=(const FT4GameDataID& InRhs) const
	{
		return (RowName != InRhs.RowName || Type != InRhs.Type) ? true : false;
	}

	FORCEINLINE friend uint32 GetTypeHash(const FT4GameDataID& InRhs)
	{
		return HashCombine(GetTypeHash(InRhs.Type), GetTypeHash(InRhs.RowName.ToString()));
	}

	FORCEINLINE bool IsValid() const
	{
		return (ET4GameDataType::Nums != Type && RowName != NAME_None) ? true : false;
	}

	FORCEINLINE bool CheckType(const ET4GameDataType InType) const
	{
		return (Type == InType) ? true : false;
	}

	FORCEINLINE FT4ActionKey ToPrimaryActionKey() const // #48
	{
		return FT4ActionKey(RowName, true);
	}

	FORCEINLINE FT4ActionKey ToOverlapActionKey() const // #49
	{
		return FT4ActionKey(RowName, false);
	}

	FORCEINLINE FString ToNameString() const
	{
		return RowName.ToString();
	}

	static const TCHAR* ToTypeString(ET4GameDataType InType)
	{
		static const TCHAR* GameDataTypeStrings[] =
		{
			TEXT("Master"),
			TEXT("World"),
			TEXT("Player"),
			TEXT("NPC"),
			TEXT("Weapon"), // #37
			TEXT("Costume"),
			TEXT("Skill"),
			TEXT("SkillSet"), // #50
			TEXT("Effect"),
			TEXT("Stat"), // #114
			TEXT("Experience"), // #114
			TEXT("None"),
		};
		static_assert(UE_ARRAY_COUNT(GameDataTypeStrings) == (uint8)(ET4GameDataType::Nums) + 1, "GameDataType doesn't match!");
		check(uint8(InType) < UE_ARRAY_COUNT(GameDataTypeStrings));
		return GameDataTypeStrings[uint8(InType)];
	}

	FORCEINLINE const TCHAR* ToTypeString() const
	{
		return ToTypeString(Type);
	}

	FORCEINLINE FString ToString() const
	{
		return FString::Printf(
			TEXT("FT4GameDataID:%s=%s"),
			*(ToNameString()),
			ToTypeString()
		);
	}
};

static const FT4GameDataID T4Const_InvalidGameDataID;

USTRUCT()
struct FT4GameWorldDataID : public FT4GameDataID
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameWorldDataID()
		: FT4GameDataID(ET4GameDataType::World)
	{
	}

	FT4GameWorldDataID(const FName& InRowName)
		: FT4GameDataID(ET4GameDataType::World, InRowName)
	{
	}

	FORCEINLINE FT4GameDataID operator=(const FT4GameWorldDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameWorldDataID operator=(const FT4GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

USTRUCT()
struct FT4GameNPCDataID : public FT4GameDataID
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameNPCDataID()
		: FT4GameDataID(ET4GameDataType::NPC)
	{
	}

	FT4GameNPCDataID(const FName& InRowName)
		: FT4GameDataID(ET4GameDataType::NPC, InRowName)
	{
	}

	FORCEINLINE FT4GameDataID operator=(const FT4GameNPCDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameNPCDataID operator=(const FT4GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

USTRUCT()
struct FT4GameSkillDataID : public FT4GameDataID
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameSkillDataID()
		: FT4GameDataID(ET4GameDataType::Skill)
	{
	}

	FT4GameSkillDataID(const FName& InRowName)
		: FT4GameDataID(ET4GameDataType::Skill, InRowName)
	{
	}

	FORCEINLINE FT4GameDataID operator=(const FT4GameSkillDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameSkillDataID operator=(const FT4GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

USTRUCT()
struct FT4GameEffectDataID : public FT4GameDataID
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameEffectDataID()
		: FT4GameDataID(ET4GameDataType::Effect)
	{
	}

	FT4GameEffectDataID(const FName& InRowName)
		: FT4GameDataID(ET4GameDataType::Effect, InRowName)
	{
	}

	FORCEINLINE FT4GameDataID operator=(const FT4GameEffectDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameEffectDataID operator=(const FT4GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

// #50
USTRUCT()
struct FT4GameWeaponDataID : public FT4GameDataID
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameWeaponDataID()
		: FT4GameDataID(ET4GameDataType::Weapon)
	{
	}

	FT4GameWeaponDataID(const FName& InRowName)
		: FT4GameDataID(ET4GameDataType::Weapon, InRowName)
	{
	}

	FORCEINLINE FT4GameDataID operator=(const FT4GameWeaponDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameWeaponDataID operator=(const FT4GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

// #50
USTRUCT()
struct FT4GameSkillSetDataID : public FT4GameDataID
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameSkillSetDataID()
		: FT4GameDataID(ET4GameDataType::SkillSet)
	{
	}

	FT4GameSkillSetDataID(const FName& InRowName)
		: FT4GameDataID(ET4GameDataType::SkillSet, InRowName)
	{
	}

	FORCEINLINE FT4GameDataID operator=(const FT4GameSkillSetDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameSkillSetDataID operator=(const FT4GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

// #114
USTRUCT()
struct FT4GamePlayerStatDataID : public FT4GameDataID
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Transient)
	ET4GameStatCategory StatCategory; // #114

public:
	FT4GamePlayerStatDataID()
		: FT4GameDataID(ET4GameDataType::Stat)
		, StatCategory(ET4GameStatCategory::Player)
	{
	}

	FT4GamePlayerStatDataID(const FName& InRowName)
		: FT4GameDataID(ET4GameDataType::Stat, InRowName)
		, StatCategory(ET4GameStatCategory::Player)
	{
	}

	FORCEINLINE FT4GameDataID operator=(const FT4GamePlayerStatDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GamePlayerStatDataID operator=(const FT4GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

// #114
USTRUCT()
struct FT4GameNPCStatDataID : public FT4GameDataID
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Transient)
	ET4GameStatCategory StatCategory; // #114

public:
	FT4GameNPCStatDataID()
		: FT4GameDataID(ET4GameDataType::Stat)
		, StatCategory(ET4GameStatCategory::NPC)
	{
	}

	FT4GameNPCStatDataID(const FName& InRowName)
		: FT4GameDataID(ET4GameDataType::Stat, InRowName)
		, StatCategory(ET4GameStatCategory::NPC)
	{
	}

	FORCEINLINE FT4GameDataID operator=(const FT4GameNPCStatDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameNPCStatDataID operator=(const FT4GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

// #114
USTRUCT()
struct FT4GameItemStatDataID : public FT4GameDataID
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Transient)
	ET4GameStatCategory StatCategory; // #114

public:
	FT4GameItemStatDataID()
		: FT4GameDataID(ET4GameDataType::Stat)
		, StatCategory(ET4GameStatCategory::Item)
	{
	}

	FT4GameItemStatDataID(const FName& InRowName)
		: FT4GameDataID(ET4GameDataType::Stat, InRowName)
		, StatCategory(ET4GameStatCategory::Item)
	{
	}

	FORCEINLINE FT4GameDataID operator=(const FT4GameItemStatDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameItemStatDataID operator=(const FT4GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

// #114
USTRUCT()
struct FT4GameSkillStatDataID : public FT4GameDataID
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Transient)
	ET4GameStatCategory StatCategory; // #114

public:
	FT4GameSkillStatDataID()
		: FT4GameDataID(ET4GameDataType::Stat)
		, StatCategory(ET4GameStatCategory::Skill)
	{
	}

	FT4GameSkillStatDataID(const FName& InRowName)
		: FT4GameDataID(ET4GameDataType::Stat, InRowName)
		, StatCategory(ET4GameStatCategory::Skill)
	{
	}

	FORCEINLINE FT4GameDataID operator=(const FT4GameSkillStatDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameSkillStatDataID operator=(const FT4GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

// #114
USTRUCT()
struct FT4GameEffectStatDataID : public FT4GameDataID
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Transient)
	ET4GameStatCategory StatCategory; // #114

public:
	FT4GameEffectStatDataID()
		: FT4GameDataID(ET4GameDataType::Stat)
		, StatCategory(ET4GameStatCategory::Effect)
	{
	}

	FT4GameEffectStatDataID(const FName& InRowName)
		: FT4GameDataID(ET4GameDataType::Stat, InRowName)
		, StatCategory(ET4GameStatCategory::Effect)
	{
	}

	FORCEINLINE FT4GameDataID operator=(const FT4GameEffectStatDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameEffectStatDataID operator=(const FT4GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};