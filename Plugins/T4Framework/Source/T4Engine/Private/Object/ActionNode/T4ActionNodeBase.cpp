// Copyright 2019-2020 SoonBo Noh. All Rights Reserved.

#include "T4ActionNodeBase.h"
#include "T4ActionNodeIncludes.h"
#include "T4ActionNodeControl.h"

#include "Public/Action/T4ActionParameters.h" // #28
#include "Object/T4GameObject.h"

#include "Public/T4Engine.h"

#include "T4EngineInternal.h"

/**
  *
 */
FT4ActionNodeBase::FT4ActionNodeBase(FT4ActionNodeControl* InControl, const FT4ActionKey& InKey)
	: ActionKey(InKey)
	, ActionControlRef(InControl)
	, ParentNode(nullptr)
	, PlayState(APS_Pending)
	, LoadState(ALS_NotUsed)
	, bDebugPaused(false) // #56
	, LifecycleType(ET4LifecycleType::Auto) // #56 : Default => Auto
	, ElapsedTimeSec(0.0f)
	, DelayTimeSec(0.0f)
	, DurationSec(0.0f)
	, OffsetTimeSec(0.0f) // #54, #56
	, GlobalElapsedTimeSec(0.0f)
	, ContiMaxPlayTimeSec(0.0f) // #58 : GetGlobalMaxPlayTimeSec() 에서 선택하도록 변경됨. 이 값은 콘티에 설정된 값!
{
	check(nullptr != ActionControlRef);
}

FT4ActionNodeBase::~FT4ActionNodeBase()
{
}

bool FT4ActionNodeBase::IsPlaying() const
{ 
	return (bDebugPaused) ? bDebugPaused : (APS_Playing == PlayState) ? true : false; 
}

template <class T, class U>
FORCEINLINE FT4ActionNodeBase* CreateNodeTemplate(
	FT4ActionNodeControl* InControl,
	const U& InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	check(nullptr != InControl);
	T* NewActionNode = T::CreateNode(InControl, InAction, InParameters);
	if (nullptr == NewActionNode)
	{
		if (ET4ActionCommandType::Code == InAction.GetActionStructType()) // #62
		{
			T4_LOG(
				Verbose,
				TEXT("Code '%s' CreateNode Failed"),
				*InAction.ToString()
			);
		}
		else
		{
			T4_LOG(
				Verbose,
				TEXT("Conti '%s' CreateNode Failed"),
				*InAction.ToString()
			);
		}
		return nullptr;
	}
	NewActionNode->SetDelayTimeSec(InAction.StartTimeSec);
	NewActionNode->SetLifecycleType(InAction.LifecycleType, InAction.DurationSec);
	return NewActionNode;
}

FT4ActionNodeBase* FT4ActionNodeBase::CreateNode(
	FT4ActionNodeControl* InControl,
	const FT4ActionCommand* InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	return CreateNode(InControl, InAction->ActionType, InAction, InParameters);
}

FT4ActionNodeBase* FT4ActionNodeBase::CreateNode(
	FT4ActionNodeControl* InControl, 
	const ET4ActionType InActionType, // #56
	const FT4ActionCommand* InAction,
	const FT4ActionParameters* InParameters // #54
)
{
	// #23
	// #T4_ADD_ACTION_TAG_CODE
	// #T4_ADD_ACTION_TAG_CONTI
	check(nullptr != InAction);
	FT4ActionNodeBase* NewChildAction = nullptr;
	switch (InActionType)
	{
		case ET4ActionType::Branch: // #54
			NewChildAction = CreateNodeTemplate<FT4ActionBranchNode>(
				InControl,
				*(static_cast<const FT4BranchAction*>(InAction)),
				InParameters // #54
			);
			break;
		                
		case ET4ActionType::SpecialMove: // #54
			NewChildAction = CreateNodeTemplate<FT4ActionSpecialMoveNode>(
				InControl,
				*(static_cast<const FT4SpecialMoveAction*>(InAction)),
				InParameters // #54
			);
			break;

		case ET4ActionType::EquipWeapon: // #22
			NewChildAction = CreateNodeTemplate<FT4ActionEquipWeaponNode>(
				InControl,
				*(static_cast<const FT4EquipWeaponAction*>(InAction)),
				InParameters // #54
			);
			break;

		case ET4ActionType::UnequipWeapon: // #48, #111
			NewChildAction = CreateNodeTemplate<FT4ActionUnequipWeaponNode>(
				InControl,
				*(static_cast<const FT4UnequipWeaponAction*>(InAction)),
				InParameters // #54
			);
			break;

		case ET4ActionType::Animation: // #17
			NewChildAction = CreateNodeTemplate<FT4ActionAnimationNode>(
				InControl,
				*(static_cast<const FT4AnimationAction*>(InAction)),
				InParameters // #54
			);
			break;

		case ET4ActionType::Mesh: // #108
			NewChildAction = CreateNodeTemplate<FT4ActionMeshNode>(
				InControl,
				*(static_cast<const FT4MeshAction*>(InAction)),
				InParameters // #54
			);
			break;

		case ET4ActionType::Particle: // #20
			NewChildAction = CreateNodeTemplate<FT4ActionParticleNode>(
				InControl,
				*(static_cast<const FT4ParticleAction*>(InAction)),
				InParameters // #54
			);
			break;

		case ET4ActionType::Decal: // #54
			NewChildAction = CreateNodeTemplate<FT4ActionDecalNode>(
				InControl,
				*(static_cast<const FT4DecalAction*>(InAction)),
				InParameters // #54
			);
			break;

		case ET4ActionType::Projectile: // #63
			NewChildAction = CreateNodeTemplate<FT4ActionProjectileNode>(
				InControl,
				*(static_cast<const FT4ProjectileAction*>(InAction)),
				InParameters // #54
			);
			break;

		case ET4ActionType::Reaction: // #76
			NewChildAction = CreateNodeTemplate<FT4ActionReactionNode>(
				InControl,
				*(static_cast<const FT4ReactionAction*>(InAction)),
				InParameters // #54
			);
			break;

		case ET4ActionType::PlayTag: // #81
			NewChildAction = CreateNodeTemplate<FT4ActionPlayTagNode>(
				InControl,
				*(static_cast<const FT4PlayTagAction*>(InAction)),
				InParameters // 
				);
			break;

		case ET4ActionType::TimeScale: // #102
			NewChildAction = CreateNodeTemplate<FT4ActionTimeScaleNode>(
				InControl,
				*(static_cast<const FT4TimeScaleAction*>(InAction)),
				InParameters // #54
			);
			break;

		case ET4ActionType::CameraWork: // #54
			NewChildAction = CreateNodeTemplate<FT4ActionCameraWorkNode>(
				InControl,
				*(static_cast<const FT4CameraWorkAction*>(InAction)),
				InParameters // #54
			);
			break;

		case ET4ActionType::CameraShake: // #101
			NewChildAction = CreateNodeTemplate<FT4ActionCameraShakeNode>(
				InControl,
				*(static_cast<const FT4CameraShakeAction*>(InAction)),
				InParameters // #54
			);
			break;

		case ET4ActionType::PostProcess: // #100
			NewChildAction = CreateNodeTemplate<FT4ActionPostProcessNode>(
				InControl,
				*(static_cast<const FT4PostProcessAction*>(InAction)),
				InParameters // #54
			);
			break;

		case ET4ActionType::Environment: // #99
			NewChildAction = CreateNodeTemplate<FT4ActionEnvironmentNode>(
				InControl,
				*(static_cast<const FT4EnvironmentAction*>(InAction)),
				InParameters // #54
			);
			break;

		case ET4ActionType::Turn:
			NewChildAction = CreateNodeTemplate<FT4ActionTurnNode>(
				InControl,
				*(static_cast<const FT4TurnAction*>(InAction)),
				InParameters // #54
			);
			break;

		case ET4ActionType::Conti: // #24
			NewChildAction = CreateNodeTemplate<FT4ActionContiNode>(
				InControl,
				*(static_cast<const FT4ContiAction*>(InAction)),
				InParameters // #54
			);
			break;

		case ET4ActionType::Dummy: // #56 : Conti Editor 에서 Invisible or Isolate 로 출력을 제어할 때 더미용으로 사용(delay, duration 동작 보장)
			NewChildAction = CreateNodeTemplate<FT4ActionDummyNode>(
				InControl,
				*InAction,
				InParameters // #54
			);
			break;

		case ET4ActionType::Hit: // #76
		case ET4ActionType::Die: // #76
		case ET4ActionType::Resurrect: // #76
			{
				check(false); // only manual node
			}
			break;

		default:
			{
				T4_LOG(
					Error,
					TEXT("No implementation Action '%s'"),
					*(InAction->ToString())
				);
			}
			break;
	};
	return NewChildAction;
}

void FT4ActionNodeBase::DestroyNode(FT4ActionNodeBase** InActionNode)
{
	// TODO : factory & memory pool
	if (nullptr != InActionNode && nullptr != *InActionNode)
	{
		delete *InActionNode;
		*InActionNode = nullptr;
	}
}

IT4ActionNode* FT4ActionNodeBase::AddChildNode(
	const FT4ActionCommand* InAction,
	float InOffsetTimeSec // #54
)
{
	check(nullptr != ActionControlRef);
	if (ET4LifecycleType::Duration == InAction->LifecycleType)
	{
		// #56 : 만약, Duration 을 사용하는 Action 이 OffsetTime 보다 작다면 플레이 할 필요가 없을 것이다!
		//       단, 몇가지 액션은 옵션으로 1회성 플레이를 보장해주어야 할 수도 있다.
		//       FT4ActionNodeControl::CreateNode 와 FT4ActionNodeBase::AddChildNode, FT4ActionCompositeNodeBase::AddChildActionNode 에서 체크하고 있다.
		if ((InAction->StartTimeSec + InAction->DurationSec) <= InOffsetTimeSec)
		{
			T4_LOG(
				VeryVerbose,
				TEXT("AddChildNode '%s' skiped. (DurationSec <= InOffsetTimeSec)"),
				*(InAction->ToString())
			);
			return nullptr;
		}
	}
	FT4ActionNodeBase* NewChildAction = FT4ActionNodeBase::CreateNode(
		ActionControlRef, 
		InAction,
		ActionParameterPtr.Get() // #54
	);
	if (nullptr == NewChildAction)
	{
		return nullptr;
	}
	// #54, #56 : AddChild 전에 호출해야 Conti 류의 CompositeData 생성에 적용이 된다.
	NewChildAction->SetOffsetTimeSec(InOffsetTimeSec);
	if (!AddChildNodeInternal(InAction, NewChildAction))
	{
		FT4ActionNodeBase::DestroyNode(&NewChildAction);
		return nullptr;
	}
	return static_cast<IT4ActionNode*>(NewChildAction);
}

bool FT4ActionNodeBase::AddChildNodeInternal(
	const FT4ActionCommand* InAction,
	FT4ActionNodeBase* InActionNode
)
{
	InActionNode->SetParentNode(this);
	if (ActionParameterPtr.IsValid())
	{
		InActionNode->SetActionParameters(ActionParameterPtr);  // #28
	}
	bool bResult = InActionNode->OnCreate(InAction);
	if (bResult)
	{
		ActionNodeGraph.AddChildNode(
			(ET4ActionCommandType::Code == InAction->GetActionStructType()) ? // #62
				static_cast<const FT4ActionCodeCommand*>(InAction)->ActionKey : FT4ActionKey::EmptyActionKey,
			InActionNode
		);
	}
	else
	{
		T4_LOG(
			VeryVerbose,
			TEXT("AddChildNode '%s' OnCreate failed."),
			*(InAction->ToString())
		);
	}
	return bResult;
}

bool FT4ActionNodeBase::RemoveChildNode(const FT4StopAction* InAction)
{
	bool bResult = ActionNodeGraph.RemoveChildNode(
		InAction->ActionKey,
		InAction->StartTimeSec,
		InAction->bSameKeyNameRemoveAll
	);
	return bResult;
}

uint32 FT4ActionNodeBase::NumChildActions() const
{
	return ActionNodeGraph.NumChildActions();
}

bool FT4ActionNodeBase::OnCreate(const FT4ActionCommand* InAction)
{
	bool bResult = Create(InAction);
	if (bResult)
	{
		if (ActionParameterPtr.IsValid()) // #112
		{
			if (ActionParameterPtr->CheckBits(ET4OverrideParamBits::DurationBit))
			{
				LifecycleType = ET4LifecycleType::Duration;
				DurationSec = ActionParameterPtr->GetOverrideParams().DurectionSec;
			}
		}
	}
	return bResult;
}

void FT4ActionNodeBase::OnDestroy()
{
	if (IsPlayed()) // #60 : IsPlaying 으로 체크하면 안된다.
	{
		OnStop();
	}
	Destroy();
	ActionNodeGraph.Reset();
	if (ActionParameterPtr.IsValid())
	{
		ActionParameterPtr.Reset(); // #28
	}
}

void FT4ActionNodeBase::OnDestroying() // #102
{
	Destroying();
}

void FT4ActionNodeBase::OnAdvance(const FT4UpdateTime& InUpdateTime)
{
	ElapsedTimeSec += InUpdateTime.ScaledTimeSec;
	if (0.0f < GetGlobalMaxPlayTimeSec())
	{
		// #54 : Conti 로 플레이 할 경우 설정되는 최대 플레이시간 설정 대응
		GlobalElapsedTimeSec += InUpdateTime.ScaledTimeSec;
	}
	if (CheckLoadState(ALS_Loading))
	{
		AdvanceLoading(InUpdateTime);
	}
	if (CheckPlayState(APS_Pending) && ElapsedTimeSec >= DelayTimeSec)
	{
		OnPlay();
	}
	if (!CheckPlayState(APS_Stopped) && !CheckPlayState(APS_Finished))
	{
		Advance(InUpdateTime);
	}
	if (IsPlayed() && CheckFinished())
	{
		Stop(); // 자동 종료도 Stop 을 호출해 자원 해제를 해주도록 조치
		SetPlayState(APS_Finished);
	}
	ActionNodeGraph.Advance(InUpdateTime);
}

void FT4ActionNodeBase::OnPlay()
{
	check(CheckPlayState(APS_Pending));
	bool bResult = Play(); // #56
	if (bResult)
	{
		SetPlayState(APS_Playing);
	}
}

void FT4ActionNodeBase::OnStop()
{
	SetPlayState(APS_Stopping);
	Stop();
	SetPlayState(APS_Stopped);
}

void FT4ActionNodeBase::OnStartLoading()
{
	check(CheckLoadState(ALS_NotUsed));
	SetLoadState(ALS_Ready);
	StartLoading();
	SetLoadState(ALS_Loading);
}

bool FT4ActionNodeBase::IsDestroyable() const
{
	bool bIsFinished = CheckStopedAndFinished();
	return bIsFinished;
}

bool FT4ActionNodeBase::CheckFinished() const
{
	if (!CheckPlayState(APS_Playing) && !CheckPlayState(APS_Ready))
	{
		return false; // 아직 플레이가 안되었다. 추후 좀 더 면밀한 체크가 필요할 듯...
	}
	bool bNodeFinished = false;
	switch (LifecycleType)
	{
		case ET4LifecycleType::Looping:
			{
				// 루핑은 외부에서 OnStop 을 호출해 정지시킨다.
			}
			break;

		case ET4LifecycleType::Duration:
			{
				if (DurationSec <= GetPlayingTime())
				{
					bNodeFinished = true;
				}
			}
			break;

		case ET4LifecycleType::Auto:
			{
				bNodeFinished = IsAutoFinished(); // 각 Node 가 스스로 소멸할 준비가 되었다면 true 를 준다.
			}
			break;

		default:
			{
				T4_LOG(
					Error,
					TEXT("Unknown LifecycleType '%u'"),
					uint8(LifecycleType)
				);
			}
			break;
	}
	if (!bNodeFinished)
	{
		return false;
	}
	bool bAutoFinished = ActionNodeGraph.CheckFinished(); // #56 : 하위 노드까지 검사하는 처리 추가
	return bAutoFinished;
}

bool FT4ActionNodeBase::CheckStopedAndFinished() const
{
	if (!CheckPlayState(APS_Stopped) &&
		!CheckPlayState(APS_Finished))
	{
		return false;
	}
	return ActionNodeGraph.CheckStopedAndFinished();
}

float FT4ActionNodeBase::GetGlobalMaxPlayTimeSec() const
{
	if (ActionParameterPtr.IsValid())
	{
		// #58 : 코드 호출 또는 툴에서 최대 시간 제한에 사용
		if (ActionParameterPtr->CheckBits(ET4OverrideParamBits::MaxPlayTimeBit))
		{
			return ActionParameterPtr->GetOverrideParams().MaxPlayTimeSec;
		}
	}
	return GetContiMaxPlayTimeSec();
}

void FT4ActionNodeBase::SetActionParameters(
	TSharedPtr<const FT4ActionParameters> InActionParameterPtr
)
{
	ActionParameterPtr = InActionParameterPtr; // #28
}

void FT4ActionNodeBase::SetLifecycleType(
	ET4LifecycleType InLifecycleType,
	float InDurationSec
)
{
	LifecycleType = InLifecycleType;
	if (ET4LifecycleType::Default == LifecycleType) // #56
	{
		LifecycleType = ET4LifecycleType::Auto;
	}
	DurationSec = InDurationSec;
}

AT4GameObject* FT4ActionNodeBase::GetGameObject() const
{
	check(nullptr != ActionControlRef);
	return ActionControlRef->GetGameObject();
}

IT4GameWorld* FT4ActionNodeBase::GetGameWorld() const
{
	return T4EngineWorldGet(GetGameObject()->GetLayerType());
}

AT4GameObject* FT4ActionNodeBase::NewClientObject(
	ET4ObjectType InWorldObjectType, // #63 : Only World Object
	const FName& InName,
	const FVector& InLocation,
	const FRotator& InRotation,
	const FVector& InScale
) // #54
{
	check(!WorldObjectID.IsValid());
	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	IT4WorldContainer* WorldContainer = GameWorld->GetContainer();
	check(nullptr != WorldContainer);
	IT4GameObject* NewWorldObject = WorldContainer->CreateClientObject(
		InWorldObjectType,
		InName,
		InLocation,
		InRotation,
		InScale
	);
	if (nullptr == NewWorldObject)
	{
		check(ET4LayerType::Server == GameWorld->GetLayerType()); // #63 : 서버는 사용할 필요가 없다!
		return nullptr;
	}
	check(InWorldObjectType == NewWorldObject->GetObjectType());
	WorldObjectID = NewWorldObject->GetObjectID();
	return static_cast<AT4GameObject*>(NewWorldObject);
}

void FT4ActionNodeBase::DeleteClientObject() // #54
{
	if (!WorldObjectID.IsValid())
	{
		return;
	}
	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	IT4WorldContainer* WorldContainer = GameWorld->GetContainer();
	check(nullptr != WorldContainer);
	WorldContainer->DestroyClientObject(WorldObjectID);
	WorldObjectID.Empty();
}

AT4GameObject* FT4ActionNodeBase::GetClientObject() const
{
	if (!WorldObjectID.IsValid())
	{
		return nullptr;
	}
	IT4GameWorld* GameWorld = GetGameWorld();
	check(nullptr != GameWorld);
	IT4WorldContainer* WorldContainer = GameWorld->GetContainer();
	check(nullptr != WorldContainer);
	IT4GameObject* WorldObject = WorldContainer->FindGameObject(WorldObjectID);
	if (nullptr == WorldObject)
	{
		return nullptr;
	}
	return static_cast<AT4GameObject*>(WorldObject);
}

FT4ActionKey FT4ActionNodeBase::FindActionKeyInParameter(
	const FT4ActionParameters* InParameters
) // #100
{
	if (nullptr != InParameters)
	{
		if (InParameters->CheckBits(ET4DefaultParamBits::ActionKeyBit))
		{
			return InParameters->GetDefaultParams().ActionKey;
		}
	}
	return FT4ActionKey::EmptyActionKey;
}

bool FT4ActionNodeBase::FindTargetObjectInParameter(
	IT4GameObject** OutTargetObject, 
	const TCHAR* InDebugString
)
{
	// #28
	if (!ActionParameterPtr.IsValid()) // #28
	{
		T4_LOG(
			Warning,
			TEXT("%s : ActionParameters is not set."),
			(nullptr != InDebugString) ? InDebugString : TEXT("FindTargetObjectInParameter")
		);
		return false;
	}
	bool bResult = ActionParameterPtr->GetTargetObject(
		GetGameObject()->GetLayerType(),
		OutTargetObject,
		InDebugString
	);
	return bResult;
}

bool FT4ActionNodeBase::FindTargetLocationInParameter(
	FVector& OutTargetLocation,
	const TCHAR* InDebugString
)
{
	// #28
	if (!ActionParameterPtr.IsValid()) // #28
	{
		T4_LOG(
			Warning,
			TEXT("%s : ActionParameters is not set."),
			(nullptr != InDebugString) ? InDebugString : TEXT("FindTargetLocationInParameter")
		);
		return false;
	}
	bool bResult = ActionParameterPtr->GetTargetLocation(
		OutTargetLocation,
		InDebugString
	);
	return bResult;
}

bool FT4ActionNodeBase::FindTargetDirectionInParameter(
	FVector& OutTargetDirection,
	const TCHAR* InDebugString
)
{
	// #28
	if (!ActionParameterPtr.IsValid()) // #28
	{
		T4_LOG(
			Warning,
			TEXT("%s : ActionParameters is not set."),
			(nullptr != InDebugString) ? InDebugString : TEXT("FindTargetDirectionInParameter")
		);
		return false;
	}
	bool bResult = ActionParameterPtr->GetTargetDirection(
		OutTargetDirection,
		InDebugString
	);
	return bResult;
}

#if !UE_BUILD_SHIPPING
void FT4ActionNodeBase::SetDebugPause(bool bInPause) // #54
{
#if 0 // Notify 또는 멈춰 있는데 추가된 Node 가 있을 수 있으니 아래 체크를 해서는 안된다.
	if (bDebugPaused == bInPause)
	{
		return; // 이미 멈춰져 있다
	}
#endif
	bDebugPaused = bInPause;
	// #54, #102 : 디버깅용으로 Pause 가 되었을 경우 외부 오브젝트(Ex:Proj) 제어를 위한 호출
	//			   일반적인 상황에서는 ScaledTimeSec = 0 으로 Pause 를 구현하고 있다.
	NotifyDebugPaused(bInPause);
	ActionNodeGraph.SetDebugPause(bInPause);
}
#endif
