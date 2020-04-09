// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GamePacketCS.h"
#include "T4GamePacketCS_Status.generated.h"

/**
  *
 */
 // #T4_ADD_PACKET_TAG_CS

// ET4GamePacketCS::Stance // #73
// ET4GamePacketCS::Posture // #106
// ET4GamePacketCS::EquipItem
// ET4GamePacketCS::UnequipItem
// ET4GamePacketCS::ExchangeItem

USTRUCT()
struct FT4GamePacketCS_Stance : public FT4GamePacketCS_Base // #73
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FName StanceName;

public:
	FT4GamePacketCS_Stance()
		: FT4GamePacketCS_Base(ET4GamePacketCS::Stance)
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
struct FT4GamePacketCS_Posture : public FT4GamePacketCS_Base // #106
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FName PostureName;

public:
	FT4GamePacketCS_Posture()
		: FT4GamePacketCS_Base(ET4GamePacketCS::Posture)
		, PostureName(NAME_None)
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
		return FString(TEXT("CS_Packet:Posture"));
	}
};

USTRUCT()
struct FT4GamePacketCS_EquipItem : public FT4GamePacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FT4GameDataID ItemWeaponDataID;

	UPROPERTY(VisibleAnywhere)
	bool bMainWeapon; // #48

public:
	FT4GamePacketCS_EquipItem()
		: FT4GamePacketCS_Base(ET4GamePacketCS::EquipItem)
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
struct FT4GamePacketCS_UnequipItem : public FT4GamePacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FT4GameDataID ItemWeaponDataID;

	UPROPERTY(VisibleAnywhere)
	bool bMainWeapon; // #48

public:
	FT4GamePacketCS_UnequipItem()
		: FT4GamePacketCS_Base(ET4GamePacketCS::UnequipItem)
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
struct FT4GamePacketCS_ExchangeItem : public FT4GamePacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FT4GameDataID ItemCostumeDataID;

public:
	FT4GamePacketCS_ExchangeItem()
		: FT4GamePacketCS_Base(ET4GamePacketCS::ExchangeItem)
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