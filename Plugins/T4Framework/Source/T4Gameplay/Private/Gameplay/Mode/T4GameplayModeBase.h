// Copyright 2019 SoonBo Noh. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "T4GameplayTypes.h"

/**
  * #40
 */
class FT4ActionTask;
class IT4PlayerController;
class IT4PacketHandlerCS;
class IT4GameFrame;
class IT4GameWorld;
class IT4GameObject;
class FT4GameplayModeBase
{
public:
	explicit FT4GameplayModeBase(ET4LayerType InLayerType);
	virtual ~FT4GameplayModeBase();

	void OnEnter();
	void OnLeave();

	void OnProcess(float InDeltaTime);

	virtual ET4GameplayGameModeType GetModeType() const = 0;

	// #33 : Player 는 조작감 향상을 위해 선이동을 한다.
	void DoMoveAsyncStart(const FVector& InInputVector, float HeadYawAngle); // #52

	bool DoMoveForward(float InAxisValue);
	bool DoMoveRight(float InAxisValue);

	virtual bool DoLockOnPressed();
	virtual bool DoLockOnReleased();

	bool DoJumpPressed();
	bool DoRollPressed(); // #46
	bool DoTeleportPressed();

	bool DoAttackPressed();

	bool DoChangePlayer();
	bool DoChangeObserver(); // #52
	bool DoLeaveObject();

	bool DoCameraRotatePressed();
	bool DoCameraRotateReleased();

	bool DoCameraZoom(float InAxisValue);
	bool DoCameraPitch(float InAxisValue);
	bool DoCameraYaw(float InAxisValue);

protected:
	friend class FT4ActionTask;

	virtual void Enter() {}
	virtual void Leave() {}

	virtual void Process(float InDeltaTime) {}

	bool CallLockOnStart(const float InSyncHeadYawAngle, FString& OutErrorMsg);
	bool CallLockOnEnd(const float InSyncHeadYawAngle, FString& OutErrorMsg);

	bool IsCameraRotationEnabled() const; // #48
	bool IsLockOnUsed() const { return bLockOnUsed; }
	bool IsMovementLocked() const; // #48

	IT4GameFrame* GetGameFrame() const;
	IT4GameWorld* GetGameWorld() const;
	IT4GameObject* GetPlayerObject() const;
	IT4PlayerController* GetPlayerController() const;
	IT4PacketHandlerCS* GetPacketHandlerCS() const;

protected:
	ET4LayerType LayerType;
	bool bLockOnUsed;
	FVector MovementInputVector;

	// #48
	enum ET4ActionTaskType
	{
		ActionTask_CameraRotate,
		ActionTask_LockOn,
		ActionTask_Jump,
		ActionTask_Roll,
		ActionTask_Attack,

		ActionTask_Nums
	};

	FT4ActionTask* ActionTasks[ActionTask_Nums];
	// ~#48
};
