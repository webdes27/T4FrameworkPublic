// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4ActionCodeCommand.h"
#include "T4ActionStatusCommands.generated.h"

/**
  *
 */
 // #T4_ADD_ACTION_TAG_CODE

// ET4ActionType::Aim // #113
// ET4ActionType::LockOn
// ET4ActionType::Stance // #73
// ET4ActionType::SubStance // #106
// ET4ActionType::EquipWeapon
// ET4ActionType::UnequipWeapon
// ET4ActionType::Costume // #72
// ET4ActionType::Hit // #76
// ET4ActionType::Die // #76
// ET4ActionType::Resurrect // #76

// #113
USTRUCT()
struct T4ENGINE_API FT4AimAction : public FT4ActionCodeCommand
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	bool bClear;

	UPROPERTY(EditAnywhere)
	float HeadYawAngle; // #40

	UPROPERTY(EditAnywhere)
	FVector TargetDirection;

public:
	FT4AimAction()
		: FT4ActionCodeCommand(StaticActionType())
		, bClear(false)
		, HeadYawAngle(T4Const_EmptyYawAngle)
		, TargetDirection(FVector::ZeroVector)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::Aim; }

	FString ToString() const override
	{
		return FString(TEXT("AimAction"));
	}
};

USTRUCT()
struct T4ENGINE_API FT4LockOnAction : public FT4ActionCodeCommand
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	bool bSetLocked;

	UPROPERTY(EditAnywhere)
	float HeadYawAngle; // #40

public:
	FT4LockOnAction()
		: FT4ActionCodeCommand(StaticActionType())
		, bSetLocked(false)
		, HeadYawAngle(T4Const_EmptyYawAngle)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::LockOn; }

	FString ToString() const override
	{
		return FString(TEXT("LockOnAction"));
	}
};

// #73
USTRUCT()
struct T4ENGINE_API FT4StanceAction : public FT4ActionCodeCommand
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FName StanceName;

	UPROPERTY(EditAnywhere)
	bool bImmediate; // #111

	UPROPERTY(EditAnywhere)
	bool bOnlyFlush; // #116 : 스탠스 전환중이면 강제로 Flush 해준다. (이후 필요하다면 별도 Action 으로 뺄 것!)

public:
	FT4StanceAction()
		: FT4ActionCodeCommand(StaticActionType())
		, StanceName(NAME_None)
		, bImmediate(false) // #111
		, bOnlyFlush(false) // #116 : 스탠스 전환중이면 강제로 Flush 해준다. (이후 필요하다면 별도 Action 으로 뺄 것!)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::Stance; }

	FString ToString() const override
	{
		return FString(TEXT("StanceAction"));
	}
};

// #106
USTRUCT()
struct T4ENGINE_API FT4SubStanceAction : public FT4ActionCodeCommand
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FName SubStanceName;

	UPROPERTY(EditAnywhere)
	bool bImmediate; // #111

public:
	FT4SubStanceAction()
		: FT4ActionCodeCommand(StaticActionType())
		, SubStanceName(NAME_None)
		, bImmediate(false)  // #111
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::SubStance; }

	FString ToString() const override
	{
		return FString(TEXT("SubStanceAction"));
	}
};

// #111
USTRUCT()
struct T4ENGINE_API FT4EquipWeaponEntityData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<class UT4WeaponEntityAsset> WeaponEntityAsset;

	UPROPERTY(EditAnywhere)
	FName OverrideEquipPoint; // #57 : BoneOrSocketName;

public:
	FT4EquipWeaponEntityData()
		: OverrideEquipPoint(NAME_None)
	{
	}
};

USTRUCT()
struct T4ENGINE_API FT4EquipWeaponAction : public FT4ActionCodeCommand
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FT4ActionKey EquipmentActionKey; // #111 : 내부에서 아이템 관리 용도로 사용될 Equip 에서 넣어준 ActionKey

	UPROPERTY(EditAnywhere)
	FT4EquipWeaponEntityData MainWeaponData;

	UPROPERTY(EditAnywhere)
	TArray<FT4EquipWeaponEntityData> SubWeaponDatas; // #111

	UPROPERTY(EditAnywhere)
	bool bChangeStanceInEntity; // #110 : Weapon Entity 에 설정된 Stance 도 함께 설정해준다.

	UPROPERTY(EditAnywhere)
	bool bChangeStanceSync; // #111 : 스탠스 변경과 동기화한다.

public:
	FT4EquipWeaponAction()
		: FT4ActionCodeCommand(StaticActionType())
		, bChangeStanceInEntity(false) // #110 : Weapon Entity 에 설정된 Stance 도 함께 설정해준다.
		, bChangeStanceSync(false) // #111 : 스탠스 변경과 동기화한다.
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::EquipWeapon; }

	bool Validate(FString& OutMsg) override
	{
		if (MainWeaponData.WeaponEntityAsset.IsNull())
		{
			OutMsg = TEXT("Invalid Main WeaponEntityAsset");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("EquipWeaponAction"));
	}
};

// #48
USTRUCT()
struct T4ENGINE_API FT4UnequipWeaponAction : public FT4ActionCodeCommand
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	FT4ActionKey EquipmentActionKey; // #111 : 내부에서 아이템 관리 용도로 사용될 Equip 에서 넣어준 ActionKey

	UPROPERTY(EditAnywhere)
	bool bChangeDefaultStance; // #110 : Default Stance 로 변경해준다.

	UPROPERTY(EditAnywhere)
	bool bChangeStanceSync; // #111 : 스탠스 변경과 동기화한다.

public:
	FT4UnequipWeaponAction()
		: FT4ActionCodeCommand(StaticActionType())
		, bChangeDefaultStance(false) // #110 : Default Stance 로 변경해준다.
		, bChangeStanceSync(false) // #111 : 스탠스 변경과 동기화한다.
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::UnequipWeapon; }

	bool Validate(FString& OutMsg) override
	{
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("UnequipWeaponAction"));
	}
};

// #37
USTRUCT()
struct T4ENGINE_API FT4CostumeAction : public FT4ActionCodeCommand
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<class UT4CostumeEntityAsset> CostumeEntityAsset;

	UPROPERTY(EditAnywhere)
	FName TargetPartsName;
	
	UPROPERTY(EditAnywhere)
	bool bClearDefault; // #72 : Character Entity 에 지정된 기본값으로 변경

public:
	FT4CostumeAction()
		: FT4ActionCodeCommand(StaticActionType())
		, TargetPartsName(NAME_None)
		, bClearDefault(false)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::Costume; }

	bool Validate(FString& OutMsg) override
	{
		if (!bClearDefault && CostumeEntityAsset.IsNull())
		{
			OutMsg = TEXT("Invalid CostumeEntityAsset");
			return false;
		}
		if (TargetPartsName == NAME_None)
		{
			OutMsg = TEXT("Not set TargetParts");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("CostumeAction"));
	}
};

// #130
USTRUCT()
struct T4ENGINE_API FT4SkinAction : public FT4ActionCodeCommand
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FName TargetSkinName;

public:
	FT4SkinAction()
		: FT4ActionCodeCommand(StaticActionType())
		, TargetSkinName(NAME_None)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::Skin; }

	bool Validate(FString& OutMsg) override
	{
		if (TargetSkinName == NAME_None)
		{
			OutMsg = TEXT("Not set TargetSkin");
			return false;
		}
		return true;
	}

	FString ToString() const override
	{
		return FString(TEXT("SkinAction"));
	}
};

// #76
USTRUCT()
struct T4ENGINE_API FT4HitAction : public FT4ActionCodeCommand
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FName ReactionName;

	UPROPERTY(EditAnywhere)
	FVector ShotDirection;

	UPROPERTY(Transient)
	bool bTransientPlay;

public:
	FT4HitAction()
		: FT4ActionCodeCommand(StaticActionType())
		, ReactionName(NAME_None)
		, ShotDirection(FVector::ZeroVector)
		, bTransientPlay(false)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::Hit; }

	bool Validate(FString& OutMsg) override
	{
		return true;
	}

	virtual FString ToString() const override
	{
		return FString(TEXT("HitAction"));
	}
};

// #76
USTRUCT()
struct T4ENGINE_API FT4DieAction : public FT4ActionCodeCommand
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FName ReactionName;

	UPROPERTY(EditAnywhere)
	FVector ShotDirection;

public:
	FT4DieAction()
		: FT4ActionCodeCommand(StaticActionType())
		, ReactionName(NAME_None)
		, ShotDirection(FVector::ZeroVector)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::Die; }

	bool Validate(FString& OutMsg) override
	{
		return true;
	}

	virtual FString ToString() const override
	{
		return FString(TEXT("DieAction"));
	}
};

// #76
USTRUCT()
struct T4ENGINE_API FT4ResurrectAction : public FT4ActionCodeCommand
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere)
	FName ReactionName;

	UPROPERTY(Transient)
	bool bTransientPlay;

public:
	FT4ResurrectAction()
		: FT4ActionCodeCommand(StaticActionType())
		, ReactionName(NAME_None)
		, bTransientPlay(false)
	{
	}

	static ET4ActionType StaticActionType() { return ET4ActionType::Resurrect; }

	bool Validate(FString& OutMsg) override
	{
		return true;
	}

	virtual FString ToString() const override
	{
		return FString(TEXT("ResurrectAction"));
	}
};
