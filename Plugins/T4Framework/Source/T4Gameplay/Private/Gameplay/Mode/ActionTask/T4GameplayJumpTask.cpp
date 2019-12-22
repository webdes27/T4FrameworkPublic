// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameplayJumpTask.h"
#include "T4GameplayDefinitions.h"

#include "Gameplay/T4GameplayInstance.h"
#include "GameDB/T4GameDB.h"

#include "Public/Protocol/T4PacketCSMinimal.h"

#include "T4Engine/Public/Action/T4ActionCodeMinimal.h"
#include "T4Engine/Public/T4Engine.h"
#include "T4Engine/Public/T4EngineSettings.h"
#include "T4Frame/Public/T4Frame.h"

#include "T4GameplayInternal.h"

/**
  * #48
 */
FT4JumpActionTask::FT4JumpActionTask(FT4GameplayModeBase* InGameplayMode)
	: FT4ActionTask(InGameplayMode)
	, bDoublePressed(false)
	, JumpHoldTimeLeft(0.0f)
{
}

FT4JumpActionTask::~FT4JumpActionTask()
{
}

void FT4JumpActionTask::Reset()
{
	bDoublePressed = false;
	JumpHoldTimeLeft = 0.0f; // #46
}

void FT4JumpActionTask::Process(float InDeltaTime)
{
	// #46
	if (!bPressed)
	{
		return;
	}
	if (bDoublePressed)
	{
		DoJump();
	}
	else
	{
		JumpHoldTimeLeft -= InDeltaTime;
		if (JumpHoldTimeLeft < 0.0f)
		{
			bPressed = false;
		}
	}
}

bool FT4JumpActionTask::Pressed(FString& OutErrorMsg)
{
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	if (PlayerController->HasPublicAction(T4ActionRollPKey))
	{
		OutErrorMsg = FString::Printf(TEXT("Already Polling."));
		return false;
	}
	if (PlayerController->HasPublicAction(T4ActionJumpPKey))
	{
		OutErrorMsg = FString::Printf(TEXT("Already Jumping."));
		return false;
	}
	if (bPressed)
	{
		bDoublePressed = true;
		return true;
	}
	const UT4EngineSettings* EngineSettings = GetDefault<UT4EngineSettings>();
	check(nullptr != EngineSettings);
	JumpHoldTimeLeft = EngineSettings->JumpHoldTimeSec;
	bPressed = true;
	return true;
}

void FT4JumpActionTask::DoJump() // #46
{
	if (!bDoublePressed)
	{
		return;
	}
	IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandlerCS();
	check(nullptr != PacketHandlerCS);

	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	if (PlayerController->HasGameObject())
	{
		FRotator ControlRotation = PlayerController->GetViewControlRotation();
		FVector JumpDirection = ControlRotation.RotateVector(FVector::ForwardVector);
		FT4PacketJumpCS NewPacketCS; // #27
		NewPacketCS.SenderID = PlayerController->GetGameObjectID();
		NewPacketCS.JumpDirection = JumpDirection;
		PacketHandlerCS->DoSendPacket(&NewPacketCS);
	}

	OnReset();
}