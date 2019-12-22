// Copyright 2019 SoonBo Noh. All Rights Reserved.

#include "T4GameplayModeBase.h"

#include "ActionTask/T4GameplayComboAttackTask.h" // #48
#include "ActionTask/T4GameplayCameraRotateTask.h" // #48
#include "ActionTask/T4GameplayJumpTask.h" // #48
#include "ActionTask/T4GameplayRollTask.h" // #48
#include "ActionTask/T4GameplayLockOnTask.h" // #48

#include "T4GameplayDefinitions.h"

#include "Gameplay/T4GameplayInstance.h"
#include "GameDB/T4GameDB.h"

#include "T4GameplaySettings.h" // #52

#include "Public/Protocol/T4PacketCSMinimal.h"

#include "T4Engine/Public/Action/T4ActionCodeMinimal.h"
#include "T4Engine/Public/T4Engine.h"
#include "T4Engine/Public/T4EngineSettings.h"
#include "T4Frame/Public/T4Frame.h"

#include "T4GameplayInternal.h"

/**
  * #40
 */
FT4GameplayModeBase::FT4GameplayModeBase(ET4LayerType InLayerType)
	: LayerType(InLayerType)
	, bLockOnUsed(false)
	, MovementInputVector(FVector::ZeroVector)
{
	for (uint32 i = 0; i < ET4ActionTaskType::ActionTask_Nums; ++i)
	{
		ActionTasks[i] = nullptr; // #48
	}
}

FT4GameplayModeBase::~FT4GameplayModeBase()
{
	OnLeave();
}

void FT4GameplayModeBase::OnEnter()
{
	MovementInputVector = FVector::ZeroVector;

	// #48
	check(nullptr == ActionTasks[ActionTask_Attack]); 
	ActionTasks[ActionTask_Attack] = new FT4ComboAttackActionTask(this);

	check(nullptr == ActionTasks[ActionTask_CameraRotate]);
	ActionTasks[ActionTask_CameraRotate] = new FT4CameraRotateActionTask(this);

	check(nullptr == ActionTasks[ActionTask_Jump]);
	ActionTasks[ActionTask_Jump] = new FT4JumpActionTask(this);

	check(nullptr == ActionTasks[ActionTask_Roll]);
	ActionTasks[ActionTask_Roll] = new FT4RollActionTask(this);

	check(nullptr == ActionTasks[ActionTask_LockOn]);
	ActionTasks[ActionTask_LockOn] = new FT4LockOnActionTask(this);
	// ~#48

	Enter();
}

void FT4GameplayModeBase::OnLeave()
{
	Leave();

	// #48
	for (uint32 i = 0; i < ET4ActionTaskType::ActionTask_Nums; ++i)
	{
		if (nullptr != ActionTasks[i]) // #48
		{
			delete ActionTasks[i];
			ActionTasks[i] = nullptr;
		}
	}
	// ~#48

	bLockOnUsed = false;
	MovementInputVector = FVector::ZeroVector;
}

void FT4GameplayModeBase::OnProcess(float InDeltaTime)
{
	// #48
	for (uint32 i = 0; i < ET4ActionTaskType::ActionTask_Nums; ++i)
	{
		if (nullptr != ActionTasks[i]) // #48
		{
			ActionTasks[i]->OnProcess(InDeltaTime);
		}
	}
	// ~#48

	Process(InDeltaTime);
}

void FT4GameplayModeBase::DoMoveAsyncStart(
	const FVector& InInputVector,
	float HeadYawAngle
) // #52
{
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	if (!PlayerController->HasGameObject())
	{
		return;
	}

	IT4GameObject* PlayerObject = PlayerController->GetGameObject();
	check(nullptr != PlayerObject);

	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);

	// #52 : C/S 레이턴시로 인해 오차가 발생할 수 밖에 없음으로 CtoS 에서도 TargetLocation 을 받고,
	//       서버에서 Velocity 로 변환해 처리하도록 수정

	// #52 : MoveToLocation 을 레이턴시를 감안한 거리로 보낸다. 클라이언트에서 레이턴시를 감안한 거리로 복원해 사용한다.
	const float DefaultNetworkLatencySec = GetDefault<UT4GameplaySettings>()->GameplayDefaultNetworkLatencySec; // #52 : 200ms

	const FT4GameObjectProperty& ObjectProperty = PlayerObject->GetPropertyConst(); // #34

	// #38, #52 : 가속 이동을 감안한 Velocity 처리 (MyPC 만 사용됨)
	const float MoveAccelerationScale = ObjectProperty.MoveAccelerationScale;

	const float FinalMoveSpeed 
		= (ObjectProperty.GetMoveSpeed(ET4MoveMode::Async) * MoveAccelerationScale) * DefaultNetworkLatencySec;

	const FVector MoveStartLocation = PlayerObject->GetNavPoint();
	FVector MoveToLocation = FVector::ZeroVector;
	FVector NewTargetLocation = MoveStartLocation;
	NewTargetLocation += InInputVector * FinalMoveSpeed;

	IT4WorldNavigationSystem* WorldNavigationSystem = GameWorld->GetNavigationSystem();
	check(nullptr != WorldNavigationSystem);

	if (!WorldNavigationSystem->ProjectPoint(NewTargetLocation, T4_INVALID_NAVEXTENT, MoveToLocation))
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] DoMoveAsyncStart failed. ProjectPoint to Navigation."),
			uint32(LayerType)
		);
		return;
	}

	if (!WorldNavigationSystem->HasReached(MoveStartLocation, MoveToLocation))
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("[SL:%u] DoMoveAsyncStart failed. has not reached. (%s => %s)"),
			uint32(LayerType),
			*(MoveStartLocation.ToString()),
			*(MoveToLocation.ToString())
		);
		return;
	}

	FT4MoveAsyncAction NewAction;
	NewAction.MoveDirection = InInputVector;
	NewAction.HeadYawAngle = HeadYawAngle; // #44
	PlayerObject->DoExecuteAction(&NewAction);

	IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandlerCS();
	check(nullptr != PacketHandlerCS);

	FT4PacketMoveCS NewPacketCS; // #27
	NewPacketCS.SenderID = PlayerController->GetGameObjectID();
	NewPacketCS.MoveToLocation = MoveToLocation; // #52
	NewPacketCS.HeadYawAngle = HeadYawAngle; // #44
	PacketHandlerCS->DoSendPacket(&NewPacketCS);
}

bool FT4GameplayModeBase::DoMoveForward(float InAxisValue)
{
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	if (IsMovementLocked())
	{
		UE_LOG(
			LogT4Gameplay,
			Verbose,
			TEXT("DoMoveForward : don't move, Movement Locked!")
		);
		return false;
	}
	FRotator ControlRotation = PlayerController->GetViewControlRotation();
	FVector MoveDirection = ControlRotation.RotateVector(FVector::ForwardVector) * InAxisValue;
	MoveDirection.Z = 0.0f;
	if (MoveDirection.IsNearlyZero())
	{
		UE_LOG(
			LogT4Gameplay,
			Verbose,
			TEXT("DoMoveForward : don't move, MoveDirection is zero!")
		);
		return false;
	}
	MoveDirection.Normalize();
	if (!PlayerController->HasGameObject())
	{
		PlayerController->SetFreeCameraMoveDirection(MoveDirection); // free camera
		return true;
	}
	MovementInputVector += MoveDirection; // #33 : XY 축 이동이 있어 조작감 향상을 위해 모아서 한 프레임에 패킷으로 전송한다.
	return true;
}

bool FT4GameplayModeBase::DoMoveRight(float InAxisValue)
{
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	if (IsMovementLocked())
	{
		UE_LOG(
			LogT4Gameplay,
			Verbose,
			TEXT("DoMoveRight : don't move, Movement Locked!")
		);
		return false;
	}
	FRotator ControlRotation = PlayerController->GetViewControlRotation();
	FVector MoveDirection = ControlRotation.RotateVector(FVector::RightVector) * InAxisValue;
	MoveDirection.Z = 0.0f;
	if (MoveDirection.IsNearlyZero())
	{
		UE_LOG(
			LogT4Gameplay,
			Verbose,
			TEXT("DoMoveRight : don't move, MoveDirection is zero!")
		);
		return false;
	}
	MoveDirection.Normalize();
	if (!PlayerController->HasGameObject())
	{
		PlayerController->SetFreeCameraMoveDirection(MoveDirection); // free camera
		return true;
	}
	MovementInputVector += MoveDirection; // #33 : XY 축 이동이 있어 조작감 향상을 위해 모아서 한 프레임에 패킷으로 전송한다.
	return true;
}

bool FT4GameplayModeBase::DoLockOnPressed() 
{ 
	bLockOnUsed = true; 
	return true; 
}

bool FT4GameplayModeBase::DoLockOnReleased() 
{ 
	bLockOnUsed = false; 
	return true; 
}

bool FT4GameplayModeBase::DoJumpPressed()
{
	if (IsMovementLocked())
	{
		return false;
	}
	if (nullptr == ActionTasks[ActionTask_Jump])
	{
		return false;
	}
	FString ErrorMsg;
	bool bResult = ActionTasks[ActionTask_Jump]->OnPressed(ErrorMsg);
	if (!bResult)
	{
		UE_LOG(
			LogT4Gameplay,
			Verbose,
			TEXT("FT4GameplayModeBase : Jump Failed! %s"),
			*ErrorMsg
		);
	}
	return bResult;
}

bool FT4GameplayModeBase::DoRollPressed()
{
	if (IsMovementLocked())
	{
		return false;
	}
	if (nullptr == ActionTasks[ActionTask_Roll])
	{
		return false;
	}
	FString ErrorMsg;
	bool bResult = ActionTasks[ActionTask_Roll]->OnPressed(ErrorMsg);
	if (!bResult)
	{
		UE_LOG(
			LogT4Gameplay,
			Verbose,
			TEXT("FT4GameplayModeBase : Roll Failed! %s"),
			*ErrorMsg
		);
	}
	return bResult;
}

bool FT4GameplayModeBase::DoTeleportPressed()
{
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	if (!PlayerController->HasGameObject())
	{
		return false;
	}

	FVector PickingLocation;
	if (!GetGameFrame()->GetMousePickingLocation(PickingLocation))
	{
		UE_LOG(
			LogT4Gameplay,
			Verbose,
			TEXT("FT4GameplayModeBase : Teleport Failed. Invalid Picking Location.")
		);
		return false;
	}

	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);

	IT4WorldNavigationSystem* NavigationSystem = GameWorld->GetNavigationSystem();
	if (nullptr == NavigationSystem)
	{
		return false;
	}

	FVector TeleportLocation = FVector::ZeroVector;
	if (!NavigationSystem->ProjectPoint(PickingLocation, T4_INVALID_NAVEXTENT, TeleportLocation))
	{
		UE_LOG(
			LogT4Gameplay,
			Error,
			TEXT("FT4GameplayModeBase : Teleport Failed. ProjectPoint to Navigation."),
		);
		return false;
	}

	IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandlerCS();
	check(nullptr != PacketHandlerCS);
	FT4PacketCmdTeleportCS NewPacketCS; // #27
	NewPacketCS.SenderID = PlayerController->GetGameObjectID();
	NewPacketCS.TargetLocation = TeleportLocation;
	PacketHandlerCS->DoSendPacket(&NewPacketCS);
	return true;
}

bool FT4GameplayModeBase::DoAttackPressed()
{
	if (nullptr == ActionTasks[ActionTask_Attack]) // #48
	{
		return false;
	}
	FString ErrorMsg;
	bool bResult = ActionTasks[ActionTask_Attack]->OnPressed(ErrorMsg);
	if (!bResult) // #45
	{
		UE_LOG(
			LogT4Gameplay,
			Verbose,
			TEXT("FT4GameplayModeBase : Attack Failed! %s"),
			*ErrorMsg
		);
	}
	return bResult;
}

bool FT4GameplayModeBase::DoChangePlayer() // #11, #52
{
	IT4GameObject* MouseOverObject = GetGameFrame()->GetMouseOverGameObject();
	if (nullptr == MouseOverObject)
	{
		return false;
	}
	if (MouseOverObject->IsPlayer())
	{
		return false;
	}
	IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandlerCS();
	check(nullptr != PacketHandlerCS);
	FT4PacketCmdChangePlayerCS NewPacketCS;
	NewPacketCS.NewPlayerObjectID = MouseOverObject->GetObjectID();
	PacketHandlerCS->DoSendPacket(&NewPacketCS);
	return true;
}

bool FT4GameplayModeBase::DoChangeObserver() // #52
{
	// Only Client
	IT4GameObject* MouseOverObject = GetGameFrame()->GetMouseOverGameObject();
	if (nullptr == MouseOverObject)
	{
		return false;
	}
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	if (MouseOverObject->IsPlayer())
	{
		PlayerController->ClearObserverObject();
	}
	else
	{
		PlayerController->SetObserverObject(MouseOverObject->GetObjectID());
	}
	return true;
}

bool FT4GameplayModeBase::DoLeaveObject()
{
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	if (!PlayerController->HasGameObject())
	{
		return false;
	}
	IT4PacketHandlerCS* PacketHandlerCS = GetPacketHandlerCS();
	check(nullptr != PacketHandlerCS);
	FT4PacketCmdLeaveCS NewPacketCS; // #27
	NewPacketCS.LeaveObjectID = PlayerController->GetGameObjectID();
	PacketHandlerCS->DoSendPacket(&NewPacketCS);
	return true;
}

bool FT4GameplayModeBase::DoCameraRotatePressed()
{
	if (nullptr == ActionTasks[ActionTask_CameraRotate])
	{
		return false;
	}
	FString ErrorMsg;
	bool bResult = ActionTasks[ActionTask_CameraRotate]->OnPressed(ErrorMsg);
	return bResult;
}

bool FT4GameplayModeBase::DoCameraRotateReleased()
{
	if (nullptr == ActionTasks[ActionTask_CameraRotate])
	{
		return false;
	}
	ActionTasks[ActionTask_CameraRotate]->OnReleased();
	return true;
}

bool FT4GameplayModeBase::DoCameraZoom(float InAxisValue)
{
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	PlayerController->SetCameraZoom(InAxisValue);
	return true;
}

bool FT4GameplayModeBase::DoCameraPitch(float InAxisValue)
{
#if !PLATFORM_ANDROID
	if (!IsCameraRotationEnabled())
	{
		return false;
	}
#endif
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	InAxisValue = -InAxisValue;
	PlayerController->SetCameraPitch(InAxisValue);
	return true;
}

bool FT4GameplayModeBase::DoCameraYaw(float InAxisValue)
{
#if !PLATFORM_ANDROID
	if (!IsCameraRotationEnabled())
	{
		return false;
	}
#endif
	IT4PlayerController* PlayerController = GetPlayerController();
	check(nullptr != PlayerController);
	PlayerController->SetCameraYaw(InAxisValue);
	return true;
}

bool FT4GameplayModeBase::CallLockOnStart(
	const float InSyncHeadYawAngle,
	FString& OutErrorMsg
)
{
	bool bResult = false;
	if (nullptr != ActionTasks[ActionTask_LockOn]) // #48
	{
		bResult = ActionTasks[ActionTask_LockOn]->OnStart(InSyncHeadYawAngle, OutErrorMsg);
	}
	return bResult;
}

bool FT4GameplayModeBase::CallLockOnEnd(
	const float InSyncHeadYawAngle,
	FString& OutErrorMsg
)
{
	bool bResult = false;
	if (nullptr != ActionTasks[ActionTask_LockOn]) // #48
	{
		bResult = ActionTasks[ActionTask_LockOn]->OnEnd(InSyncHeadYawAngle, OutErrorMsg);
	}
	return true;
}

bool FT4GameplayModeBase::IsCameraRotationEnabled() const // #48
{
	if (nullptr == ActionTasks[ActionTask_CameraRotate])
	{
		return false;
	}
	return ActionTasks[ActionTask_CameraRotate]->IsPressed();
}

bool FT4GameplayModeBase::IsMovementLocked() const // #48
{
	if (nullptr == ActionTasks[ActionTask_Attack])
	{
		return false;
	}
	return ActionTasks[ActionTask_Attack]->IsMovementLocked();
}

IT4GameFrame* FT4GameplayModeBase::GetGameFrame() const
{
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	return GameFrame;
}

IT4GameWorld* FT4GameplayModeBase::GetGameWorld() const
{
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	return GameFrame->GetGameWorld();
}

IT4GameObject* FT4GameplayModeBase::GetPlayerObject() const
{
	IT4PlayerController* PlayerController = GetPlayerController();
	if (nullptr == PlayerController || PlayerController->HasGameObject())
	{
		return nullptr;
	}
	return PlayerController->GetGameObject();
}

IT4PlayerController* FT4GameplayModeBase::GetPlayerController() const
{
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	IT4PlayerController* PlayerController = GameFrame->GetPlayerController();
	check(nullptr != PlayerController);
	return PlayerController;
}

IT4PacketHandlerCS* FT4GameplayModeBase::GetPacketHandlerCS() const
{
	check(ET4LayerType::Max > LayerType);
	IT4GameFrame* GameFrame = T4FrameGet(LayerType);
	check(nullptr != GameFrame);
	FT4GameplayInstance* GameplayInstance = FT4GameplayInstance::CastFrom(
		GameFrame->GetGameplayInstance()
	);
	if (nullptr == GameplayInstance)
	{
		return nullptr;
	}
	return GameplayInstance->GetPacketHandlerCS();
}
