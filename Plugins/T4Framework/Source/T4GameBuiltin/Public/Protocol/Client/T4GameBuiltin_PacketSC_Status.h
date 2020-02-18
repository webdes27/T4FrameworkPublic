// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameBuiltin_PacketSC.h"
#include "T4GameBuiltin_PacketSC_Status.generated.h"

/**
  *
 */
 // #T4_ADD_PACKET_TAG_SC

// ET4GameBuiltin_PacketSC::Stance // #73
// ET4GameBuiltin_PacketSC::SubStance // #106
// ET4GameBuiltin_PacketSC::EquipItem
// ET4GameBuiltin_PacketSC::UnequipItem
// ET4GameBuiltin_PacketSC::ExchangeItem
// ET4GameBuiltin_PacketSC::Die // #76
// ET4GameBuiltin_PacketSC::Resurrect // #76

USTRUCT()
struct FT4GameBuiltin_PacketSC_Stance : public FT4GameBuiltin_PacketSC_Base // #73
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FName StanceName; // TODO : Stance Table?

public:
	FT4GameBuiltin_PacketSC_Stance()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::Stance)
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
struct FT4GameBuiltin_PacketSC_SubStance : public FT4GameBuiltin_PacketSC_Base // #106
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FName SubStanceName; // TODO : SubStance Table?

public:
	FT4GameBuiltin_PacketSC_SubStance()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::SubStance)
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
struct FT4GameBuiltin_PacketSC_EquipItem : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID ItemWeaponDataID;

	UPROPERTY(VisibleAnywhere)
	bool bMainWeapon; // #48

	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID UnequipItemWeaponDataID; // #114, #116 : 이전에 장착한 무기가 있다면 해제 후 장착

public:
	FT4GameBuiltin_PacketSC_EquipItem()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::EquipItem)
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
struct FT4GameBuiltin_PacketSC_UnequipItem : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID ItemWeaponDataID;

	UPROPERTY(VisibleAnywhere)
	bool bMainWeapon; // #48

public:
	FT4GameBuiltin_PacketSC_UnequipItem()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::UnequipItem)
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
struct FT4GameBuiltin_PacketSC_ExchangeItem : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID ItemCostumeDataID;

public:
	FT4GameBuiltin_PacketSC_ExchangeItem()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::ExchangeItem)
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
struct FT4GameBuiltin_PacketSC_Die : public FT4GameBuiltin_PacketSC_Base
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
	FT4GameBuiltin_PacketSC_Die()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::Die)
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
struct FT4GameBuiltin_PacketSC_Resurrect : public FT4GameBuiltin_PacketSC_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID ObjectID;

public:
	FT4GameBuiltin_PacketSC_Resurrect()
		: FT4GameBuiltin_PacketSC_Base(ET4GameBuiltin_PacketSC::Resurrect)
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