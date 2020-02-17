// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4FrameGameTypes.h"
#include "T4Asset/Public/Entity/T4EntityKey.h" // #114
#include "T4Engine/Public/T4EngineTypes.h" // #63
#include "T4FrameEditorSupport.generated.h"

/**
  * #60 : Only Editor
 */
enum ET4EditorGameDataType
{
	EdData_PC,
	EdData_NPC,
	EdData_Weapon,
	EdData_Costume,
	EdData_Skill,
	EdData_Effect,
};

UENUM()
enum class ET4EditorPlayRole : uint8
{
	Attacker,
	Defender,

	None UMETA(Hidden),
};

class UT4ContiAsset;
USTRUCT()
struct FT4EditorSkillDataInfo
{
	GENERATED_USTRUCT_BODY()

public:
	// #T4_ADD_SKILL_CONTENT_TAG 

	UPROPERTY(VisibleAnywhere)
	FName Name;

	UPROPERTY(VisibleAnywhere)
	ET4GameAttackType AttackType; // #63

	UPROPERTY(EditAnywhere)
	float HitDelayTimeSec;

	UPROPERTY(EditAnywhere)
	float DurationSec;

	UPROPERTY(EditAnywhere)
	float ProjectileSpeed; // #63

	UPROPERTY(EditAnywhere)
	bool bMoveable;

	UPROPERTY(EditAnywhere)
	bool bLockOn; // #113

	UPROPERTY(EditAnywhere)
	bool bAiming; // #113

	UPROPERTY(EditAnywhere)
	float RotationRateSpeed; // #112, #113 : 캐릭터 RotationYawRate * Speed (1 일 경우 기본값 사용)

	UPROPERTY(EditAnywhere)
	ET4GameFindTargetType FindTargetType; // #117 : 공객 대상을 찾을 경우에 대한 옵션 (TODO : Tribe or Enemy)

	UPROPERTY(VisibleAnywhere)
	FName ResultEffectDataID;

	UPROPERTY(VisibleAnywhere)
	TSoftObjectPtr<UT4ContiAsset> ContiAsset;

	UPROPERTY(VisibleAnywhere)
	TSoftObjectPtr<UT4ContiAsset> AimingContiAsset;

	UPROPERTY(VisibleAnywhere)
	TSoftObjectPtr<UT4ContiAsset> IndicateContiAsset; // #116

public:
	FT4EditorSkillDataInfo()
	{
		Reset();
	}

	void Reset()
	{
		Name = NAME_None;
		AttackType = ET4GameAttackType::Melee;
		HitDelayTimeSec = 0.0f;
		DurationSec = 0.0f;;
		ProjectileSpeed = 0.0f; // #63
		bMoveable = false;
		bLockOn = false; // #113
		bAiming = false; // #113
		RotationRateSpeed = 1.0f; // #113
		ResultEffectDataID = NAME_None;
		FindTargetType = ET4GameFindTargetType::All; // #117
	}
};

USTRUCT()
struct FT4EditorEffectDataInfo
{
	GENERATED_USTRUCT_BODY()

public:
	// #T4_ADD_EFFECT_CONTENT_TAG

	UPROPERTY(VisibleAnywhere)
	FName Name;

	UPROPERTY(VisibleAnywhere)
	ET4GameEffectType EffectType;

	UPROPERTY(EditAnywhere)
	float HitDelayTimeSec;

	UPROPERTY(EditAnywhere)
	float AreaRange;

	UPROPERTY(VisibleAnywhere)
	FName DamageEffectDataID;

	UPROPERTY(VisibleAnywhere)
	TSoftObjectPtr<UT4ContiAsset> ContiAsset;

public:
	FT4EditorEffectDataInfo()
	{
		Reset();
	}

	void Reset()
	{
		Name = NAME_None;
		EffectType = ET4GameEffectType::Direct;
		HitDelayTimeSec = 0.0f;
		AreaRange = 0.0f;
		DamageEffectDataID = NAME_None;
	}
};


#if WITH_EDITOR
enum ET4WorldTravelResult // #79
{
	TravelResult_Success,
	TravelResult_ChangeContext,

	TravelResult_Failed,
};

struct FWorldContext;
class FViewport;
class IT4EditorViewportClient
{
public:
	virtual ~IT4EditorViewportClient() {}

	virtual FViewport* GetViewport() const = 0; // #68

	virtual bool IsPreviewMode() const = 0;

	virtual void SetUpdateCameraForPlayer(bool bEnable) = 0; // #79

	virtual void SetMouseLocation(const int InX, const int InY) = 0;
	virtual bool GetMousePosition(float& InLocationX, float& InLocationY) = 0;
	virtual bool GetMousePositionToWorldRay(FVector& OutStartPosition, FVector& OutStartDirection) = 0;

	virtual void ShowMouseCursor(bool InShow) = 0;
	virtual void SetMouseCursorType(EMouseCursor::Type InMouseCursorType) = 0;

	virtual void SetInitialLocationAndRotation(const FVector& InLocation, const FRotator& InRotation) = 0; // #86

	virtual bool IsEditWidgetModeEnabled() const = 0; // #94, #118
	virtual void SetEnableEditWidgetMode(bool bInEnable) = 0; // #94, #118

	virtual bool IsKeyLeftShiftPressed() const = 0; // #118
	virtual bool IsMouseLeftButtonPressed() const = 0; // #111
	virtual bool IsMouseRightButtonPressed() const = 0; // #111, #118
};

// #114 : 에디터에서 N종의 게임 컨텐츠 데이터에서 정보를 얻기 위한 인터페이스
//        컨텐츠 쪽에서 구현해주어야 에디터에서 사용할 수 있음 (T4GameBuiltin 을 사용하지 않을 경우를 위함)
class T4FRAME_API IT4EditorGameDatabase // #60
{
public:
	virtual ~IT4EditorGameDatabase() {}

	virtual FName GetGameDataTypeName(ET4EditorGameDataType InEditorGameDataType) = 0; // #118
	virtual void GetGameDataIDList(ET4EditorGameDataType InEditorGameDataType, TArray<FName>& OutDataNameIDs) = 0;
	virtual class UT4EntityAsset* GetGameDataEntityAsset(ET4EditorGameDataType InEditorGameDataType, const FName& InDataNameID) = 0;

	virtual bool GetSkillDataInfo(const FName& InSkillDataNameID, FT4EditorSkillDataInfo& OutSkillData) = 0;
	virtual bool GetEffectDataInfo(const FName& InEffectDataNameID, FT4EditorEffectDataInfo& OutEffectData) = 0;
};

static const FName T4Editor_SkillDataNameID = TEXT("T4Editor_SkillDataNameID");
static const FName T4Editor_EffectDataNameID = TEXT("T4Editor_EffectDataNameID");

// #114 : 에디터에서 N종의 게임 로직을 컨트롤 하기 위해 에디터상에서 구현해야 할 인터페이스
//        (T4GameBuiltin 을 사용하지 않을 경우를 위함)
class T4FRAME_API IT4EditorGameplayContoller
{
public:
	virtual ~IT4EditorGameplayContoller() {}

	virtual bool IsSimulating() const = 0; // #102

	virtual bool IsUsedGameplaySettings() const = 0; // #104 : conti 에서만 true, world 에서는 false, 아래 옵션 사용 여부

	//{ IsUsedGameplaySettings = true
	virtual bool IsAISystemDisabled() const = 0;

	virtual bool IsSandbagAttackable() const = 0;
	virtual bool IsSandbagOneHitDie() const = 0; // #76

	virtual bool IsOverrideSkillData() const = 0; // #63
	virtual bool IsOverrideEffectData() const = 0; // #68

	virtual FName GetOverrideSkillDataNameID() const = 0;
	virtual FName GetOverrideEffectDataNameID() const = 0;
	virtual FName GetOverrideDieReactionNameID() const = 0; // #76 : TODO : 사망 리엑션 값을 테이블에서 가져와야 함!

	virtual const FT4EditorSkillDataInfo& GetOverrideSkillDataInfo() const = 0;
	virtual const FT4EditorEffectDataInfo& GetOverrideEffectDataInfo() const = 0;

	virtual const FSoftObjectPath& GetOverridePlayContiPath() const = 0;
	//}
};

// #114 : 에디터에서 N종의 게임 로직을 사용하기 위한 인터페이스
//        (T4GameBuiltin 을 사용하지 않을 경우를 위함)
class T4FRAME_API IT4EditorGameplayCommand
{
public:
	virtual ~IT4EditorGameplayCommand() {}

	virtual bool DoNPCSpawn(
		const FName& InNPCDataNameID, 
		const FVector& InLocation, 
		const FRotator& InRotation, 
		uint32 InReservedObjectID
	) = 0; // #60 : to player

	virtual bool DoNPCSpawn(
		const FT4EntityKey& InEntityKey,
		const FVector& InLocation,
		const FRotator& InRotation,
		uint32 InReservedObjectID,
		bool bInPlayer
	) = 0; // #114: Conti Editor

	virtual bool DoDespawn(const FT4ObjectID& InObjectID) = 0; // #114
	virtual bool DoDespawnAll(bool bClearPlayerActor) = 0; // #68

	virtual bool DoChangeStance(FName InStanceName) = 0;// #73, #114
	virtual bool DoChangeSubStance(FName InSubStanceName) = 0; // #106, #114

	virtual bool DoEquipWeaponItem(const FName& InWeaponDataNameID, bool bInUnequip) = 0; // #60 : to player
	virtual bool DoExchangeCostumeItem(const FName& InCostumeDataNameID) = 0; // #60 : to player
};

#endif
