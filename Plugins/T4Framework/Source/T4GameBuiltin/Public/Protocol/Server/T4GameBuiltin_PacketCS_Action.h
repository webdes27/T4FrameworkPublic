// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameBuiltin_PacketCS.h"
#include "T4GameBuiltin_PacketCS_Action.generated.h"

/**
  *
 */
 // #T4_ADD_PACKET_TAG_CS

// ET4GameBuiltin_PacketCS::AimSet // #113, #116
// ET4GameBuiltin_PacketCS::AimClear // #113, #116
// ET4GameBuiltin_PacketCS::SkillTarget

// #113, #116
USTRUCT()
struct FT4GameBuiltin_PacketCS_AimSet : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;
	
	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID SkillDataID;

	UPROPERTY(VisibleAnywhere)
	bool bAimingStart; // #113 : 첫 호출에서만 true

	UPROPERTY(VisibleAnywhere)
	FVector TargetLocation; // #116

public:
	FT4GameBuiltin_PacketCS_AimSet()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::AimSet)
		, bAimingStart(false)
		, TargetLocation(FVector::ZeroVector) // #116
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!SenderID.IsValid())
		{
			OutMsg = TEXT("Invalid Send ObjectID!");
			return false;
		}
		if (!SkillDataID.IsValid())
		{
			OutMsg = TEXT("Invalid SkillDataID!");
			return false;
		}
		if (TargetLocation.IsNearlyZero())
		{
			OutMsg = TEXT("Invalid Target Location");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:AimSet"));
	}
};

// #113, #116
USTRUCT()
struct FT4GameBuiltin_PacketCS_AimClear : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

public:
	FT4GameBuiltin_PacketCS_AimClear()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::AimClear)
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
		return FString(TEXT("CS_Packet:AimClear"));
	}
};

USTRUCT()
struct FT4GameBuiltin_PacketCS_SkillTarget : public FT4GameBuiltin_PacketCS_Base
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FT4ObjectID SenderID;

	UPROPERTY(VisibleAnywhere)
	FT4GameBuiltin_GameDataID SkillDataID;

	UPROPERTY(VisibleAnywhere)
	ET4GameBuiltin_AttackTarget TargetType; // #112

	UPROPERTY(VisibleAnywhere)
	FT4ObjectID TargetObjectID; // #63 : 타겟이 있으면 먼저 체크! 없으면 Direct 을 사용한다.

	UPROPERTY(VisibleAnywhere)
	FName TargetHitBone; // #112 : TargetObjectID Valid 일 경우만, 현재는 순수 비쥬얼 용도

	UPROPERTY(VisibleAnywhere)
	FVector TargetLocationOrDirection; // #49, #68 : Area, #112

public:
	FT4GameBuiltin_PacketCS_SkillTarget()
		: FT4GameBuiltin_PacketCS_Base(ET4GameBuiltin_PacketCS::SkillTarget)
		, TargetType(ET4GameBuiltin_AttackTarget::None)
		, TargetHitBone(NAME_None) // #112
		, TargetLocationOrDirection(FVector::ZeroVector)
	{
	}

	bool Validate(FString& OutMsg) override
	{
		if (!SenderID.IsValid())
		{
			OutMsg = TEXT("Invalid Send ObjectID!");
			return false;
		}
		if (!SkillDataID.IsValid())
		{
			OutMsg = TEXT("Invalid SkillDataID!");
			return false;
		}
		if (ET4GameBuiltin_AttackTarget::ObjectID == TargetType)
		{
			if (!TargetObjectID.IsValid())
			{
				OutMsg = TEXT("Invalid Target ObjectID");
				return false;
			}
		}
		else if (ET4GameBuiltin_AttackTarget::Location == TargetType)
		{
			if (TargetLocationOrDirection.IsNearlyZero())
			{
				OutMsg = TEXT("Invalid Target Location");
				return false;
			}
		}
		else if (ET4GameBuiltin_AttackTarget::Direction == TargetType)
		{
			if (TargetLocationOrDirection.IsNearlyZero())
			{
				OutMsg = TEXT("Invalid Target Direction");
				return false;
			}
		}
		else
		{
			OutMsg = TEXT("Invalid Target Type");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("CS_Packet:Attack"));
	}
};
