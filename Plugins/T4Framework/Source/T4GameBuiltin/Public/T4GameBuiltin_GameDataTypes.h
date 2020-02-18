// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4Engine/Public/Action/T4ActionKey.h" // #48
#include "T4Framework/Public/T4FrameworkGameplay.h" // #114
#include "T4GameBuiltin_GameDataTypes.generated.h"

/**
  * #48
 */
// #T4_ADD_GAME_DATATABLE
UENUM()
enum class ET4GameBuiltin_GameDataType : uint8
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
enum class ET4GameBuiltin_GameDataValidation : uint8
{
	Checkit,
	NotSet,
	Pass,
	Fail,

	Nums UMETA(Hidden),
};

USTRUCT()
struct FT4GameBuiltin_GameDataID
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, meta = (DisplayName = "Data Type"))
	ET4GameBuiltin_GameDataType Type;

	UPROPERTY(EditAnywhere)
	FName RowName;

	UPROPERTY(Transient)
	ET4GameBuiltin_GameDataValidation ValidationResult; // #118

public:
	FT4GameBuiltin_GameDataID()
		: Type(ET4GameBuiltin_GameDataType::Nums)
		, RowName(NAME_None)
		, ValidationResult(ET4GameBuiltin_GameDataValidation::Checkit)
	{
	}

	FT4GameBuiltin_GameDataID(const ET4GameBuiltin_GameDataType InType)
		: Type(InType)
		, RowName(NAME_None)
		, ValidationResult(ET4GameBuiltin_GameDataValidation::Checkit)
	{
	}

	FT4GameBuiltin_GameDataID(const ET4GameBuiltin_GameDataType InType, const TCHAR* InRowName)
		: Type(InType)
		, RowName(InRowName)
		, ValidationResult(ET4GameBuiltin_GameDataValidation::Checkit)
	{
	}

	FT4GameBuiltin_GameDataID(const ET4GameBuiltin_GameDataType InType, const FName& InRowName)
		: Type(InType)
		, RowName(InRowName)
		, ValidationResult(ET4GameBuiltin_GameDataValidation::Checkit)
	{
	}

	FT4GameBuiltin_GameDataID(const ET4GameBuiltin_GameDataType InType, const FString& InRowName)
		: Type(InType)
		, RowName(*InRowName)
		, ValidationResult(ET4GameBuiltin_GameDataValidation::Checkit)
	{
	}

	FT4GameBuiltin_GameDataID(const FT4GameBuiltin_GameDataID& InGameDataID)
		: Type(InGameDataID.Type)
		, RowName(InGameDataID.RowName)
		, ValidationResult(ET4GameBuiltin_GameDataValidation::Checkit)
	{
	}

	FORCEINLINE FT4GameBuiltin_GameDataID operator=(const FT4GameBuiltin_GameDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameBuiltin_GameDataID operator=(const FString& InRhs)
	{
		RowName = *(InRhs);
		return *this;
	}

	FORCEINLINE bool operator==(const FT4GameBuiltin_GameDataID& InRhs) const
	{
		return (RowName == InRhs.RowName && Type == InRhs.Type) ? true : false;
	}

	FORCEINLINE bool operator!=(const FT4GameBuiltin_GameDataID& InRhs) const
	{
		return (RowName != InRhs.RowName || Type != InRhs.Type) ? true : false;
	}

	FORCEINLINE friend uint32 GetTypeHash(const FT4GameBuiltin_GameDataID& InRhs)
	{
		return HashCombine(GetTypeHash(InRhs.Type), GetTypeHash(InRhs.RowName.ToString()));
	}

	FORCEINLINE bool IsValid() const
	{
		return (ET4GameBuiltin_GameDataType::Nums != Type && RowName != NAME_None) ? true : false;
	}

	FORCEINLINE bool CheckType(const ET4GameBuiltin_GameDataType InType) const
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

	FORCEINLINE const TCHAR* ToTypeString() const
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
		static_assert(UE_ARRAY_COUNT(GameDataTypeStrings) == (uint8)(ET4GameBuiltin_GameDataType::Nums) + 1, "GameDataType doesn't match!");
		check(uint8(Type) < UE_ARRAY_COUNT(GameDataTypeStrings));
		return GameDataTypeStrings[uint8(Type)];
	}

	FORCEINLINE FString ToString() const
	{
		return FString::Printf(
			TEXT("FT4GameBuiltin_GameDataID:%s=%s"),
			*(ToNameString()),
			ToTypeString()
		);
	}
};

static const FT4GameBuiltin_GameDataID T4Const_InvalidGameDataID;

USTRUCT()
struct FT4GameBuiltin_GameWorldDataID : public FT4GameBuiltin_GameDataID
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameBuiltin_GameWorldDataID()
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::World)
	{
	}

	FT4GameBuiltin_GameWorldDataID(const FName& InRowName)
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::World, InRowName)
	{
	}

	FORCEINLINE FT4GameBuiltin_GameDataID operator=(const FT4GameBuiltin_GameWorldDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameBuiltin_GameWorldDataID operator=(const FT4GameBuiltin_GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

USTRUCT()
struct FT4GameBuiltin_GameNPCDataID : public FT4GameBuiltin_GameDataID
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameBuiltin_GameNPCDataID()
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::NPC)
	{
	}

	FT4GameBuiltin_GameNPCDataID(const FName& InRowName)
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::NPC, InRowName)
	{
	}

	FORCEINLINE FT4GameBuiltin_GameDataID operator=(const FT4GameBuiltin_GameNPCDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameBuiltin_GameNPCDataID operator=(const FT4GameBuiltin_GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

USTRUCT()
struct FT4GameBuiltin_GameSkillDataID : public FT4GameBuiltin_GameDataID
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameBuiltin_GameSkillDataID()
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::Skill)
	{
	}

	FT4GameBuiltin_GameSkillDataID(const FName& InRowName)
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::Skill, InRowName)
	{
	}

	FORCEINLINE FT4GameBuiltin_GameDataID operator=(const FT4GameBuiltin_GameSkillDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameBuiltin_GameSkillDataID operator=(const FT4GameBuiltin_GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

USTRUCT()
struct FT4GameBuiltin_GameEffectDataID : public FT4GameBuiltin_GameDataID
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameBuiltin_GameEffectDataID()
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::Effect)
	{
	}

	FT4GameBuiltin_GameEffectDataID(const FName& InRowName)
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::Effect, InRowName)
	{
	}

	FORCEINLINE FT4GameBuiltin_GameDataID operator=(const FT4GameBuiltin_GameEffectDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameBuiltin_GameEffectDataID operator=(const FT4GameBuiltin_GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

// #50
USTRUCT()
struct FT4GameBuiltin_GameWeaponDataID : public FT4GameBuiltin_GameDataID
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameBuiltin_GameWeaponDataID()
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::Weapon)
	{
	}

	FT4GameBuiltin_GameWeaponDataID(const FName& InRowName)
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::Weapon, InRowName)
	{
	}

	FORCEINLINE FT4GameBuiltin_GameDataID operator=(const FT4GameBuiltin_GameWeaponDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameBuiltin_GameWeaponDataID operator=(const FT4GameBuiltin_GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

// #50
USTRUCT()
struct FT4GameBuiltin_GameSkillSetDataID : public FT4GameBuiltin_GameDataID
{
	GENERATED_USTRUCT_BODY()

public:
	FT4GameBuiltin_GameSkillSetDataID()
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::SkillSet)
	{
	}

	FT4GameBuiltin_GameSkillSetDataID(const FName& InRowName)
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::SkillSet, InRowName)
	{
	}

	FORCEINLINE FT4GameBuiltin_GameDataID operator=(const FT4GameBuiltin_GameSkillSetDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameBuiltin_GameSkillSetDataID operator=(const FT4GameBuiltin_GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

// #114
USTRUCT()
struct FT4GameBuiltin_GamePlayerStatDataID : public FT4GameBuiltin_GameDataID
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Transient)
	ET4GameplayStatCategory StatCategory; // #114

public:
	FT4GameBuiltin_GamePlayerStatDataID()
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::Stat)
		, StatCategory(ET4GameplayStatCategory::Player)
	{
	}

	FT4GameBuiltin_GamePlayerStatDataID(const FName& InRowName)
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::Stat, InRowName)
		, StatCategory(ET4GameplayStatCategory::Player)
	{
	}

	FORCEINLINE FT4GameBuiltin_GameDataID operator=(const FT4GameBuiltin_GamePlayerStatDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameBuiltin_GamePlayerStatDataID operator=(const FT4GameBuiltin_GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

// #114
USTRUCT()
struct FT4GameBuiltin_GameNPCStatDataID : public FT4GameBuiltin_GameDataID
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Transient)
	ET4GameplayStatCategory StatCategory; // #114

public:
	FT4GameBuiltin_GameNPCStatDataID()
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::Stat)
		, StatCategory(ET4GameplayStatCategory::NPC)
	{
	}

	FT4GameBuiltin_GameNPCStatDataID(const FName& InRowName)
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::Stat, InRowName)
		, StatCategory(ET4GameplayStatCategory::NPC)
	{
	}

	FORCEINLINE FT4GameBuiltin_GameDataID operator=(const FT4GameBuiltin_GameNPCStatDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameBuiltin_GameNPCStatDataID operator=(const FT4GameBuiltin_GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

// #114
USTRUCT()
struct FT4GameBuiltin_GameItemStatDataID : public FT4GameBuiltin_GameDataID
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Transient)
	ET4GameplayStatCategory StatCategory; // #114

public:
	FT4GameBuiltin_GameItemStatDataID()
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::Stat)
		, StatCategory(ET4GameplayStatCategory::Item)
	{
	}

	FT4GameBuiltin_GameItemStatDataID(const FName& InRowName)
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::Stat, InRowName)
		, StatCategory(ET4GameplayStatCategory::Item)
	{
	}

	FORCEINLINE FT4GameBuiltin_GameDataID operator=(const FT4GameBuiltin_GameItemStatDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameBuiltin_GameItemStatDataID operator=(const FT4GameBuiltin_GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

// #114
USTRUCT()
struct FT4GameBuiltin_GameSkillStatDataID : public FT4GameBuiltin_GameDataID
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Transient)
	ET4GameplayStatCategory StatCategory; // #114

public:
	FT4GameBuiltin_GameSkillStatDataID()
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::Stat)
		, StatCategory(ET4GameplayStatCategory::Skill)
	{
	}

	FT4GameBuiltin_GameSkillStatDataID(const FName& InRowName)
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::Stat, InRowName)
		, StatCategory(ET4GameplayStatCategory::Skill)
	{
	}

	FORCEINLINE FT4GameBuiltin_GameDataID operator=(const FT4GameBuiltin_GameSkillStatDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameBuiltin_GameSkillStatDataID operator=(const FT4GameBuiltin_GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};

// #114
USTRUCT()
struct FT4GameBuiltin_GameEffectStatDataID : public FT4GameBuiltin_GameDataID
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Transient)
	ET4GameplayStatCategory StatCategory; // #114

public:
	FT4GameBuiltin_GameEffectStatDataID()
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::Stat)
		, StatCategory(ET4GameplayStatCategory::Effect)
	{
	}

	FT4GameBuiltin_GameEffectStatDataID(const FName& InRowName)
		: FT4GameBuiltin_GameDataID(ET4GameBuiltin_GameDataType::Stat, InRowName)
		, StatCategory(ET4GameplayStatCategory::Effect)
	{
	}

	FORCEINLINE FT4GameBuiltin_GameDataID operator=(const FT4GameBuiltin_GameEffectStatDataID& InRhs)
	{
		Type = InRhs.Type;
		RowName = InRhs.RowName;
		return *this;
	}

	FORCEINLINE FT4GameBuiltin_GameEffectStatDataID operator=(const FT4GameBuiltin_GameDataID& InRhs)
	{
		RowName = InRhs.RowName;
		return *this;
	}
};