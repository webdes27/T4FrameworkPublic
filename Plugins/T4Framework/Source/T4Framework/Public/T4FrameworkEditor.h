// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4FrameworkGameplay.h"
#include "T4Asset/Public/Entity/T4EntityKey.h" // #114
#include "T4Engine/Public/T4EngineTypes.h" // #63
#include "T4FrameworkEditor.generated.h"

/**
  * #60 : Only Editor
 */
enum ET4EditorGameDataType
{
	EdData_PC,
	EdData_NPC,
	EdData_Weapon,
	EdData_Costume,
	EdData_SkillSet, // #120
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

class UT4ActionSetAsset;

USTRUCT()
struct FT4EditorNPCDataInfo // #135
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = Common)
	FName InitializeSkinName;
};

USTRUCT()
struct FT4EditorSkillDataInfo
{
	GENERATED_USTRUCT_BODY()

public:
	// #T4_ADD_SKILL_CONTENT_TAG 

	UPROPERTY(VisibleAnywhere, Category = Common)
	FName Name;

	UPROPERTY(VisibleAnywhere, Category = Common)
	ET4GameplayAttackType AttackType; // #63

	UPROPERTY(EditAnywhere, Category = Common)
	float HitDelayTimeSec;

	UPROPERTY(EditAnywhere, Category = Common)
	float DurationSec;

	UPROPERTY(EditAnywhere, Category = Common)
	float MovementSpeed; // #135

	UPROPERTY(EditAnywhere, Category = Common)
	float ProjectileSpeed; // #63

	UPROPERTY(EditAnywhere, Category = Common)
	bool bMoveable;

	UPROPERTY(EditAnywhere, Category = Common)
	bool bLockOn; // #113

	UPROPERTY(EditAnywhere, Category = Common)
	float RotationRateSpeed; // #112, #113 : 캐릭터 RotationYawRate * Speed (1 일 경우 기본값 사용)

	UPROPERTY(EditAnywhere, Category = Common)
	ET4GameplayFindTarget FindTargetType; // #117 : 공객 대상을 찾을 경우에 대한 옵션 (TODO : Tribe or Enemy)

	UPROPERTY(VisibleAnywhere, Category = Common)
	FName ResultEffectSetDataID; // #68, #135

	UPROPERTY(EditAnywhere, Category = ClientOnly)
	bool bCasting; // #113, #135

	UPROPERTY(EditAnywhere, Category = ClientOnly, meta = (EditCondition = "bCasting", ClampMin = "-45.0", ClampMax = "45.0"))
	float AimingPitchAngle; // #127

	UPROPERTY(VisibleAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UT4ActionSetAsset> DefaultActionSetAsset;

	UPROPERTY(VisibleAnywhere, Category = ClientOnly, meta = (EditCondition = "bCasting"))
	TSoftObjectPtr<UT4ActionSetAsset> CastingActionSetAsset;

	UPROPERTY(VisibleAnywhere, Category = ClientOnly, meta = (EditCondition = "bCasting"))
	TSoftObjectPtr<UT4ActionSetAsset> CancelActionSetAsset; // #135

	UPROPERTY(VisibleAnywhere, Category = ClientOnly)
	TSoftObjectPtr<UT4ActionSetAsset> IndicateActionSetAsset; // #116

public:
	FT4EditorSkillDataInfo()
	{
		Reset();
	}

	void Reset()
	{
		Name = NAME_None;
		AttackType = ET4GameplayAttackType::Swing;
		HitDelayTimeSec = 0.0f;
		DurationSec = 0.0f;;
		MovementSpeed = 0.0f; // #135
		ProjectileSpeed = 0.0f; // #63
		bMoveable = false;
		bLockOn = false; // #113
		bCasting = false; // #113
		AimingPitchAngle = 0.0f; // #127
		RotationRateSpeed = 1.0f; // #113
		ResultEffectSetDataID = NAME_None;
		FindTargetType = ET4GameplayFindTarget::All; // #117
	}
};

USTRUCT()
struct FT4EditorEffectDataInfo
{
	GENERATED_USTRUCT_BODY()

public:
	// #T4_ADD_EFFECT_TAG_DATA

	UPROPERTY(VisibleAnywhere)
	FName Name;

	UPROPERTY(VisibleAnywhere)
	ET4GameplayEffectType EffectType;

	UPROPERTY(EditAnywhere)
	float HitDelayTimeSec;

	UPROPERTY(EditAnywhere)
	float DurationSec;

	UPROPERTY(EditAnywhere)
	float MinMoveDistance; // #135 : Area or Knockback

	UPROPERTY(EditAnywhere)
	float MaxMoveDistance; // #135 : Area or Knockback

	UPROPERTY(EditAnywhere)
	float MaxMoveHeight; // #135 : Area or Knockback, Airborne

	UPROPERTY(VisibleAnywhere)
	FName ChainEffectDataID;

	UPROPERTY(VisibleAnywhere)
	TSoftObjectPtr<UT4ActionSetAsset> ActionSetAsset;

public:
	FT4EditorEffectDataInfo()
	{
		Reset();
	}

	void Reset()
	{
		Name = NAME_None;
		EffectType = ET4GameplayEffectType::None;
		HitDelayTimeSec = 0.0f;
		DurationSec = 0.0f;
		MinMoveDistance = 0.0f;
		MaxMoveDistance = 0.0f;
		MaxMoveHeight = 0.0f;
		ChainEffectDataID = NAME_None;
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

	virtual void SetCustomCameraControl(bool bEnable) = 0; // #79

	virtual void SetMouseLocation(const int InX, const int InY) = 0;
	virtual bool GetMousePosition(float& InLocationX, float& InLocationY) = 0;

	virtual bool GetScreenCenterToWorldRay(const FVector2D& InScreenOffset, FRay& OutWorldRay) = 0; // #121 : Mode 에 따라 마우스 또는 화면 중앙(FPS)의 Ray 를 리턴
	virtual bool GetScreenPositionToWorldRay(const FVector2D& InScreenPosition, FRay& OutWorldRay) = 0; // #131
	virtual bool GetMousePositionToWorldRay(FRay& OutWorldRay) = 0;

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
//        컨텐츠 쪽에서 구현해주어야 에디터에서 사용할 수 있음 (T4Gameplay 을 사용하지 않을 경우를 위함)
class UT4EntityAsset;
class UT4ActionSetAsset;
class T4FRAMEWORK_API IT4EditorGameData // #60
{
public:
	virtual ~IT4EditorGameData() {}

	virtual FName GetGameDataTypeName(ET4EditorGameDataType InEditorGameDataType) = 0; // #118
	virtual void GetGameDataIDList(ET4EditorGameDataType InEditorGameDataType, TArray<FName>& OutDataNameIDs) = 0;

	virtual UT4EntityAsset* GetEntityAssetInGameData(ET4EditorGameDataType InEditorGameDataType, const FName& InDataNameID) = 0;
	virtual UT4EntityAsset* GetWeaponEntityAssetInGameData(ET4EditorGameDataType InEditorGameDataType, const FName& InDataNameID) = 0; // #120
	virtual UT4ActionSetAsset* GetActionSetAssetInGameData(ET4EditorGameDataType InEditorGameDataType, const FName& InDataNameID) = 0; // #120

	virtual bool GetNPCDataInfo(const FName& InDataNameID, FT4EditorNPCDataInfo& OutData) = 0; // #135
	virtual bool GetSkillDataInfo(const FName& InDataNameID, FT4EditorSkillDataInfo& OutData) = 0;
	virtual bool GetEffectDataInfo(const FName& InDataNameID, FT4EditorEffectDataInfo& OutData) = 0;
};

static const FName T4Editor_SkillDataNameID = TEXT("T4Editor_SkillDataNameID");
static const FName T4Editor_EffectDataNameID = TEXT("T4Editor_EffectDataNameID");

// #114 : 에디터에서 N종의 게임 로직을 컨트롤 하기 위해 에디터상에서 구현해야 할 인터페이스
//        (T4Gameplay 을 사용하지 않을 경우를 위함)
class T4FRAMEWORK_API IT4EditorGameplayContoller
{
public:
	virtual ~IT4EditorGameplayContoller() {}

	virtual bool IsSimulating() const = 0; // #102

	virtual float GetDefaultMoveSpeed() const = 0; // #114

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
//        (T4Gameplay 을 사용하지 않을 경우를 위함)
class UT4ContentSpawnAsset;
class T4FRAMEWORK_API IT4EditorGameplayCommand
{
public:
	virtual ~IT4EditorGameplayCommand() {}

	virtual bool DoSpawn(
		ET4EditorGameDataType InGameDataType, // #126
		const FName& InDataNameID, 
		const FVector& InLocation, 
		const FRotator& InRotation, 
		const FT4ObjectID& InReservedObjectID
	) = 0; // #60 : to player

	virtual bool DoSpawnByContentSpawn(
		UT4ContentSpawnAsset* InSpawnAsset, 
		const FName& InSpawnActorID, 
		const FT4ObjectID& InReservedObjectID
	) = 0; // #126
	virtual bool DoSpawnByContentSpawn(UT4ContentSpawnAsset* InSpawnAsset) = 0; // #126

	virtual bool DoDespawn(const FT4ObjectID& InObjectID) = 0; // #114
	virtual bool DoDespawnAll(bool bClearPlayerActor) = 0; // #68

#if WITH_EDITOR
	// #114 : 툴에서만 호출됨!! Server 류는 Entity 정보가 테이블에 있기 때문에 테이블 데이터 없이 스폰하기 위한 처리임!
	// #134 : 툴용도의 Server Send => Client Recv 대체 처리. 즉, 아래 코드에서 C/S 모드를 처리하고 있음에 유의!!
	virtual bool DoSpawnByEntityKey(
		const FT4ObjectID& InReservedObjectID, // #134 : GetPlayerController()->GetObjectID() 로 비교해 Player 를 판단한다.
		const FT4EntityKey& InEntityKey,
		const FVector& InLocation,
		const FRotator& InRotation,
		bool bInClientOnly
	) = 0;
	virtual bool DoDespawnWithEditorOnly(const FT4ObjectID& InObjectID, bool bInClientOnly) = 0;
#endif

	virtual bool DoChangeStance(FName InStanceName) = 0;// #73, #114
	virtual bool DoChangePosture(FName InPostureName) = 0; // #106, #114

	virtual bool DoEquipWeaponItem(const FName& InWeaponDataNameID, bool bInUnequip) = 0; // #60 : to player
	virtual bool DoExchangeCostumeItem(const FName& InCostumeDataNameID) = 0; // #60 : to player
};

#endif
