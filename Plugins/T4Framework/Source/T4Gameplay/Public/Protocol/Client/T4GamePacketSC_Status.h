// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GamePacketSC.h"
#include "T4GamePacketSC_Status.generated.h"

/**
  *
 */
 // #T4_ADD_PACKET_TAG_SC

// ET4GamePacketSC::Stance // #73
// ET4GamePacketSC::SubStance // #106
// ET4GamePacketSC::EquipItem
// ET4GamePacketSC::UnequipItem
// ET4GamePacketSC::ExchangeItem
// ET4GamePacketSC::Die // #76
// ET4GamePacketSC::Resurrect // #76

USTRUCT()
struct FT4GamePacketSC_Stance : public FT4GamePacketSC_Base // #73
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FName StanceName; // TODO : Stance Table?

public:
	FT4GamePacketSC_Stance()
		: FT4GamePacketSC_Base(ET4GamePacketSC::Stance)
		, StanceName(NAME_None)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!ObjectID.IsValid())
		{
			OutMsg = TEXT("Invalid ObjectID");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:Stance"));
	}
};

USTRUCT()
struct FT4GamePacketSC_SubStance : public FT4GamePacketSC_Base // #106
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FName SubStanceName; // TODO : SubStance Table?

public:
	FT4GamePacketSC_SubStance()
		: FT4GamePacketSC_Base(ET4GamePacketSC::SubStance)
		, SubStanceName(NAME_None)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!ObjectID.IsValid())
		{
			OutMsg = TEXT("Invalid ObjectID");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:SubStance"));
	}
};

USTRUCT()
struct FT4GamePacketSC_EquipItem : public FT4GamePacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FT4GameDataID ItemWeaponDataID;

	UPROPERTY(VisibleAnywhere)
	bool bMainWeapon; // #48

	UPROPERTY(VisibleAnywhere)
	FT4GameDataID UnequipItemWeaponDataID; // #114, #116 : 이전에 장착한 무기가 있다면 해제 후 장착

public:
	FT4GamePacketSC_EquipItem()
		: FT4GamePacketSC_Base(ET4GamePacketSC::EquipItem)
		, bMainWeapon(false)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!ObjectID.IsValid())
		{
			OutMsg = TEXT("Invalid ObjectID");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:Equip"));
	}
};

USTRUCT()
struct FT4GamePacketSC_UnequipItem : public FT4GamePacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FT4GameDataID ItemWeaponDataID;

	UPROPERTY(VisibleAnywhere)
	bool bMainWeapon; // #48

public:
	FT4GamePacketSC_UnequipItem()
		: FT4GamePacketSC_Base(ET4GamePacketSC::UnequipItem)
		, bMainWeapon(false)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!ObjectID.IsValid())
		{
			OutMsg = TEXT("Invalid ObjectID");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:Unequip"));
	}
};

USTRUCT()
struct FT4GamePacketSC_ExchangeItem : public FT4GamePacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FT4GameDataID ItemCostumeDataID;

public:
	FT4GamePacketSC_ExchangeItem()
		: FT4GamePacketSC_Base(ET4GamePacketSC::ExchangeItem)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!ObjectID.IsValid())
		{
			OutMsg = TEXT("Invalid ObjectID");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:Exchange"));
	}
};

// #76
USTRUCT()
struct FT4GamePacketSC_Die : public FT4GamePacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FName ReactionName;

	UPROPERTY(VisibleAnywhere)
	FT4ObjectID AttackerObjectID;

public:
	FT4GamePacketSC_Die()
		: FT4GamePacketSC_Base(ET4GamePacketSC::Die)
		, ReactionName(NAME_None)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!ObjectID.IsValid())
		{
			OutMsg = TEXT("Invalid ObjectID");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:Die"));
	}
};

// #76
USTRUCT()
struct FT4GamePacketSC_Resurrect : public FT4GamePacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

public:
	FT4GamePacketSC_Resurrect()
		: FT4GamePacketSC_Base(ET4GamePacketSC::Resurrect)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!ObjectID.IsValid())
		{
			OutMsg = TEXT("Invalid ObjectID");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("SC_Packet:Resurrect"));
	}
};