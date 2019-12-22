// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameplayModeTPS.h"
#include "T4GameplayDefinitions.h"

#include "Public/Protocol/T4PacketCSMinimal.h"

#include "T4Engine/Public/Action/T4ActionCodeMinimal.h"
#include "T4Engine/Public/T4Engine.h"
#include "T4Frame/Public/T4Frame.h"

#include "T4GameplayInternal.h"

/**
  * #40
 */
inline float GetHeadYawAngle2D(const FVector& InDirection)
{
	return FMath::Atan2(InDirection.Y, InDirection.X) * (180.f / PI); // #40
}

FT4GameplayModeTPS::FT4GameplayModeTPS(ET4LayerType InLayerType)
	: FT4GameplayModeBase(InLayerType)
{
}

FT4GameplayModeTPS::~FT4GameplayModeTPS()
{
}

void FT4GameplayModeTPS::Enter()
{
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	PlayerController->SwitchCameraType(ET4CameraType::TPS);
}

void FT4GameplayModeTPS::Leave()
{
	DoCameraRotateReleased();
	DoLockOnReleased();
}

void FT4GameplayModeTPS::Process(float InDeltaTime)
{
	ProcessMovement(InDeltaTime);
	ProcessTurn(InDeltaTime);
}

void FT4GameplayModeTPS::ProcessMovement(float InDeltaTime)
{
	if (MovementInputVector.IsZero())
	{
		return;
	}
	// #33 : XY 축 이동이 있어 조작감 향상을 위해 모아서 한 프레임에 패킷으로 전송한다.
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	if (PlayerController->HasGameObject())
	{
		MovementInputVector.Normalize();
		{
			float ApplyHeadYawAngle = 0.0f;
			if (bLockOnUsed)
			{
				// #40 : 락온 일 경우는 카메라 방향
				ApplyHeadYawAngle = PlayerController->GetCameraRotation().Yaw;
			}
			else
			{
				// #40 : 락온이 아니면 캐릭터 방향으로...
				ApplyHeadYawAngle = GetHeadYawAngle2D(MovementInputVector);
			}
			// #33 : Player 는 조작감 향상을 위해 선이동을 한다.
			DoMoveAsyncStart(MovementInputVector, ApplyHeadYawAngle); // #52
		}
	}
	MovementInputVector = FVector::ZeroVector;
}

void FT4GameplayModeTPS::ProcessTurn(float InDeltaTime)
{
	if (!bLockOnUsed || !IsCameraRotationEnabled())
	{
		return;
	}
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	// #40 : TODO : dirty check
	if (PlayerController->HasGameObject())
	{
		IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandlerCS();
		check(nullptr != PacketHandlerCS);

		FT4PacketTurnCS NewPacketCS; // #40
		NewPacketCS.SenderID = PlayerController->GetGameObjectID();
		NewPacketCS.TargetYawAngle = PlayerController->GetCameraRotation().Yaw;
		PacketHandlerCS->DoSendPacket(&NewPacketCS);
	}
}

bool FT4GameplayModeTPS::DoLockOnPressed()
{
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	// #40 : 락온은 카메라 방향이 기준이기 때문에 카메라 방향으로 위치를 맞춰준다.
	const FRotator SyncRotation = PlayerController->GetCameraRotation();
	FString ErrorMsg;
	bool bResult = CallLockOnStart(SyncRotation.Yaw, ErrorMsg);
	if (bResult)
	{
		bLockOnUsed = true;
	}
	return bResult;
}

bool FT4GameplayModeTPS::DoLockOnReleased()
{
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	// #40 : 락온 해제는 캐릭터 현재 방향으로 동기화 해준다.
	FRotator SyncRotation = FRotator::ZeroRotator;
	if (PlayerController->HasGameObject())
	{
		SyncRotation = PlayerController->GetGameObject()->GetRotation();
	}
	FString ErrorMsg;
	bool bResult = CallLockOnEnd(SyncRotation.Yaw, ErrorMsg);
	bLockOnUsed = false;
	return bResult;
}
