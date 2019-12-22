// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameplayLockOnTask.h"
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
FT4LockOnActionTask::FT4LockOnActionTask(FT4GameplayModeBase* InGameplayMode)
	: FT4ActionTask(InGameplayMode)
{
}

FT4LockOnActionTask::~FT4LockOnActionTask()
{
}

void FT4LockOnActionTask::Reset()
{
}

void FT4LockOnActionTask::Process(float InDeltaTime)
{
}

bool FT4LockOnActionTask::Start(
	const float InSyncHeadYawAngle, 
	FString& OutErrorMsg
)
{
	IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandlerCS();
	check(nullptr != PacketHandlerCS);

	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	if (!PlayerController->HasGameObject())
	{
		OutErrorMsg = FString::Printf(TEXT("PlayerObject is Not set."));
		return false;
	}
	if (PlayerController->HasPublicAction(T4ActionLockOnPKey))
	{
		OutErrorMsg = FString::Printf(TEXT("Already LockOn played."));
		return false;
	}
	FT4PacketLockOnCS NewPacketCS; // #27
	NewPacketCS.SenderID = PlayerController->GetGameObjectID();
	NewPacketCS.HeadYawAngle = InSyncHeadYawAngle;
	PacketHandlerCS->DoSendPacket(&NewPacketCS);
	bStarted = true;
	return true;
}

bool FT4LockOnActionTask::End(
	const float InSyncHeadYawAngle, 
	FString& OutErrorMsg
)
{
	bStarted = false;
	IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandlerCS();
	check(nullptr != PacketHandlerCS);

	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	if (!PlayerController->HasGameObject())
	{
		OutErrorMsg = FString::Printf(TEXT("PlayerObject is Not set."));
		return false;
	}
	if (!PlayerController->HasPublicAction(T4ActionLockOnPKey))
	{
		OutErrorMsg = FString::Printf(TEXT("Already LockOn played."));
		return false;
	}
	FT4PacketLockOffCS NewPacketCS; // #27
	NewPacketCS.SenderID = PlayerController->GetGameObjectID();
	NewPacketCS.HeadYawAngle = InSyncHeadYawAngle; // #38
	PacketHandlerCS->DoSendPacket(&NewPacketCS);
	return true;
}