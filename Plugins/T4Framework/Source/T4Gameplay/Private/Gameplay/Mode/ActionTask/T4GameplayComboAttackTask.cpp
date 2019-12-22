// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameplayComboAttackTask.h"

#include "Gameplay/T4GameplayInstance.h"
#include "GameDB/T4GameDB.h"

#include "Public/Protocol/T4PacketCSMinimal.h"

#include "Classes/Controller/Player/T4GameplayPlayerController.h" // #42

#include "T4Engine/Public/Action/T4ActionCodeMinimal.h"
#include "T4Engine/Public/T4Engine.h"
#include "T4Engine/Public/T4EngineSettings.h"
#include "T4Frame/Public/T4Frame.h"

#include "T4GameplayInternal.h"

/**
  * #48
 */
FT4ComboAttackActionTask::FT4ComboAttackActionTask(FT4GameplayModeBase* InGameplayMode)
	: FT4ActionTask(InGameplayMode)
	, bPendingComboAttack(false) // #48
	, ComboAttackPendingClearTimeLeft(0.0f) // #48
	, ComboAttackSelected(ET4ComboAttackSeqeunce::Ready) // #48
	, ComboAttackPlayTimeLeft(0.0f) // #48
{
}

FT4ComboAttackActionTask::~FT4ComboAttackActionTask()
{
}

void FT4ComboAttackActionTask::Reset()
{
	bPendingComboAttack = false;
	ComboAttackSelected = ET4ComboAttackSeqeunce::Ready; // #45
	ComboAttackPendingClearTimeLeft = 0.0f; // #45
	ComboAttackPlayTimeLeft = 0.0f;
	bMovementLcoked = false;
}

void FT4ComboAttackActionTask::Process(float InDeltaTime)
{
	if (!bPendingComboAttack)
	{
		return;
	}
	ComboAttackPendingClearTimeLeft -= InDeltaTime;
	ComboAttackPlayTimeLeft -= InDeltaTime;
	
	if (ComboAttackPlayTimeLeft > 0.0f)
	{
		return; // #60 : 재사용 불가, Reset 은 아님에 유의!
	}

	if (ComboAttackPendingClearTimeLeft <= 0.0f)
	{
		Reset();
		return;
	}

	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	if (!PlayerController->HasGameObject())
	{
		Reset();
		return;
	}

	// #60
	FT4GameSkillDataID SelectSkillDataID;
	bool bMoveable = false;
	float SkillDataHitDelayTimeSec = 0.0f;
	float SkillDataDurationSec = 0.0f;

#if WITH_EDITOR
	IT4GameFrame* ClientFramework = GetGameFrame();
	check(nullptr != ClientFramework);
	IT4EditorGameplayHandler* EditorGameplayHandler = ClientFramework->GetEditorGameplayCustomHandler();
	if (nullptr != EditorGameplayHandler)
	{
		if (!EditorGameplayHandler->IsSimulating()) // #102
		{
			Reset();
			return;
		}
		if (EditorGameplayHandler->IsUsedGameplaySettings()) // #104
		{
			// #60 : Editor AI Handler 가 있다면 AI를 별도로 제어함
			SelectSkillDataID.RowName = EditorGameplayHandler->GetOverrideSkillDataNameID();
			if (SelectSkillDataID.RowName == NAME_None)
			{
				Reset();
				return;
			}
			const FT4EditorSkillDataInfo& EditorSkillData = EditorGameplayHandler->GetOverrideSkillDataInfo();
			SkillDataHitDelayTimeSec = EditorSkillData.HitDelayTimeSec;
			SkillDataDurationSec = EditorSkillData.DurationSec;
			bMoveable = EditorSkillData.bMoveable;
			if (SelectSkillDataID.RowName == EditorSkillDataNameID && 0.0f >= SkillDataDurationSec)
			{
				Reset();
				return;
			}
		}
	}
#endif

	if (!SelectSkillDataID.IsValid())
	{
		const FT4GameDataID MainWeaponDataID = GetMainWeaponDataID();
		if (!MainWeaponDataID.IsValid())
		{
			Reset();
			return;
		}
		FT4GameDB& GameDB = GetGameDB();
		const FT4GameItemWeaponData* ItemData = GameDB.GetGameData<FT4GameItemWeaponData>(MainWeaponDataID);
		if (nullptr == ItemData)
		{
			Reset();
			return;
		}
		const FT4GameSkillSetData* SkillSetData = GameDB.GetGameData<FT4GameSkillSetData>(ItemData->RawData.SkillSetNameID);
		if (nullptr == SkillSetData) // #50
		{
			Reset();
			return;
		}
		switch (ComboAttackSelected)
		{
			case ET4ComboAttackSeqeunce::Ready:
			case ET4ComboAttackSeqeunce::Finish:
				ComboAttackSelected = ET4ComboAttackSeqeunce::Primary;
				SelectSkillDataID = SkillSetData->RawData.ComboPrimaryAttackNameID;
				break;
			case ET4ComboAttackSeqeunce::Primary:
				ComboAttackSelected = ET4ComboAttackSeqeunce::Secondary;
				SelectSkillDataID = SkillSetData->RawData.ComboSecondaryAttackNameID;
				break;
			case ET4ComboAttackSeqeunce::Secondary:
				ComboAttackSelected = ET4ComboAttackSeqeunce::Tertiary;
				SelectSkillDataID = SkillSetData->RawData.ComboTertiaryAttackNameID;
				break;
			case ET4ComboAttackSeqeunce::Tertiary:
				ComboAttackSelected = ET4ComboAttackSeqeunce::Finish;
				SelectSkillDataID = SkillSetData->RawData.FinishAttackNameID;
				break;
			default:
				{
					UE_LOG(
						LogT4Gameplay,
						Error,
						TEXT("ProcessComboAttack::ComboAttackSelected '%u' failed. no implementation."),
						uint8(ComboAttackSelected)
					);
					Reset();
					return;
				}
				break;
		}
		const FT4GameSkillData* SkillData = GameDB.GetGameData<FT4GameSkillData>(SelectSkillDataID);
		if (nullptr == SkillData)
		{
			Reset();
			return;
		}
		SkillDataHitDelayTimeSec = SkillData->RawData.HitDelayTimeSec;
		SkillDataDurationSec = SkillData->RawData.DurationSec;
		bMoveable = SkillData->RawData.bMoveable;
	}

	if (!SelectSkillDataID.IsValid())
	{
		Reset();
		return;
	}

	IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandlerCS();
	check(nullptr != PacketHandlerCS);

	IT4GameObject* PlayerObject = PlayerController->GetGameObject();
	check(nullptr != PlayerObject);

	FT4ObjectID TargetObjectID;
	FVector TargetLocation = FVector::ZeroVector;
	FVector TargetDirection = FVector::ZeroVector;
	ET4GameplayGameModeType CurrentMode = GetModeType();
	IT4GameObject* MouseOverObject = GetGameFrame()->GetMouseOverGameObject();
	if (nullptr != MouseOverObject)
	{
		TargetObjectID = MouseOverObject->GetObjectID();
		TargetDirection = FVector(
			MouseOverObject->GetRootLocation() - PlayerObject->GetRootLocation()
		);
		TargetDirection.Normalize();
		TargetLocation = MouseOverObject->GetRootLocation();
	}
	else if (GetGameFrame()->GetMousePickingLocation(TargetLocation))
	{
		const FVector PlayerRootLocation = PlayerObject->GetRootLocation();
		TargetDirection = FVector(
			TargetLocation.X - PlayerRootLocation.X, TargetLocation.Y - PlayerRootLocation.Y, 0.0f
		);
		TargetDirection.Normalize();
	}
	else
	{
		TargetDirection = PlayerObject->GetFrontVector();
	}

	FT4PacketAttackCS NewPacketCS;
	NewPacketCS.SenderID = PlayerController->GetGameObjectID();
	NewPacketCS.SkillDataID = SelectSkillDataID;
	NewPacketCS.TargetObjectID = TargetObjectID; // #63
	NewPacketCS.TargetLocation = TargetLocation; // #68
	NewPacketCS.TargetDirection = TargetDirection; // #49
	PacketHandlerCS->DoSendPacket(&NewPacketCS);

	ComboAttackPlayTimeLeft = SkillDataDurationSec;
	bMovementLcoked = !bMoveable;
}

bool FT4ComboAttackActionTask::Pressed(FString& OutErrorMsg)
{
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	if (!PlayerController->HasGameObject())
	{
		OutErrorMsg = FString::Printf(TEXT("PlayerObject is Not set."));
		Reset();
		return false;
	}

	bool bCheckMainWeapon = true;

#if WITH_EDITOR
	IT4GameFrame* ClientFramework = GetGameFrame();
	check(nullptr != ClientFramework);
	IT4EditorGameplayHandler* EditorGameplayHandler = ClientFramework->GetEditorGameplayCustomHandler();
	if (nullptr != EditorGameplayHandler) // #60
	{
		if (EditorGameplayHandler->IsUsedGameplaySettings()) // #104
		{
			if (!EditorGameplayHandler->IsSimulating()) // #102
			{
				Reset();
				return false;
			}
			bCheckMainWeapon = false;
		}
	}
#endif

	if (bCheckMainWeapon)
	{
		const FT4GameDataID MainWeaponDataID = GetMainWeaponDataID();
		if (!MainWeaponDataID.IsValid())
		{
			OutErrorMsg = FString::Printf(TEXT("No Weapon Equipped."));
			Reset();
			return false;
		}
		FT4GameDB& GameDB = GetGameDB();
		const FT4GameItemWeaponData* ItemData = GameDB.GetGameData<FT4GameItemWeaponData>(MainWeaponDataID);
		if (nullptr == ItemData)
		{
			OutErrorMsg = FString::Printf(TEXT("MainWeapon '%s' Not Found."), *(MainWeaponDataID.ToString()));
			Reset();
			return false;
		}
	}

	bPendingComboAttack = true;
	const UT4EngineSettings* EngineSettings = GetDefault<UT4EngineSettings>();
	check(nullptr != EngineSettings);
	ComboAttackPendingClearTimeLeft = EngineSettings->ComboAttackKeepTimeSec; // #48 : 콤보 공격, 시간 경과시 Primary 로 변경
	return true;
}

FT4GameDataID FT4ComboAttackActionTask::GetMainWeaponDataID() // #49
{
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController)
	{
		return FT4GameDataID();
	}
	AT4GameplayPlayerController* GamePC = Cast<AT4GameplayPlayerController>(PlayerController->GetAController());
	if (nullptr == GamePC)
	{
		return FT4GameDataID();
	}
	return GamePC->GetMainWeaponDataID();
}