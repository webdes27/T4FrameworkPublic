// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameBuiltin_PacketCS.h"
#include "T4GameBuiltin_PacketCS_Status.generated.h"

/**
  *
 */
 // #T4_ADD_PACKET_TAG_CS

// ET4GameBuiltin_PacketCS::Stance // #73
// ET4GameBuiltin_PacketCS::SubStance // #106
// ET4GameBuiltin_PacketCS::EquipItem
// ET4GameBuiltin_PacketCS::UnequipItem
// ET4GameBuiltin_PacketCS::ExchangeItem

USTRUCT()
struct FT4GameBuiltin_PacketCS_Stance : public FT4GameBuiltin_PacketCS_Base // #73
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FName StanceName;

public:
	FT4GameBuiltin_PacketCS_Stance()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::Stance)
		, StanceName(NAME_None)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!SenderID.IsValid())
		{
			OutMsg = TEXT("Invalid Send ObjectID!");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:Stance"));
	}
};

USTRUCT()
struct FT4GameBuiltin_PacketCS_SubStance : public FT4GameBuiltin_PacketCS_Base // #106
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FName SubStanceName;

public:
	FT4GameBuiltin_PacketCS_SubStance()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::SubStance)
		, SubStanceName(NAME_None)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!SenderID.IsValid())
		{
			OutMsg = TEXT("Invalid Send ObjectID!");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:SubStance"));
	}
};

USTRUCT()
struct FT4GameBuiltin_PacketCS_EquipItem : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID ItemWeaponDataID;

	UPROPERTY(VisibleAnywhere)
	bool bMainWeapon; // #48

public:
	FT4GameBuiltin_PacketCS_EquipItem()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::EquipItem)
		, bMainWeapon(false)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!SenderID.IsValid())
		{
			OutMsg = TEXT("Invalid Send ObjectID!");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:Equip"));
	}
};

USTRUCT()
struct FT4GameBuiltin_PacketCS_UnequipItem : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID ItemWeaponDataID;

	UPROPERTY(VisibleAnywhere)
	bool bMainWeapon; // #48

public:
	FT4GameBuiltin_PacketCS_UnequipItem()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::UnequipItem)
		, bMainWeapon(false)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!SenderID.IsValid())
		{
			OutMsg = TEXT("Invalid Send ObjectID!");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:Unequip"));
	}
};

USTRUCT()
struct FT4GameBuiltin_PacketCS_ExchangeItem : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID ItemCostumeDataID;

public:
	FT4GameBuiltin_PacketCS_ExchangeItem()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::ExchangeItem)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!SenderID.IsValid())
		{
			OutMsg = TEXT("Invalid Send ObjectID!");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:Exchange"));
	}
};